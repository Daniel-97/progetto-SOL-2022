#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "includes/globals.h"
#include "includes/config.h"
#include "includes/queue.h"
#include "includes/fileQueue.h"

static void *worker(void *arg);

int main(int argc, char *argv[]){

    /****** CONFIG INIT *******/
    serverConfig = malloc(sizeof(Config));
    int res = readConfig(serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(serverConfig);

    /***** FILE QUEUE INIT ******/
    fileQueue = initQueue();
    //        CODICE DI TEST CODA FILE
//    FileNode fn1 = {.pathname = "tmp1", .fd = NULL};
//    FileNode fn2 = {.pathname = "tmp2", .fd = NULL};
//    FileNode fn3 = {.pathname = "tmp3", .fd = NULL};
//    FileNode *removedFile = NULL;
//    insertFile(fileQueue,&fn1,&removedFile);
//    insertFile(fileQueue,&fn2,&removedFile);
//    insertFile(fileQueue,&fn3,&removedFile);
//    if (removedFile)
//        printf("File rimosso: %s\n", removedFile->pathname);
//
//    FileNode *f4 = findFile(fileQueue, "tmp3");
//    if (f4)
//        printf("File trovato: %s\n", f4->pathname);
//    exit(0);
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

            printf("[MASTER] Nuova connessione ricevuta, fd_skt:%d\n",*fd_client_skt);
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
    Request *request = malloc(sizeof(Request));
    Response *response = malloc(sizeof(Response));

    printf("[%lu] Worker start\n", self);

    while(1){
        printf("[%lu] In attesa di nuova richiesta...\n",self);
        fd_client_skt = pop(connectionQueue);

        if (*fd_client_skt != -1){

            /* Leggo la richiesta del clint */
            printf("[%lu] Leggo richiesta del client con socket: %d!\n",self,*fd_client_skt);
            read(*fd_client_skt, request,sizeof(Request));
            printf("[%lu]Client Request:\n-operation: %d,\n-filepath: %s\n",self,request->operation, request->filepath);

            /* Preparo la risposta per il client */
            response->statusCode = 200;
            response->success = 1;
            strcpy(response->message, "All ok!");
            printf("[%lu] Sending response to client...\n",self);
            write(*fd_client_skt,response,sizeof(Response));

        }else{
            printf("[%lu] Errore pop coda: %d\n",self,*fd_client_skt);
            break;
        }

    }
    return 0;

}