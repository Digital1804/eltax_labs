#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
char running = 1;
void signal_handler(int signal) {
    printf("Server stopped.\n");
    running = 0;
    exit(0);
}

// Функция для обработки подключения клиента
void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    char buffer[BUFFER_SIZE] = "";
    int bytes_read;

    // Отправка приветственного сообщения клиенту
    const char *welcome_message = "Welcome to server!";
    send(client_socket, welcome_message, strlen(welcome_message), 0);

    // Чтение данных от клиента
    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0'; // Завершение строки
        printf("Recieved message: %s\n", buffer);
        // Эхо-ответ клиенту
        send(client_socket, buffer, bytes_read, 0);
    }

    // Закрытие сокета клиента
    close(client_socket);
    printf("Client disconnected.\n");
    free(client_socket_ptr);
    return NULL;
}

int main() {
    struct sigaction sa;// Структура для настройки сигнала
    sa.sa_handler = signal_handler; // Установка функции-обработчика
    sa.sa_flags = 0; // Нет дополнительных флагов
    sigemptyset(&sa.sa_mask); // Пустая маска блокировки

    // Установка обработчика сигнала SIGUSR1
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("set_error");
        exit(EXIT_FAILURE);
    }
    int server_socket, *client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;
    close(server_socket);
    // Создание серверного сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket_error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Принимать подключения от любого IP
    server_addr.sin_port = htons(0); 
    // Привязка сокета к адресу и порту
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Перевод сервера в режим ожидания подключений
    if (listen(server_socket, 10) < 0) {
        perror("listen_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    socklen_t len = sizeof(server_addr);
    if (getsockname(server_socket, (struct sockaddr *)&server_addr, &len) == -1) {
        perror("getsockname_error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d...\n", ntohs(server_addr.sin_port));
    


    // Основной цикл ожидания и обработки подключений
    while (running) {
        client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("accept_error");
            free(client_socket);
        }

        // Создание нового потока для обработки клиента
        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("pthread_create_error");
            free(client_socket);
        }

        // Отсоединение потока для автоматического освобождения ресурсов при завершении
        pthread_detach(thread_id);
    }

    // Закрытие серверного сокета (эта строка фактически недостижима)
    close(server_socket);

    return 0;
}
