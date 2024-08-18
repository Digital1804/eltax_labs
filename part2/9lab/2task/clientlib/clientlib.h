#include <ncurses.h>
#include <fcntl.h>

#ifndef CLIENT_H
#define CLIENT_H

#define SHM_MSG "/shm_msg"
#define SEM_TEXT "/sem_msg"
#define SHM_MEMBERS "/shm_members"
#define SEM_MEMBERS "/sem_members"
#define SHM_CHAT "/shm_chat"
#define SEM_CHAT "/sem_chat"
#define MAX_SIZE 237
#define MAX_NAME_LEN 16
#define MAX_CLIENTS 10
#define MAX_LINES 50
#define TEXT 2
#define NAME 1

/** @brief Структура сообщения
    @param message_t::type Тип сообщения
    @param message_t::client_name Имя клиента
    @param message_t::text Текст сообщения
*/
typedef struct {
    long type;
    char client_name[MAX_NAME_LEN];
    char text[MAX_SIZE];
} message_t;

/** @brief Структура списка участников чата
    @param members_t::names Массив имен участников
    @param members_t::count Количество участников
*/
typedef struct {
    char names[MAX_CLIENTS][MAX_NAME_LEN];
    int count;
} members_t;

/** @brief Структура чата
    @param chat_t::text Массив сообщений чата
    @param chat_t::count Количество сообщений в чате
*/
typedef struct {
    char text[MAX_LINES][MAX_SIZE+MAX_NAME_LEN+3];
    int count;
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
void print_name(WINDOW *down_win, char text[]);
void print_members(WINDOW *right_win, members_t *members);
void print_chat(WINDOW *left_win, chat_t *chat, int chat_line_count);
int create_windows(WINDOW *down_win, WINDOW *left_win, WINDOW *right_win);
void init_pairs();
void signal_handler(int sig);

#endif