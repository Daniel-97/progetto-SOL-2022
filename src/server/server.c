#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/select.h>

#include "includes/globals.h"
#include "../common/common.h"
#include "includes/config.h"
#include "includes/queue.h"
#include "includes/fileStorage.h"
#include "includes/utils.h"
#include "includes/controller.h"
#include "includes/logger.h"
#include "includes/statistic.h"

static void *worker(void *arg);
static void *signalThreadHandler(void *arg);

int handler_signal_pipe[2];
int worker_signal_pipe[2];

int main(int argc, char *argv[]){

    /***** SIGNAL HANDLER INIT *****/
    /* Maschero i segnali */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGHUP);
    pthread_sigmask(SIG_SETMASK,&set,NULL);

    /* Creo il thread per i segnali in modalità detached */
    pthread_t tid_signal_thread;
    pthread_create(&tid_signal_thread, NULL, &signalThreadHandler, NULL);

    /****** CONFIG INIT *******/
    int res = readConfig(&serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(&serverConfig);

    /****** NUM CONNECTION INIT *****/
    n_connections = 0;
    acceptNewConnection = true;
    closeServer = false;

    /***** LOGGER INIT *******/
    loggerInit();

    /***** STATISTIC INIT ******/
    statInit();

    /***** FILE QUEUE INIT ******/
    fileQueue = initQueue();
    pthread_mutex_init(&file_queue_mutex, NULL);
    pthread_cond_init(&file_queue_cond, NULL);

    pthread_mutex_init(&file_lock_mutex, NULL);
    pthread_cond_init(&file_lock_cond, NULL);

    /***** CONNECTION QUEUE INIT *****/
    connectionQueue = initQueue();
    pthread_mutex_init(&connection_queue_mutex, NULL);
    pthread_cond_init(&connection_queue_cond, NULL);

    /**** PIPE INIT ****/
    handler_signal_pipe[0] = -1;
    handler_signal_pipe[1] = -1;

    if (pipe(handler_signal_pipe) == -1){
        perror("Error creazione pipe handler\n");
        exit(EXIT_FAILURE);
    }

    worker_signal_pipe[0] = -1;
    worker_signal_pipe[1] = -1;
    if (pipe(worker_signal_pipe) == -1){
        perror("Error creazione pipe worker\n");
        exit(EXIT_FAILURE);
    }

    /******* THREAD INIT *****/

    for(int i = 0; i<serverConfig.thread_workers; i++) {
        pthread_create(&threadPool[i], NULL, &worker, NULL);
    }

    /***** SOCKET INIT *****/
    int fd_server_skt;
    int fd_client_skt; //File descriptor per connessioni client
    struct sockaddr_un socketAddress;

    strncpy(socketAddress.sun_path, serverConfig.socket_path,100);
    socketAddress.sun_family = AF_UNIX; // Socket di tipo AF_UNIX

    //Creazione socket
    if ( (fd_server_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ){
        printf("[MASTER] Errore creazione socket, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }
    // Unlink vecchio socket se esistente. unlink() - elimina un nome e se possibile il file a cui si riferisce
    unlink(serverConfig.socket_path);

    //Bind socket con address
    if ( (bind(fd_server_skt, (struct sockaddr*) &socketAddress, sizeof(socketAddress)) ) == -1 ){
        printf("[MASTER] Errore bind socket, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }

    if ( listen(fd_server_skt,SOMAXCONN) == -1){
        printf("[MASTER] Errore listen socket, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }

    /**** FD SET INIT *****/
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    int max_sd = fd_server_skt;
    // Imposto i descrittori di file che devo monitorare
    FD_SET(fd_server_skt,&master_set);
    FD_SET(handler_signal_pipe[0],&master_set);
    int desc_ready;

    /****** MAIN SERVER LOOP *****/
    printf("\n[MASTER] ATTENDO NUOVE CONNESSIONI...\n");

    while(acceptNewConnection && !closeServer) {

        memcpy(&working_set, &master_set, sizeof(master_set));

        /* La select rimane in ascolto dei file descriptor e risveglia il thread quando uno dei fd è pronto */
        printf("[MASTER] Waiting on select...\n");
        if((desc_ready = select(max_sd+1, &working_set, NULL, NULL, NULL)) == -1){
            perror("[MASTER] Error select");
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i <= max_sd && desc_ready > 0; i++){

            if(!FD_ISSET(i, &working_set))
                continue;

            if( i == fd_server_skt ){

                printf("Server socket is ready!\n");
                if ((fd_client_skt = accept(fd_server_skt, NULL, 0)) == -1) {

                    printf("[MASTER] Errore accept socket, errno: %d, %s\n", errno, strerror(errno));
                    break;

                } else {

                    printf("\n[MASTER] Nuova connessione ricevuta, fd_skt:%d\n",fd_client_skt);

                    pthread_mutex_lock(&connection_queue_mutex);

                    if( push(connectionQueue,&fd_client_skt) != -1)
                        printf("[MASTER] File descriptor client socket inserito nella coda\n");
                    else
                        printf("[MASTER] Errore inserimento file descriptor client socket nella coda\n");


                    printf("[MASTER] Sveglio un thread in attesa per avvertirlo della nuova connessione\n");

                    /* Avverto uno dei worker che è presente un nuovo fd nella coda */
                    pthread_cond_signal(&connection_queue_cond);
                    pthread_mutex_unlock(&connection_queue_mutex);

                }

            }

            if( i == handler_signal_pipe[0] ){
                printf("[MASTER] Ricevuto segnale da pipe per terminare i processi\n");
                break;
            }
        }

    }

    printf("[MASTER] Attendo che tutti i client chiudano la connessione\n");
        for(int i = 0; i<serverConfig.thread_workers; i++)
            pthread_join(threadPool[i], NULL);

    printf("\n[MASTER] Tutte le connessioni sono chiuse\n");

    printf("[MASTER] Attendo che il thread signal termini...\n");
    pthread_join(tid_signal_thread, NULL);
    printf("[MASTER] Signal thread terminato, chiudo il programma...\n");

    /* Stampo le statistiche */
    printStat(fileQueue);

    pthread_mutex_lock(&file_queue_mutex);
    deleteFileQueue(fileQueue);
    pthread_mutex_unlock(&file_queue_mutex);

    pthread_mutex_lock(&connection_queue_mutex);
    deleteQueue(connectionQueue);
    pthread_mutex_unlock(&connection_queue_mutex);

    close(fd_server_skt);

    loggerEnd();

    return 0;
}

static void *worker(void *arg){

    pthread_t self = pthread_self();
    int fd_client_skt;
    Request request;

    printf("[%lu] Worker start\n", self);

    while(1){

        printf("[%lu] In attesa di nuova richiesta...\n",self);

        pthread_mutex_lock(&connection_queue_mutex);
        while(!closeServer && acceptNewConnection && connectionQueue->len == 0)
            pthread_cond_wait(&connection_queue_cond, &connection_queue_mutex);

        if(!acceptNewConnection || closeServer){
            printf("[%lu] Mi termino...\n",self);
            pthread_mutex_unlock(&connection_queue_mutex); //Release mutex
            break;
        }

        int *fd = pop(connectionQueue);
        fd_client_skt = *fd;
        pthread_mutex_unlock(&connection_queue_mutex); //Release mutex

        printf("[%lu] Pop di fd: %d, queue len: %lu\n",self,fd_client_skt,connectionQueue->len);
        addConnectionCont();

        if (fd_client_skt != -1){

            /* Continuo a leggere dal socket del client fintanto che ci sono dati */
            while( read(fd_client_skt, &request,sizeof(Request)) > 0 ) {

                if(closeServer) {
                    printf("[%lu] Forcing connection close\n",self);
                    break;
                }

                /* Leggo la richiesta del client */
                printf("\n[%lu] Client Request:{CLIENT_ID: %d, OPERATION: %d, FILEPATH: %s, FLAGS: %d }\n", self,
                       request.clientId, request.operation, request.filepath, request.flags);

                switch (request.operation) {

                    case OP_OPEN_FILE:
                        open_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_APPEND_FILE:

                        append_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_READ_FILE:

                        read_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_DELETE_FILE:

                        delete_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_WRITE_FILE:
                        write_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_CLOSE_FILE:

                        close_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_LOCK_FILE:

                        lock_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_UNLOCK_FILE:

                        unlock_file_controller(&fd_client_skt, &request);
                        break;

                    case OP_READ_N_FILES:

                        readn_file_controller(&fd_client_skt, &request);
                        break;

                    default:
                        printf("Received unknown operation: %d\n", request.operation);

                }

            }

            close(fd_client_skt);

            printf("[%lu] Connessione con client chiusa\n", self);
            if(!acceptNewConnection || closeServer){
                printf("[%lu] Mi termino...\n",self);
                break;
            }

        }else{
            printf("[%lu] Errore pop coda: %d\n",self,fd_client_skt);
            break;
        }

        subConnectionCont();

    }

    return 0;

}

static void *signalThreadHandler(void *arg){

    printf("****** SIGNAL THREAD INIT *****\n");
    printf("Avvio thread per la gestione dei signal\n");

    /* Maschero i segnali */
    sigset_t set; int sig;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGHUP);

    sigwait(&set, &sig); //Mi blocco in attesa di un segnale

    switch (sig) {

        /* Non accetto più connessioni, finisco di servire quelle attuali */
        case SIGHUP:
            printf("\n[SIGNAL-THREAD] Ricevuto segnale SIGHUP\n");
            acceptNewConnection = false;

            printf("[SIGNAL-THREAD] Blocco le nuove richieste di connessione al server\n");
            close(handler_signal_pipe[1]);
            pthread_mutex_lock(&connection_queue_mutex);
            pthread_cond_broadcast(&connection_queue_cond);
            pthread_mutex_unlock(&connection_queue_mutex);

            /* Sveglia eventuali thread che sono in attesa di mutex su file */
            pthread_mutex_lock(&file_queue_mutex);
            pthread_cond_broadcast(&file_queue_cond);
            pthread_mutex_unlock(&file_queue_mutex);

            break;

        /* Non accetto più connessioni, termino il prima possibile */
        case SIGINT:
        case SIGQUIT:
            printf("\n[SIGNAL-THREAD] Ricevuto segnale SIGINT o SIGQUIT\n");
            closeServer = true;

            close(handler_signal_pipe[1]);
            pthread_mutex_lock(&connection_queue_mutex);
            pthread_cond_broadcast(&connection_queue_cond);
            pthread_mutex_unlock(&connection_queue_mutex);

            /* Sveglia eventuali thread che sono in attesa di mutex su file */
            pthread_mutex_lock(&file_queue_mutex);
            pthread_cond_broadcast(&file_queue_cond);
            pthread_mutex_unlock(&file_queue_mutex);

            break;

        default:
            printf("\nRicevuto segnale non gestito:%d\n", sig);
    }

    return 0;
}
