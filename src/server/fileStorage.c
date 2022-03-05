//
// Created by daniele on 17/11/21.
//

#include "includes/fileStorage.h"
#include "includes/globals.h"
#include "includes/utils.h"

int findFileNode(Queue *queue,const char *pathname){

    Node *node;
    FileNode *fileNode;
    FileNode *data = NULL;

    if( queue == NULL || pathname == NULL ) return -1;

    pthread_mutex_lock(&queue->qlock);

    node = queue->head;

    while( (node = node->next) != NULL){
        fileNode = node->data;
        if(strcmp(fileNode->pathname, pathname) == 0){
            data = fileNode;
            break;
        }
    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    if(data)
        return 0;
    else
        return -1;
}

//ATTENZIONE MUTEX DA ESEGUIRE FUORI DA FUNZIONE
int editFileNode(Queue *queue, const char *pathname, char *file,int size, int clientId){

    Node *node = NULL;
    FileNode *fileNode;
    int status;

    if (queue == NULL) return -1;

    pthread_mutex_lock(&queue->qlock);

    node = queue->head;

    while( (node = node->next) != NULL){
        fileNode = node->data;
        if(strcmp(fileNode->pathname, pathname) == 0){
            break;
        }
    }

    if(node){

        /* ATTENZIONE DEVO LIBERARE LA MEMORIA */
        if (file) {
            fileNode->file = file;
            fileNode->size = size;
        }
        if (clientId != 0)
            fileNode->client_id = clientId;
        status = 0;

    }else{
        status = -1;
    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return status;

}

//ATTENZIONE MUTEX DA ESEGUIRE FUORI DA FUNZIONE
FileNode *getFileNode(Queue *queue, const char *pathname){

    Node *node;
    FileNode *fileNode;
    FileNode *data = NULL;

    if( queue == NULL || pathname == NULL ) return NULL;

//    pthread_mutex_lock(&queue->qlock);

    node = queue->head;

    while( (node = node->next) != NULL){
        fileNode = node->data;
        if(strcmp(fileNode->pathname, pathname) == 0){
            data = fileNode;
            break;
        }
    }

//    pthread_cond_signal(&queue->qcond);
//    pthread_mutex_unlock(&queue->qlock);

    if(data)
        return data;
    else
        return NULL;
}

//ATTENZIONE MUTEX DA ESEGUIRE FUORI DA FUNZIONE
FileNode *getFirstNode(Queue *queue){

    Node *node;
    node = queue->head;

    if(node != NULL)
        return node->data;

    return NULL;
}

char* getFileList(Queue *queue){

    Node *tmp;
    FileNode *fileNode;
    char *fileList = NULL;
    char *s;

    pthread_mutex_lock(&queue->qlock);
    tmp = queue->head;

    while( (tmp = tmp->next) != NULL ){

        fileNode = tmp->data;
        if(fileList == NULL){
            fileList = allocateMemory(1,strlen(fileNode->pathname)+1);
            strncpy(fileList, fileNode->pathname, strlen(fileNode->pathname)+1);
        }
        else {
            s = allocateMemory(1,strlen(fileList) + strlen(fileNode->pathname) + 2);
            strcpy(s, fileList);
            strcat(s, ",");
            strcat(s, fileNode->pathname);
            free(fileList);
            fileList = allocateMemory(1, strlen(s) + 1);
            strncpy(fileList, s, strlen(s) + 1);
            free(s);
        }
    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return fileList;

}

int getStorageSize(Queue *queue) {

    Node *tmp;
    FileNode *fileNode;
    int size = 0;

    pthread_mutex_lock(&queue->qlock);
    tmp = queue->head;

    while ((tmp = tmp->next) != NULL) {
        fileNode = tmp->data;
        size = size + fileNode->size;
    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return size;
}


int removeNode(Queue *queue, FileNode *node){

    Node *tmp;
    Node *tmp2;
    Node *prec;

    if (queue == NULL || node == NULL) return -1;

    tmp = queue->head;
    prec = queue->head;

    while( (tmp = tmp->next) != NULL){

        if(tmp->data == node){
            /* Se è l ultimo elemento aggiorno il puntatore di coda*/
            if(queue->tail == tmp)
                queue->tail = prec;

            tmp2 = prec->next;
            prec->next = tmp->next;
            free(tmp2);
            free(node->file);
//            free(node);
            queue->len--;
            return 0;
        }
        prec = tmp;
    }

    return -1;

}

int openVirtualFile(Queue *queue, const char* pathname, int flags, int clientId){

    pthread_t self = pthread_self();
    int found;

    found = findFileNode(queue, pathname);

    /* Esiste il file ed è stato passato il flag O_CREATE */
    if ( (found !=  -1 ) && ( (flags & O_CREATE) == O_CREATE) ){

        printf("[%lu] Il file %s esiste gia, impossibile crearne uno nuovo con lo stesso nome\n",self, pathname);
        return -1;

    }

    if( (found == -1) && ( (flags & O_LOCK) == O_LOCK && (flags & O_CREATE) != O_CREATE) ){

        printf("[%lu] Il file %s non esiste, impossibile aprire\n",self, pathname);
        return -1;

    }

    if (found == 0){

        printf("[%lu] Il file %s esiste gia!\n",self, pathname);

        /* Se il client ha passato il flag di lock devo prendere il lock sul file */
        if ( (flags & O_LOCK) == O_LOCK ) {

            if (editFileNode(queue, pathname, NULL, 0, clientId) == 0) {
                printf("[%lu] Lock acquisito sul file!\n", self);

                return 0;
            }
        }

    }else{

        printf("[%lu] Il file NON ESISTE, tento di crearlo...\n",self);

        if( queue->len == serverConfig.max_file ){
            printf("[%lu] Raggiunta capacità massima storage, impossibile aggiungere nuovi file\n",self);
            return -1;
        }

        FileNode *newNode = allocateMemory(1, sizeof(FileNode));

        printf("[%lu] File %s creato correttamente!\n",self,pathname);
        strcpy(newNode->pathname, pathname);
        newNode->file = NULL;
        newNode->size = 0;
        if ( (flags & O_LOCK) == O_LOCK)
            newNode->client_id = clientId;
        else
            newNode->client_id = 0;
        newNode->isOpen = 1;

        /* Inserisco il nuovo file creato nella coda */
        if( push(queue, newNode) ){

            printf("[%lu] Nuovo file inserito nella coda!\n",self);
            int max_file = getMaxNumFile();
            if(fileQueue->len > max_file)
                setMaxNumFile(fileQueue->len);

            return 0;

        }else{

            printf("[%lu] Errore inserimento file sulla coda\n",self);
            return -1;

        }

    }

    return 0;

}

int readVirtualFile(Queue *queue, const char* pathname, void **buf, size_t *size){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);

    file = getFileNode(queue, pathname);

    if( file == NULL ){

       printf("[%lu] Il file %s non esiste, impossibile leggere\n",self,pathname);
       status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è chiuso, impossibile leggere\n",self,pathname);
            signalQueue(queue);
            return -1;
        }

        printf("[%lu] Tento di leggere file %s dim: %zu byte\n",self,pathname,file->size);
        *buf = malloc(file->size); //Alloco spazio buffer file da leggere

        if(*buf) {

            memcpy(*buf, file->file, file->size);
            *size = file->size;
            printf("[%lu] Letti %zu byte da file %s\n", self, file->size, pathname);
            status = 0;

        }else{

            printf("[%lu] Impossibile allocare buffer per lettura file %s\n", self, pathname);
            status = -1;

        }

    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return status;

}

// todo: Attenzione da testare! (non sono sicuro che funzioni bene
int writeVirtualFile(Queue *queue, const char* pathname, void *buf, size_t size){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);
    file = getFileNode(queue, pathname);

    if(file == NULL){

        printf("[%lu] Il file %s non esiste, impossibile scrivere\n",self,pathname);
        status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è chiuso, impossibile leggere\n",self,pathname);
            signalQueue(queue);
            return -1;
        }

        /* Devo liberare la memoria */
        if (file->file != NULL){

            free(file->file);
        }

        file->file = allocateMemory(1, size);

        if (file->file) {

            memcpy(file->file, buf, size); //Copio il contenuto del file nel buffer
            file->size = size;
            printf("[%lu] File scritto correttamente in buffer!\n",self);

            status = 0;

        }else{

            printf("[%lu] Errore malloc buffer per file!\n",self);
            status = -1;

        }

    }

    signalQueue(queue);

    int max_size = getMaxFileStorageSize();
    int actual_size = getStorageSize(queue);
    if(actual_size > max_size)
        setMaxFileStorageSize(actual_size);

    return status;

}

//todo aggiungere ritorno di file elimianto se coda piena
int appendVirtualFile(Queue *queue, const char* pathname, void *buf, size_t size){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);
    file = getFileNode(queue, pathname);

    if(file == NULL){

        printf("[%lu] Il file %s non esiste, impossibile scrivere\n",self,pathname);
        status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è chiuso, impossibile leggere\n",self,pathname);
            signalQueue(queue);
            return -1;
        }

        /* LA WRITE LAVORA SEMPRE IN APPEND */

        if(file->file == NULL) { //Prima scrittura

            file->file = malloc(size);

            if (file->file) {

                memcpy(file->file, buf, size); //Copio il contenuto del file nel buffer
                file->size = size;
                printf("[%lu] File scritto correttamente in buffer!\n",self);
                status = 0;

            }else{

                printf("[%lu] Errore malloc buffer per file!\n",self);
                status = -1;

            }

        }else{ //Il buffer contiene già dei dati

            char *tmp = malloc(file->size+size); //Alloco un buffer con la nuova dimensione

            if (tmp) {

                memcpy(tmp, file->file, file->size); //Copio il vecchio contenuto del file nel buffer
                memcpy((tmp + size), buf, size); //Copio in append il nuovo contenuto del file
                file->size += size;
                free(file->file);
                file->file = tmp;
                printf("[%lu] File scritto correttamente in buffer!\n",self);
                status = 0;

            }else{
                printf("[%lu] Errore malloc buffer per file!\n",self);
                status = -1;
            }

        }

    }
    signalQueue(queue);

    int max_size = getMaxFileStorageSize();
    int actual_size = getStorageSize(queue);
    if(actual_size > max_size)
        setMaxFileStorageSize(actual_size);

    return status;

}

