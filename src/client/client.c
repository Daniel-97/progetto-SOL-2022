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
#include <time.h>

#include "includes/utils.h"
#include "includes/serverApi.h"
#include "../common/common.h"

#define SOCKET_NAME "./connection.sk"

int main(int argc, char *argv[]){

    int opt = 0;
    opt += 0;
    struct timespec time;

    time.tv_sec = 2;
    time.tv_nsec = 0;

    if(openConnection(SOCKET_NAME,1000, time) == 0){
        printf("CONNESSO CON SERVER!\n");
    }else{
        exit(-1);
    }

    Request request = {.operation = OP_OPEN_FILE, .filepath="tmp"};
    Response *response = malloc(sizeof(Response));

    printf("Invio richiesta al server...\n");

    /* Invio richiesta al server */
    write(fd_socket,&request,sizeof(request));

    /* Attendo risposta dal server*/
    read(fd_socket,response,sizeof(Response));

    printf("Server response:\n-statusCode: %d\n-message: %s\n",response->statusCode, response->message);

    close(fd_socket);
    return 0;
}