#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <config.h>
#include <connectionQueue.h>

static Config* serverConfig;
static ConnectionQueue *queue;

static void* worker();

int main(int argc, char *argv[]){

    /****** CONFIG INIT *******/
    serverConfig = malloc(sizeof(Config));
    int res = readConfig(serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(serverConfig);

    /***** CONNECTION QUEUE INIT *****/
    queue = initQueue();

    /******* THREAD INIT *****/
    pthread_t *threadPool;
    int *threadPoolStatus;
    // Alloco un array per i descrittori di processo
    threadPool = (pthread_t*)malloc(serverConfig->thread_workers * sizeof(pthread_t) );
    threadPoolStatus = (int*) malloc(serverConfig->thread_workers * sizeof(int));

//    for(int i = 0; i<serverConfig->thread_workers; i++) {
//        pthread_create(&threadPool[i], NULL, &worker, NULL);
//    }

//    for(int i = 0; i<serverConfig->thread_workers; i++) {
//        pthread_join(threadPool[i], (void *) &threadPoolStatus[i]);
//    }

    /***** SOCKET INIT *****/
    int fd_server_skt;
    int fd_client_skt; //File descriptor per connessioni client
    struct sockaddr_un socketAddress;

    strncpy(socketAddress.sun_path, serverConfig->socket_path,100);
    socketAddress.sun_family = AF_UNIX; // Socket di tipo AF_UNIX

    //Creazione socket
    if ( (fd_server_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ){
        printf("Errore creazione socket, errno: %d\n",errno);
        exit(-1);
    }
    //Bind socket con address
    if ( (bind(fd_server_skt, (struct sockaddr*) &socketAddress, sizeof(socketAddress)) ) == -1 ){
        printf("Errore bind socket, errno: %d\n",errno);
        exit(-1);
    }

    if ( listen(fd_server_skt,SOMAXCONN) == -1){
        printf("Errore listen socket, errno: %d\n",errno);
        exit(-1);
    }
    /****** MAIN SERVER LOOP *****/
    printf("\n***** ATTENDO NUOVE CONNESSIONI *****\n");

    if ( (fd_client_skt = accept(fd_server_skt,NULL,0)) == -1 ){
        printf("Errore accept socket, errno: %d\n",errno);
    }else{
        printf("NUOVA CONNESSIONE RICEVUTA!\n");
    }

    return 0;
}

static void* worker(void *arg){

    printf("Ciao a tutti sono un worker con tid: %d\n", getpid());
    sleep(2);
    return 0;

}