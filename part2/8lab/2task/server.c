#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define QUEUE_NAME  "/chat_queue"
#define QUEUE_PERMISSIONS 0666
#define MAX_SIZE 512
#define MSG_BUFFER_SIZE (MAX_SIZE + 10)
#define MAX_NAME_LEN 15
#define MAX_CLIENTS 10
#define QUIT 0
#define NAME 1
#define TEXT 2
#define CHAT 3
#define MEMBERS 4

typedef struct {
    long type;
    char text[MAX_SIZE];
    char client_name[MAX_NAME_LEN+1];
} message_t;

int main() {
    int client_count = 0;
    char clients[MAX_CLIENTS][MAX_NAME_LEN+1];
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MSG_BUFFER_SIZE];

    // Очистка буфера
    memset(buffer, 0, sizeof(buffer));

    // Настройка атрибутов очереди
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;

    // Создание очереди сообщений
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, QUEUE_PERMISSIONS, &attr);
    if (mq == -1) {
        perror("mq_open");
        exit(1);
    }

    // Отправка сообщения клиенту
    // if (mq_send(mq, "Hi!", strlen("Hi!") + 1, 0) == -1) {
    //     perror("mq_send");
    //     exit(1);
    // }
    // Ожидание ответа от клиента
    message_t msg;
    while (1) {
        if (mq_receive(mq, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        }
        switch (msg.type ){
            case NAME:
                strncpy(clients[client_count], msg.client_name, MAX_NAME_LEN);
                printf("New member: %s\n", msg.client_name);
                client_count++;
                break;
            case TEXT:
                printf("%s: %s\n", msg.client_name, msg.text);
                break;
            case CHAT:
                printf("%s: %s\n", msg.client_name, msg.text);
                break;
            case MEMBERS:
                printf("Members: %d\n", client_count);
                break;
            case QUIT:
                client_count--;
                printf("Member left: %s\n", msg.client_name);
                break;
        }
        if (client_count == 0)
            break;
    }
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
