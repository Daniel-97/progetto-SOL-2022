#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "includes/utils.h"

#define SOCKET_NAME "./connection.sk"

int main(int argc, char *argv[]){

    int opt;
    int cont = 0;
    int fd_socket;
    struct sockaddr_un socketAddress;
//    while( (opt = getopt(argc, argv, "h")) != -1 ){
//        switch (opt) {
//            case 'h':
//                help();
//                exit(0);
//        }
//    }

    strncpy(socketAddress.sun_path, SOCKET_NAME,100);
    socketAddress.sun_family = AF_UNIX;

    /* Creazione socket */
    if( (fd_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        printf("Errore creazione socket, errno: %d\n",errno);
        exit(-1);
    }

    while(connect(fd_socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) == -1) {
        printf("Tentativo numero %d di connessione al server...\n",++cont);
        if(errno == ENOENT){
            sleep(1);
        }
        else{
            printf("Errore connect socket, errno: %d, %s\n",errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    printf("CONNESSO CON SERVER!\n");
    close(fd_socket);
    return 0;
}