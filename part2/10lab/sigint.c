#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(){
    sigset_t sigset;// Определяем набор сигналов

    // Инициализация набора сигналов
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);

    // Блокировка сигнала SIGINT
    if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1) {
        perror("block_error");
        exit(EXIT_FAILURE);
    }

    printf("SIGINT blocked. PID: %d\n", getpid());
    printf("Use <Ctrl+C> or <kill -SIGINT %d>.\n", getpid());

    // Бесконечный цикл ожидания
    while (1) {
        pause(); // Ожидание сигнала (любой сигнал разблокирует)
    }

    return 0;
}