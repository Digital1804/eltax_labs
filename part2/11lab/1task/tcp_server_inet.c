#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2000
#define BUFFER_SIZE 256

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server, client;
    char buffer[BUFFER_SIZE];
    
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {// Создание сокета
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {// Привязка сокета к адресу
        perror("bind_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 1) == -1) {// Ожидание соединений
        perror("listen_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server wait connect...\n");

    socklen_t client_size = sizeof(client);
    if ((client_socket = accept(server_socket, (struct sockaddr*)&client, &client_size)) == -1) {// Принятие соединения
        perror("Ошибка принятия соединения");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Client connected at IP:PORT %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    

    // Получение данных от клиента
    if (read(client_socket, buffer, BUFFER_SIZE - 1) == -1) {// Получение данных от клиента
        perror("read_error");
        close(client_socket);
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Received message: %s\n", buffer);
    memset(buffer, 0, BUFFER_SIZE);
    strncpy(buffer, "Hello!", BUFFER_SIZE);
    if (send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
        perror("write_error");
        close(client_socket);
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Send message: %s\n", buffer);

    // Закрытие соединения и сокета
    close(client_socket);
    close(server_socket);

    return 0;
}
