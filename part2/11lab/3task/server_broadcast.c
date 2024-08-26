#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

int main() {
    int server_socket;
    struct sockaddr_in broadcast;
    int flag = 1;

    // Создание UDP сокета
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Установка разрешения на широковещательную отправку
    if (setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)) < 0) {
        perror("setsockopt_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для широковещательной рассылки
    memset(&broadcast, 0, sizeof(broadcast));
    broadcast.sin_family = AF_INET;
    broadcast.sin_port = htons(PORT);
    broadcast.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    char buffer[BUFFER_SIZE] = "Hello, this is a broadcast message!";
    // Цикл отправки сообщений
    while (1) {
        if (sendto(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&broadcast, sizeof(broadcast)) < 0) {
            perror("sendto_error");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        printf("Message send: %s\n", buffer);
        sleep(2); // Пауза перед следующей отправкой
    }

    close(server_socket);
    return 0;
}
