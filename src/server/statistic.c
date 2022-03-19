//
// Created by daniele on 21/02/22.
//

#include "includes/statistic.h"
#include "includes/utils.h"

void statInit(){
    max_num_file = 0;
    max_file_storage_size = 0;
    num_file_replacement = 0;
}

void printStat(Queue *queue){

    pthread_mutex_lock(&mutex_stats);
    printf("\n***** SERVER STATS *****\n");
    printf("- Num file max: %d\n",max_num_file);
    printf("- Max file storage: %f MB\n",(float )max_file_storage_size/1000000);
    printf("- Num file replacement: %d\n",num_file_replacement);
    pthread_mutex_unlock(&mutex_stats);

    printf("*** SERVER FILE LIST ***\n");

    char *fileList = getFileList(queue);

    if(fileList != NULL) {
        char *token = strtok(fileList, ",");
        while (token != NULL) {
            printf("- %s\n", token);
            token = strtok(NULL, ",");
        }
    }

    safeFree(fileList);

}

void setMaxNumFile(int n){

    pthread_mutex_lock(&mutex_stats);
    max_num_file = n;
    pthread_mutex_unlock(&mutex_stats);

}

int getMaxNumFile(){
    int n;
    pthread_mutex_lock(&mutex_stats);
    n = max_num_file;
    pthread_mutex_unlock(&mutex_stats);
    return n;
}

void setMaxFileStorageSize(int n){
    pthread_mutex_lock(&mutex_stats);
    max_file_storage_size = n;
    pthread_mutex_unlock(&mutex_stats);
}
int getMaxFileStorageSize(){
    int n;
    pthread_mutex_lock(&mutex_stats);
    n = max_file_storage_size;
    pthread_mutex_unlock(&mutex_stats);
    return n;
}

void incNumFileReplacement(){
    pthread_mutex_lock(&mutex_stats);
    num_file_replacement = num_file_replacement + 1;
    pthread_mutex_unlock(&mutex_stats);
}
int getNumFileReplacement(){
    int n;
    pthread_mutex_lock(&mutex_stats);
    n = num_file_replacement;
    pthread_mutex_unlock(&mutex_stats);
    return n;
}
