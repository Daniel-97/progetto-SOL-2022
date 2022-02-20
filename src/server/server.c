#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include "includes/globals.h"
#include "../common/common.h"
#include "includes/config.h"
#include "includes/queue.h"
#include "includes/fileStorage.h"
#include "includes/utils.h"
#include "includes/controller.h"
#include "includes/logger.h"

static void *worker(void *arg);
static void *signalThread(void *arg);


int main(int argc, char *argv[]){

    /***** SIGNAL HANDLER INIT *****/
    /* Maschero i segnali */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGHUP);
    pthread_sigmask(SIG_SETMASK,&set,NULL);

    pthread_t tid_signal_thread;
    pthread_create(&tid_signal_thread, NULL, &signalThread, NULL);

    /****** CONFIG INIT *******/
    serverConfig = malloc(sizeof(Config));
    int res = readConfig(serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(serverConfig);

    /****** NUM CONNECTION INIT *****/
    n_connections = 0;

    /***** LOGGER INIT *******/
    loggerInit();

    /***** FILE QUEUE INIT ******/
    fileQueue = initQueue();

    /***** CONNECTION QUEUE INIT *****/
    connectionQueue = initQueue();
    if (connectionQueue == NULL){
        printf("Errore init queue, aborting...");
        exit(-1);
    }

    /******* THREAD INIT *****/
    pthread_t *threadPool;
//    int *threadPoolStatus;
    // Alloco un array per i descrittori di processo
    threadPool = (pthread_t*)malloc(serverConfig->thread_workers * sizeof(pthread_t) );
//    threadPoolStatus = (int*) malloc(serverConfig->thread_workers * sizeof(int));

    for(int i = 0; i<serverConfig->thread_workers; i++) {
        pthread_create(&threadPool[i], NULL, &worker, NULL);
    }

//    for(int i = 0; i<serverConfig->thread_workers; i++) {
//        pthread_join(threadPool[i], (void *) &threadPoolStatus[i]);
//    }

    /***** SOCKET INIT *****/
    int fd_server_skt;
    int *fd_client_skt; //File descriptor per connessioni client
    struct sockaddr_un socketAddress;

    strncpy(socketAddress.sun_path, serverConfig->socket_path,100);
    socketAddress.sun_family = AF_UNIX; // Socket di tipo AF_UNIX

    //Creazione socket
    if ( (fd_server_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ){
        printf("[MASTER] Errore creazione socket, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }
    // Unlink vecchio socket se esistente
    unlink(serverConfig->socket_path);

    //Bind socket con address
    if ( (bind(fd_server_skt, (struct sockaddr*) &socketAddress, sizeof(socketAddress)) ) == -1 ){
        printf("[MASTER] Errore bind socket, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }

    if ( listen(fd_server_skt,SOMAXCONN) == -1){
        printf("[MASTER] Errore listen socket, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }
    /****** MAIN SERVER LOOP *****/
    printf("\n***** ATTENDO NUOVE CONNESSIONI *****\n");

    while(1) {

        fd_client_skt = malloc(sizeof(int)); //Alloco puntatore per nuovo id file descriptor

        if ((*fd_client_skt = accept(fd_server_skt, NULL, 0)) == -1) {

            printf("[MASTER] Errore accept socket, errno: %d, %s\n", errno, strerror(errno));
            break;

        } else {

            printf("\n[MASTER] Nuova connessione ricevuta, fd_skt:%d\n",*fd_client_skt);
            if( push(connectionQueue,fd_client_skt) != -1){
                printf("[MASTER] File descriptor client socket inserito nella coda\n");

            }else{
                printf("[MASTER] Errore inserimento file descriptor client socket nella coda\n");

            }
        }
    }
    close(fd_server_skt);
    return 0;
}

static void *worker(void *arg){

    pthread_t self = pthread_self();
    int *fd_client_skt = NULL;
    Request *request =  malloc(sizeof(Request));

    printf("[%lu] Worker start\n", self);

    /* Maschero i signal per i nuovi worker per evitare che vengano interrotti */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGHUP);
    pthread_sigmask(SIG_SETMASK,&set,NULL);

    while(1){
        printf("[%lu] In attesa di nuova richiesta...\n",self);
        fd_client_skt = pop(connectionQueue);

        addConnectionCont();

        if (*fd_client_skt != -1){

            /* Continuo a leggere dal socket del client fintanto che ci sono dati */
            while( read(*fd_client_skt, request,sizeof(Request)) > 0 ) {

                /* Leggo la richiesta del client */
                printf("\n[%lu] Client Request:{CLIENT_ID: %d, OPERATION: %d, FILEPATH: %s, FLAGS: %d }\n", self,
                       request->clientId, request->operation, request->filepath, request->flags);

                switch (request->operation) {

                    case OP_OPEN_FILE:
                        open_file_controller(fd_client_skt, request);
                        break;

                    case OP_APPEND_FILE:

                        append_file_controller(fd_client_skt, request);
                        break;

                    case OP_READ_FILE:

                        read_file_controller(fd_client_skt, request);
                        break;

                    case OP_DELETE_FILE:

                        delete_file_controller(fd_client_skt, request);
                        break;

                    case OP_WRITE_FILE:
                        write_file_controller(fd_client_skt, request);
                        break;

                    case OP_CLOSE_FILE:

                        close_file_controller(fd_client_skt, request);
                        break;

                    case OP_LOCK_FILE:

                        lock_file_controller(fd_client_skt, request);
                        break;

                    case OP_UNLOCK_FILE:

                        unlock_file_controller(fd_client_skt, request);
                        break;

                    case OP_READ_N_FILES:

                        readn_file_controller(fd_client_skt, request);
                        break;

                    default:
                        printf("Received unknown operation: %d\n", request->operation);

                }

                /* Preparo la risposta per il client */
//                response->statusCode = 0;
//                strcpy(response->message, "All ok!");
//                printf("[%lu] Invio risposta al client...\n", self);
//
//                if (write(*fd_client_skt, response, sizeof(Response)) != -1){
//                    printf("[%lu] Risposta inviata al client!\n",self);
//                }
            }

        }else{
            printf("[%lu] Errore pop coda: %d\n",self,*fd_client_skt);
            break;
        }

        subConnectionCont();

    }
    return 0;

}

static void *signalThread(void *arg){

    printf("****** SIGNAL THREAD INIT *****\n");
    printf("Avvio thread per la gestione dei signal\n");

    sigset_t set; int sig;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGHUP);
    pthread_sigmask(SIG_SETMASK,&set,NULL);

    sigwait(&set,&sig); //Mi blocco in attesa di un segnale

    switch (sig) {
        case SIGINT:
            printf("\nRicevuto segnale SIGINT\n");
            break;
        case SIGQUIT:
            printf("\nRicevuto segnale SIGQUIT\n");
            break;
        case SIGHUP:
            printf("\nRicevuto segnale SIGHUP\n");
            break;
    }
    exit(0);

    return NULL;
}