int lockVirtualFile(Queue *queue, const char* pathname, int clientId){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);

    file = getFileNode(queue,pathname);

    if (file == NULL){

        printf("[%lu] File %s non esistente, impossibile acquisire il lock\n",self,pathname);
        status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è chiuso, impossibile leggere\n",self,pathname);
            signalQueue(queue);
            return -1;
        }

        if(file->client_id == clientId){

            printf("[%lu] Lock già acquisito sul file %s\n",self,pathname);
            status = 0;

        }else if (file->client_id == 0){

            file->client_id = clientId; //Acquisisco il lock sul file
            printf("[%lu] Lock acquisito sul file %s\n",self,pathname);
            status = 0;

        }else{

            printf("[%lu] File %s in lock da un altro client\n",self,pathname);
            status = -1;

        }

    }

    signalQueue(queue);

    return status;
}

int unlockVirtualFile(Queue *queue, const char* pathname, int clientId){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);

    file = getFileNode(queue,pathname);

    if (file == NULL){

        printf("[%lu] File %s non esistente, unlock impossibile\n",self,pathname);
        status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è chiuso, impossibile leggere\n",self,pathname);
            signalQueue(queue);
            return -1;
        }

        if(file->client_id == clientId){

            file->client_id = 0; //Tolgo il lock sul file
            printf("[%lu] Unlock sul file eseguito correttamente %s\n",self,pathname);
            status = 0;

        }else if (file->client_id == 0){

            printf("[%lu] Il file %s non è in lock\n",self,pathname);
            status = 0;

        }else{

            printf("[%lu] File %s in lock da un altro client, impossibile togliere lock\n",self,pathname);
            status = -1;

        }

    }

    signalQueue(queue);

    return status;
}

