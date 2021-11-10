#include <stdlib.h>
#include <stdio.h>
#include <config.h>

static Config* serverConfig;

int main(int argc, char *argv[]){

    /* CONFIG SECTION INIT*/
    serverConfig = malloc(sizeof(Config));
    int res = readConfig(serverConfig);
    if(res == -1){
        exit(-1);
    }
    printConfig(serverConfig);


    return 0;
}