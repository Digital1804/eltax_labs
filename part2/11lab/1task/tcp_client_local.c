#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "/tmp/tcp_local"
#define BUFFER_SIZE 256

int main() {
    int client_socket;
    struct sockaddr_un server;

    // Создание сокета
    if ((client_socket = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    strncpy(server.sun_path, SOCKET_PATH, sizeof(server.sun_path) - 1);

    printf("Connecting to server...\n");
    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connect_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected.\n");

    // Отправка данных серверу
    char buffer[BUFFER_SIZE] = "Hi!";
    printf("Send message: %s\n", buffer);
    if (send(client_socket, buffer, BUFFER_SIZE, 0) == -1) {
        perror("write_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Получение ответа от сервера
    if (read(client_socket, buffer, BUFFER_SIZE) == -1) {
        perror("read_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Recieved message: %s\n", buffer);

    // Закрытие сокета
    close(client_socket);

    return 0;
}
