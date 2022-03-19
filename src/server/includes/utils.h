//
// Created by daniele on 11/01/22.
//

#ifndef PROGETTO_SOL_2022_UTILS_H
#define PROGETTO_SOL_2022_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "fileStorage.h"
#include "globals.h"

void sendFileToClient(int fd_client_skt, const char* pathname, int statusCode);

void sendBufferFileToClient(int fd_client_skt, FileNode *file, int statusCode);

int getFreeSpace(Queue *queue);

FileNode* expelFile(Queue  *queue, int requiredSpace);

void addConnectionCont();

void subConnectionCont();

int getNumConnections();

void *allocateMemory(size_t n, size_t size);

void safeFree(void *pointer);

#endif //PROGETTO_SOL_2022_UTILS_H
