#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

int main() {
    int server_socket;
    struct sockaddr_in multicast;
    char buffer[BUFFER_SIZE] = "Hello, this is a multicast message!";

    // Создание UDP сокета
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для мультикаст рассылки
    memset(&multicast, 0, sizeof(multicast));
    multicast.sin_family = AF_INET;
    multicast.sin_port = htons(PORT);
    multicast.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);

    // Цикл отправки сообщений
    while (1) {
        if (sendto(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&multicast, sizeof(multicast)) < 0) {
            perror("sendto_error");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        printf("Send message: %s\n", buffer);
        sleep(2); // Пауза перед следующей отправкой
    }

    close(server_socket);
    return 0;
}
