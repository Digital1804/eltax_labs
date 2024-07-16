#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_INPUT_SIZE 128 /**< Максимальный размер входной строки */
#define MAX_ARG_COUNT 16 /**< Максимальное количество аргументов */
#define MAX_PIPE_COUNT 16 /**< Максимальное количество конвейеров */

/**
 * @brief Главная функция простого командного интерпретатора с поддержкой конвейеров.
 * 
 * @return int Статус завершения программы.
 */
int main() {
    char input[MAX_INPUT_SIZE]; /**< Массив для входных данных */
    char *args[MAX_ARG_COUNT]; /**< Массив для аргументов команды */
    char *commands[MAX_PIPE_COUNT]; /**< Массив для команд в конвейере */
    char *token; /**< Токен для разбиения строки ввода */
    pid_t pid; /**< Идентификатор процесса */
    int status; /**< Код завершения процесса */
    int arg_count; /**< Количество аргументов */
    int cmd_count; /**< Количество команд в конвейере */
    int pipe_fd[2]; /**< Файловые дескрипторы для канала */
    int in_fd = 0; /**< Входной файловый дескриптор для следующей команды */

    while (1) {
        // Вывод приглашения для ввода
        printf("bush> ");
        fflush(stdout);

        // Считывание строки ввода
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            // Обработка ошибки
            perror("fgets failed");
            continue;
        }

        // Удаление символа новой строки
        input[strcspn(input, "\n")] = '\0';

        // Проверка на команду выхода
        if ((strcmp(input, "exit") == 0) || (strcmp(input, "e") == 0)) {
            break;
        }

        // Разбиение строки ввода на команды
        cmd_count = 0;
        token = strtok(input, "|");
        while (token != NULL && cmd_count < MAX_PIPE_COUNT - 1) {
            commands[cmd_count++] = token;
            token = strtok(NULL, "|");
        }
        commands[cmd_count] = NULL;

        for (int i = 0; i < cmd_count; ++i) {
            // Разбиение команды на аргументы
            arg_count = 0;
            token = strtok(commands[i], " ");
            while (token != NULL && arg_count < MAX_ARG_COUNT - 1) {
                args[arg_count++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            // Создаем канал, если это не последняя команда
            if (i < cmd_count - 1) {
                if (pipe(pipe_fd) == -1) {
                    perror("pipe failed");
                    exit(EXIT_FAILURE);
                }
            }

            // Порождаем новый процесс
            switch (pid = fork()) {
                case -1:
                    perror("fork failed");
                    exit(EXIT_FAILURE);
                case 0:
                    // Дочерний процесс: перенаправляем ввод/вывод и выполняем команду
                    if (in_fd != 0) {
                        dup2(in_fd, 0);
                        close(in_fd);
                    }
                    if (i < cmd_count - 1) {
                        close(pipe_fd[0]);
                        dup2(pipe_fd[1], 1);
                        close(pipe_fd[1]);
                    }
                    execvp(args[0], args);
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                default:
                    // Родительский процесс: ждет завершения дочернего процесса
                    waitpid(pid, &status, 0);
                    if (in_fd != 0) close(in_fd);
                    if (i < cmd_count - 1) {
                        close(pipe_fd[1]);
                        in_fd = pipe_fd[0];
                    }
            }
        }
    }
    return 0;
}
