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

    // Создание очереди сообщений
    msgid = msgget(QUEUE_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Отправка сообщения
    msg.msg_type = 1;
    memcpy(msg.msg_text, "Hi!", sizeof("Hi!"));
    if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    // Ожидание ответа от клиента
    if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 2, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    printf("Server received: %s\n", msg.msg_text);

    // Удаление очереди сообщений
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}
