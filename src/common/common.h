//
// Created by daniele on 17/11/21.
//

#ifndef PROGETTO_SOL_2022_COMMON_H
#define PROGETTO_SOL_2022_COMMON_H

#include <stdint.h>
#include <limits.h>

/* CODICI PER COMUNICARE TIPO DI OPERAZIONE AL SERVER */
#define OP_OPEN_FILE    0
#define OP_WRITE_FILE   1
#define OP_READ_FILE    2
#define OP_DELETE_FILE  3
#define OP_APPEND_FILE  4
#define OP_CLOSE_FILE   5
#define OP_LOCK_FILE    6
#define OP_UNLOCK_FILE  7
#define OP_READ_N_FILES 8

/* CODICI FLAGS FILE BINARIO */
//#define O_CREATE    0b01
//#define O_LOCK      0b10

/* CODICI FLAGS FILE ESADECIMALE */
#define O_CREATE    0x01
#define O_LOCK      0x02

//#define MAX_PATH_SIZE       100
#define MAX_MESSAGE_SIZE    100

typedef struct Request{

    int     clientId;                   /* Id del client che effettua la richiesta */
    int     operation;                  /* Tipo di operazione: crea, elimina, scrivi, leggi, lock,... */
    char    filepath[PATH_MAX];    /* Path del file su cui eseguire l'operazione */
    int     flags;                      /* Eventuali flags: O_CREAT, O_LOCK */
    size_t fileSize;                    /* Eventuale dimensione del file che il client sta inviando al server */

}Request;

typedef struct Response{

    int     statusCode; /* 0 -> ok, -1 -> error */
    size_t  fileSize; /* Dimensione del file, usata quando il server deve inviare dati al client */
    char    message[MAX_MESSAGE_SIZE];
    char    fileName[PATH_MAX];

}Response;

#endif //PROGETTO_SOL_2022_COMMON_H
