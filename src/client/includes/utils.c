//
// Created by daniele on 12/11/21.
//

#include "utils.h"
#include "serverApi.h"

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

    if(enablePrintDebug)
        printf("Server response: {STATUS_CODE: %d, MESSAGE: %s, FILE_SIZE: %zu , FILE_NAME: %s }\n",response->statusCode, response->message,response->fileSize,response->fileName);

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

int saveFileDir(void *buf, size_t size,const char* dirname,const char* fileName){

    const char *name = getFileNameFromPath(fileName);
    char *path = malloc(sizeof(dirname)+sizeof(name));
    strcat(path,dirname);
    strcat(path,name);
    printf("Salvataggio file %s in corso\n",path);
    return saveFile(path,buf,size);

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

char* getFileListFromDir(const char* dirname, int maxFile){

    char *fileList = NULL;
    char *tmp = NULL;
    int cont = 0;

    FILE *fp;
    char command[255];
    char output[1000];
    strcat(command,"/bin/find ");
    strcat(command,dirname);
    strcat(command," -type f");

    printf("%s\n",command);
    fp = popen(command,"r");

    if(fp != NULL){
        /* Read the output a line at a time - output it. */
        while (fgets(output, sizeof(output), fp) != NULL) {

            if(fileList == NULL){

                fileList = malloc(strlen(output)-1);
                strncpy(fileList, output, strlen(output)-1);

            }else{
                tmp = malloc(strlen(fileList));
                strcpy(tmp, fileList);
                free(fileList);
                fileList = malloc(strlen(tmp)+strlen(output));
                strncpy(fileList, tmp, strlen(tmp));
                strcat(fileList,",");
                strncat(fileList, output, strlen(output)-1);
                free(tmp);

            }
            /* Exit when reached max file */
            if(cont == maxFile)
                break;
            /* Limit max file only if maxFile is -1 */
            if(maxFile != -1)
                cont = cont + 1;
        }

        pclose(fp);
        return fileList;
    }

    return NULL;

}

const char* getFileNameFromPath(const char* path){

    if(path == NULL) return NULL;

    char *last = strrchr(path, '/');

    if(last != NULL)
        return last+1;
    else
        return path;

}