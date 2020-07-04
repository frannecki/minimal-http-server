#include "tiny.h"

const int MAXHEADERLEN = 200;
const int MAXSTATUSLEN = 50;
const int MAXFILETYPELEN = 40;
const int MAXMETVERLEN = 10;
const int MAXURILEN = 60;
const int MAXPOSTLEN = 1000;
const int MAXTEXTLEN = 200;


int loop_task(void* ptr){
    int ret;
    int recvlen;
    char* filename;
    char* argvlist;
    char resp_header[MAXHEADERLEN];
    char req_header[MAXHEADERLEN];
    char req_text[MAXTEXTLEN];
    char method[MAXMETVERLEN];
    char uri[MAXURILEN];
    char version[MAXMETVERLEN];
    char* const environs[] = {NULL};
    char* emptylist[] = {NULL, NULL, NULL};
    char filetype[MAXFILETYPELEN];
    char resp_status[MAXSTATUSLEN];
    char request[MAXPOSTLEN];
    char tmp[30];
    char mainpage[] = "index.html";
    int fd = *(int*)ptr;

    bzero(request, MAXPOSTLEN);
    if((recvlen = recv(fd, request, MAXPOSTLEN, 0)) < 0){
        perror("recv()");
        close(fd);
        return -1;
    }
    else if(recvlen == 0){
        fprintf(stdout, "Client aborted connection\n.");
        close(fd);
        return -4;
    }
    //fprintf(stdout, "HTTP Request: %s\n", request);
    sscanf(request, "%s %s %s", method, uri, version);
    fprintf(stdout, "Request: %s %s %s\n", method, uri, version);
    if(strlen(uri) == 0 || uri[0] != '/'){
        fprintf(stderr, "Invalid! URI mustn't be empty and should start with a '/'.\n");
        close(fd);
        return -2;
    }
    if(0 == strcmp(method, "GET")){
        if((ret = parse_uri_get(uri, mainpage, &filename, &argvlist)) == 0){
            // static;
            if(0 == send_static_content(fd, filename, resp_status, resp_header, filetype, 1)){
                //fprintf(stdout, "Static served: %s\n", filename);
            }
            else{
                fprintf(stderr, "Error while serving static: %s\n\n", filename);
            }
        }
        else if(ret == 1){
            // dynamic
            send_dynamic_content(fd, filename, argvlist, resp_status, emptylist, environs);
        }
    }
    else if(0 == strcmp(method, "POST")){
        int offset1 = find_str_kmp(request, "\r\n");        // end of Method-Get-Version
        int offset2 = find_str_kmp(request, "\r\n\r\n");    // end of request header
        sscanf(request, "%s %s %s\n", method, uri, version);
        snprintf(req_header, offset2-offset1-3+1, "%s", request+offset1+3);
        sscanf(request+offset2+4, "%[^\a]", req_text);
        fprintf(stdout, "Request Text: %s\n", req_text);
        send_dynamic_content(fd, uri+1, req_text, resp_header, emptylist, environs);
    }
    else if(0 == strcmp(method, "HEAD")){
        send_static_content(fd, uri+1, resp_header, resp_header, filetype, 0);
    }
    else{
        fprintf(stderr, "The method '%s' is not currently supported.\n", method);
        sprintf(resp_status, "HTTP/1.0 400 Bad Request\n");
        write(fd, resp_status, strlen(resp_status));
        close(fd);
        return -3;
    }
    
    close(fd);
    return 0;
}


