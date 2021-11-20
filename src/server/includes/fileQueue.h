//
// Created by daniele on 16/11/21.
//

#ifndef PROGETTO_SOL_2022_FILEQUEUE_H
#define PROGETTO_SOL_2022_FILEQUEUE_H

#include "queue.h"
#include "globals.h"
#include "../../common/common.h"
#include <string.h>

typedef struct FileNode{

    char    pathname[MAX_PATH_SIZE];
    FILE   *file;   /* Puntatore al file */
    int     size;   /* Dimensione in byte del file memorizzato */
    int     client_id; /* Se il file e' in lock da un client contiene l'identificatore del client */

}FileNode;

/*  Inserisce il file all interno della coda.
 *  Se la coda e' piena elimina il file meno recente e lo ritorna in removedFile
 */
int insertFile(Queue *queue, FileNode *fileNode, FileNode **removedFile);

/* Cerca il file all interno della coda dei file (senza eliminarlo) */
int findFile(Queue *queue, const char *pathname);

/* Modifica un nodo della coda */
int editFile(Queue *queue, const char *pathname, FILE *file,int size, int clientId);

/* Elimina il file dalla coda (liberando lo spazio in memoria) */
//int deleteFile(Queue *fileQueue, FileNode *fileNode);

#endif //PROGETTO_SOL_2022_FILEQUEUE_H