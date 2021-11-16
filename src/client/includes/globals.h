//
// Created by daniele on 15/11/21.
//

#ifndef PROGETTO_SOL_2022_GLOBALS_H
#define PROGETTO_SOL_2022_GLOBALS_H

typedef struct Request{

    int operation;
    char filepath[100];

}Request;

typedef struct Response{

    int statusCode;
    int success;
    char message[100];

}Response;

#endif //PROGETTO_SOL_2022_GLOBALS_H