int deleteVirtualFile(Queue *queue, const char* pathname, int clientId){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);

    file = getFileNode(queue,pathname);

    if (file == NULL){

        printf("[%lu] File %s non esistente, impossibile eliminare\n",self,pathname);
        status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è chiuso, impossibile leggere\n",self,pathname);
            signalQueue(queue);
            return -1;
        }

        if(file->client_id == clientId){
//            fclose(file->file); //Chiudo il file
            if ( removeNode(queue,file) == 0 ){
                printf("[%lu] File eliminato correttamente %s\n",self,pathname);
                status = 0;
            }else{
                printf("[%lu] Errore eliminazione file %s\n",self,pathname);
                status = -1;
            }


        }else if (file->client_id == 0){

            printf("[%lu] Il file %s non è in lock, impossbile eliminare\n",self,pathname);
            status = -1;

        }else{

            printf("[%lu] File %s in lock da un altro client, impossibile eliminare\n",self,pathname);
            status = -1;

        }

    }

    signalQueue(queue);

    return status;
}

int closeVirtualFile(Queue *queue, const char* pathname, int clientId){


    pthread_t self = pthread_self();
    FileNode *file;
    int status;

    pthread_mutex_lock(&queue->qlock);

    file = getFileNode(queue,pathname);

    if (file == NULL){

        printf("[%lu] File %s non esistente, impossibile chiudere\n",self,pathname);
        status = -1;

    }else{

        if (!file->isOpen){
            printf("[%lu] Il file %s è già chiuso\n",self,pathname);
            signalQueue(queue);
            return 0;
        }

        if(file->client_id == clientId){

            file->isOpen = 0;
            printf("[%lu] File %s chiuso\n",self,pathname);

        }else if (file->client_id == 0){

            printf("[%lu] Il file %s non è in lock, impossbile chiudere\n",self,pathname);
            status = -1;

        }else{

            printf("[%lu] File %s in lock da un altro client, impossibile chiudere\n",self,pathname);
            status = -1;

        }

    }

    signalQueue(queue);

    return status;

}


