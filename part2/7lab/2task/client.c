#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FIFO_NAME "/tmp/my_fifo" /** Название канала */
/**
 * @brief Открытие именованного канала, чтение из него сообщения и удаление канала.
 * 
 * @return int
*/
int main() {
    int fd; /** Дескриптор */
    char buffer[10]; /** Буфер */

    // Открываем канал на чтение
    fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Читаем сообщение из канала
    read(fd, buffer, sizeof(buffer));

    // Выводим сообщение на экран
    printf("Client received: %s\n", buffer);

    // Закрываем дескриптор файла
    close(fd);

    // Удаляем канал
    if (unlink(FIFO_NAME) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
