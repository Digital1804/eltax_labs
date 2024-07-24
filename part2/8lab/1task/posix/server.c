#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define QUEUE_NAME  "/posix_queue"
#define QUEUE_PERMISSIONS 0666
#define MAX_SIZE 1024
#define MSG_BUFFER_SIZE (MAX_SIZE + 10)

int main() {
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MSG_BUFFER_SIZE];

    // Очистка буфера
    memset(buffer, 0, sizeof(buffer));

    // Настройка атрибутов очереди
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    // Создание очереди сообщений
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, QUEUE_PERMISSIONS, &attr);
    if (mq == -1) {
        perror("mq_open");
        exit(1);
    }

    // Отправка сообщения клиенту
    if (mq_send(mq, "Hi!", strlen("Hi!") + 1, 0) == -1) {
        perror("mq_send");
        exit(1);
    }
    printf("Сообщение отправлено\n");
    // Ожидание ответа от клиента
    sleep(4);
    if (mq_receive(mq, buffer, MSG_BUFFER_SIZE, NULL) == -1) {
        perror("mq_receive");
        exit(1);
    }
    printf("Ответ от клиента: %s\n", buffer);
    // Закрытие и удаление очереди
    if (mq_close(mq) == -1) {
        perror("mq_close");
        exit(1);
    }
    if (mq_unlink(QUEUE_NAME) == -1) {
        perror("mq_unlink");
        exit(1);
    }

    return 0;
}
