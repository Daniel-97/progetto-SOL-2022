//
// Created by daniele on 15/11/21.
//

#ifndef PROGETTO_SOL_2022_GLOBALS_H
#define PROGETTO_SOL_2022_GLOBALS_H

#include "config.h"
#include "queue.h"

static Config *serverConfig;
static Queue *connectionQueue; /* Coda per le connessioni in arrivo */
static Queue *fileQueue; /* Coda per i file dei client */

typedef struct Request{

    int operation;

}Request;

typedef struct Response{

    int statusCode;

}Response;

#endif //PROGETTO_SOL_2022_GLOBALS_H
