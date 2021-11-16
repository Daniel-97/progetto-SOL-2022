//
// Created by daniele on 11/11/21.
//

#ifndef PROGETTO_SOL_2022_QUEUE_H
#define PROGETTO_SOL_2022_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

typedef struct Node{
    void *data; //Socket file descriptor
    struct Node *next;
}Node;

typedef struct Queue {

    Node *head;
    Node *tail;
    unsigned long len;
    pthread_mutex_t qlock; //Mutex per la coda
    pthread_cond_t qcond; // Conditional variable per la coda

}Queue;

/* Funzione che inizializza la coda delle connessioni */
Queue* initQueue();

/* Funzione che inserisce i file descriptor nella coda */
int push(Queue *q,void *data);

/* Funzione che toglie un elemento dalla coda */
void *pop(Queue *q);

#endif //PROGETTO_SOL_2022_QUEUE_H