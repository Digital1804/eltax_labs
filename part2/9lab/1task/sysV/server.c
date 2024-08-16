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

    // Создаем сегмент разделяемой памяти
    shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
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

    // Записываем сообщение
    strcpy(shm_ptr, "Hi!");

    printf("Сообщение сервером отправлено: %s\n", shm_ptr);

    // Ожидаем ответа от клиента
    while (strcmp(shm_ptr, "Hello!") != 0) {
        continue;
    }

    printf("Ответ от клиента: %s\n", shm_ptr);

    // Отключаемся от сегмента и удаляем его
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
