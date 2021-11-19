//
// Created by daniele on 17/11/21.
//

#include "fileStorage.h"

int openVirtualFile(const char* pathname, int flags, int clientId){

    pthread_t self = pthread_self();
    FileNode *file;
    FILE *newFile;

    file = findFile(fileQueue, pathname);

    /* Esiste il file ed è stato passato il flag O_CREATE */
    if ( file && ( (flags & O_CREATE) == O_CREATE) ){
        printf("[%lu] Il file %s esiste gia, impossibile crearne uno nuovo con lo stesso nome\n",self, pathname);
        return -1;
    }

    if( !file && ( (flags & O_LOCK) == O_LOCK ) ){
        printf("[%lu] Il file %s non esiste, impossibile acquisire il lock\n",self, pathname);
        return -1;
    }

    if (file){

        printf("[%lu] Il file %s esiste gia!\n",self, pathname);

        /* Se il client ha passato il flag di lock devo prendere il lock sul file */
        if ( (flags & O_LOCK) == O_LOCK ){

        }else{

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
            if( push(fileQueue, newNode) ){

                printf("[%lu] Nuovo file inserito nella coda!\n",self);
                return 0;

            }else{

                printf("[%lu] Errore inserimento file sulla coda\n",self);
                return -1;

            }
        }

    }


    /* Il file non esiste, lo creo */
//    if (file == NULL){
//
//    }else{
//
//    }

    return 0;

}

int writeVirtualFile(const char* pathname){

    FileNode *file = NULL;

    file = findFile(fileQueue,pathname);

    if (file == NULL){
        return -1;
    }
    else{
        return 0;
    }

}
