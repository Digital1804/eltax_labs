#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Обработчик сигнала SIGUSR1
void handle_sigusr1(int sig) {
    printf("Recieved SIGUSR1\n");
}

int main() {
    struct sigaction sa;// Структура для настройки сигнала
    sa.sa_handler = handle_sigusr1; // Установка функции-обработчика
    sa.sa_flags = 0; // Нет дополнительных флагов
    sigemptyset(&sa.sa_mask); // Пустая маска блокировки

    // Установка обработчика сигнала SIGUSR1
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("set_error");
        exit(EXIT_FAILURE);
    }

    printf("Wait SIGUSR1... (PID: %d)\n", getpid());

    // Бесконечный цикл ожидания
    while (1) {
        pause(); // Ожидание сигнала
    }

    return 0;
}
