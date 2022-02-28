//
// Created by daniele on 15/11/21.
//

#ifndef PROGETTO_SOL_2022_GLOBALS_H
#define PROGETTO_SOL_2022_GLOBALS_H

#include "config.h"
#include "queue.h"
#include <stdbool.h>

#define MAX_THREAD 20

pthread_t threadPool[MAX_THREAD];
Config serverConfig;
Queue *connectionQueue; /* Coda per le connessioni in arrivo */
Queue *fileQueue; /* Coda per i file dei client */

pthread_mutex_t mutex_n_connections;
int n_connections;

bool acceptNewConnection;
bool closeServer;

#endif //PROGETTO_SOL_2022_GLOBALS_H
