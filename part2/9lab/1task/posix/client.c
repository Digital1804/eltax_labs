#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/posix_shm"
#define SHM_SIZE 1024

int main() {
    int shm_fd;
    char *shm_ptr;

    // Подключаемся к существующему сегменту разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Мапируем сегмент в адресное пространство
    shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Читаем сообщение от сервера
    printf("Сообщение от сервера: %s\n", shm_ptr);

    // Отправляем ответ серверу
    strcpy(shm_ptr, "Hello!");

    return 0;
}
