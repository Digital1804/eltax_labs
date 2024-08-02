#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "info.h"
int running = 1;
void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("Server stopped\n");
        running = 0;
        exit(0);
    }
}

void read_file_from_end(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        return;
    }

    // Переместим указатель файла в конец
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    char buffer[MAX_SIZE];
    long pos = file_size;
    int i;

    while (pos > 0) {
        // Ищем конец строки
        for (i = pos - 1; i >= 0; i--) {
            fseek(file, i, SEEK_SET);
            if (fgetc(file) == '\n') {
                break;
            }
        }

        // Определяем длину строки
        int len = pos - i - 1;

        // Считываем строку
        fseek(file, i + 1, SEEK_SET);
        fread(buffer, 1, len, file);
        buffer[len] = '\0';
        
        // Выводим строку
        printf("%s\n", buffer);

        pos = i;
    }

    fclose(file);
}

void concatenate_strings(char *result, char strings[][MAX_NAME_LEN+1], int num_strings) {
    // Убедимся, что результирующая строка пустая
    result[0] = '\0';
    for (int i = 0; i < num_strings; i++) {
        // Проверим, что строка не пустая
        if (strings[i] != NULL && strlen(strings[i]) > 0) {
            strcat(result, strings[i]);

            // Добавляем разделитель, если не последняя строка
            if (i < num_strings - 1) {
                strcat(result, " "); // Используем пробел в качестве разделителя
            }
        }
    }
    result[strlen(result)] = '\0';
}

int main() {
    printf("Server started\n");
    FILE *fp, *chat;
    fp = fopen("log.txt", "w");
    fclose(fp);
    int client_count = 0;
    char clients[MAX_CLIENTS][MAX_NAME_LEN+1] = {};
    mqd_t service_queue, client_queue;
    struct mq_attr attr;

    // Настройка атрибутов очереди
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_CLIENTS;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;
    signal(SIGINT, signal_handler);
    // Создание очереди сообщений
    service_queue = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, QUEUE_PERMISSIONS, &attr);
    if (service_queue == -1) {
        perror("mq_open");
        exit(1);
    }

    client_queue = mq_open(CLIENT_QUEUE_NAME, O_CREAT | O_RDWR, QUEUE_PERMISSIONS, &attr); 
    if (client_queue == -1) {
        perror("mq_open");
        exit(1);
    }
    message_t msg;
    while (running) {
        strncmp(msg.text, "", 1);
        if (mq_receive(client_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        }
        else{
            fp = fopen("log.txt", "a");
            fprintf(fp, "SERVER recieve %ld\t%s\t%s\n", msg.type, msg.text, msg.client_name);
            fclose(fp);
            switch (msg.type) {
                case NAME:
                    strncpy(clients[client_count++], msg.client_name, MAX_NAME_LEN);
                    // Заполняем именами всех клиентов
                    concatenate_strings(msg.text, clients, client_count);
                    msg.type = MEMBERS;
                    for (int i = 0; i < client_count; i++) {
                        if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                            perror("mq_send");
                        }
                    }
                    fp = fopen("log.txt", "a");
                    fprintf(fp, "SERVER send MEMBERS %s\n", msg.text);
                    fclose(fp);
                    break;
                case TEXT:
                    chat = fopen("chat.txt", "a");
                    fprintf(chat, "%s: %s\n", msg.client_name, msg.text);
                    fclose(chat);
                    msg.type = CHAT;
                    for (int i = 0; i < client_count; i++) {
                        if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                            perror("mq_send");
                        }
                    }
                    break;
                case QUIT:
                    for (int i = 0; i < client_count; i++) {
                        if (strcmp(clients[i], msg.client_name) == 0) {
                            for (int j = i; j < client_count - 1; j++) {
                                strcpy(clients[j], clients[j + 1]);
                            }
                            client_count--;
                            break;
                        }
                    }
                    concatenate_strings(msg.text, clients, client_count);
                    msg.type = MEMBERS;
                    for (int i = 0; i < client_count; i++) {
                        if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                            perror("mq_send");
                        }
                    }
                    fprintf(fp, "SERVER send MEMBERS %s\n", msg.text);
                    break;
            }
        }
    }
    printf("Server shutting down...\n");
    // Закрытие и удаление очереди
    if (mq_close(service_queue) == -1) {
        perror("service mq_close");
        exit(1);
    }
    if (mq_unlink(QUEUE_NAME) == -1) {
        perror("service mq_unlink");
        exit(1);
    }
    if (mq_close(client_queue) == -1) {
        perror("client mq_close");
        exit(1);
    }
    if (mq_unlink(CLIENT_QUEUE_NAME) == -1) {
        perror("client mq_unlink");
        exit(1);
    }

    return 0;
}
