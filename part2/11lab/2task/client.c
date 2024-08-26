#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 7777
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define NUM_CLIENTS 1000 // Количество клиентов

int client_number = 0;

// Функция для каждого клиента
void *client_thread(void *arg) {
    int client_id = ++client_number;
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Создание клиентского сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket_error");
        pthread_exit(NULL);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton_error");
        close(client_socket);
        pthread_exit(NULL);
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect_error");
        close(client_socket);
        pthread_exit(NULL);
    }
    // Отправка сообщения серверу
    snprintf(buffer, BUFFER_SIZE, "Hello from client %d!", client_id);
    send(client_socket, buffer, strlen(buffer), 0);
    printf("Client %d sent message: %s\n", client_id, buffer);
    memset(buffer, 0, BUFFER_SIZE);

    // Получение ответа от сервера
    if (recv(client_socket, buffer, BUFFER_SIZE - 1, 0) > 0) {
        printf("Client %d recieved message: %s\n", client_id, buffer);
    } else {
        printf("Client %d didn't recieve message.\n", client_id);
    }

    // Закрытие клиентского сокета
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    int num_clients = NUM_CLIENTS;
    if (argc > 1) {
        num_clients = atoi(argv[2]);
    }

    // Создание нескольких клиентских потоков
    pthread_t clients[num_clients];

    // Создание нескольких клиентских потоков
    for (int i = 0; i < num_clients; i++) {
        if (pthread_create(&clients[i], NULL, client_thread, NULL) != 0) {
            perror("pthread_create_error");
        }
    }

    // Ожидание завершения всех клиентских потоков
    for (int i = 0; i < num_clients; i++) {
        pthread_join(clients[i], NULL);
    }

    printf("All clients deactivated.\n");
    return 0;
}
