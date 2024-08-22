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
void create_semaphores(client_UI *client){
    snprintf(client->win_hold, sizeof(client->win_hold), "/sem_hold_%s", client->name);
    // snprintf(client->sem_text, sizeof(client->sem_text), "/sem_text_%s", client->name);
    snprintf(client->sem_members, sizeof(client->sem_members), "/sem_memb_%s", client->name);
    snprintf(client->sem_chat, sizeof(client->sem_chat), "/sem_chat_%s", client->name);
}

/** @brief Функция для записи сообщений в разделяемую память
    @param arg - не используется
    @return NULL
*/
void *write_to_shm(void *arg) {
    client_UI *client = (client_UI *)arg;
    int msg_fd = shm_open(SHM_MSG, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (msg_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    message_t *msg_ptr = mmap(0, sizeof(message_t), PROT_READ |  PROT_WRITE, MAP_SHARED, msg_fd, 0);// Мапируем сегмент в адресное пространство
    if (msg_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    msg_ptr->type = NAME;
    msg_ptr->client = *client;
    sem_t *sem_text = sem_open(SEM_TEXT, 0);
    if (sem_text == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_t *win_hold = sem_open(client->win_hold, 0);
    if (win_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    sem_post(sem_text);
    while (client->running) {;
        sem_wait(win_hold);
        wclear(client->down_win);
        wbkgd(client->down_win, COLOR_PAIR(3));
        box(client->down_win, 0, 0);
        wmove(client->down_win, 1, 1);
        curs_set(TRUE);
        sleep(1);
        mvwprintw(client->down_win, 1, 1, "%s: ", client->name);
        wrefresh(client->down_win);
        wgetnstr(client->down_win, msg_ptr->text, MAX_SIZE);
        curs_set(FALSE);
        if (strcmp(msg_ptr->text, "exit") == 0) {
            client->running = 0;
            msg_ptr->client = *client;
            msg_ptr->type = EXIT;
            sem_post(sem_text);
            sleep(1);
            sem_post(win_hold);
            break;
        }
        msg_ptr->type = TEXT;
        msg_ptr->client = *client;
        sem_post(win_hold);
        sem_post(sem_text);
    }
    client->running = 0;
    munmap(msg_ptr, sizeof(message_t));
    close(msg_fd);
    sem_close(sem_text);
    return NULL;
}

/** @brief Функция для отслеживания списка участников чата
    @param arg - не используется
    @return NULL
*/
void *members_control(void *arg) {
    client_UI *client = (client_UI *)arg;
    int members_fd = shm_open(SHM_MEMBERS, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (members_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    members_t *members_ptr = mmap(0, sizeof(members_t), PROT_READ |  PROT_WRITE, MAP_SHARED, members_fd, 0);// Мапируем сегмент в адресное пространство
    if (members_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_members = sem_open(client->sem_members, 0);
    if (sem_members == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_t *win_hold = sem_open(client->win_hold, 0);
    if (win_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    while (client->running) {
        sem_wait(sem_members);
        sem_wait(win_hold);
        print_members(client, members_ptr);
        sem_post(win_hold);
    }
    munmap(members_ptr, sizeof(members_t));
    close(members_fd);
    return NULL;
}

/** @brief Функция для отслеживания сообщений чата
    @param arg - не используется
    @return NULL
*/
void *chat_control(void *arg) {
    client_UI *client = (client_UI *)arg;
    int chat_fd = shm_open(SHM_CHAT, O_RDWR, 0666);
    if (chat_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    chat_t *chat_ptr = mmap(0, sizeof(chat_t), PROT_READ |  PROT_WRITE, MAP_SHARED, chat_fd, 0);
    if (chat_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_chat = sem_open(client->sem_chat, 0);
    if (sem_chat == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_t *win_hold = sem_open(client->win_hold, 0);
    if (win_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    while (client->running) {
        sem_wait(sem_chat);
        sem_wait(win_hold);
        // print_chat(client, chat_ptr);

        wclear(client->left_win);
        box(client->left_win, 0, 0);
        chat_ptr->max_chat_lines = client->max_chat_lines;
        int start = (chat_ptr->count > client->max_chat_lines) ? chat_ptr->count - client->max_chat_lines : 0;
        for (int i = start; i < chat_ptr->count; i++) {
            mvwprintw(client->left_win, i - start + 1, 1, "%s", chat_ptr->text[i]);
        }
        wrefresh(client->left_win);
        sem_post(win_hold);
    }
    munmap(chat_ptr, sizeof(chat_t));
    close(chat_fd);
    return NULL;
}
