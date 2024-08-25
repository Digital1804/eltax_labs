#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/tcp_local"
#define BUFFER_SIZE 256

int main() {
    int server_socket, client_socket;
    struct sockaddr_un server;
    char buffer[BUFFER_SIZE];
    
    if ((server_socket = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {// Создание сокета
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    strncpy(server.sun_path, SOCKET_PATH, sizeof(server.sun_path) - 1);

    unlink(SOCKET_PATH); // Удаление предыдущего файла сокета, если он существует
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

    if ((client_socket = accept(server_socket, NULL, NULL)) == -1) {// Принятие соединения
        perror("Ошибка принятия соединения");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Получение данных от клиента
    if (recv(client_socket, buffer, BUFFER_SIZE - 1, 0) == -1) {// Получение данных от клиента
        perror("read_error");
        close(client_socket);
        close(server_socket);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }
    printf("Received message: %s\n", buffer);
    memset(buffer, 0, BUFFER_SIZE);
    strncpy(buffer, "Hello!", BUFFER_SIZE);
    if (send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
        perror("write_error");
        close(client_socket);
        close(server_socket);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }
    printf("Send message: %s\n", buffer);

    // Закрытие соединения и сокета
    close(client_socket);
    close(server_socket);
    unlink(SOCKET_PATH);

    return 0;
}
