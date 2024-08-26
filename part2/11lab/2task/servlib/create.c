#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "serverlib.h"

void *handle_create_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Чтение данных от клиента
    while (recv(client_socket, buffer, BUFFER_SIZE, 0) > 0) {
        printf("Recieved message: %s\n", buffer);
        strncpy(buffer, "Welcome to server!", BUFFER_SIZE);
        send(client_socket, buffer, BUFFER_SIZE, 0);
    }

    // Закрытие сокета клиента
    close(client_socket);
    free(client_socket_ptr);
    return NULL;
}

void start_server_create(){
    int server_socket;
    struct sockaddr_in server, client;
    socklen_t client_size = sizeof(client);

    // Создание сокета
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
    if (listen(server_socket, 3) < 0) {
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

    while (1) {
        // Принятие нового соединения
        int *client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(server_socket, (struct sockaddr *)&client, &client_size)) < 0) {
            perror("accept_error");
            free(client_socket);
            continue;
        }
        // Создание нового потока для обработки клиента
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_create_client, client_socket) != 0) {
            perror("pthread_create_error");
            free(client_socket);
            close(*client_socket);
            continue;
        }

        // Отсоединение потока, чтобы ресурсы освобождались автоматически по завершению
        pthread_detach(thread_id);
    }

    // Закрытие серверного сокета (не будет достигнуто в этом примере)
    close(server_socket);
}