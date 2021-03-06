//
// Created by daniele on 11/01/22.
//

#include "includes/controller.h"

void open_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();
    int status;

    logRequest(*request,0,0,NULL);

    safeMutexLock(&file_queue_mutex);

    safeMutexLock(&file_lock_mutex);
    while ( (status = openVirtualFile(fileQueue,request->filepath, request->flags, request->clientId)) == -2) {
        /* We need to wait on the lock for the file but release the mutex on the queue */
        printf("[%lu] Waiting for other client to release lock on file\n",self);
        safeMutexUnlock(&file_queue_mutex);
        pthread_cond_wait(&file_lock_cond, &file_lock_mutex);
    }
    safeMutexUnlock(&file_lock_mutex);

    if(status == 0){
        /* Preparo la risposta per il client */
        response->statusCode = 0;
        strcpy(response->message, "File opened!");

    }else{
        response->statusCode = -1;
        strcpy(response->message, "Impossibile aprire il file!");
    }

    printf("[%lu] Invio risposta al client con fd: %d\n", self,*fd_client_skt);
    if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
        printf("[%lu] Risposta inviata al client!\n", self);
    }

    safeFree(response);

    safeMutexUnlock(&file_queue_mutex);

}

void append_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();
    void *buf;
    size_t size;
    FileNode *data;
    int freeSpace;

    safeMutexLock(&file_queue_mutex);

    freeSpace = getFreeSpace(fileQueue);

    /* C'è bisogno di espellere un file per capacity miss. Informo il client di questa cosa */
    if(freeSpace < request->fileSize){

        printf("[%lu] No space left, need to expel a file\n",self);
        data = expelFile(fileQueue, request->fileSize);

        /* Impossibile espellere file. */
        if(data == NULL){ }

        //1 è lo status code
        sendBufferFileToClient(*fd_client_skt, data, 1);

        logRequest(*request, 0,request->fileSize,data->pathname);
        if( removeNode(fileQueue, data) != -1 ){
//            free(data); leave comment
        }

    }else{

        logRequest(*request,0, request->fileSize, NULL);
    }

    /* Controllo prima che il client abbia il lock sul file */
    if (hasFileLock(fileQueue,request->filepath,request->clientId) == 0){
        strcpy(response->message, "Ready to receive file, client has lock");
        response->statusCode = 0;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {

            size = request->fileSize;
            printf("[%lu] Il client sta per inviare un file di %zu byte\n",self,request->fileSize);
            buf = allocateMemory(1, size); //Alloco il buffer per la ricezione del file

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

    safeMutexUnlock(&file_queue_mutex);

}

void read_file_controller(int *fd_client_skt, Request *request){

    int rbyte = 0;
    safeMutexLock(&file_queue_mutex);
    if ((rbyte = sendFileToClient(*fd_client_skt, request->filepath,0)) == 0)
        logRequest(*request, rbyte,0,NULL);

    safeMutexUnlock(&file_queue_mutex);


}

void delete_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();

    logRequest(*request, 0,0,NULL);

    safeMutexLock(&file_queue_mutex);

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

    safeFree(response);

    safeMutexUnlock(&file_queue_mutex);

}

void write_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();
    FileNode *data;
    void *buf;
    int freeSpace;
    int rbyte;

    safeMutexLock(&file_queue_mutex);

    freeSpace = getFreeSpace(fileQueue);

    if( request->fileSize > serverConfig.max_mem_size ){
        strcpy(response->message, "The file is too big for the server");
        response->statusCode = -1;
        write(*fd_client_skt, response, sizeof(Response));
        safeFree(response);
        return;
    }

    /* C'è bisogno di espellere un file per capacity miss. Informo il client di questa cosa */
    if(freeSpace < request->fileSize){

        printf("[%lu] No space left, need to expel a file\n",self);
        data = expelFile(fileQueue, request->fileSize);

        /* Impossibile espellere file. */
        if(data == NULL){ }

        //1 è lo status code
        sendBufferFileToClient(*fd_client_skt, data, 1);

        logRequest(*request, 0,request->fileSize,data->pathname);
        if( removeNode(fileQueue, data) != -1 )
            incNumFileReplacement();


    }else
        logRequest(*request,0, request->fileSize, NULL);

    /* Controllo prima che il client abbia il lock sul file */
    if (hasFileLock(fileQueue,request->filepath,request->clientId) == 0){

        strcpy(response->message, "Ready to receive file, client has lock");
        response->statusCode = 0;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {

            printf("[%lu] Il client sta per inviare un file di %zu byte\n",self,request->fileSize);
            buf = allocateMemory(1, request->fileSize); //Alloco il buffer per la ricezione del file

            rbyte = readData(*fd_client_skt, buf, request->fileSize);

            if( rbyte != -1 ){
                printf("[%lu] File %s ricevuto correttamente! rbyte:%d\n",self,request->filepath,rbyte);
//                                    printf("buffer ricevuto: %s, dim: %zu\n",(char*)buf,size);

                if ( writeVirtualFile(fileQueue,request->filepath,buf,request->fileSize) != -1){
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

            safeFree(buf);
        }

    }else{

        strcpy(response->message, "The client doesnt have lock, aborting");
        response->statusCode = -1;

        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
            printf("[%lu] Risposta inviata al client!\n", self);
        }

    }

    safeFree(response);

    safeMutexUnlock(&file_queue_mutex);

}

