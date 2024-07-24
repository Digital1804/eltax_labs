#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define QUEUE_KEY 12345

struct message {
    long msg_type;
    char msg_text[100];
};

int main() {
    int msgid;
    struct message msg;

    // Подключение к очереди сообщений
    msgid = msgget(QUEUE_KEY, 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Чтение сообщения от сервера
    if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    printf("Client received: %s\n", msg.msg_text);

    // Отправка ответа серверу
    msg.msg_type = 2;
    strcpy(msg.msg_text, "Hello!");
    if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    return 0;
}
