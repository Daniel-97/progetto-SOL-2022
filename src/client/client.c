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
#include <stdbool.h>
#include <limits.h>

#include "includes/utils.h"
#include "includes/serverApi.h"
#include "../common/common.h"

#define DEFAULT_SOCKET_NAME "./connection.sk"

int main(int argc, char *argv[]){
    print("test %d\n", 4);
    char *socket_name = NULL;
    char *expFileDir = NULL;
    char *destFolder = NULL;
    int opt = 0;
    struct timespec time;
    void *buff;
    size_t size;
    char *token;
    int numFiles;
    char abspath[PATH_MAX];
//    char *fileList;

    /* CONFIG INIT */
    enablePrintDebug = false;
    waitingTime = 0;

    time.tv_sec = 3;
    time.tv_nsec = 0;

    while( (opt = getopt(argc, argv, "hf:W:w:D:d:r:R:l:u:c:t:p")) != -1 ){

        switch (opt) {

            case 'h':  /* Print help menu */
                help();
                exit(0);

            case 'f': {
                socket_name = allocateMemory(1, strlen(optarg)+1);
                strcpy(socket_name, optarg);

                /* Apro connessione con il server */
                if (openConnection(socket_name, 1000, time) == 0) {
                    print("CONNESSO CON SERVER SU SOCKET: %s\n", socket_name);
                } else {
                    exit(-1);
                }

                break;
            }

            case 'w': { /* Scriver sul server i file nella directory specificata */

                char *fileList;
                char *dir = strtok(optarg,",");
                token = strtok(NULL, ",");

                if(token != NULL){
                    int maxFile = atoi(token);
                    fileList = getFileListFromDir(dir, maxFile);
                } else{
                    fileList = getFileListFromDir(dir, -1);
                }

//                printf("Sto per inviare i seguenti file: %s\n",fileList);
                token = strtok(fileList, ",");

                while (token != NULL){

                    realpath(token, abspath);
                    if(openFile(abspath, O_CREATE | O_LOCK) != -1) {
                        writeFile(abspath, expFileDir);
                        unlockFile(abspath);
                    }
                    token = strtok(NULL, ",");
                }
                free(fileList);
                break;
            }

            case 'W': { /* Lista di file da inviare separati da , */

                token = strtok(optarg, ",");
                while (token != NULL) {

                    realpath(token, abspath);
                    if (openFile(abspath, O_CREATE | O_LOCK) != -1) {
                        writeFile(abspath, expFileDir);
//                        sleep(10);
                        unlockFile(abspath);
                    }
                    token = strtok(NULL, ",");
                }
                break;
            }

            case 'D': { /* Specifica cartella dove scrivere file rimossi per capacity miss*/

                expFileDir = allocateMemory(1, strlen(optarg)+1);
                strcpy(expFileDir, optarg);
                break;
            }

            case 'r': { /* Lista di file da leggere separari da , */
                token = strtok(optarg, ",");
                while (token != NULL) {

                    realpath(token, abspath);
                    if (openFile(abspath, O_LOCK) != -1) {
                        if (readFile(abspath, &buff, &size) != -1)
                            saveFileDir(buff, size, destFolder, token);
                        unlockFile(abspath);
                    }

                    token = strtok(NULL, ",");
                }
                break;
            }

            case 'R': {/* Legge n file qualsiasi dal server */

                numFiles = atoi(optarg);
                if (numFiles == 0) {
                    readNFiles(-1, destFolder);
                } else {
                    readNFiles(numFiles, destFolder);
                }

                break;
            }

            case 'd': {/* Specifica cartella in cui salvare file lato client */
                destFolder = allocateMemory(1, strlen(optarg)+1);
                strcpy(destFolder, optarg);
                break;
            }

            case 't': { /* tempo tra una richiesta e l'altra */
                long time = atoi(optarg);

                if (time < 0) {
                    print("Errore, parametro -t deve essere un intero positivo\n");
                    exit(-1);
                }
                waitingTime = time;
                break;
            }

            case 'l': { /* Lista di file su cui fare lock separati da ,*/
                token = strtok(optarg, ",");

                while (token != NULL) {

                    realpath(token, abspath);
                    lockFile(abspath);
                    token = strtok(NULL, ",");
                }

                break;
            }

            case 'u': { /* Lista di file su cui fare unlock separati da , */
                token = strtok(optarg, ",");

                while (token != NULL) {

                    realpath(token, abspath);
                    unlockFile(abspath);
                    token = strtok(NULL, ",");
                }
                break;
            }

            case 'c': { /* Lista di file da rimuovere dal server separati da , */
                token = strtok(optarg, ",");

                while (token != NULL) {
                    realpath(token, abspath);
                    lockFile(abspath);
                    removeFile(abspath);
                    token = strtok(NULL, ",");
                }
                break;
            }

            case 'p': /* Abilita print di debug*/
                enablePrintDebug = true;
                break;
        }

    }

    closeConnection(socket_name);
    return 0;

}