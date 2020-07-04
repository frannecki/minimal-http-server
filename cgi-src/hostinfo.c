#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int MAXARGNUM = 5;
char* argvlist[5] = {NULL};

int main(int argc, char **argv){
    int i;
    int argvlen;
    int start;
    int end;
    int flags;
    int ret;
    int cnt;
    struct addrinfo *result;
    struct addrinfo *p;
    struct addrinfo hints;
    const int MAXHOST = 50;
    char host[NI_MAXHOST];
    printf("argc: %d\n", argc);
    for(int i = 0; i < argc; ++i){
        printf("Argv[%d]: %s\n", i, argv[i]);
    }
    if(argc < 2){
        //fprintf(stderr, "Usage: %s [domain name 1] [domain name 2] ...\n", argv[0]);
        return 0;
    }
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    result = NULL;
    
    // parse argv list
    cnt = 0;
    start = 0;
    end = 0;
    argvlen = strlen(argv[1]);
    while(cnt < MAXARGNUM && start < argvlen){
        while(end < argvlen && argv[1][end] != '&'){
            ++end;
        }
        argv[1][end] = '\0';
        argvlist[cnt++] = argv[1]+start;
        //snprintf(argvlist[cnt], end-start, argv[1]);
        start = ++end;
    }
    
    for(i = 0; i < cnt; ++i){
        getaddrinfo(argvlist[i], NULL, &hints, &result);
        p = result;
        while(p){
            ret = getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
            if(ret == 0){
                //write(fileno(stdout), host, strlen(host));
                fprintf(stdout, "%s\t%s\n", argvlist[i], host);
            }
            else{
                //fprintf(stderr, "getnameinfo() failed: %s\n", gai_strerror(ret));
            }
            p = p->ai_next;
        }
    }
    freeaddrinfo(result);
    return 0;
}