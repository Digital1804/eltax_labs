#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PATH "/tmp/udp_server_local"
#define BUFFER_SIZE 256

int main() {
    int server_socket;
    struct sockaddr_un server, client;
    char buffer[BUFFER_SIZE];
    socklen_t client_size;

    // Создание сокета
    if ((server_socket = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    strncpy(server.sun_path, SERVER_PATH, sizeof(server.sun_path) - 1);

    // Привязка сокета к адресу
    unlink(SERVER_PATH); // Удаление предыдущего файла сокета, если он существует
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("bind_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server wait message...\n");

    // Получение данных от клиента
    client_size = sizeof(client);
    if (recvfrom(server_socket, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client, &client_size) == -1) {
        perror("recvfrom_error");
        close(server_socket);
        unlink(SERVER_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Recieved message: %s\n", buffer);
    // Отправка ответа клиенту
    memset(buffer, 0, BUFFER_SIZE);
    strncpy(buffer, "Hello!", BUFFER_SIZE) ;
    printf("Send message: %s\n", buffer);
    if (sendto(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, client_size) == -1) {
        perror("sendto_error");
    }

    // Закрытие сокета
    close(server_socket);
    unlink(SERVER_PATH);

    return 0;
}
