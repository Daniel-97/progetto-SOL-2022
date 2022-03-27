//
// Created by daniele on 11/01/22.
//

#ifndef PROGETTO_SOL_2022_CONTROLLER_H
#define PROGETTO_SOL_2022_CONTROLLER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "fileStorage.h"
#include "utils.h"
#include "config.h"
#include "queue.h"
#include "globals.h"
#include "logger.h"
#include "statistic.h"

void open_file_controller(int *fd_client_skt, Request *request);

void append_file_controller(int *fd_client_skt, Request *request);

void read_file_controller(int *fd_client_skt, Request *request);

void delete_file_controller(int *fd_client_skt, Request *request);

void write_file_controller(int *fd_client_skt, Request *request);

void close_file_controller(int *fd_client_skt, Request *request);

void lock_file_controller(int *fd_client_skt, Request *request);

void unlock_file_controller(int *fd_client_skt, Request *request);

void readn_file_controller(int *fd_client_skt, Request *request);

#endif //PROGETTO_SOL_2022_CONTROLLER_H
