#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handle_sigusr1(int sig){
    printf("Caught signal %d (SIGUSR1)\n", sig);
}

int main(){
    struct sigaction new_action, old_action;
    new_action.sa_handler = handle_sigusr1;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGUSR1, &new_action, &old_action);
    printf("Running...\n");
    while(1){
    }
    return 0;
}