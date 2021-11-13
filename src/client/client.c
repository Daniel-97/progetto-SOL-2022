#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "includes/utils.h"


int main(int argc, char *argv[]){

    int opt;

    while( (opt = getopt(argc, argv, "h")) != -1 ){
        switch (opt) {
            case 'h':
                help();
                exit(0);
        }
    }

    return 0;
}