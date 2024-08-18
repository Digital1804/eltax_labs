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

#endif