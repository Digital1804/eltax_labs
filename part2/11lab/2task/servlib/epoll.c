#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>

#include "serverlib.h"

#define MAX_EVENTS 20
// Очередь для клиентов
client_data_t client_queue_epoll[MAX_CLIENTS];
int queue_epoll_start = 0, queue_epoll_end = 0;
pthread_mutex_t queue_epoll_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_epoll_cond = PTHREAD_COND_INITIALIZER;

// Функция для обработки клиентов
void *handle_epoll_client(void *arg) {
    client_data_t *client_data;
    char buffer[BUFFER_SIZE];
    while (1) {
        // Ожидание клиента в очереди
        pthread_mutex_lock(&queue_epoll_mutex);
        while (queue_epoll_start == queue_epoll_end) {
            pthread_cond_wait(&queue_epoll_cond, &queue_epoll_mutex);
        }

        // Извлечение клиента из очереди
        client_data = &client_queue_epoll[queue_epoll_start];
        queue_epoll_start = (queue_epoll_start + 1) % MAX_CLIENTS;
        pthread_mutex_unlock(&queue_epoll_mutex);

        // Обработка клиента
        while (recv(client_data->client_socket, buffer, BUFFER_SIZE, 0) > 0) {
            printf("Server recieved message: %s\n", buffer);
            strncpy(buffer, "Welcome to server", BUFFER_SIZE);
            // Ответ клиенту
            send(client_data->client_socket, buffer, BUFFER_SIZE, 0);
        }
        // Закрытие клиентского сокета
        close(client_data->client_socket);
    }

    return NULL;
}

// Функция для запуска сервера
void start_server_epoll(const int queue_size) {
    // Основной серверный сокет
    int server_socket;
    struct sockaddr_in server, client;
    socklen_t client_size = sizeof(client);
    pthread_t threads[queue_size];
    int epoll_fd;
    struct epoll_event event, events[MAX_EVENTS];

    // Создание серверного сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    // Настройка адреса сервера
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

    // Привязка сокета к адресу и порту
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        if (errno == EINVAL)
            server.sin_port = htons(0);
        else{
            perror("bind_error");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    // Ожидание входящих соединений
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(server);
    if (getsockname(server_socket, (struct sockaddr *)&server, &len) == -1) {
        perror("getsockname_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d...\n", ntohs(server.sin_port));



    // Создание epoll-инстанса
    if ((epoll_fd = epoll_create1(0)) == -1) {
        perror("epoll_create1_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Добавление серверного сокета в epoll для отслеживания входящих соединений
    event.events = EPOLLIN;
    event.data.fd = server_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("epoll_ctl_epoll");
        close(server_socket);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    // Создание потоков для обработки клиентов
    for (int i = 0; i < queue_size; i++) {
        pthread_create(&threads[i], NULL, handle_epoll_client, NULL);
    }
    // Основной цикл сервера
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait_error");
            close(server_socket);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_socket) {
                // Принятие нового соединения
                int client_socket = accept(server_socket, (struct sockaddr *)&client, &client_size);
                if (client_socket < 0) {
                    perror("accept_error");
                    continue;
                }
                // Добавление клиента в очередь
                pthread_mutex_lock(&queue_epoll_mutex);
                client_queue_epoll[queue_epoll_end].client_socket = client_socket;
                client_queue_epoll[queue_epoll_end].client = client;
                queue_epoll_end = (queue_epoll_end + 1) % MAX_CLIENTS;
                pthread_cond_signal(&queue_epoll_cond);
                pthread_mutex_unlock(&queue_epoll_mutex);
            }
        }
    }

    // Закрытие серверного сокета (никогда не выполняется в этом примере)
    close(server_socket);
    close(epoll_fd);
}