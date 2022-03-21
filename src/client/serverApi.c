//
// Created by daniele on 17/11/21.
//

#include "includes/serverApi.h"

int openConnection(const char* sockname, int msec, const struct timespec abstime){

    struct timespec start_time;
    struct timespec current_time;

    clock_gettime(CLOCK_REALTIME,&start_time); //Get current time

    struct sockaddr_un socketAddress;

    strncpy(socketAddress.sun_path, sockname,100);
    socketAddress.sun_family = AF_UNIX;

    /* Creazione socket */
    if( (fd_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        print("Errore creazione socket, errno: %d\n",errno);
        return -1;
    }

    while(connect(fd_socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) == -1) {

        clock_gettime(CLOCK_REALTIME,&current_time); //Get the current time
        //print("current_time: %ld, start_time: %ld\n",current_time.tv_sec,abstime.tv_sec);

        if( current_time.tv_sec > (start_time.tv_sec+abstime.tv_sec) ){
            print("Tempo massimo raggiunto per i tentativi di connessione, exiting\n");
            return -1;
        }

        if(errno == 111){ //Connessione rifiutata dal server
            print("Connessione rifiutata dal server, riprovo tra %d ms ...\n",msec);
            sleep(msec/1000);
        }
        else{
            print("Errore connect socket, errno: %d, %s\n",errno, strerror(errno));
            return -1;
        }

    }

    return 0;
}

int closeConnection(const char* sockname){
    msleep(waitingTime);
    return close(fd_socket);

}

int openFile(const char* pathname, int flags){

    msleep(waitingTime);

    Request request;
    Response response;
    int ret = 0;

//    char absPath[PATH_MAX];
//    realpath(pathname, absPath);

    request.clientId = getpid();
    request.operation = OP_OPEN_FILE;
    request.flags = flags;
    strncpy(request.filepath, pathname,PATH_MAX);

    print("Invio richiesta apertura per filepath: %s\n",request.filepath);

    /* Invio richiesta al server */
    if ( write(fd_socket,&request,sizeof(Request)) != -1 ){

        /* Attendo risposta dal server */
        if ( read(fd_socket,&response,sizeof(Response)) != -1 ){

            printServerResponse(&response);
            ret = response.statusCode;

        }else{
            print("Errore write socket, errno: %d\n",errno);
            ret =  -1;
        }

    }else{
        print("Errore write socket, errno: %d\n",errno);
        ret = -1;
    }
    return ret;
}

int readFile(const char* pathname, void** buf, size_t* size){

    msleep(waitingTime);

    Request request;
//    char absPath[PATH_MAX];
//    realpath(pathname, absPath);

    request.operation = OP_READ_FILE;
    strncpy(request.filepath,pathname,PATH_MAX);
    request.clientId = getpid();
    request.flags = 0;


    print("Invio richiesta lettura per filepath: %s\n",request.filepath);

    /* Invio richiesta al server */
    if ( write(fd_socket,&request,sizeof(Request)) != -1 ){

//        print("Attendo dimensione del file dal server...\n");
        return  waitServerFile(buf, size);

    }else{
        print("Errore write socket, errno: %d, %s\n",errno, strerror(errno));
        return -1;
    }

}

int writeFile(const char* pathname, const char* dirname){

    msleep(waitingTime);

    Request request;
    Response response;
    FILE *file;
    void *buf;
    int isServerFull = 0;
//    char absPath[PATH_MAX];
    int wbyte;

    if (pathname == NULL) return -1;

//    realpath(pathname, absPath);

    /* Apro il file locale che devo inviare al server */
    file = fopen(pathname, "r");

    if(file == NULL){
        print("Errore apertura file %s, errno: %s\n",pathname, strerror(errno));
        return -1;
    }
    /* Mi posiziono all inizio del file */
    if ( fseek(file,0L, SEEK_END) != 0) {
        print("Errore fseek file %s\n", pathname);
        return -1;
    }

    request.operation = OP_WRITE_FILE;
    strncpy(request.filepath, pathname, PATH_MAX);
    request.clientId = getpid();
    request.fileSize = ftell(file);

    print("Invio richiesta scrittura per file: %s con dimensione %zu\n",request.filepath,request.fileSize);

    /* Invio richiesta al server */
    if ( write(fd_socket, &request, sizeof(Request)) != -1){

        /* Attendo risposta dal server */
        if ( read(fd_socket, &response, sizeof(Response)) != -1){


            //Il server è pieno, mi invierà il file che è stato espulso
            if(response.statusCode == 1){

                printServerResponse(&response);
                print("Il server sta per inviare il file che ha espulso\n");
                buf = malloc(response.fileSize);
                if (read(fd_socket,buf,response.fileSize) != -1)
                    print("Ricevuto file espulso dal server\n");

                if(dirname != NULL){ //save the file on disk
                    saveFileDir(buf, response.fileSize, dirname,response.fileName);
                }
                free(buf);

                //Ri-effettuo una read per leggere il file effettivo che il server mi deve inviare
                read(fd_socket, &response, sizeof(Response));

            }

            if(response.statusCode == -1){
                printServerResponse(&response);
                return -1;
            }

            /* Leggo il contenuto del nuovo file */
            buf = allocateMemory(1, request.fileSize); //Alloco buffer per lettura file
            rewind(file); //Mi riposiziono all inizio del file
            fread(buf,request.fileSize,1,file);

            print("Invio il file %s al server\n", pathname);
            /* Invio al server il file! */
            if ( (wbyte = write(fd_socket,buf,request.fileSize)) != -1){

                print("File inviato correttamente al server!. wbyte: %d\n",wbyte);

                /* Attendo risposta dal server con messaggio di successo */
                if( read(fd_socket, &response, sizeof(Response)) )
                    printServerResponse(&response);

//                fsync(fd_socket);
                /* Faccio la unlock sul file */
//                sleep(10);
//                unlockFile(absPath);

                return response.statusCode;

            }else{

                print("Errore invio file al server\n");
                return -1;
            }

        }else{
            print("Errore ricezione risposta dal server. errno %d,%s",errno, strerror(errno));
            return -1;
        }

    }else{
        print("Errore invio richiesta di scrittura al server. errno %d,%s",errno, strerror(errno));
        return -1;
    }

    /* Il server è pieno, deve inviarmi il file meno recente */
    if(isServerFull){

        void **buf1 = NULL;
        size_t size;
        waitServerFile(buf1,&size);

        /* If the dirname path is not null save the ejected file from the server */
        if(dirname != NULL){
            char *path = malloc(sizeof(dirname)+sizeof("ejected.txt")+1);
            strcat(path, dirname);
            strcat(path, "ejected.txt");
            saveFile(path, buf1, size);
        }

    }
    return 0;


}

int appendToFile(const char* pathname, void *buf, size_t size, const char* dirname){

    msleep(waitingTime);

    Request request;
    Response response;
    void *buf2;
//    char absPath[PATH_MAX];

    if (pathname == NULL || buf == NULL) return -1;

//    realpath(pathname, absPath);

    request.operation = OP_APPEND_FILE;
    strncpy(request.filepath, pathname, PATH_MAX);
    request.clientId = getpid();
    request.fileSize = size;

    print("Invio richiesta scrittura in append per file: %s con dimensione %zu\n",request.filepath,request.fileSize);

    /* Invio richiesta al server */
    if ( write(fd_socket, &request, sizeof(Request)) != -1){

        /* Attendo risposta dal server */
        if ( read(fd_socket, &response, sizeof(Response)) != -1){

            printServerResponse(&response);

            //Il server è pieno, mi invierà il file che è stato espulso
            if(response.statusCode == 1){

                print("Il server sta per inviare il file che ha espulso\n");
                buf2 = malloc(response.fileSize);
                if (read(fd_socket,buf2,response.fileSize) != -1)
                    print("Ricevuto file espulso dal server\n");

                if(dirname != NULL){ //save the file on disk
                    saveFileDir(buf2, response.fileSize, dirname,response.fileName);
                }
                free(buf2);

                //Ri-effettuo una read per leggere il file effettivo che il server mi deve inviare
                read(fd_socket, &response, sizeof(Response));

            }

            if(response.statusCode == -1)
                return -1;

            print("Invio i dati per append file %s al server\n",pathname);
            /* Invio al server il file! */
            if ( write(fd_socket,buf,request.fileSize) != -1){

                print("File inviato correttamente al server!\n");

                /* Attendo risposta dal server con messaggio di successo */
                if( read(fd_socket, &response, sizeof(Response)) ){

                    printServerResponse(&response);
                    return response.statusCode;

                }else{
                    print("Errore ricezione file al server\n");
                    return -1;
                }

            }else{

                print("Errore invio file al server\n");
                return -1;
            }

        }else{
            print("Errore ricezione risposta dal server. errno %d,%s",errno, strerror(errno));
            return -1;
        }

    }else{
        print("Errore invio richiesta di scrittura append al server. errno %d,%s",errno, strerror(errno));
        return -1;
    }

}

int lockFile(const char* pathname){

    msleep(waitingTime);

    Request request;
    Response response;
//    char absPath[PATH_MAX];

    if(pathname == NULL) return -1;

//    realpath(pathname, absPath);

    request.operation = OP_LOCK_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    print("Invio richiesta di lock per file %s\n", pathname);

    return sendRequest(&request,&response);

}

int unlockFile(const char* pathname){

    msleep(waitingTime);

    Request request;
    Response response;
//    char absPath[PATH_MAX];

    if(pathname == NULL) return -1;

//    realpath(pathname, absPath);

    request.operation = OP_UNLOCK_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    print("Invio richiesta di unlock per file %s\n", pathname);

    return sendRequest(&request,&response);
}

int removeFile(const char* pathname){

    msleep(waitingTime);

    Request request;
    Response response;
//    char absPath[PATH_MAX];

    if(pathname == NULL) return -1;

//    realpath(pathname, absPath);

    request.operation = OP_DELETE_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    print("Invio richiesta di cancellazione per file %s\n", pathname);

    return sendRequest(&request, &response);
}

int closeFile(const char* pathname){

    msleep(waitingTime);

    Request request;
    Response response;
//    char absPath[PATH_MAX];

    if(pathname == NULL) return -1;

//    realpath(pathname, absPath);

    request.operation = OP_CLOSE_FILE;
    strcpy(request.filepath, pathname);
    request.clientId = getpid();

    print("Invio richiesta di chiusura per file %s\n", pathname);

    return sendRequest(&request, &response);

}

int readNFiles(int N, const char *dirname){

    msleep(waitingTime);

    void *buff;
    size_t size;
    char *token;

    Request request;
    Response response;

    if(dirname == NULL){
        print("Dirname non può essere nullo\n");
        return -1;
    }

    request.operation = OP_READ_N_FILES;
//    strncpy(request.filepath, pathname,MAX_PATH_SIZE);
    request.clientId = getpid();
    request.flags = N; /* Uso il campo flag per comunicare al server quanti file voglio */

    print("Invio richiesta lettura di %d files\n",N);

    if ( write(fd_socket,&request,sizeof(Request)) != -1 ){

        if ( read(fd_socket,&response,sizeof(Response)) != -1 ){

            printServerResponse(&response);
            if(response.statusCode == -1)
                return -1;

            /* Nella variabile statusCode ci sono il numero di file che il server effettivamente sta per inviare */
            int comingFiles = response.statusCode;
            print("Il server sta per inviare %d files\n", comingFiles);

            /* Dentro message c'è la lista di file che il server può inviare */
            token = strtok(response.message,",");

            /* Per ogni file nella lista faccio una read al server */
            while( token != NULL){

                print("Richiedo al server il file %s\n",token);

                if( readFile(token,&buff,&size) != -1 ) {
                    saveFileDir(buff, size, dirname, token);
                }

                token = strtok(NULL, ",");
            }

        }else{

            print("Errore read socket, errno: %d, %s\n",errno, strerror(errno));
            return -1;
        }


        }else{

        print("Errore write socket, errno: %d, %s\n",errno, strerror(errno));
        return -1;

    }

    return 0;

    }

int sendRequest(Request *request, Response *response){

    /* Invio richiesta al server */
    if ( write(fd_socket,request,sizeof(Request)) != -1 ){

        /* Attendo risposta dal server */
        if ( read(fd_socket,response,sizeof(Response)) != -1 ){

            printServerResponse(response);
            return 0;

        }else{
            print("Errore write socket, errno: %d\n",errno);
            return -1;
        }

    }else{
        print("Errore write socket, errno: %d\n",errno);
        return -1;
    }

}

