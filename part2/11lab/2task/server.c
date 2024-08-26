#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "servlib/serverlib.h"

int main(int argc, char *argv[]){
    printf("Input server type:\n");
    printf("\t 1)create\n");
    printf("\t 2)queue\n");
    printf("\t 3)epoll\n");
    int pull_size;
    switch (getc(stdin)){
        case '1':
            start_server_create();
            break;
        case '2':
            printf("Input threads count: ");
            scanf("%d", &pull_size);
            start_server_queue((const int)pull_size);
            break;
        case '3':
            printf("Input threads count: ");
            scanf("%d", &pull_size);
            start_server_epoll((const int)pull_size);
            break;
        default:
            break;
    }
}