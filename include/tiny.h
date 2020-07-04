#ifndef TINY_H
#define TINY_H
#ifdef __cplusplus
extern "C"{
#endif
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#ifdef __cplusplus
}
#endif

int loop_task(void*);

int send_static_content(int, const char*, char*, char*, char*, unsigned char);

int send_dynamic_content(int, char*, char*, char*, char**, char* const[]);

int parse_uri_get(char*, char*, char**, char**);

int startswithstr(const char*, const char*);

int endswithstr(const char*, const char*);

int find_str_kmp(const char* str, const char* pat);

#endif