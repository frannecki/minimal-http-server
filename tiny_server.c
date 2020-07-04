#include "tiny.h"
#include "threadpool.h"


int main(int argc, char **argv){
    const int MAXEPOLL = 5;
    struct addrinfo *result;
    struct addrinfo *p;
    struct addrinfo hints;
    int serverfd;
    int clientfd;
    int optval = 1;
    socklen_t size;
    struct threadpool_t thpool;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // on any IP address
    hints.ai_flags |= AI_NUMERICSERV; // using port number
    result = NULL;
    
    fprintf(stdout, "Starting server at %s:%s ...\n", (argc<2)?"localhost":argv[1], (argc<3)?"11311":argv[2]);
    getaddrinfo((argc<2)?"localhost":argv[1], (argc<3)?"11311":argv[2], &hints, &result);
    p = result;
    
    while(p){
        if((serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            continue;
        }
        setsockopt(serverfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
        size = sizeof(*(p->ai_addr));
        if(bind(serverfd, p->ai_addr, size) == 0){
            break;
        }
        p = p->ai_next;
    }
    if(listen(serverfd, 20) < 0){
        perror("listen()");
        exit(-1);
    }
    
    if(0 == threadpool_create(&thpool, 20)){
		fprintf(stdout, "Thread pool created.\n");
	}
    else{
        fprintf(stderr, "Failed to create thread pool.\n");
        close(serverfd);
        return 0;
    }

    while(1){
        if((clientfd = accept(serverfd, p->ai_addr, &size)) < 0){
            perror("accept()");
            return 0;
        }
        threadpool_assign_task(&thpool, &loop_task, clientfd);
    }
    
    close(serverfd);
    threadpool_stop(&thpool);
    return 0;
}
