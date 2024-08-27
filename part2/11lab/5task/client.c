#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// Размер буфера для приема пакетов
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2000
#define CLIENT_PORT 3000

int main() {
    int client_socket;
    struct sockaddr_in server;
    socklen_t server_size = sizeof(server);
    // Создание raw-сокета
    client_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (client_socket < 0) {
        perror("socket_error");
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    char buffer[BUFFER_SIZE];

    // Заполняем UDP заголовок
    memset(buffer, 0, BUFFER_SIZE);
    struct udphdr *udp_header = (struct udphdr *)buffer;
    udp_header->source = htons(CLIENT_PORT);
    udp_header->dest = htons(SERVER_PORT);
    udp_header->len = htons(sizeof(struct udphdr) + strlen("Hello, server!"));
    udp_header->check = 0;
    strncpy(buffer + sizeof(struct udphdr), "Hello, server!", strlen("Hello, server!"));

    if (sendto(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server, server_size) < 0) {
        perror("sendto_error");
        return 1;
    }
    printf("Send message to server: %s\n", buffer + sizeof(struct udphdr));
    do{
        if (recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server, &server_size) < 0) {
            perror("recvfrom_error");
            return 1;
        }
        udp_header = (struct udphdr *)(buffer + sizeof(struct iphdr));
    } while (ntohs(udp_header->dest) != CLIENT_PORT);

    printf("Receive message from server %s\n", buffer + sizeof(struct udphdr) + sizeof(struct iphdr));
    close(client_socket);
    return 0;
}
