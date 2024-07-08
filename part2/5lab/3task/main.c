#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_INPUT_SIZE 128 /**< Максимальный размер входной строки */
#define MAX_ARG_COUNT 16 /**< Максимальное количество аргументов */
/**
 * @brief Главная функция простого командного интерпретатора.
 * 
 * @return int Статус завершения программы.
 */
int main() {
    char input[MAX_INPUT_SIZE];/**< Массив для входных данных */
    char *args[MAX_ARG_COUNT];/**< Массив для аргументов команды */
    char *token;/**< Токен для разбиения строки ввода */
    pid_t pid;/**< Идентификатор процесса */
    int status;/**< Код завершения процесса */
    int arg_count;/**< Количество аргументов */
    while (1) {
        // Вывод приглашения для ввода
        printf("bush> ");
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

        // Разбиение строки ввода на аргументы
        arg_count = 0;
        token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARG_COUNT - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        // Порождаем новый процесс
        switch (pid = fork()){
            case -1:
                perror("fork failed");
                continue;
            case 0:
                // Дочерний процесс: выполняет команду
                execvp(args[0], args);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            default:
                // Родительский процесс: ждет завершения дочернего процесса
                waitpid(pid, &status, 0);
        }
    }
    return 0;
}