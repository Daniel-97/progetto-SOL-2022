//
// Created by daniele on 17/11/21.
//

#ifndef PROGETTO_SOL_2022_FILESTORAGE_H
#define PROGETTO_SOL_2022_FILESTORAGE_H
#define _POSIX_C_SOURCE 200809L //Necessaria per utilizzare funzione posix fmemopen

#include <stdio.h>
#include <stdlib.h>

#include "fileQueue.h"
#include "config.h"

int openVirtualFile(const char* pathname, int flags, int clientId);

int writeVirtualFile(const char* pathname);


#endif //PROGETTO_SOL_2022_FILESTORAGE_H
