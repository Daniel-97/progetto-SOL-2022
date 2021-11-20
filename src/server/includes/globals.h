//
// Created by daniele on 15/11/21.
//

#ifndef PROGETTO_SOL_2022_GLOBALS_H
#define PROGETTO_SOL_2022_GLOBALS_H

#include "config.h"
#include "queue.h"

Config *serverConfig;
//int epoll_descriptor; /* File descriptor epoll */
Queue *fileQueue; /* Coda per i file dei client */
Queue *connectionQueue; /* Coda per le connessioni in arrivo */

#endif //PROGETTO_SOL_2022_GLOBALS_H
