//
// Created by daniele on 12/11/21.
//

#include "utils.h"

void help(){
    printf("***** ARGOMENTI CLIENT ******\n");
    printf("-h\n");
    printf("-f filename\n");
    printf("-w dirname[,n=0]\n");
    printf("-W file1[,file2]\n");
    printf("-D dirname\n");
    printf("-r file1[,file2]\n");
    printf("-R [n=0]\n");
    printf("-d dirname\n");
    printf("-t time\n");
    printf("-l file1[,file2]\n");
    printf("-u file1[,file2]\n");
    printf("-c file1[,file2]\n");
    printf("-p\n");
}

void printServerResponse(Response *response){

    printf("Server response: {STATUS_CODE: %d, MESSAGE: %s, FILE_SIZE: %zu }\n",response->statusCode, response->message,response->fileSize);

}

int saveFile(const char* pathname, void *buf, size_t size){

    FILE *file = fopen(pathname, "w");

    if (file == NULL) return -1;

    if ( fwrite(buf,size,1,file) > 0 ){
        printf("File %s salvato correttamente su disco\n",pathname);
        fclose(file);
        return 0;
    }
    else{
        printf("Errore salvataggio file %s su disco\n",pathname);
        fclose(file);
        return -1;
    }

}

int saveFileDir(void *buf, size_t size, const char* dirname){
    return 0;
}