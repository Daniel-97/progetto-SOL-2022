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
    void *buff;
    size_t size;

    time.tv_sec = 3;
    time.tv_nsec = 0;

    if(openConnection(SOCKET_NAME,1000, time) == 0){
        printf("CONNESSO CON SERVER!\n");
    }else{
        exit(-1);
    }

    openFile("./prova.txt",O_CREATE | O_LOCK);
//    lockFile("./prova.txt");
    removeFile("./prova.txt");
//    unlockFile("./prova.txt");
//    writeFile("./prova.txt","/tmp");
//    if (readFile("./prova.txt", &buff,&size) == 0 ) {
//        printf("%s\n",(char*)buff);
////        for(int i = 0; i < size; i++)
////            printf("%s\n",(char*)buff);
//        saveFile("./prova.txt", buff, size);
//    }

    closeConnection(SOCKET_NAME);

    return 0;
}