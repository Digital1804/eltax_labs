#include <ncurses.h>
#include <fcntl.h>
#include <semaphore.h>

#ifndef CLIENT_H
#define CLIENT_H

#define SHM_MSG "/shm_msg"
#define SEM_TEXT "/sem_msg"
#define SHM_MEMBERS "/shm_members"
#define SEM_MEMBERS "/sem_members"
#define SHM_CHAT "/shm_chat"
#define SEM_CHAT "/sem_chat"
#define SEM_HOLD "/sem_hold"
#define MAX_SIZE 237
#define MAX_NAME_LEN 16
#define MAX_CLIENTS 10
#define MAX_LINES 50
#define TEXT 2
#define NAME 1
#define EXIT 0

/** @brief Структура клиента
    @param client_UI::left_win окно чата
    @param client_UI::right_win окно вывода  чата
    @param client_UI::down_win окно ввода
    @param client_UI::name Имя клиента
    @param client_UI::win_hold Семафор для блокировки окон
 */
typedef struct client_UI
{
    WINDOW *left_win;
    WINDOW *right_win;
    WINDOW *down_win;
    char name[MAX_NAME_LEN];
    int max_chat_lines;
    bool running;
    char win_hold[MAX_NAME_LEN+10];
    // char sem_text[MAX_NAME_LEN+10];
    char sem_members[MAX_NAME_LEN+10];
    char sem_chat[MAX_NAME_LEN+10];
}client_UI;

/** @brief Структура сообщения
    @param message_t::type Тип сообщения
    @param message_t::client_name Имя клиента
    @param message_t::text Текст сообщения
*/
typedef struct {
    long type;
    // char client_name[MAX_NAME_LEN];
    client_UI client;
    char text[MAX_SIZE];
} message_t;

/** @brief Структура списка участников чата
    @param members_t::names Массив имен участников
    @param members_t::count Количество участников
*/
typedef struct {
    client_UI clients[MAX_CLIENTS];
    // char names[MAX_CLIENTS][MAX_NAME_LEN];
    int count;
} members_t;

/** @brief Структура чата
    @param chat_t::text Массив сообщений чата
    @param chat_t::count Количество сообщений в чате
*/
typedef struct {
    char text[MAX_LINES][MAX_SIZE+MAX_NAME_LEN+3];
    int count;
    int max_chat_lines;
} chat_t;

/** @brief Структура для хранения строки чата
    @param chat_line::text Текст сообщения
    @param chat_line::name Имя отправителя
*/
typedef struct chat_line{
    char *text;
    char *name;
} chat_line;

void *write_to_shm(void *arg);
void *members_control(void *arg);
void *chat_control(void *arg);
void print_members(client_UI *client, members_t *members_ptr);
void print_chat(client_UI *client, chat_t *chat_ptr);
void create_semaphores(client_UI *client);
void create_windows(client_UI *client);
void init_pairs();
void signal_handler(int sig);
void start_screen(client_UI *client);

#endif