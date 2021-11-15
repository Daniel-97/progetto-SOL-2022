//
// Created by daniele on 15/11/21.
//

#ifndef PROGETTO_SOL_2022_GLOBALS_H
#define PROGETTO_SOL_2022_GLOBALS_H

#include "config.h"
#include "connectionQueue.h"

static Config *serverConfig;
static ConnectionQueue *queue;

typedef struct Request{

    int operation;

}Request;

typedef struct Response{

    int statusCode;

}Response;

#endif //PROGETTO_SOL_2022_GLOBALS_H
