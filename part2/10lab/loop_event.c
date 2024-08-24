#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);
    while (1){
        sigwait(&set, &sig);
        printf("Caught signal %d\n", sig);
    }
}