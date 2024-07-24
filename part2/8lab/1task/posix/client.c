#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define QUEUE_NAME  "/posix_queue"
#define MAX_SIZE 1024
#define MSG_BUFFER_SIZE (MAX_SIZE + 10)

int main() {
    mqd_t mq;
    char buffer[MSG_BUFFER_SIZE];

    // Очистка буфера
    memset(buffer, 0, sizeof(buffer));

    // Подключение к существующей очереди сообщений
    mq = mq_open(QUEUE_NAME, O_RDWR);
    if (mq == -1) {
        perror("mq_open");
        exit(1);
    }

    // Считывание сообщения от сервера
    if (mq_receive(mq, buffer, MSG_BUFFER_SIZE, NULL) == -1) {
        perror("mq_receive");
        exit(1);
    }
    printf("Сообщение от сервера: %s\n", buffer);

    // Отправка ответа серверу
    if (mq_send(mq, "Hello!", strlen("Hello!") + 1, 0) == -1) {
        perror("mq_send");
        exit(1);
    }
    printf("Ответ отправлен\n");

    // Закрытие очереди
    if (mq_close(mq) == -1) {
        perror("mq_close");
        exit(1);
    }

    return 0;
}
