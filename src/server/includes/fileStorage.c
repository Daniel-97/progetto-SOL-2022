//
// Created by daniele on 17/11/21.
//

#include "fileStorage.h"

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
int editFileNode(Queue *queue, const char *pathname, FILE *file,int size, int clientId){

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

int openVirtualFile(Queue *queue, const char* pathname, int flags, int clientId){

    pthread_t self = pthread_self();
    int found;
    FILE *newFile;

    found = findFileNode(queue, pathname);

    /* Esiste il file ed è stato passato il flag O_CREATE */
    if ( (found !=  -1 ) && ( (flags & O_CREATE) == O_CREATE) ){
        printf("[%lu] Il file %s esiste gia, impossibile crearne uno nuovo con lo stesso nome\n",self, pathname);
        return -1;
    }

    if( (found == -1) && ( (flags & O_LOCK) == O_LOCK ) ){
        printf("[%lu] Il file %s non esiste, impossibile acquisire il lock\n",self, pathname);
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

        newFile = fmemopen(NULL,10,"a+");

        if(newFile == NULL){

            printf("[%lu] Errore nella creazione del file\n",self);
            return -1;

        }else{

            printf("[%lu] File %s creato correttamente!\n",self,pathname);
            FileNode *newNode = malloc(sizeof(FileNode));
            strcpy(newNode->pathname, pathname);
            newNode->file = newFile;
            newNode->size = 0;
            if ( (flags & O_LOCK) == O_LOCK)
                newNode->client_id = clientId;
            else
                newNode->client_id = 0;

            /* Inserisco il nuovo file creato nella coda */
            if( push(queue, newNode) ){

                printf("[%lu] Nuovo file inserito nella coda!\n",self);
                return 0;

            }else{

                printf("[%lu] Errore inserimento file sulla coda\n",self);
                return -1;

            }
        }

    }

    return 0;

}

int readVirtualFile(Queue *queue, const char* pathname, void **buf, size_t *size){

    pthread_t self = pthread_self();
    FileNode *file;
    int cont;
    int status;

    pthread_mutex_lock(&queue->qlock);

    file = getFileNode(queue, pathname);

    if( file == NULL ){

       printf("[%lu] Il file %s non esiste, impossibile leggere\n",self,pathname);
       status = -1;

    }else{
        printf("[%lu] Tento di leggere file %s dim: %d byte\n",self,pathname,file->size);
       *buf = malloc(file->size); //Alloco spazio buffer file da leggere
       cont = fread(*buf,file->size,1,file->file); //Ritorna elementi letti

        if(cont == 1){
            printf("[%lu] Letti %d byte da file %s\n", self, file->size, pathname);
            *size = file->size;
            status = 0;
        }else{
           printf("[%lu] Errore lettura file %s. %s\n", self, pathname, strerror(errno));
           status = -1;
        }

    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return status;

}

int writeVirtualFile(Queue *queue, const char* pathname, void *buf, size_t size){

    pthread_t self = pthread_self();
    FileNode *file;
    int status;
    int cont;

    pthread_mutex_lock(&queue->qlock);
    file = getFileNode(queue, pathname);

    if(file == NULL){
        printf("[%lu] Il file %s non esiste, impossibile scrivere\n",self,pathname);
        status = -1;
    }else{
        //Inserire qui codice per scrivere su file
        cont = fwrite(buf,size,1,file->file);
        if (cont == 1){
            printf("[%lu] Scritti %zu byte su file %s\n",self,size,pathname);
            file->size = size;
            status = 0;
        }else{
            printf("[%lu] Errore scrittura dati su file %s", self, pathname);
            status = -1;
        }
    }
    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return status;

}
