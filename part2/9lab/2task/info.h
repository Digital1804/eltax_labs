#ifndef INFO_H
#define INFO_H

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
    sem_t *win_hold;
    // sem_t *sem_text;
    // sem_t *sem_members;
    // sem_t *sem_chat;
}client_UI;

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

#endif