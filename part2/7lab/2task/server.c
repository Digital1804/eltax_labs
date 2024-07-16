#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FIFO_NAME "/tmp/my_fifo" /**< Название канала */

/**
 * @brief Создание именованного канала, запись в него сообщения и его закрытие.
 * 
 * @return int 
*/

int main() {
    int fd; /** Дескриптор */
    const char message[10] = "Hi!"; /** Сообщение */
    
    // Создаем канал
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    // Открываем канал на запись
    fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Записываем сообщение в канал
    write(fd, message, sizeof(message));

    // Закрываем дескриптор файла
    close(fd);

    return 0;
}
