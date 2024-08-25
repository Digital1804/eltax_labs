#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define CLIENT_PATH "/tmp/udp_client_local"
#define SERVER_PATH "/tmp/udp_server_local"
#define BUFFER_SIZE 256

int main() {
    int client_socket;
    struct sockaddr_un server, client;

    // Создание сокета
    if ((client_socket = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса клиента
    memset(&client, 0, sizeof(client));
    client.sun_family = AF_LOCAL;
    strncpy(client.sun_path, CLIENT_PATH, sizeof(client.sun_path) - 1);

    // Привязка сокета к адресу клиента
    unlink(CLIENT_PATH); // Удаление предыдущего файла сокета, если он существует
    if (bind(client_socket, (struct sockaddr *)&client, sizeof(client)) == -1) {
        perror("connect_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    strncpy(server.sun_path, SERVER_PATH, sizeof(server.sun_path) - 1);

    // Отправка данных серверу
    char buffer[BUFFER_SIZE] = "Hi!";
    printf("Send message: %s\n", buffer);
    if (sendto(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("sendto_error");
        close(client_socket);
        unlink(CLIENT_PATH);
        exit(EXIT_FAILURE);
    }

    // Получение ответа от сервера
    if (recvfrom(client_socket, buffer, BUFFER_SIZE - 1, 0, NULL, NULL) == -1) {
        perror("recvfrom_error");
        close(client_socket);
        unlink(CLIENT_PATH);
        exit(EXIT_FAILURE);
    }
    printf("Recieved message: %s\n", buffer);

    // Закрытие сокета
    close(client_socket);
    unlink(CLIENT_PATH);

    return 0;
}
