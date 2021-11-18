//
// Created by daniele on 17/11/21.
//

#include "serverApi.h"

int openConnection(const char* sockname, int msec, const struct timespec abstime){

    struct timespec start_time;
    struct timespec current_time;

    clock_gettime(CLOCK_REALTIME,&start_time); //Get current time

    //TODO DA AGGIUNGERE GESTIONE TIMER
    struct sockaddr_un socketAddress;

    strncpy(socketAddress.sun_path, sockname,100);
    socketAddress.sun_family = AF_UNIX;

    /* Creazione socket */
    if( (fd_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        printf("Errore creazione socket, errno: %d\n",errno);
        return -1;
    }

    while(connect(fd_socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) == -1) {

        clock_gettime(CLOCK_REALTIME,&current_time); //Get the current time
        //printf("current_time: %ld, start_time: %ld\n",current_time.tv_sec,abstime.tv_sec);

        if( current_time.tv_sec > (start_time.tv_sec+abstime.tv_sec) ){
            printf("Tempo massimo raggiunto per i tentativi di connessione, exiting\n");
            return -1;
        }

        if(errno == 111){ //Connessione rifiutata dal server
            printf("Connessione rifiutata dal server, riprovo tra %d ms ...\n",msec);
            sleep(msec/1000);
        }
        else{
            printf("Errore connect socket, errno: %d, %s\n",errno, strerror(errno));
            return -1;
        }

    }

    return 0;
}

int closeConnection(const char* sockname){

    return close(fd_socket);

}