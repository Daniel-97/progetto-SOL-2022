//
// Created by daniele on 11/01/22.
//

#include "controller.h"

void open_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();

    if (openVirtualFile(fileQueue,request->filepath, request->flags, request->clientId) == 0) {
        /* Preparo la risposta per il client */
        response->statusCode = 0;
        strcpy(response->message, "File opened!");

    }else{
        response->statusCode = -1;
        strcpy(response->message, "Impossibile aprire il file!");
    }

    printf("[%lu] Invio risposta al client...\n", self);
    if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
        printf("[%lu] Risposta inviata al client!\n", self);
    }

}

void append_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();
    void *buf;
    size_t size;

    /* Controllo prima che il client abbia il lock sul file */
    if (hasFileLock(fileQueue,request->filepath,request->clientId) == 0){
        strcpy(response->message, "Ready to receive file, client has lock");
        response->statusCode = 0;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {

            size = request->fileSize;
            printf("[%lu] Il client sta per inviare un file di %zu byte\n",self,request->fileSize);
            buf = malloc(size); //Alloco il buffer per la ricezione del file

            if( read(*fd_client_skt,buf, size) != -1 ){
                printf("[%lu] File %s ricevuto correttamente!\n",self,request->filepath);
//                                    printf("buffer ricevuto: %s, dim: %zu\n",(char*)buf,size);

                if ( appendVirtualFile(fileQueue,request->filepath,buf,size) != -1){
                    response->statusCode = 0;
                    strcpy(response->message, "File scritto correttamente!");
                }else{
                    response->statusCode = -1;
                    strcpy(response->message, "Errore scrittura file");
                }

                printf("[%lu] Invio risposta al client...\n", self);
                if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
                    printf("[%lu] Risposta inviata al client!\n", self);
                }

            }else{
                printf("[%lu] Errore ricezione file %s da client\n",self,request->filepath);
            }
        }

    }else{

        strcpy(response->message, "The client doesnt have lock, aborting");
        response->statusCode = -1;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
            printf("[%lu] Risposta inviata al client!\n", self);
        }

    }

}

void read_file_controller(int *fd_client_skt, Request *request){

    sendFileToClient(*fd_client_skt, request->filepath,0);

}

void delete_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();

    if(deleteVirtualFile(fileQueue,request->filepath, request->clientId) == 0){
        response->statusCode = 0;
        strcpy(response->message, "File eliminato correttamente!");
    }else{
        response->statusCode = 0;
        strcpy(response->message, "Errore eliminazione file");
    }

    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }

}

void write_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();
    int serverIsFull = 0;
    FileNode *data;
    void *buf;
    size_t size;
    int freeSpace;

    /* C'è bisogno di espellere un file per capacity miss. Informo il client di questa cosa */
    freeSpace = getFreeSpace(fileQueue);

    //Todo capire se questa parte di free space funziona
    if(freeSpace < request->fileSize){

        printf("[%lu] No space left, need to expel a file\n",self);
        data = expelFile(fileQueue, request->fileSize);

        /* Impossibile espellere file. */
        if(data == NULL){ }

        //1 è lo status code
        sendFileToClient(*fd_client_skt, data->pathname,1);

        removeNode(fileQueue, data);

    }

    /* Controllo prima che il client abbia il lock sul file */
    if (hasFileLock(fileQueue,request->filepath,request->clientId) == 0){
        strcpy(response->message, "Ready to receive file, client has lock");
        response->statusCode = 0;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {

            size = request->fileSize;
            printf("[%lu] Il client sta per inviare un file di %zu byte\n",self,request->fileSize);
            buf = malloc(size); //Alloco il buffer per la ricezione del file

            if( read(*fd_client_skt,buf, size) != -1 ){
                printf("[%lu] File %s ricevuto correttamente!\n",self,request->filepath);
//                                    printf("buffer ricevuto: %s, dim: %zu\n",(char*)buf,size);

                if ( writeVirtualFile(fileQueue,request->filepath,buf,size) != -1){
                    response->statusCode = 0;
                    strcpy(response->message, "File scritto correttamente!");
                }else{
                    response->statusCode = -1;
                    strcpy(response->message, "Errore scrittura file");
                }

                printf("[%lu] Invio risposta al client...\n", self);
                if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
                    printf("[%lu] Risposta inviata al client!\n", self);
                }

            }else{
                printf("[%lu] Errore ricezione file %s da client\n",self,request->filepath);
            }
        }

    }else{

        strcpy(response->message, "The client doesnt have lock, aborting");
        response->statusCode = -1;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
            printf("[%lu] Risposta inviata al client!\n", self);
        }

    }

}

void close_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();

    if(closeVirtualFile(fileQueue,request->filepath, request->clientId) == 0){
        response->statusCode = 0;
        strcpy(response->message, "File chiuso correttamente!");
    }else{

        response->statusCode = 0;
        strcpy(response->message, "Errore chiusura file");

    }

    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }

}

void lock_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();

    /* Tento di acquisire il lock sul file */
    if(lockVirtualFile(fileQueue,request->filepath,request->clientId) == 0){
        response->statusCode = 0;
        strcpy(response->message, "Lock correttamente acquisito sul file!");
    }else{
        response->statusCode = -1;
        strcpy(response->message,"Impossible acquisire il lock sul file");
    }
    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }
}

void unlock_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();

    /* Tentativo di unlock sul file */
    if(unlockVirtualFile(fileQueue,request->filepath,request->clientId) == 0){
        response->statusCode = 0;
        strcpy(response->message, "Unlock eseguito con successo!");
    }else{
        response->statusCode = -1;
        strcpy(response->message,"Impossible effettuare unlock");
    }
    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }

}

void readn_file_controller(int *fd_client_skt, Request *request){

    Response *response = malloc(sizeof(Response));
    pthread_t self = pthread_self();
    char **arr = NULL;
    char *fileList;
    size_t size;

    fileList = getFileList(fileQueue, &arr, &size);
    printf("File size %ld\n", size);
    response->statusCode = size;
    strcpy(response->message, "Pronto per inviare files");

    if(size == 0)
        if (write(*fd_client_skt, response, sizeof(Response)) != -1){
            printf("[%lu] Risposta inviata al client!\n",self);
            return;
        }

    strcpy(response->message, fileList);

    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }
    //todo capire se far inviare i file qui oppure tramite la readFIle

    /* Send files */
//                        for(int i = 0; i < size; i++){
//                            printf("Sending file %s to client\n", arr[i]);
//                            sendFileToClient(*fd_client_skt, arr[i]);
//                        }

}

