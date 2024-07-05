#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int exit_code;
    // Создание нового процесса
    switch (pid = fork()){ 
        case -1:
            // Ошибка при создании процесса
            perror("fork failed");
            exit(EXIT_FAILURE);
            break;
        case 0: 
            // Дочерний процесс
            printf("Child process: PID = %d, PPID = %d\n", getpid(), getppid());
            // Завершение дочернего процесса
            exit(&exit_code);
            break;
        default:
            // Родительский процесс
            int status;
            printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);
            // Ожидание завершения дочернего процесса
            pid_t child_pid = waitpid(pid, &status, 0);
            if (child_pid < 0) {
                perror("waitpid failed");
                exit(EXIT_FAILURE);
            }
            // Вывод статуса завершения дочернего процесса
            printf("Child process terminated with exit status = %d\n", WEXITSTATUS(status));
    }
    return 0;
}
