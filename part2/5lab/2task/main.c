#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void create_fork(int num) {
    pid_t pid = fork();
    int status;

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Process %d: PID = %d, PPID = %d\n", num, getpid(), getppid());
        // Завершение дочернего процесса
        exit(EXIT_SUCCESS);
    }
    else{
        // Родительский процесс ждет завершения дочернего процесса
        waitpid(pid, &status, 0);
    }
}

int main() {
    pid_t pid1, pid2; 
    int status;
    switch (pid1 = fork()){ // Создание процесса1
        case -1:
            perror("fork failed");
            exit(EXIT_FAILURE);
        case 0:
            // Создание и ожидание завершения процессов3 и процессов4
            create_fork(3);
            create_fork(4);
            exit(EXIT_SUCCESS);
        default:
            // Родительский процесс ждет завершения процесса1
            waitpid(pid1, &status, 0);
            // Родительский процесс 1
            printf("Process 1: PID = %d, PPID = %d\n", getpid(), getppid());
            break;
    }
    switch (pid2 = fork()){ // Создание процесса2
        case -1:
            perror("fork failed");
            exit(EXIT_FAILURE);
        case 0:
            // Создание и ожидание завершения процесса5
            create_fork(5);
            exit(EXIT_SUCCESS);
        default:
            // Родительский процесс ждет завершения процесса2
            waitpid(pid2, &status, 0);
            printf("Process 2: PID = %d, PPID = %d\n", getpid(), getppid());
            break;
    }
    printf("Process 0: PID = %d, PPID = %d\n", getpid(), getppid());
    return 0;
}
