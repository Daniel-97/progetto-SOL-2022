//
// Created by daniele on 12/11/21.
//

#ifndef PROGETTO_SOL_2022_UTILS_H
#define PROGETTO_SOL_2022_UTILS_H

#include <stdio.h>
#include "../../common/common.h"

void help();

void printServerResponse(Response *response);

/* Salva il file su disco */
int saveFile(const char* pathname, void *buf, size_t size);

/* Salva il file nella directory specificata */
int saveFileDir(void *buf, size_t size, const char* dirname);

#endif //PROGETTO_SOL_2022_UTILS_H
