//
// Created by daniele on 16/02/22.
//

#ifndef PROGETTO_SOL_2022_LOGGER_H
#define PROGETTO_SOL_2022_LOGGER_H

#include "config.h"
#include "globals.h"
#include "../../common/common.h"

FILE *logFile;

void loggerInit();
void logRequest(Request request,int readByte, int writeByte, char *replacedFile);

#endif //PROGETTO_SOL_2022_LOGGER_H
