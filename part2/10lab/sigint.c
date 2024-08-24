#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    sigset_t set, oset, pset;
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, &oset);
    printf("Running...\n");
    while(1){
    }
    return 0;
}