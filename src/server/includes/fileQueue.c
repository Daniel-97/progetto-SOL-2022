//
// Created by daniele on 16/11/21.
//

#include "fileQueue.h"

FileNode *findFile(Queue *fileQueue,const char *pathname){

    Node *node;
    FileNode *fileNode;
    FileNode *data = NULL;

    if( fileQueue == NULL || pathname == NULL ) return NULL;

    pthread_mutex_lock(&fileQueue->qlock);

    node = fileQueue->head;

    while( (node = node->next) != NULL){
        fileNode = node->data;
        if(strcmp(fileNode->pathname, pathname) == 0){
            data = fileNode;
            break;
        }
    }

    pthread_cond_signal(&fileQueue->qcond);
    pthread_mutex_unlock(&fileQueue->qlock);

    return data;

}

int insertFile(Queue *fileQueue, FileNode *fileNode, FileNode **removedFile){

    int result;

    if( fileQueue == NULL || fileNode == NULL) return -1;

    /* La coda ha raggiunto il massimo numero di file, devo estrarre quello piu vecchio (FIFO) */
    if(fileQueue->len >= serverConfig->max_file){
        printf("La coda ha raggiunto il limite max di file, estrazione del meno recente...\n");
        *removedFile = pop(fileQueue);
    }

    result = push(fileQueue, fileNode);
    if (result != 0)
        printf("Inserimento file %s sulla coda riuscito!\n",fileNode->pathname);

    return result;
}

//int deleteFile(Queue *fileQueue, FileNode *fileNode){
//
//}


