#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s <PID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Получение PID из аргумента командной строки
    pid_t pid = atoi(argv[1]);

    // Отправка сигнала SIGUSR1
    if (kill(pid, SIGUSR1) == -1) {
        perror("kill_error");
        exit(EXIT_FAILURE);
    }

    printf("SIGUSR1 sended to process with PID %d\n", pid);
    return 0;
}
