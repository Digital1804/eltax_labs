#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// Размер буфера для приема пакетов
#define BUFFER_SIZE 65536

int main() {
    int raw_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    // Создание raw-сокета
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_socket < 0) {
        perror("socket_error");
        return 1;
    }

    printf("Wait UDP packets...\n");

    while (1) {
        // Получение пакета
        if (recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&source_addr, &addr_len) < 0) {
            perror("recvfrom_error");
            close(raw_socket);
            return 1;
        }
        printf("Recieved UDP packet from %s:%d\n", inet_ntoa(source_addr.sin_addr), ntohs(source_addr.sin_port));
    }

    close(raw_socket);
    return 0;
}
