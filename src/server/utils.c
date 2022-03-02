//
// Created by daniele on 11/01/22.
//

#include "includes/utils.h"
#include "includes/globals.h"

void sendFileToClient(int fd_client_skt, const char* pathname, int statusCode){

    void *buf;
    size_t size;
    pthread_t self = pthread_self();
    Response *response = allocateMemory(1, sizeof(Response));

    /* Leggo il file */
    if ( readVirtualFile(fileQueue,pathname,&buf,&size) == 0) {
        response->statusCode = statusCode;
        response->fileSize = size;
        strcpy(response->message, "Ready to send file");
        strcpy(response->fileName, pathname);
//                            printf("buffer letto: %s\n",(char*)buf);
        /* Invio al client la dimensione del file che sta per leggere */
        if (write(fd_client_skt, response, sizeof(Response)) != -1){
            printf("[%lu] Risposta inviata al client con dimensione file!\n",self);

            /* Invio al client il file effettivo */
            if (write(fd_client_skt, buf, size) != -1){
                printf("[%lu] File %s inviato correttamente!\n",self, pathname);
            }else{
                printf("[%lu] Errore invio file %s,%s \n",self, pathname, strerror(errno));

            }

        }
    }else{ /* In caso di errore invio il messaggio di errore al client */

        response->statusCode = -1;
        strcpy(response->message,"Errore, impossibile leggere il file");

        if (write(fd_client_skt, response, sizeof(Response)) != -1){
            printf("[%lu] Risposta inviata al client!\n",self);
        }
    }

    free(response);
    if(buf != NULL)
        free(buf);

}

void sendBufferFileToClient(int fd_client_skt, FileNode *file, int statusCode){

    pthread_t self = pthread_self();

    Response response;
    response.statusCode = statusCode;
    strcpy(response.message, "Ready to send file");
    strcpy(response.fileName, file->pathname);
    response.fileSize = file->size;

    /* Invio al client la dimensione del file che sta per leggere */
    if (write(fd_client_skt, &response, sizeof(Response)) != -1){
        printf("[%lu] Risposta inviata al client con dimensione file!\n",self);

        /* Invio al client il file effettivo */
        if (write(fd_client_skt, file->file, file->size) != -1){
            printf("[%lu] File %s inviato correttamente!\n",self, file->pathname);
        }else{
            printf("[%lu] Errore invio file %s,%s \n",self, file->pathname, strerror(errno));

        }

    }
}

int getFreeSpace(Queue *queue){

    Node *node = queue->head;
    FileNode *fileNode;

    int size = 0;
    int available = 0;

    pthread_mutex_lock(&queue->qlock);

    while( (node = node->next) != NULL){

        fileNode = node->data;
        size += fileNode->size;

    }

    pthread_mutex_unlock(&queue->qlock);

    available = serverConfig.max_mem_size - size;
//    printf("Current available space: %d bytes\n",available);

    return available;

}

FileNode* expelFile(Queue  *queue, int requiredSpace){

    Node *node = queue->head;
    FileNode *fileNode;
    FileNode *expelFile;
    int freeSpace = getFreeSpace(queue);

    pthread_mutex_lock(&queue->qlock);

    while( (node = node->next) != NULL){

        fileNode = node->data;
        if( freeSpace+fileNode->size >= requiredSpace) {
            expelFile = fileNode;
            break;
        }

    }

    pthread_mutex_unlock(&queue->qlock);

    return expelFile;

}

/* Funzioni per tenere traccia del numero di connessioni concorrenti */
void addConnectionCont(){
    pthread_mutex_lock(&mutex_n_connections);
    n_connections += 1;
    pthread_mutex_unlock(&mutex_n_connections);
}

void subConnectionCont(){
    pthread_mutex_lock(&mutex_n_connections);
    n_connections -= 1;
    pthread_mutex_unlock(&mutex_n_connections);
}

int getNumConnections(){
    int n;
    pthread_mutex_lock(&mutex_n_connections);
    n = n_connections;
    pthread_mutex_unlock(&mutex_n_connections);

    return n;
}

void *allocateMemory(size_t n, size_t size){
    void *mem;
    mem = calloc(n, size);
    if( mem == NULL){
        perror("Impossibile allocare memoria (calloc)\n");
        exit(EXIT_FAILURE);
    }
    return mem;
}



