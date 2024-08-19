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

void start_screen(char name[], message_t *msg_ptr){
    sem_t *sem;
    sem = sem_open(SEM_TEXT, 0);
    if (sem == SEM_FAILED) {
        endwin();
        perror("sem_open");
        exit(1);
    }
    WINDOW *wnd;
    initscr();
    curs_set(TRUE);
    refresh();

    // Создаем окно для ввода имени
    wnd = newwin(5, 23, 2, 2);
    wbkgd(wnd, COLOR_PAIR(1));
    wattron(wnd, A_BOLD);
    wprintw(wnd, "Enter your name...\n");
    wgetnstr(wnd, name, MAX_NAME_LEN);  // Получаем имя пользователя
    if (strlen(name) == 0) {
        sem_close(sem);
        sem_unlink(SEM_TEXT);
        start_screen(name, msg_ptr);  // Если имя пустое, повторяем ввод
        return;
    }
    msg_ptr->type = NAME;
    strncpy(msg_ptr->client_name, name, MAX_NAME_LEN);
    for (int i = 0; i < 3; i++) {
        sem_post(sem);
    }
    sem = sem_open(SEM_HOLD, 0);
    if (sem == SEM_FAILED) {
        endwin();
        perror("sem_open");
        exit(1);
    }
    wrefresh(wnd);
    delwin(wnd);
    endwin();
    refresh();
    sem_post(sem);
    sem_close(sem);
    
    return;
}

/** @brief Функция для записи сообщений в разделяемую память
    @param arg - не используется
    @return NULL
*/
void *write_to_shm(void *arg) {
    WINDOW *down_win = (WINDOW *)arg;
    sem_t *sem_text, *sem_hold;
    message_t *msg_ptr;
    int msg_fd;
    msg_fd = shm_open(SHM_MSG, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (msg_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    msg_ptr = mmap(0, sizeof(message_t), PROT_WRITE, MAP_SHARED, msg_fd, 0);// Мапируем сегмент в адресное пространство
    if (msg_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_text = sem_open(SEM_TEXT, 0);
    if (sem_text == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_hold = sem_open(SEM_HOLD, 0);
    if (sem_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    start_screen(msg_ptr->client_name, msg_ptr);
    sem_wait(sem_hold);
    printf("write\n");
    sleep(3);
    sem_close(sem_hold);
    
    msg_ptr->type = TEXT;
    while (1) {;
        print_name(down_win, "Your message-> ");
        wgetnstr(down_win, msg_ptr->text, MAX_SIZE);
        // msg_ptr->text[strcspn(msg_ptr->text, "\n")] = 0;
        sem_post(sem_text);
        if (strcmp(msg_ptr->text, "exit") == 0) {
            running = false;
            break;
        }
    }
    running = false;
    munmap(msg_ptr, sizeof(message_t));
    shm_unlink(SHM_MSG);
    sem_close(sem_text);
    sem_unlink(SEM_TEXT);
    return NULL;
}

/** @brief Функция для отслеживания списка участников чата
    @param arg - не используется
    @return NULL
*/
void *members_control(void *arg) {
    WINDOW *right_win = (WINDOW *)arg;
    int members_fd;
    members_fd = shm_open(SHM_MEMBERS, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (members_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    members_t *members_ptr;
    members_ptr = mmap(0, sizeof(members_t), PROT_READ, MAP_SHARED, members_fd, 0);// Мапируем сегмент в адресное пространство
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
    sem_t *sem_hold = sem_open(SEM_HOLD, 0);
    if (sem_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_wait(sem_hold);
    printf("mem\n");
    sem_close(sem_hold);
    
    while (running) {
        sem_wait(sem_members);
        print_members(right_win, members_ptr);
    }
    munmap(members_ptr, sizeof(members_t));
    shm_unlink(SHM_MEMBERS);
    sem_close(sem_members);
    sem_unlink(SEM_MEMBERS);
    return NULL;
}

/** @brief Функция для отслеживания сообщений чата
    @param arg - не используется
    @return NULL
*/
void *chat_control(void *arg) {
    WINDOW *left_win = (WINDOW *)arg;
    int chat_fd;
    chat_fd = shm_open(SHM_CHAT, O_RDWR, 0666);
    if (chat_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    chat_t *chat_ptr;
    chat_ptr = mmap(0, sizeof(chat_t), PROT_READ, MAP_SHARED, chat_fd, 0);
    if (chat_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_chat;
    sem_chat = sem_open(SEM_CHAT, 0);
    if (sem_chat == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_t *sem_hold = sem_open(SEM_HOLD, 0);
    if (sem_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_wait(sem_hold);
    printf("chat\n");
    sem_close(sem_hold);
    
    while (running) {
        sem_wait(sem_chat);
        print_chat(left_win, chat_ptr, getmaxy(left_win));
    }
    munmap(chat_ptr, sizeof(chat_t));
    shm_unlink(SHM_CHAT);
    sem_close(sem_chat);
    sem_unlink(SEM_CHAT);
    return NULL;
}
