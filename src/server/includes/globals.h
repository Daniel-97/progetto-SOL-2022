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

/* Mutex and cond for connection queue */
pthread_mutex_t connection_queue_mutex;
pthread_cond_t connection_queue_cond;

/* Mutex and cond for file queue */
pthread_mutex_t file_queue_mutex;
pthread_cond_t  file_queue_cond;

/* Mutex and cond for lock file */
pthread_mutex_t file_lock_mutex;
pthread_cond_t  file_lock_cond;

pthread_mutex_t mutex_n_connections;
int n_connections;

bool acceptNewConnection;
bool closeServer;

#endif //PROGETTO_SOL_2022_GLOBALS_H
