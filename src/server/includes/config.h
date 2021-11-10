//
// Created by daniele on 09/11/21.
//

#ifndef PROGETTO_SOL_2022_CONFIG_H
#define PROGETTO_SOL_2022_CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *fp;

typedef struct Config{
    char socket_path[100];
    unsigned tread_workers;
    unsigned max_mem_size; /* Size in MB */
    unsigned max_file;
}Config;

void print();

int readConfig(struct Config* serverConfig);

void printConfig(Config* config);

#endif //PROGETTO_SOL_2022_CONFIG_H