int send_static_content(int fd, const char* filename, char* resp_status, 
    char* resp_header, char* filetype, unsigned char method_get)
{
    int filefd;
    int filesize;
    void* mmfd;
    struct stat st;
    char tmp[50];    // set size 30 will cause stack overflow (stack smashing error)
    if(stat(filename,&st)==0){
        filesize = st.st_size;
    }
    else{
        fprintf(stderr, "File %s not found.\n", filename);
        strcpy(resp_status, "HTTP/1.0 404 Not Found\n");
        write(fd, resp_status, strlen(resp_status));
        return -1;
    }
    
    // get file type
    if(!endswithstr(filename, ".html")){
        strcpy(filetype, "text/html; charset=UTF-8");
    }
    else if(!endswithstr(filename, ".js")){
        strcpy(filetype, "txt/javascript");
    }
    else if(!endswithstr(filename, ".css")){
        strcpy(filetype, "text/css");
    }
    else if(!endswithstr(filename, ".jpg")){
        strcpy(filetype, "image/jpeg");
    }
    else if(!endswithstr(filename, ".png")){
        strcpy(filetype, "image/png");
    }
    else if(!endswithstr(filename, ".gif")){
        strcpy(filetype, "image/gif");
    }
    else if(!endswithstr(filename, ".ico")){
        strcpy(filetype, "image/icon");
    }
    else{
        strcpy(filetype, "text/plain; charset=UTF-8");
    }

    // resp_header is neccessary for the client browser to display the content properly.
    // but it must follow some specific rules
    
    if(method_get && (filefd = open(filename, O_RDONLY)) < 0){
        strcpy(resp_status, "HTTP/1.0 404 Not Found\r\n");
        write(fd, resp_status, strlen(resp_status));
        return -2;
    }

    strcpy(resp_status, "HTTP/1.0 200 OK\r\n");
    strcpy(resp_header, "Connection: close\r\nServer: TWS\r\n");
    sprintf(tmp, "Content-Type: %s\r\n", filetype);
    strcat(resp_header, tmp);
    sprintf(tmp, "Content-Length: %d\r\n", filesize);
    strcat(resp_header, tmp);
    strcat(resp_header, "\r\n");

    write(fd, resp_status, strlen(resp_status));
    write(fd, resp_header, strlen(resp_header));
    
    if(method_get){
        mmfd = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, filefd, 0);
        close(filefd);
        write(fd, (char*)mmfd, filesize);
        munmap(mmfd, filesize);
        /*
        sendfile(fd, filefd, NULL, filesize);
        close(filefd);
        */
    }
    return 0;
}


int send_dynamic_content(int fd, char* filename, char* args, char* resp_header, char** emptylist, 
        char* const environs[])
{
    char tmp[30];
    int pid;
    emptylist[0] = filename;
    emptylist[1] = (strlen(args)>0)?args:NULL;
    strcpy(resp_header, "HTTP/1.0 200 OK\r\nConnection: close\r\nServer: TWS\r\nContent-Type: text/plain\r\n\r\n");
    write(fd, resp_header, strlen(resp_header));
    if((pid = fork()) < 0){
        perror("fork()");
        return -1;
    }
    else if(pid == 0){
        dup2(fd, STDOUT_FILENO);
        execve(filename, emptylist, environs);
        exit(0);
    }
    else{
        wait(NULL);
    }
    return 0;
}


int parse_uri_get(char* uri, char* mainpage, char** filename, char** args){
    int len = strlen(uri);
    int cnt = 1;
    char *ptr = uri+1;
    if(len == 1){
        *filename = mainpage;
        return 0;
    }
    *filename = ptr;
    if(!startswithstr(ptr, "cgi-bin")){
        // dynamic;
        while(*ptr != '?'){
            if((++cnt) >= len){
                return 0;
            }
            ptr++;
        }
        *ptr = '\0';
        *args = ptr+1;
        return 1;
    }
    return 0;
}


int startswithstr(const char* str, const char* sub){
    int len1 = strlen(str);
    int len2 = strlen(sub);
    char *buf;
    return strncmp(str, sub, len2);
}


int endswithstr(const char* str, const char* sub){
    int len1 = strlen(str);
    int len2 = strlen(sub);
    return strcmp(sub, str+len1-len2);
}


int find_str_kmp(const char* str, const char* pat){
    int len1 = strlen(str);
    int len2 = strlen(pat);
    int* next = (int*)malloc((1+len2)*sizeof(int));
    int t;
    int i, j;
    i = 0;
    t = next[0] = -1;
    while(i < len2){
        if(t < 0 || pat[t] == pat[i]){
            next[++i] = ++t;
        }
        else{
            t = next[t];
        }
    }
    i = j = 0;
    while(i < len1 && j < len2){
        if(j < 0 || str[i] == pat[j]){
            ++i;
            ++j;
        }
        else{
            j = next[j];
        }
    }
    if(j == len2)  return i-len2;
    else  return -1;
    free(next);
}