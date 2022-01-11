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

void sendFileToClient(int fd_client_skt, const char* pathname);

#endif //PROGETTO_SOL_2022_UTILS_H
