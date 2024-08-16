#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int main() {
    key_t key;
    int shmid;
    char *shm_ptr;

    // Создаем уникальный ключ
    key = ftok("shmfile", 65);

    // Подключаемся к существующему сегменту разделяемой памяти
    shmid = shmget(key, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // Подключаемся к сегменту разделяемой памяти
    shm_ptr = (char*) shmat(shmid, NULL, 0);
    if (shm_ptr == (char*)-1) {
        perror("shmat");
        exit(1);
    }

    // Читаем сообщение от сервера
    printf("Сообщение от сервера: %s\n", shm_ptr);

    // Отправляем ответ серверу
    strcpy(shm_ptr, "Hello!");

    // Отключаемся от сегмента
    shmdt(shm_ptr);

    return 0;
}
