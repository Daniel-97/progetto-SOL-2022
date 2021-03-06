//
// Created by daniele on 09/11/21.
//

#include "includes/config.h"

int readConfig(struct Config* serverConfig){

    char buf[1000]; /*buffer for read line */
    char key[50];
    char value[50];
    char *token;
    int i;

    if( (fp = fopen("./config/config.txt","r")) != NULL ){

        while( !feof(fp) ){

            fgets(buf,1000, fp);
            i = 0;
            token = strtok(buf,":");

            while(token){

                if(i == 0)
                    strcpy(key, token);
                else if (i == 1)
                    strcpy(value, token);

                token = strtok(NULL, ":");
                i++;
            }

            /* Copia il valore fino al new line (non copia il new line nel setting)*/
            if(strcmp(key,"socket-path") == 0)
                strncpy(serverConfig->socket_path,value, strcspn(value,"\n"));

            else if(strcmp(key,"thread-workers") == 0)
                serverConfig->thread_workers = strtol(value, NULL,10);

            else if(strcmp(key,"max-mem-size") == 0)
                serverConfig->max_mem_size = strtol(value, NULL,10);

            else if(strcmp(key,"max-file") == 0)
                serverConfig->max_file = strtol(value, NULL,10);

            else if(strcmp(key, "log-file") == 0)
                strncpy(serverConfig->log_file, value, strcspn(value,"\n"));

            else
                printf("Setting not recognized: %s",key);

        }

        fclose(fp);
        return 0;

    }else{
        printf("Cant open config file!\n");
        return -1;
    }
}

void printConfig(Config* config){
    printf("****** SERVER CONFIG *****\n");
    printf("- socket-path: %s\n",config->socket_path);
    printf("- thread-workers: %d\n",config->thread_workers);
    printf("- max-mem-size: %d\n",config->max_mem_size);
    printf("- max-file: %d\n",config->max_file);
}
