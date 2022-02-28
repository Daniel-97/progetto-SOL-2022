//
// Created by daniele on 21/02/22.
//

#ifndef PROGETTO_SOL_2022_STATISTIC_H
#define PROGETTO_SOL_2022_STATISTIC_H

#include <pthread.h>
#include "queue.h"
#include "fileStorage.h"

pthread_mutex_t mutex_stats;
int max_num_file;
int max_file_storage_size;
int num_file_replacement;

void statInit();
void printStat(Queue *queue);

void setMaxNumFile(int n);
int getMaxNumFile();

void setMaxFileStorageSize(int n);
int getMaxFileStorageSize();

void incNumFileReplacement();
int getNumFileReplacement();

#endif //PROGETTO_SOL_2022_STATISTIC_H
