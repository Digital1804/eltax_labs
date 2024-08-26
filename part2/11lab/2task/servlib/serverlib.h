#ifndef _SERVER_H_
#define _SERVER_H_

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define SERVER_PORT 7777

/** @brief 
 *  @param int:client_socket Сокет клиента
 *  @param sockaddr_in:client Структура с информацией о клиенте
*/
typedef struct {
    int client_socket;
    struct sockaddr_in client;
} client_data_t;

/** @brief Функция для обработки подключения клиента для сервера создающего потоки в момент подключения клиента
 *  @param client_socket_ptr Указатель на сокет клиента
 *  @return NULL
*/
void *handle_create_client(void *client_socket_ptr);

/** @brief Функция для обработки подключения клиента для сервера создающего потоки до подключения клиентов
 *  @return NULL
*/
void *handle_queue_client(void *arg);

/** @brief Функция для обработки подключения клиента для сервера создающего потоки до подключения клиентов
 *  @return NULL
*/
void *handle_epoll_client(void *arg);

/** @brief Функция для запуска сервера создающего потоки в момент подключения клиента
 *  @return NULL
*/
void start_server_create();

/** @brief Функция для запуска сервера создающего потоки до подключения клиентов
 *  @param pool_size Размер пула потоков
 *  @return NULL
*/
void start_server_queue(const int pool_size);

/** @brief Функция для запуска сервера создающего потоки до подключения клиентов
 *  @param pool_size Размер пула потоков
 *  @return NULL
*/
void start_server_epoll(const int pool_size);


#endif