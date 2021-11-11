#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>

#include <config.h>
#include <connectionQueue.h>

static Config* serverConfig;

static void* worker();

int main(int argc, char *argv[]){

    /****** CONFIG SECTION *******/
    serverConfig = malloc(sizeof(Config));
    int res = readConfig(serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(serverConfig);

    /******* THREAD SECTION *****/
    pthread_t *threadPool;
    threadPool =(pthread_t*)malloc(serverConfig->thread_workers * sizeof(pthread_t) ); // Alloco un array per i descrittori di processo

    for(int i = 0; i<serverConfig->thread_workers; i++) {
        pthread_create(&threadPool[i], NULL, &worker, NULL);
    }
    int status;
    for(int i = 0; i<serverConfig->thread_workers; i++) {
        pthread_join(threadPool[i], (void *) &status);
    }

    return 0;
}

static void* worker(void *arg){

    printf("Ciao a tutti sono un worker con tid: %d\n", getpid());
    sleep(2);
    return 0;

}