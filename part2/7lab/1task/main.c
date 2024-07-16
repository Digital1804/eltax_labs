#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief Создание и использование неименованного канала родительским и дочерним процессами 
 *
 * @return int
 */

int main() {
    int fd[2]; /** Дескриптор */
    pid_t pid; /**< Идентификатор процесса */
    char buffer[10]; /** Буфер */
    
    // Создаем неименованный канал
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    // Порождаем процесс
    switch (pid = fork()){
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
            break;
        case 0:
            // Дочерний процесс
            close(fd[1]); // Закрываем дескриптор для записи
            
            // Читаем строку из канала
            read(fd[0], buffer, sizeof(buffer));
            
            // Выводим строку на экран
            printf("Child process received: %s\n%ld\n", buffer, sizeof(buffer));
            
            // Закрываем дескриптор для чтения
            close(fd[0]);
            
            // Завершаем дочерний процесс
            exit(EXIT_SUCCESS);
            break;
        default:
            int status;
            // Родительский процесс
            close(fd[0]); // Закрываем дескриптор для чтения
            
            // Записываем строку в канал
            char msg[10] = "Hi!";
            write(fd[1], msg, sizeof(msg));
            
            // Закрываем дескриптор для записи
            close(fd[1]);
            
            // Ожидаем завершения дочернего процесса
            waitpid(pid, &status, 0);
            break;
    }
    return 0;
}
