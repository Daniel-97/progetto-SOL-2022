#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>

#include "includes/globals.h"
#include "../common/common.h"
#include "includes/config.h"
#include "includes/queue.h"
#include "includes/fileQueue.h"
#include "includes/fileStorage.h"

static void *worker(void *arg);

//static int *epoll_descriptors;
static int epoll_descriptor;

int main(int argc, char *argv[]){

//    int cont = 0;

    /****** CONFIG INIT *******/
    serverConfig = malloc(sizeof(Config));
    int res = readConfig(serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(serverConfig);

    /***** EPOOL INIT *****/
    //Inizializzo spazio per descrittori epoll workers
//    epoll_descriptors = malloc(serverConfig->thread_workers * sizeof(int));
//
//    for(int i = 0; i < serverConfig->thread_workers; i++){
//        if ( (epoll_descriptors[i] = epoll_create(100) ) == -1 ){
//            printf("[MASTER] Errore nella creazione epoll worker n.%d, errno: %d, %s\n",i,errno, strerror(errno));
//            exit(-1);
//        }
//    }
    if ( (epoll_descriptor = epoll_create(100) ) == -1 ){
        printf("[MASTER] Errore nella creazione epoll, errno: %d, %s\n",errno, strerror(errno));
        exit(-1);
    }

    /***** FILE QUEUE INIT ******/
    fileQueue = initQueue();

    /***** CONNECTION QUEUE INIT *****/
    connectionQueue = initQueue();
    if (connectionQueue == NULL){
        printf("[MASTER] Errore init queue, aborting...");
        exit(-1);
    }

    /******* THREAD INIT *****/
    pthread_t *threadPool;
    // Alloco un array per i descrittori di processo
    threadPool = (pthread_t*)malloc(serverConfig->thread_workers * sizeof(pthread_t) );

    for(int i = 0; i<serverConfig->thread_workers; i++) {
        pthread_create(&threadPool[i], NULL, &worker, NULL);
    }

    /***** SOCKET INIT *****/
    int fd_server_skt;  //File descriptor server socket
    int *fd_client_skt; //File descriptor client socket
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

            printf("[MASTER] Nuova connessione ricevuta, fd_skt:%d\n",*fd_client_skt);

            /* Alloco lo spazio per le strutture dati necessarie all elemento della epoll e inizializzo*/
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
            ev.data.fd = *fd_client_skt;

            if ( epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, *fd_client_skt, &ev) == -1){

                printf("[MASTER] Errore inserimento client file descriptor in epoll. errno; %d, %s\n",errno, strerror(errno));

            }else{
                printf("[MASTER] File descriptor %d inserito correttamente in epoll con fd %d\n",*fd_client_skt,epoll_descriptor);
            }

        }
    }
    close(fd_server_skt);
    return 0;
}


/****** WORKER ******/
static void *worker(void *arg){

    pthread_t self = pthread_self();
//    int *epoll_fd = (int*)arg;
    struct epoll_event *client_epoll_event = malloc(sizeof(struct epoll_event));

    Request *request = malloc(sizeof(Request));
    Response *response = malloc(sizeof(Response));

    printf("[%lu] Worker start with epoll file descriptor: %d\n", self,epoll_descriptor);

    while(1){

        printf("[%lu] In attesa di nuovo evento su epoll...\n",self);
        if ( epoll_wait(epoll_descriptor, client_epoll_event, 1, -1) != -1){

            printf("[%lu] Nuovo evento da epoll ricevuto. fd: %d. Eseguo read()\n",self, client_epoll_event->data.fd);
            read(client_epoll_event->data.fd, request,sizeof(Request));
            printf("[%lu] Client Request:{CLIENT_ID: %d, OPERATION: %d, FILEPATH: %s, FLAGS: %d }\n",self,request->clientId,request->operation, request->filepath,request->flags);

            /* Preparo la risposta per il client */
            response->statusCode = 0;
            response->success = 1;
            strcpy(response->message, "All ok!");
            printf("[%lu] Invio risposta al client...\n",self);
            write(client_epoll_event->data.fd,response,sizeof(Response));

            epoll_ctl(epoll_descriptor, EPOLL_CTL_MOD,client_epoll_event->data.fd,client_epoll_event);

        }else{

            printf("[%lu] Errore attesa nuovo evento su epool\n", self);
            break;
        }

//        if (*fd_client_skt != -1){
//
//            /* Leggo la richiesta del clint */
//            printf("[%lu] Leggo richiesta del client con socket: %d!\n",self,*fd_client_skt);
//            read(*fd_client_skt, request,sizeof(Request));
//            printf("[%lu] Client Request:{CLIENT_ID: %d, OPERATION: %d, FILEPATH: %s, FLAGS: %d }\n",self,request->clientId,request->operation, request->filepath,request->flags);
//
//            switch (request->operation) {
//
//                case OP_OPEN_FILE:
//                    openVirtualFile(request->filepath, request->flags, request->clientId);
//                    break;
//                case OP_WRITE_FILE:
//                    break;
//                case OP_READ_FILE:
//                    break;
//                case OP_DELETE_FILE:
//                    break;
//                case OP_APPEND_FILE:
//                    break;
//                case OP_CLOSE_FILE:
//                    break;
//                case OP_LOCK_FILE:
//                    break;
//                case OP_UNLOCK_FILE:
//                    break;
//                default:
//                    printf("Received unknown operation: %d\n",request->operation);
//
//            }
//            /* Preparo la risposta per il client */
//            response->statusCode = 0;
//            response->success = 1;
//            strcpy(response->message, "All ok!");
//            printf("[%lu] Invio risposta al client...\n",self);
//            write(*fd_client_skt,response,sizeof(Response));
//
//        }else{
//            printf("[%lu] Errore pop coda: %d\n",self,*fd_client_skt);
//            break;
//        }


    }
    return 0;

}