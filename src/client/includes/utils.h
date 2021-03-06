//
// Created by daniele on 12/11/21.
//

#ifndef PROGETTO_SOL_2022_UTILS_H
#define PROGETTO_SOL_2022_UTILS_H

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../../common/common.h"
#include "serverApi.h"
#include <limits.h>
#include <stdarg.h>

void help();

void printServerResponse(Response *response);

void print(char *string,...);

/* Salva il file su disco */
int saveFile(const char* pathname, void *buf, size_t size);

/* Salva il file nella directory specificata */
int saveFileDir(void *buf, size_t size,const char* dirname, const char* fileName);

int waitServerFile(void** buf, size_t* size);

char* getFileListFromDir(const char* dirname, int maxFile);

const char* getFileNameFromPath(const char* path);

void *allocateMemory(size_t n, size_t size);

void msleep(long millis);

int readData(int fd, void *buf, size_t size);

#endif //PROGETTO_SOL_2022_UTILS_H
