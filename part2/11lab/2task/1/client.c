#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define NUM_CLIENTS 5 // Количество клиентов
int client_number = 0;
// Функция для каждого клиента
void *client_thread(void *arg) {
    int client_id = ++client_number;
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int server_port = *(int *)arg;

    // Создание клиентского сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket_error");
        pthread_exit(NULL);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
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

    printf("Client %d connected.\n", client_id);

    // Отправка сообщения серверу
    snprintf(buffer, BUFFER_SIZE, "Hello from client %d!", client_id);
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);

    // Получение ответа от сервера
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Client %d recieved message: %s\n", client_id, buffer);
    } else {
        printf("client %d didn't recieve message.\n", client_id);
    }

    // Закрытие клиентского сокета
    close(client_socket);
    printf("Client %d disconnected.\n", client_id);
    return NULL;
}

int main(int argc, char *argv[]) {
    int num_clients = NUM_CLIENTS, server_port = SERVER_PORT;
    if (argc > 1) {
        printf("Server port: %s\n", argv[1]);
        server_port = atoi(argv[1]);
        printf("Server port: %d\n", server_port);
    }
    if (argc > 2) {
        num_clients = atoi(argv[2]);
    }

    // Создание нескольких клиентских потоков
    pthread_t clients[num_clients];

    // Создание нескольких клиентских потоков
    for (int i = 0; i < num_clients; i++) {
        if (pthread_create(&clients[i], NULL, client_thread, &server_port) != 0) {
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
