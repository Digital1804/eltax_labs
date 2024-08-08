#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "info.h"

typedef struct {
    char name[MAX_NAME_LEN + 1];
    mqd_t mq;
} client_t;

int running = 1;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("Server stopped\n");
        running = 0;
        exit(0);
    }
}

void concatenate_strings(char *result, client_t *clients, int num_strings) {
    result[0] = '\0';
    for (int i = 0; i < num_strings; i++) {
        strcat(result, clients[i].name);
        if (i < num_strings - 1) {
            strcat(result, " ");
        }
    }
    result[strlen(result)] = '\0';
}

char **create_lines_list_from_end(const char *filename, int max_lines) {
    int lines_count = 0;
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        return NULL;
    }

    // Переместим указатель файла в конец

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    char **lines = (char **)malloc(max_lines * sizeof(char *));
    if (!lines) {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }

    char buffer[MAX_SIZE];
    long pos = file_size;
    int i;

    while (pos > 0 && lines_count < max_lines) {

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
        if (strlen(buffer) < 2) continue;
        buffer[len] = '\0';

        // Сохраняем строку в массиве
        lines[lines_count] = strdup(buffer);
        if (!lines[lines_count]) {
            perror("Memory allocation error");
            break;
        }
        lines_count++;

        // Обновляем позицию
        pos = i;
    }

    fclose(file);
    return lines;
}

void log_send(message_t msg) {
    FILE *fp = fopen("log.txt", "a");
    switch (msg.type) {
        case QUIT:
            fprintf(fp, "SERVER send QUIT\tto %s\n", msg.client_name);
            break;
        case CHAT:
            fprintf(fp, "SERVER send MESSAGE\t\"%s\"\tFROM %s\n", msg.text, msg.client_name);
            break;
        case HISTORY:
            fprintf(fp, "SERVER send HISTORY MESSAGE\t\"%s\"\tTO %s\n", msg.text, msg.client_name);
            break;
        case MEMBERS:
            fprintf(fp, "SERVER send MEMBERS\t\"%s\"\tTO  %s\n", msg.text, msg.client_name);
            break;
        default:
            break;
    }
    fclose(fp);
}

void log_rcv(message_t msg) {
    FILE *fp = fopen("log.txt", "a");
    switch (msg.type) {
        case NAME:
            fprintf(fp, "SERVER receive NEW NAME\t%s\n", msg.client_name);
            break;
        case QUIT:
            fprintf(fp, "SERVER receive QUIT FROM %s\n", msg.client_name);
            break;
        case CHAT:
            fprintf(fp, "SERVER receive MESSAGE\t\"%s\"\tFROM %s\n", msg.text, msg.client_name);
            break;
        case HISTORY:
            fprintf(fp, "SERVER receive HISTORY REQUEST FROM %s\n", msg.client_name);
            break;
        case MEMBERS:
            fprintf(fp, "SERVER receive MEMBERS REQUEST FROM %s\n", msg.client_name);
            break;
        default:
            break;
    }
    fclose(fp);
}

void create_service_queue(mqd_t *service_queue) {
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_CLIENTS;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;

    *service_queue = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, QUEUE_PERMISSIONS, &attr);
    if (*service_queue == -1) {
        perror("service mq_open");
        exit(1);
    }
}

void connect_client_queue(char *client_name, mqd_t *client_queue) {
    char client_queue_name[MAX_NAME_LEN + 15];
    snprintf(client_queue_name, sizeof(client_queue_name), "/client_queue_%s", client_name);
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;

    *client_queue = mq_open(client_queue_name, O_WRONLY, QUEUE_PERMISSIONS, &attr);
    if (*client_queue == -1) {
        perror("client queue mq_open");
        exit(1);
    }
}

