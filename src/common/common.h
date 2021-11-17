//
// Created by daniele on 17/11/21.
//

#ifndef PROGETTO_SOL_2022_COMMON_H
#define PROGETTO_SOL_2022_COMMON_H

#define OP_OPEN_FILE    0
#define OP_WRITE_FILE   1
#define OP_READ_FILE    2
#define OP_DELETE_FILE  3
#define OP_CLOSE_FILE   4
#define OP_LOCK_FILE    5
#define OP_UNLOCK_FILE  6

typedef struct Request{

    int  operation;
    char filepath[100];

}Request;

typedef struct Response{

    int statusCode;
    int success;
    char message[100];

}Response;

#endif //PROGETTO_SOL_2022_COMMON_H
