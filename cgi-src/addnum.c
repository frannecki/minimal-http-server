#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int MAXARGNUM = 5;

int main(int argc, char **argv){
    int i;
    int argvlen;
    int start;
    int end;
    int figures[2] = {0};
    if(argc < 2){
        fprintf(stderr, "Usage: %s 'figure1=[figure1]&figure2=[figure2]'\n", argv[0]);
        return 0;
    }

    // parse argv list
    start = 0;
    end = 0;
    argvlen = strlen(argv[1]);
    for(i = 0; i < 2; ++i){
        while(end < argvlen && argv[1][end] != '='){
            ++end;
        }
        start = ++end;
        while(end < argvlen && argv[1][end] != '&'){
            ++end;
        }
        argv[1][end] = '\0';
        sscanf(argv[1]+start, "%d", &figures[i]);
    }

    //fprintf(stdout, "%d + %d = %d", figures[0], figures[1], figures[0] + figures[1]);
    fprintf(stdout, "%d", figures[0] + figures[1]);

    return 0;
}