int main() {
    mqd_t service_queue;
    client_t clients[MAX_CLIENTS];
    FILE *chat;
    int client_count = 0;
    char client_queue_name[MAX_NAME_LEN + 15];
    
    create_service_queue(&service_queue);
    signal(SIGINT, signal_handler);
    FILE *fp = fopen("log.txt", "w");
    fclose(fp);
    printf("Server started\n");
    while (running) {
        message_t msg;
        if (mq_receive(service_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            continue;
        }
        log_rcv(msg);
        // Обработка получаемых сообщений
        switch (msg.type) {
            case NAME: // Подключение клиента
                msg.type = MEMBERS;
                mqd_t client_queue;

                connect_client_queue(msg.client_name, &client_queue);
                chat = fopen("chat.txt", "a");
                fprintf(chat, "%s joined the chat\n", msg.client_name);
                fclose(chat);

                if (client_count < MAX_CLIENTS) {
                    strncpy(clients[client_count].name, msg.client_name, MAX_NAME_LEN);
                    clients[client_count].mq = client_queue;
                    client_count++;
                }

                concatenate_strings(msg.text, clients, client_count);

                for (int i = 0; i < client_count; i++) {
                    log_send(msg);
                    if (mq_send(clients[i].mq, (char *)&msg, sizeof(msg), 0) == -1) {
                        perror("mq_send");
                    }
                }
                break;
            case TEXT: // Клиент отправил сообщение
                msg.type = CHAT;
                chat = fopen("chat.txt", "a");
                fprintf(chat, "%s: %s\n", msg.client_name, msg.text);
                fclose(chat);

                for (int i = 0; i < client_count; i++) {
                    log_send(msg);
                    if (mq_send(clients[i].mq, (char *)&msg, sizeof(msg), 0) == -1) {
                        perror("mq_send");
                    }
                }
                break;
            case HISTORY: // Клиент запрашивает историю сообщений
                char **lines = create_lines_list_from_end("chat.txt", atoi(msg.text));
                char name[MAX_NAME_LEN+1];
                strncpy(name, msg.client_name, MAX_NAME_LEN);
                //  Отправляем строки
                msg.type = HISTORY;
                snprintf(msg.client_name, MAX_NAME_LEN + 1, "%s", "SERVER");
                for (int i = 0; i < client_count; i++) {
                    if (strcmp(clients[i].name, msg.client_name) == 0) {
                        strncpy(msg.client_name, clients[i].name, MAX_NAME_LEN+1);
                        client_queue = clients[i].mq;
                        break;
                    }
                }

                for (int i = sizeof(lines) * 2-1; i >=0 ; i--) {
                    if (i == 0) snprintf(msg.client_name, MAX_NAME_LEN + 1, "%s", name);;
                    strncpy(msg.text, lines[i], MAX_SIZE);
                    log_send(msg);
                    if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                        perror("mq_send");
                    }
                    free(lines[i]); // Освобождаем память, выделенную для строки
                }
                // snprintf(msg.client_name, MAX_NAME_LEN + 1, "%s", name);
                // if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                //     perror("mq_send");
                // }
                free(lines);
                break;
            case QUIT: // Клиент отключается
                msg.type = MEMBERS;
                for (int i = 0; i < client_count; i++) {
                    if (strcmp(clients[i].name, msg.client_name) == 0) {
                        for (int j = i; j < client_count - 1; j++) {
                            clients[j] = clients[j + 1];
                        }
                        client_count--;
                        break;
                    }
                }
                concatenate_strings(msg.text, clients, client_count);
                for (int i = 0; i < client_count; i++) {
                    log_send(msg);
                    if (mq_send(clients[i].mq, (char *)&msg, sizeof(msg), 0) == -1) {
                        perror("mq_send");
                    }
                }
                break;
        }
    }

    printf("Server shutting down...\n");

    if (mq_close(service_queue) == -1) {
        perror("service mq_close");
        exit(1);
    }
    if (mq_unlink(QUEUE_NAME) == -1) {
        perror("service mq_unlink");
        exit(1);
    }

    return 0;
}
