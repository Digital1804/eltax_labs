#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

int main() {
    int client_socket;
    struct sockaddr_in multicast;
    struct ip_mreq multicast_request;
    char buffer[BUFFER_SIZE] = "Hello, this is a broadcast message!";

    // Создание UDP сокета
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса клиента для прослушивания
    memset(&multicast, 0, sizeof(multicast));
    multicast.sin_family = AF_INET;
    multicast.sin_port = htons(PORT);
    multicast.sin_addr.s_addr = htonl(INADDR_ANY);

    // Привязка сокета к локальному адресу
    if (bind(client_socket, (struct sockaddr *)&multicast, sizeof(multicast)) < 0) {
        perror("bind_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Настройка параметров мультикаста
    multicast_request.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_request.imr_interface.s_addr = htonl(INADDR_ANY);

    // Присоединение к мультикаст-группе
    if (setsockopt(client_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) < 0) {
        perror("setsockopt_in_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Wait multicast message...\n");

    // Цикл приема сообщений
    while (1) {
        if (recvfrom(client_socket, buffer, BUFFER_SIZE - 1, 0, NULL, NULL) < 0) {
            perror("recvfrom_error");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        printf("Recieve message: %s\n", buffer);
    }

    // Выход из мультикаст-группы перед завершением
    if (setsockopt(client_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) < 0) {
        perror("setsockopt_exit_error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    close(client_socket);
    return 0;
}
