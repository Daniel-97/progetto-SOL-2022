//
// Created by daniele on 17/11/21.
//

#ifndef PROGETTO_SOL_2022_SERVERAPI_H
#define PROGETTO_SOL_2022_SERVERAPI_H

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

#include "../../common/common.h"
#include "utils.h"

int fd_socket;  // Variabile per socket

int openConnection(const char* sockname, int msec,const struct timespec abstime);

int closeConnection(const char* sockname);

int openFile(const char* pathname, int flags);

int readFile(const char* pathname, void** buf, size_t* size);

#endif //PROGETTO_SOL_2022_SERVERAPI_H
