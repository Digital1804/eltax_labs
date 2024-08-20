#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>

#include "clientlib.h"

bool running = true;

/** @brief Функция для записи сообщений в разделяемую память
    @param arg - не используется
    @return NULL
*/
void *write_to_shm(void *arg) {
    client_UI *client = (client_UI *)arg;
    sem_t *sem_text;
    message_t *msg_ptr;
    int msg_fd;
    msg_fd = shm_open(SHM_MSG, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (msg_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    msg_ptr = mmap(0, sizeof(message_t), PROT_READ |  PROT_WRITE, MAP_SHARED, msg_fd, 0);// Мапируем сегмент в адресное пространство
    if (msg_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_text = sem_open(SEM_TEXT, 0);
    if (sem_text == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    msg_ptr->type = NAME;
    strcpy(msg_ptr->client_name, client->name);
    sem_post(sem_text);
    msg_ptr->type = TEXT;
    while (running) {;
        sem_wait(client->win_hold);
        wbkgd(client->down_win, COLOR_PAIR(3));
        box(client->down_win, 0, 0);
        sleep(1);
        mvwprintw(client->down_win, 1, 1, "%s: ", client->name);
        wgetnstr(client->down_win, msg_ptr->text, MAX_SIZE);
        wclear(client->down_win);
        wrefresh(client->down_win);
        sem_post(client->win_hold);
        sem_post(sem_text);
        if (strcmp(msg_ptr->text, "exit") == 0) {
            running = false;
            delwin(client->left_win);
            delwin(client->right_win);
            delwin(client->down_win);
            endwin();
            exit(0);
            break;
        }
    }
    running = false;
    munmap(msg_ptr, sizeof(message_t));
    return NULL;
}

/** @brief Функция для отслеживания списка участников чата
    @param arg - не используется
    @return NULL
*/
void *members_control(void *arg) {
    client_UI *client = (client_UI *)arg;
    int members_fd;
    members_fd = shm_open(SHM_MEMBERS, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (members_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    members_t *members_ptr;
    members_ptr = mmap(0, sizeof(members_t), PROT_READ |  PROT_WRITE, MAP_SHARED, members_fd, 0);// Мапируем сегмент в адресное пространство
    if (members_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_members;
    sem_members = sem_open(SEM_MEMBERS, 0); 
    if (sem_members == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    while (running) {
        sem_wait(sem_members);
        sem_wait(client->win_hold);
        wclear(client->right_win);
        wbkgd(client->right_win, COLOR_PAIR(2));
        box(client->right_win, 0, 0);
        mvwprintw(client->right_win, 1, 1, "Members:");
        for (int i = 0; i < members_ptr->count; i++) {
            wmove(client->right_win, i + 2, 1);
            wprintw(client->right_win, "%s", members_ptr->names[i]);
        }
        wrefresh(client->right_win);
        sem_post(client->win_hold);
    }
    munmap(members_ptr, sizeof(members_t));
    return NULL;
}

/** @brief Функция для отслеживания сообщений чата
    @param arg - не используется
    @return NULL
*/
void *chat_control(void *arg) {
    client_UI *client = (client_UI *)arg;
    int chat_fd;
    chat_fd = shm_open(SHM_CHAT, O_RDWR, 0666);
    if (chat_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    chat_t *chat_ptr;
    chat_ptr = mmap(0, sizeof(chat_t), PROT_READ |  PROT_WRITE, MAP_SHARED, chat_fd, 0);
    if (chat_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_chat = sem_open(SEM_CHAT, 0);
    if (sem_chat == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    while (running) {
        sem_wait(sem_chat);
        sem_wait(client->win_hold);
        wclear(client->left_win);
        box(client->left_win, 0, 0);
        chat_ptr->max_chat_lines = client->max_chat_lines;
        int start = (chat_ptr->count > client->max_chat_lines) ? chat_ptr->count - client->max_chat_lines : 0;
        for (int i = start; i < chat_ptr->count; i++) {
            mvwprintw(client->left_win, i - start + 1, 1, "%s", chat_ptr->text[i % client->max_chat_lines]);
        }
        wrefresh(client->left_win);
        sem_post(client->win_hold);
    }
    munmap(chat_ptr, sizeof(chat_t));
    return NULL;
}