void close_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();

    logRequest(*request,0,0, NULL);

    safeMutexLock(&file_queue_mutex);

    if(closeVirtualFile(fileQueue,request->filepath, request->clientId) == 0){
        response->statusCode = 0;
        strcpy(response->message, "File chiuso correttamente!");
    }else{

        response->statusCode = 0;
        strcpy(response->message, "Errore chiusura file");

    }

    if (write(*fd_client_skt, response, sizeof(Response)) != -1)
        printf("[%lu] Risposta inviata al client!\n",self);

    safeMutexUnlock(&file_queue_mutex);

}

void lock_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();
    int status;

    logRequest(*request,0,0, NULL);

    safeMutexLock(&file_queue_mutex);

    /* Tento di acquisire il lock sul file */
    safeMutexLock(&file_lock_mutex);
    while((status = lockVirtualFile(fileQueue,request->filepath,request->clientId)) == -2) {

        /* We need to wait on the lock for the file but release the mutex on the queue */
        printf("[%lu] Waiting for other client to release lock on file\n",self);
        safeMutexUnlock(&file_queue_mutex);
        pthread_cond_wait(&file_lock_cond, &file_lock_mutex);

    }
    safeMutexUnlock(&file_lock_mutex);

    if(status == 0){ //Ok lock acquisito
        response->statusCode = 0;
        strcpy(response->message, "Lock correttamente acquisito sul file!");

    }
    else{ //Impossibile acquisire lcok
        response->statusCode = -1;
        strcpy(response->message,"Impossible acquisire il lock sul file");
    }

    if (write(*fd_client_skt, response, sizeof(Response)) != -1)
        printf("[%lu] Risposta inviata al client!\n",self);


    safeFree(response);

    safeMutexUnlock(&file_queue_mutex);

}

void unlock_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();

    logRequest(*request,0,0, NULL);

    safeMutexLock(&file_queue_mutex);

    /* Tentativo di unlock sul file */
    if(unlockVirtualFile(fileQueue,request->filepath,request->clientId) == 0){
        response->statusCode = 0;
        strcpy(response->message, "Unlock eseguito con successo!");

        /* Wake up all the worker waiting for the mutex on the file */
        safeMutexLock(&file_lock_mutex);
        pthread_cond_broadcast(&file_lock_cond);
        safeMutexUnlock(&file_lock_mutex);

    }else{
        response->statusCode = -1;
        strcpy(response->message,"Impossible effettuare unlock");
    }
    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }

    safeFree(response);

    safeMutexUnlock(&file_queue_mutex);

}

void readn_file_controller(int *fd_client_skt, Request *request){

    Response *response = allocateMemory(1, sizeof(Response));
    pthread_t self = pthread_self();
    char *fileList;
    size_t size;

    logRequest(*request,0,0,NULL);

    safeMutexLock(&file_queue_mutex);

    /* The flag in the request contains the number of requested files */
    fileList = getNFileList(fileQueue, &size, request->flags);
    printf("File list size: %ld\n", size);
    response->statusCode = size;
    strcpy(response->message, "Pronto per inviare files");

    if(size == 0) {
        response->statusCode = -1;
        strcpy(response->message, "0 file da leggere");
        if (write(*fd_client_skt, response, sizeof(Response)) != -1) {
            printf("[%lu] Risposta inviata al client!\n", self);
            return;
        }
    }

    strcpy(response->message, fileList);

    if (write(*fd_client_skt, response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client!\n",self);
    }

    safeFree(fileList);
    safeFree(response);
    safeMutexUnlock(&file_queue_mutex);

}

