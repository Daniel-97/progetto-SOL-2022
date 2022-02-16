//
// Created by daniele on 16/02/22.
//

#include "logger.h"

void loggerInit(){

    printf("\n***** INIZIALIZZAZIONE LOGGER ****\n");
    printf("Log file: %s\n",serverConfig->log_file);

    if( (logFile = fopen(serverConfig->log_file, "w")) == NULL){
        printf("ERRORE APERTURA FILE DI LOG\n");
    }

    fclose(logFile);

//    Request r;
//    r.fileSize = 100;
//    r.operation = 1;
//    r.clientId = 4535353;
//    strcpy(r.filepath,"prova.txt");
//    logRequest(r, NULL);

}
/* FORMATO LOG
 * DATA-ORA,ID-CLIENT, OPERAZIONE, FILE-TARGET, BYTE-LETTI, BYTE-SCRITTI, FILE-RIMPIAZZATO
 */
void logRequest(Request request,int readByte, int writeByte, char *replacedFile){

    char s[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(s, sizeof(s)-1,"%d/%m/%Y-%H:%M:%S",t);

    if( (logFile = fopen(serverConfig->log_file, "a")) != NULL ){

        fprintf(logFile, "%s,%d,%d,%s,%d,%d",s,request.clientId,request.operation,request.filepath,readByte,writeByte);
        if(replacedFile != NULL)
            fprintf(logFile,",%s\n", replacedFile);
        else
            fprintf(logFile,"%s","\n");

        fclose(logFile);

    }

}