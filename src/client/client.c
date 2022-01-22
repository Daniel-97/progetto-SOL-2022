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

#define DEFAULT_SOCKET_NAME "./connection.sk"

int main(int argc, char *argv[]){

    char *socket_name = NULL;
    char *dirname = NULL;
    char *destFolder = NULL;
    int opt = 0;
    opt += 0;
    struct timespec time;
    void *buff;
    size_t size;
    char *token;
    int numFiles;
    int enableDebug = 0;
//    char *fileList;

    time.tv_sec = 3;
    time.tv_nsec = 0;

    while( (opt = getopt(argc, argv, "hf:W:w:D:d:r:R:l:u:c:")) != -1 ){

        switch (opt) {

            case 'h':  /* Print help menu */
                help();
                exit(0);
                break;

            case 'f':
                socket_name = malloc(sizeof(optarg));
                strcpy(socket_name, optarg);

                /* Apro connessione con il server */
                if(openConnection(socket_name,1000, time) == 0){
                    printf("CONNESSO CON SERVER SU SOCKET: %s\n",socket_name);
                }else{
                    exit(-1);
                }

                break;

            case 'w': /* Scriver sul server i file nella directory specificata */
                token = getFileListFromDir(optarg);
                printf("File to send: %s\n",token);

                break;

            case 'W': /* Lista di file da inviare separati da , */

                token = strtok(optarg,",");
                while ( token != NULL ){

                    if ( openFile(token,O_CREATE | O_LOCK) != -1)
                        writeFile(token, dirname);
                    token = strtok(NULL, ",");
                }
                break;

            case 'D': /* Specifica cartella dove scrivere file rimossi per capacity miss*/
                dirname = malloc(sizeof(optarg));
                strcpy(dirname, optarg);
                break;

            case 'r': /* Lista di file da leggere separari da , */
                token = strtok(optarg,",");
                while ( token != NULL ){

                    if ( openFile(token, O_LOCK) != -1)
                        if( readFile(token, &buff, &size) != -1 )
                            saveFileDir(buff, size, destFolder, token);

                    token = strtok(NULL, ",");
                }
                break;

            case 'R': /* Legge n file qualsiasi dal server */
                numFiles = atoi(optarg);
                if(numFiles == 0){
                    readNFiles(-1, destFolder);
                }else{
                    readNFiles(numFiles, destFolder);
                }
                break;

            case 'd': /* Specifica cartella in cui salvare file lato client */
                destFolder = malloc(sizeof(optarg));
                strcpy(destFolder, optarg);
                break;

            case 't': /* tempo tra una richiesta e l'altra */
                break;

            case 'l': /* Lista di file su cui fare lock separati da ,*/
                token = strtok(optarg,",");

                while( token != NULL){
                    lockFile(token);
                    token = strtok(NULL, ",");
                }

                break;

            case 'u': /* Lista di file su cui fare unlock separati da , */
                token = strtok(optarg,",");

                while( token != NULL){
                    unlockFile(token);
                    token = strtok(NULL, ",");
                }
                break;

            case 'c': /* Lista di file da rimuovere dal server separati da , */
                token = strtok(optarg,",");

                while( token != NULL){
                    removeFile(token);
                    token = strtok(NULL, ",");
                }
                break;

            case 'p': /* Abilita print di debug*/
                enableDebug = 1;
                break;
        }

    }
    exit(0);
    openFile("./prova.txt",O_CREATE | O_LOCK);
    openFile("./test.txt",O_CREATE | O_LOCK);
//    closeFile("./prova.txt");
//    lockFile("./prova.txt");
//    writeFile("./prova.txt", "./tmp");

    appendToFile("./prova.txt","file1",sizeof("file1"),"./tmp/");
    appendToFile("./test.txt","file2",sizeof("file2"),"./tmp/");
//    readNFiles(10, "./tmp/");
//    appendToFile("./prova.txt","123",sizeof("123"),"./tmp");

//    removeFile("./prova.txt");
//    unlockFile("./prova.txt");
//    writeFile("./prova.txt","/tmp");
//    writeFile("./prova.txt","/tmp");

//    if (readFile("./prova.txt", &buff,&size) == 0 ) {
//        printf("%s\n",(char*)buff);
////        for(int i = 0; i < size; i++)
////            printf("%s\n",(char*)buff);
//        saveFile("./tmp/prova.txt", buff, size);
//    }

//    unlockFile("./prova.txt");
    closeConnection(socket_name);

    return 0;
}