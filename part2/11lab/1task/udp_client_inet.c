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
    int client_socket;
    struct sockaddr_in server;

    // Создание сокета
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса клиента
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP); 

    // Отправка данных серверу
    char buffer[BUFFER_SIZE] = "Hi!";
    printf("Send message: %s\n", buffer);
    if (sendto(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("sendto_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Получение ответа от сервера
    int bytes_received = recvfrom(client_socket, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
    if (bytes_received == -1) {
        perror("recvfrom_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Recieved message: %s\n", buffer);

    // Закрытие сокета
    close(client_socket);

    return 0;
}
