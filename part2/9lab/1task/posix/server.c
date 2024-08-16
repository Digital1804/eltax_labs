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

    // Создаем сегмент разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Устанавливаем размер сегмента
    ftruncate(shm_fd, SHM_SIZE);

    // Мапируем сегмент в адресное пространство
    shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Записываем сообщение
    strcpy(shm_ptr, "Hi!");

    printf("Сообщение сервером отправлено: %s\n", shm_ptr);

    // Ожидаем ответа от клиента
    while (strcmp(shm_ptr, "Hello!") != 0) {
        continue;
    }

    printf("Ответ от клиента: %s\n", shm_ptr);

    // Удаляем сегмент разделяемой памяти
    shm_unlink(SHM_NAME);

    return 0;
}
