//
// Created by daniele on 15/11/21.
//

#ifndef PROGETTO_SOL_2022_GLOBALS_H
#define PROGETTO_SOL_2022_GLOBALS_H

#include "config.h"
#include "queue.h"

Config *serverConfig;
Queue *connectionQueue; /* Coda per le connessioni in arrivo */
Queue *fileQueue; /* Coda per i file dei client */

pthread_mutex_t mutex_n_connections;
int n_connections;

#endif //PROGETTO_SOL_2022_GLOBALS_H