int hasFileLock(Queue *queue, const char *pathname, int clientId){

    pthread_t self = pthread_self();
    FileNode *fileNode;
    int status;

    pthread_mutex_lock(&queue->qlock);

    fileNode = getFileNode(queue,pathname);

    if (fileNode == NULL){
        printf("[%lu] File %s non esistente, lock non acquisito\n",self,pathname);
        status = -1;
    }else{

        if(fileNode->client_id == clientId){
            printf("[%lu] Il client %d ha il lock sul file %s\n", self, clientId, pathname);
            status = 0;
        }else{
            printf("[%lu] Il client %d NON ha il lock sul file %s\n", self, clientId, pathname);
            status = -1;
        }
    }

    signalQueue(queue);

    return status;

}

char* getNFileList(Queue *queue, size_t *size, int N){

//    pthread_t self = pthread_self();
    FileNode *fileNode;
    Node *node;
    int cont = 0;
    char *fileList = NULL;
    char *tmp = NULL;
    int s = 0;

    if (queue == NULL) return NULL;

    pthread_mutex_lock(&queue->qlock);
//    printf("FILE LIST\n");

    if( N == -1 || N > queue->len) N = queue->len;
    *size = N;

    // Alloco un array con la stessa dimensione della mia coda
//    *files = malloc(N * sizeof(char *));

    node = queue->head;
    while( cont < N && ((node = node->next) != NULL)){

        fileNode = node->data;
        //Alloco lo spazio per il nome del file dentro l array
//        *files[cont] = malloc(strlen(fileNode->pathname));
//        strcpy(*files[cont], fileNode->pathname); //Copio il nome del file nell array
        printf("- %s\n", fileNode->pathname);

        /* concat file list */

        if(fileList != NULL) {
            s = strlen(fileList)+1+strlen(fileNode->pathname);
            tmp = malloc(s);
            strcpy(tmp, fileList);
            strcat(tmp, ",");
            strcat(tmp, fileNode->pathname);
            free(fileList);
            fileList = malloc(s);
            strcpy(fileList, tmp);
            free(tmp);
        }else{
            fileList = malloc(strlen(fileNode->pathname));
            strcpy(fileList, fileNode->pathname);
        }

        cont++;

    }

    signalQueue(queue);

    printf("CONCAT FILE LIST %s\n", fileList);
    return fileList;

}

void deleteFileQueue(Queue *queue){

    Node *tmp;
    FileNode *fileNode;

    pthread_mutex_lock(&queue->qlock);

    while( queue->head != NULL){

        tmp = queue->head;

        fileNode = tmp->data;
        if(fileNode != NULL && fileNode->file != NULL)
            free(fileNode->file);
        free(fileNode);

        queue->head = queue->head->next;
        free(tmp);

    }

    pthread_mutex_unlock(&queue->qlock);

    free(queue);
}
