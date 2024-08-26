#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

int main() {
    int client_socket;
    struct sockaddr_in client_addr;
    char buffer[BUFFER_SIZE] = "Hello, this is a broadcast message!";

    // Создание UDP сокета
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса клиента для прослушивания
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    // Привязка сокета к адресу и порту
    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Wait broadcast message...\n");

    // Цикл приема сообщений
    while (1) {
        if (recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL) < 0) {
            perror("recvfrom_error");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        printf("Recieved message: %s\n", buffer);
    }

    close(client_socket);
    return 0;
}
