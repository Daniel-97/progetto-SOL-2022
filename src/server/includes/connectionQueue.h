//
// Created by daniele on 11/11/21.
//

#ifndef PROGETTO_SOL_2022_CONNECTIONQUEUE_H
#define PROGETTO_SOL_2022_CONNECTIONQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

typedef struct Node{
    int sfd; //Socket file descriptor
    struct Node *next;
}Node;

typedef struct ConnectionQueue {

    Node *head;
    Node *tail;
    unsigned long len;
    pthread_mutex_t qlock; //Mutex per la coda
    pthread_cond_t qcond; // Conditional variable per la coda

}ConnectionQueue;

/* Funzione che inizializza la coda delle connessioni */
ConnectionQueue* initQueue();

/* Funzione che inserisce i file descriptor nella coda */
int push(ConnectionQueue *q, int sfd);

/* Funzione che toglie un elemento dalla coda */
int pop(ConnectionQueue *q);

#endif //PROGETTO_SOL_2022_CONNECTIONQUEUE_H
