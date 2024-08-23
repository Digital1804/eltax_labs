#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>


#include "clientlib/clientlib.h"

int main() {
    initscr();
    system("clear");  // Очистка экрана
    nocbreak();  // Включаем режим cbreak (не буферизированный ввод)
    refresh();
    init_pairs();  // Инициализация цветовых пар
    curs_set(TRUE);  // Отключаем отображение курсора
    client_UI client;
    start_screen(&client);
    create_semaphores(&client);
    sem_t *sem_chat = sem_open(client.sem_chat, O_CREAT, 0644, 1);
    sem_t *sem_members = sem_open(client.sem_members, O_CREAT, 0644, 1);
    sem_t *win_hold = sem_open(client.win_hold, O_CREAT, 0644, 1);
    int height, width;
    getmaxyx(stdscr, height, width);
    create_windows(&client);  // Создаем окна интерфейса
    sem_post(win_hold);
    signal(SIGWINCH, signal_handler);  // Назначаем обработчик сигнала изменения размера окна

    pthread_t write_thread;
    pthread_create(&write_thread, NULL, write_to_shm, &client);

    pthread_t members_thread;
    pthread_create(&members_thread, NULL, members_control, &client);

    pthread_t chat_thread;
    pthread_create(&chat_thread, NULL, chat_control, &client);
    while (1){
        if (client.running == 0){
            // pthread_join(write_thread, NULL);
            // pthread_join(members_thread, NULL);
            // pthread_join(chat_thread, NULL);
            // pthread_cancel(write_thread);
            // pthread_cancel(members_thread);
            // pthread_cancel(chat_thread);
            delwin(client.left_win);
            delwin(client.right_win);
            delwin(client.down_win);
            endwin();
            sem_unlink(client.win_hold);
            sem_unlink(client.sem_chat);
            sem_unlink(client.sem_members);
            exit(0);
        }
    }
    // sem_close(client.win_hold);
    // snprintf(sem_hold_name, 32, "/sem_hold_%s", client.name);
    // sem_unlink(sem_hold_name);
    return 0;
}
