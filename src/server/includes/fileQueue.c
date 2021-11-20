//
// Created by daniele on 16/11/21.
//

#include "fileQueue.h"

//Attenzione non dovrebbe ritornare un puntatore perchè la modifica non è thread safe
int findFile(Queue *queue,const char *pathname){

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

int insertFile(Queue *queue, FileNode *fileNode, FileNode **removedFile){

    int result;

    if( queue == NULL || fileNode == NULL) return -1;

    /* La coda ha raggiunto il massimo numero di file, devo estrarre quello piu vecchio (FIFO) */
    if(queue->len >= serverConfig->max_file){
        printf("La coda ha raggiunto il limite max di file, estrazione del meno recente...\n");
        *removedFile = pop(fileQueue);
    }

    result = push(queue, fileNode);
    if (result != 0)
        printf("Inserimento file %s sulla coda riuscito!\n",fileNode->pathname);

    return result;
}

int editFile(Queue *queue, const char *pathname, FILE *file,int size, int clientId){

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
        if(file) {
            fileNode->file = file;
            fileNode->size = size;
        }
        if(clientId != 0)
            fileNode->client_id = clientId;
        status = 0;

    }else{
        status = -1;
    }

    pthread_cond_signal(&queue->qcond);
    pthread_mutex_unlock(&queue->qlock);

    return status;

}


