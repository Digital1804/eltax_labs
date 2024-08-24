#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

int main() {    
    sigset_t sigset;// Определяем набор сигналов
    int sig;

    // Инициализация набора сигналов
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);

    // Блокировка сигнала SIGUSR1
    if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1) {
        perror("block_error");
        exit(EXIT_FAILURE);
    }

    printf("Wait SIGUSR1... (PID: %d)\n", getpid());

    // Бесконечный цикл ожидания сигнала
    while (1) {
        // Ожидание сигнала
        if (sigwait(&sigset, &sig) == 0) {
            if (sig == SIGUSR1) {
                printf("Recieved SIGUSR1 ");
            }
        } else {
            perror("wait_error");
        }
        printf("continue cicle.\n");
    }

    return 0;
}