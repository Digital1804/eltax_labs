#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>

// Размер буфера для приема пакетов
#define BUFFER_SIZE 1024
#define CLIENT_IP "192.168.0.3"
#define SERVER_IP "192.168.0.5"
#define SERVER_PORT 2000
#define CLIENT_PORT 3000

void check_sum(struct iphdr *ip_header) {
    unsigned short *ptr = (unsigned short *)ip_header;
    int sum = 0;
    for (int i = 0; i < ip_header->ihl * 4; i++) {
        sum += *ptr++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ip_header->check = ~sum;
}

int main() {
    char name[10];
    int client_socket;
    struct sockaddr_ll server;
    socklen_t server_size = sizeof(server);
    // Создание raw-сокета
    client_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (client_socket < 0) {
        perror("socket_error");
        return 1;
    }
    int flag = 1;
    if (setsockopt(client_socket, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(int)) < 0) {
        perror("setsockopt_error");
        return 1;
    }
    // Заполнение адреса сервера
    printf("Enter network interface name: ");
    fgets(name, sizeof(name), stdin);
    unsigned char src_mac[6] = {0x74, 0x56, 0x3c, 0x69, 0x1a, 0xc2}; // MAC-адрес источника
    unsigned char dst_mac[6] = {0xa5, 0x41, 0xf4, 0x37, 0x64, 0x6e}; // MAC-адрес сервера

    memset(&server, 0, sizeof(server));
    server.sll_family = AF_PACKET;
    server.sll_ifindex = if_nametoindex(name);
    server.sll_halen = 6 ;
    memcpy(server.sll_addr, dst_mac, 6);

    char buffer[BUFFER_SIZE];

    // Заполняем UDP заголовок
    memset(buffer, 0, BUFFER_SIZE);
    struct ethhdr *eth_header = (struct ethhdr *)buffer;
    memcpy(eth_header->h_source, src_mac, 6);
    memcpy(eth_header->h_dest, dst_mac, 6);


    struct iphdr *ip_header = (struct iphdr *)buffer + sizeof(struct ethhdr);
    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen("Hello, server!"));
    ip_header->id = 0;
    ip_header->frag_off = 0;
    ip_header->ttl = 255;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;
    ip_header->saddr = inet_addr(CLIENT_IP);
    ip_header->daddr = inet_addr(SERVER_IP);

    struct udphdr *udp_header = (struct udphdr *)(buffer + sizeof(struct iphdr) + sizeof(struct ethhdr));
    udp_header->source = htons(CLIENT_PORT);
    udp_header->dest = htons(SERVER_PORT);
    udp_header->len = htons(sizeof(struct udphdr) + strlen("Hello, server!"));
    udp_header->check = 0;
    strncpy(buffer + sizeof(struct udphdr) + sizeof(struct iphdr), "Hello, server!", strlen("Hello, server!"));
    check_sum(ip_header);
    if (sendto(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server, server_size) < 0) {
        perror("sendto_error");
        return 1;
    }
    printf("Send message to server: %s\n", buffer + sizeof(struct udphdr) + sizeof(struct iphdr)+ sizeof(struct ethhdr));
    do{
        if (recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server, &server_size) < 0) {
            perror("recvfrom_error");
            return 1;
        }
        udp_header = (struct udphdr *)(buffer + sizeof(struct iphdr) + sizeof(struct ethhdr));
    } while (ntohs(udp_header->dest) != CLIENT_PORT);

    printf("Receive message from server: %s\n", buffer + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr));
    close(client_socket);
    return 0;
}
