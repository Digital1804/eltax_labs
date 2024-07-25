#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "info.h"


int main() {
    int client_count = 0;
    char clients[MAX_CLIENTS][MAX_NAME_LEN+1];
    mqd_t mq;
    struct mq_attr attr;

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

    message_t msg;
    while (1) {
        if (mq_receive(mq, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        }
        switch (msg.type ){
            case NAME:
                strncpy(clients[client_count++], msg.client_name, MAX_NAME_LEN);
                printf("New member: %s\n", msg.client_name);
                // Заполняем именами всех клиентов
                char *ptr = msg.text;
                for (int i = 0; i < client_count; i++) {
                    strcpy(ptr, clients[i]);
                    ptr += strlen(clients[i]);
                    strcpy(ptr, "\n ");
                    ptr++;
                }
                *ptr = '\0';
                msg.type = MEMBERS;
                sleep(1);
                mq_send(mq, (char *)&msg, sizeof(msg), MEMBERS);
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