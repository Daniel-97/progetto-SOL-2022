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

/* Funzione unica per attesa di un file in arrivo dal server */
int waitServerFile(void** buf, size_t* size){

    Response response;

    /* Attendo risposta dal server con dimensione del file che sto per ricevere */
    if ( read(fd_socket,&response,sizeof(Response)) != -1 ){

        printServerResponse(&response);

        if(response.statusCode == -1)
            return -1;

        /* Mi salvo la dimensione del file */
        *size = response.fileSize;
        *buf = malloc(response.fileSize); //Alloco il buffer per la lettura del file
        // Leggo effettivamente il file dal server
        if (read(fd_socket,*buf,*size) != -1){

            printf("File ricevuto correttamente!\n");
            return 0;

        }else{

            printf("Errore ricezione file. errno: %d. %s", errno, strerror(errno));
            return -1;

        }


    }else{
        printf("Errore read socket, errno: %d, %s\n",errno, strerror(errno));
        return -1;
    }

}

char* getFileListFromDir(const char* dirname){

    DIR *d = opendir(dirname);
    struct dirent* file;
    struct stat file_stat;
    char *fileList = NULL;
    char *fileName = NULL;
    char *tmp = NULL;
    int size;

    /* Add the '/' */
    if(strcmp(dirname,"/") < 0){
        tmp = malloc(strlen(dirname)+1);
        strcpy(tmp, dirname);
        strcat(tmp, "/");
        dirname = tmp;
    }

    while( (file = readdir(d)) != NULL){

        if(strcmp(file->d_name,"..") == 0 || strcmp(file->d_name,".") == 0)
            continue;

        tmp = malloc(strlen(dirname)+strlen(file->d_name));
        strcpy(tmp, dirname);
        strcat(tmp, file->d_name);

        lstat(tmp, &file_stat);

        if(S_ISDIR(file_stat.st_mode)){
            fileName = getFileListFromDir(tmp);
        }else{
            fileName = file->d_name;
        }

        free(tmp);

        if (fileList == NULL) {
            fileList = malloc(strlen(fileName));
            strcpy(fileList, fileName);
            continue;
        }

        size = strlen(fileList) + strlen(fileName) + 1;
        tmp = malloc(size);
        strcpy(tmp, fileList);
        strcat(tmp, ",");
        strcat(tmp, fileName);

        free(fileList);
        fileList = malloc(size);
        strcpy(fileList, tmp);
        free(tmp);

    }

//    printf("FileList: %s\n",fileList);
    closedir(d);
    return fileList;

}