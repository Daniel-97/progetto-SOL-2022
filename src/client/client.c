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

    time.tv_sec = 3;
    time.tv_nsec = 0;

    if(openConnection(SOCKET_NAME,1000, time) == 0){
        printf("CONNESSO CON SERVER!\n");
    }else{
        exit(-1);
    }

    openFile("/tmp/prova.txt",O_CREATE);
    openFile("/tmp/provaaaa2.txt",O_LOCK);

    closeConnection(SOCKET_NAME);

    return 0;
}