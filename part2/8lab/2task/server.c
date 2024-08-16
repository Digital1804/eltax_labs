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
/** @brief Обработчик сигнала
    При сочетании клавиш <Ctrl+C> вызывает функцию signal_handler 
    @param signal 
 */
void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("Server stopped\n");
        running = 0;
        exit(0);
    }
}

/** @brief Складывает массив клиентов в одну строку с именами 
    @param result Итоговая строка
    @param clients Массив строк
    @param num_strings Количество клиентов
*/
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
/**
    @brief Проверяет, пуст ли файл
    @param filename Имя файла
    @return 1, если файл пуст, 0, если файл не пуст, -1, если произошла ошибка при открытии файла
 */
int is_file_empty(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1; // Ошибка при открытии файла
    }

    fseek(file, 0, SEEK_END);  // Перемещение указателя в конец файла
    long size = ftell(file);   // Получение текущей позиции указателя (размер файла)
    fclose(file);              // Закрытие файла

    return size == 0; // Возвращает 1, если файл пуст, иначе 0
}

/**
    @brief Создает массив строк из файла
    @param filename Имя файла
    @param max_lines Максимальное количество строк
    @return Массив строк, NULL, если произошла ошибка при открытии файла или пустой файл
*/
char **create_lines_list_from_end(const char *filename, int max_lines) {
    int lines_count = 0;

    if (is_file_empty(filename)) {
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        return NULL;
    }

    // Переместим указатель файла в конец
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    // Создаем массив строк
    char **lines = (char **)calloc(max_lines, sizeof(char *));
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
        buffer[len] = '\0';

        // Проверка на пустую строку
        if (strlen(buffer) == 0) {
            pos = i;
            continue;
        }

        // Сохраняем строку в массиве
        lines[lines_count] = strdup(buffer); // strdup выделяет память и копирует строку
        if (!lines[lines_count]) {
            perror("Memory allocation error");
            break;
        }
        lines_count++;

        // Обновляем позицию
        pos = i;
    }

    fclose(file);
    return lines; // Удаляем free, чтобы вернуть указатель на строки
}

/**
    @brief Логирование отправки сообщения
    @param msg Отправленное сообщение
 */
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

/**
    @brief Логирование получения сообщения
    @param msg Принятое сообщение
 */
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

/**
    @brief Создание очереди сообщений сервера 
    @param client_queue Очередь сообщений сервера
 */
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

/**
    @brief Подключение к очереди сообщений клиента
    @param client_name Имя клиента
    @param client_queue Очередь сообщений клиента
 */
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
    
    struct mq_attr attr;

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
            case HISTORY: {
                int max_lines = atoi(msg.text);
                char **lines = create_lines_list_from_end("chat.txt", max_lines);
                char name[MAX_NAME_LEN+1];
                strncpy(name, msg.client_name, MAX_NAME_LEN);

                if (lines == NULL) {
                    msg.text[0] = '\0';
                    log_send(msg);
                    for (int i = 0; i < client_count; i++) {
                        if (strcmp(clients[i].name, name) == 0) {
                            if (mq_send(clients[i].mq, (char *)&msg, sizeof(msg), 0) == -1) {
                                perror("mq_send");
                            }
                            break;
                        }
                    }
                    break;
                }

                mqd_t client_queue = (mqd_t) -1;
                for (int i = 0; i < client_count; i++) {
                    if (strcmp(clients[i].name, name) == 0) {
                        client_queue = clients[i].mq;
                        break;
                    }
                }

                if (client_queue == (mqd_t) -1) {
                    perror("Client queue not found");
                    break;
                }

                int num_lines = 0;
                for (int i = 0; i < max_lines; i++) {
                    if (lines[i] != NULL) {
                        num_lines++;
                    }
                }
                msg.type = HISTORY;
                for (int i = 0; i < num_lines; i++) {
                    strncpy(msg.text, lines[i], MAX_SIZE);
                    log_send(msg);
                    if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                        perror("mq_send");
                    }
                    free(lines[i]); // Освобождаем память, выделенную для строки
                }
                free(lines);
                break;
            }
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
                chat = fopen("chat.txt", "a");
                fprintf(chat, "%s left the chat\n", msg.client_name);
                fclose(chat);
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
