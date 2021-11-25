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

int openFile(const char* pathname, int flags){

    Request *request = malloc(sizeof(Request));
    Response *response = malloc(sizeof(Response));

    request->clientId = getpid();
    request->operation = OP_OPEN_FILE;
    request->flags = flags;
    strncpy(request->filepath,pathname,MAX_PATH_SIZE);

    printf("Invio richiesta apertura per filepath: %s\n",request->filepath);

    /* Invio richiesta al server */
    if ( write(fd_socket,request,sizeof(Request)) != -1 ){

        /* Attendo risposta dal server */
        if ( read(fd_socket,response,sizeof(Response)) != -1 ){

            printServerResponse(response);
            return response->statusCode;

        }else{
            printf("Errore write socket, errno: %d\n",errno);
            return -1;
        }

    }else{
        printf("Errore write socket, errno: %d\n",errno);
        return -1;
    }

}

int readFile(const char* pathname, void** buf, size_t* size){

    Request request;
    Response response;

    request.operation = OP_READ_FILE;
    strncpy(request.filepath, pathname,MAX_PATH_SIZE);
    request.clientId = getpid();
    request.flags = 0;


    printf("Invio richiesta lettura per filepath: %s\n",request.filepath);

    /* Invio richiesta al server */
    if ( write(fd_socket,&request,sizeof(Request)) != -1 ){

//        printf("Attendo dimensione del file dal server...\n");
        /* Attendo risposta dal server con dimensione del file che sto per ricevere */
        if ( read(fd_socket,&response,sizeof(Response)) != -1 ){

            printServerResponse(&response);

            if(response.statusCode == -1)
                return -1;

            /* Mi salvo la dimensione del file */
            *size = response.fileSize;
            *buf = malloc(response.fileSize); //Alloco il buffer per la lettura del file
            // Leggo effettivamente il file dal server
            if (read(fd_socket,*buf,*size) != -1){

                printf("File ricevuto correttamente!\n");
                return 0;

            }else{

                printf("Errore ricezione file. errno: %d. %s", errno, strerror(errno));
                return -1;

            }


        }else{
            printf("Errore read socket, errno: %d, %s\n",errno, strerror(errno));
            return -1;
        }

    }else{
        printf("Errore write socket, errno: %d, %s\n",errno, strerror(errno));
        return -1;
    }

}

int writeFile(const char* pathname, const char* dirname){

    Request request;
    Response response;
    FILE *file;
    void *buf;

    if (pathname == NULL) return -1;

    /* Apro il file locale che devo inviare al server */
    file = fopen(pathname, "r");
    fseek(file,0L, SEEK_END); /* Mi posiziono all inizio del file */

    request.operation = OP_WRITE_FILE;
    strncpy(request.filepath, pathname, MAX_PATH_SIZE);
    request.clientId = getpid();
    request.fileSize = ftell(file);

    printf("Invio richiesta scrittura per file: %s con dimensione %zu\n",request.filepath,request.fileSize);

    /* Invio richiesta al server */
    if ( write(fd_socket, &request, sizeof(Request)) != -1){

        /* Attendo risposta dal server */
        if ( read(fd_socket, &response, sizeof(Response)) != -1){

            printServerResponse(&response);

            if(response.statusCode == -1) return -1;

            /* Leggo il contenuto del nuovo file */
            buf = malloc(request.fileSize); //Alloco buffer per lettura file
            rewind(file); //Mi riposiziono all inizio del file
            int cont = fread(buf,request.fileSize,1,file);

            printf("Invio il file %s al server\n",pathname);
            /* Invio al server il file! */
            if ( write(fd_socket,buf,request.fileSize) != -1){

                printf("File inviato correttamente al server!\n");

                /* Attendo risposta dal server con messaggio di successo */
                if( read(fd_socket, &response, sizeof(Response)) ){

                    printServerResponse(&response);
                    return response.statusCode;

                }

            }else{

                printf("Errore invio file al server\n");
                return -1;
            }

        }else{
            printf("Errore ricezione risposta dal server. errno %d,%s",errno, strerror(errno));
            return -1;
        }

    }else{
        printf("Errore invio richiesta di scrittura al server. errno %d,%s",errno, strerror(errno));
        return -1;
    }


}

//Todo implementare questa funzione
int appendToFile(const char* pathname, void *buf, size_t size, const char* dirname){

    Request request;
    Response response;

    if (pathname == NULL || buf == NULL) return -1;

    request.operation = OP_APPEND_FILE;
    strncpy(request.filepath, pathname, MAX_PATH_SIZE);
    request.clientId = getpid();
    request.fileSize = size;

    printf("Invio richiesta scrittura in append per file: %s con dimensione %zu\n",request.filepath,request.fileSize);

    /* Invio richiesta al server */
    if ( write(fd_socket, &request, sizeof(Request)) != -1){

        /* Attendo risposta dal server */
        if ( read(fd_socket, &response, sizeof(Response)) != -1){

            printServerResponse(&response);

            if(response.statusCode == -1) return -1;

            printf("Invio i dati per append file %s al server\n",pathname);
            /* Invio al server il file! */
            if ( write(fd_socket,buf,request.fileSize) != -1){

                printf("File inviato correttamente al server!\n");

                /* Attendo risposta dal server con messaggio di successo */
                if( read(fd_socket, &response, sizeof(Response)) ){

                    printServerResponse(&response);
                    return response.statusCode;

                }

            }else{

                printf("Errore invio file al server\n");
                return -1;
            }

        }else{
            printf("Errore ricezione risposta dal server. errno %d,%s",errno, strerror(errno));
            return -1;
        }

    }else{
        printf("Errore invio richiesta di scrittura append al server. errno %d,%s",errno, strerror(errno));
        return -1;
    }

}

int lockFile(const char* pathname){

    Request request;
    Response response;

    if(pathname == NULL) return -1;

    request.operation = OP_LOCK_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    printf("Invio richiesta di lock per file %s\n", pathname);

    return sendRequest(&request,&response);

}

int unlockFile(const char* pathname){

    Request request;
    Response response;

    if(pathname == NULL) return -1;

    request.operation = OP_UNLOCK_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    printf("Invio richiesta di unlock per file %s\n", pathname);

    return sendRequest(&request,&response);
}

int removeFile(const char* pathname){

    Request request;
    Response response;

    if(pathname == NULL) return -1;

    request.operation = OP_DELETE_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    printf("Invio richiesta di cancellazione per file %s\n", pathname);

    return sendRequest(&request, &response);
}

int sendRequest(Request *request, Response *response){

    /* Invio richiesta al server */
    if ( write(fd_socket,request,sizeof(Request)) != -1 ){

        /* Attendo risposta dal server */
        if ( read(fd_socket,response,sizeof(Response)) != -1 ){

            printServerResponse(response);
            return 0;

        }else{
            printf("Errore write socket, errno: %d\n",errno);
            return -1;
        }

    }else{
        printf("Errore write socket, errno: %d\n",errno);
        return -1;
    }

}

