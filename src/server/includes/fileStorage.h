//
// Created by daniele on 17/11/21.
//

#ifndef PROGETTO_SOL_2022_FILESTORAGE_H
#define PROGETTO_SOL_2022_FILESTORAGE_H
#define _POSIX_C_SOURCE 200809L //Necessaria per utilizzare funzione posix fmemopen

#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
#include "config.h"
#include "../../common/common.h"

#define READ_CHUNK_SIZE 100

typedef struct FileNode{

    char    pathname[MAX_PATH_SIZE];
    FILE   *file;   /* Puntatore al file */
    int     size;   /* Dimensione in byte del file memorizzato */
    int     client_id; /* Se il file e' in lock da un client contiene l'identificatore del client */

}FileNode;

/***** FUNZIONI SU CODA *****/

/* Cerca il file all interno della coda dei file */
int findFileNode(Queue *queue, const char *pathname);

int editFileNode(Queue *queue, const char *pathname, FILE *file,int size, int clientId);

int removeNode(Queue *queue, FileNode *node);

FileNode *getFileNode(Queue *queue, const char *pathname);


/***** FUNZIONI SU FILE ******/
int openVirtualFile(Queue *queue,const char* pathname, int flags, int clientId);

int readVirtualFile(Queue *queue,const char* pathname, void **buf, size_t *size);

int writeVirtualFile(Queue *queue, const char* pathname, void *buf, size_t size);

int lockVirtualFile(Queue *queue, const char* pathname, int clientId);

int unlockVirtualFile(Queue *queue, const char* pathname, int clientId);

int deleteVirtualFile(Queue *queue, const char* pathname, int clientId);

int hasFileLock(Queue *queue, const char *pathname, int clientId);




#endif //PROGETTO_SOL_2022_FILESTORAGE_H
