#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2000
#define BUFFER_SIZE 256

int main() {
    int server_socket;
    struct sockaddr_in server, client;
    char buffer[BUFFER_SIZE];
    socklen_t client_size;

    // Создание сокета
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("bind_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server wait message in IP:PORT %s:%d...\n", SERVER_IP, ntohs(SERVER_PORT));


    // Получение данных от клиента
    client_size = sizeof(client);
    int bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client, &client_size);
    if (bytes_received == -1) {
        perror("recvfrom_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Received message %s from IP:PORT %s:%d\n", buffer, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    // Отправка ответа клиенту
    strncpy(buffer, "Hello!", BUFFER_SIZE) ;
    printf("Send message: %s\n", buffer);
    if (sendto(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, client_size) == -1) {
        perror("sendto_error");
    }

    // Закрытие сокета
    close(server_socket);

    return 0;
}
