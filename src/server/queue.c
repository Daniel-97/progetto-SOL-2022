//
// Created by daniele on 11/11/21.
//

#include "includes/queue.h"
#include "includes/utils.h"

Queue* initQueue(){

    Queue *q = allocateMemory(1, sizeof(Queue)); //Alloco spazio per coda

    if(!q) return NULL;

    q->head = malloc(sizeof(Node));
    q->head->data = NULL;
    q->head->next = NULL;

    q->tail = q->head;
    q->len = 0;

    return q;
}

int push(Queue *q, void *data){

    if( (q == NULL) || (data == NULL) )
        return -1;

    Node *n = allocateMemory(1, sizeof(Node));
    if(!n) return -1;
    n->data = data;
    n->next = NULL;

    q->tail->next = n;
    q->tail = n;
    q->len++;

    return(q->len);

}

void *pop(Queue *q){

    void *data;

    if(q == NULL){
        printf("[POP] queue pointer is null!\n");
        return (void *)-1;
    }

    Node *n = q->head->next;
    data = n->data;
    q->head->next = n->next;

    if(q->head->next == NULL)
        q->tail = q->head;

    q->len--;

    //Libero la memoria del nodo rimosso
    safeFree(n);

    return data;
}

void deleteQueue(Queue *queue){

    Node *tmp;

    while( queue->head != NULL){

        tmp = queue->head;
        queue->head = queue->head->next;
        safeFree(tmp);

    }

    safeFree(queue);

}