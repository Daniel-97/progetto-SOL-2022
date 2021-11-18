//
// Created by daniele on 17/11/21.
//

#include "fileStorage.h"

int openVirtualFile(const char* pathname, int flags){

    FileNode *file;
    FILE *newFile;

    file = findFile(fileQueue, pathname);

    newFile = fmemopen(NULL,10,"r");
    if(newFile == NULL){
        printf("Errore nella creazione del file\n");
    }

    /* Il file non esiste, lo creo */
    if (file == NULL){

    }else{

    }

}

int writeVirtualFile(const char* pathname){

    FileNode file = NULL;

    file = findFile(fileQueue,pathname);

    if (file == NULL){

    }

}
