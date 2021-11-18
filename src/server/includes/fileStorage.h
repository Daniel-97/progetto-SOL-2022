//
// Created by daniele on 17/11/21.
//

#ifndef PROGETTO_SOL_2022_FILESTORAGE_H
#define PROGETTO_SOL_2022_FILESTORAGE_H

#include "fileQueue.h"
#include "config.h"

int openVirtualFile(const char* pathname, int flags);

int writeVirtualFile(const char* pathname);


#endif //PROGETTO_SOL_2022_FILESTORAGE_H
