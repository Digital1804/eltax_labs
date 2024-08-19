#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>



#include <string.h>
#define MAX_NAME_LEN 16

#include "clientlib/clientlib.h"

int main() {
    // char name[MAX_NAME_LEN];
    // start_screen(name);
    sem_t *sem_hold = sem_open(SEM_HOLD, O_CREAT, 0666, 0);
    if (sem_hold == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    WINDOW *left_win, *right_win, *down_win;
    initscr();
    system("clear");  // Очистка экрана
    cbreak();  // Включаем режим cbreak (не буферизированный ввод)
    refresh();
    init_pairs();  // Инициализация цветовых пар
    curs_set(TRUE);  // Отключаем отображение курсора
    signal(SIGWINCH, signal_handler);  // Назначаем обработчик сигнала изменения размера окна
    // create_windows(left_win, right_win, down_win);  // Создаем окна интерфейса

    pthread_t write_thread;
    pthread_create(&write_thread, NULL, write_to_shm, down_win);

    pthread_t members_thread;
    pthread_create(&members_thread, NULL, members_control, right_win);

    pthread_t chat_thread;
    pthread_create(&chat_thread, NULL, chat_control, left_win);
    
    pthread_join(write_thread, NULL);
    pthread_join(members_thread, NULL);
    pthread_join(chat_thread, NULL);
    pthread_cancel(write_thread);
    pthread_cancel(members_thread);
    pthread_cancel(chat_thread);
    delwin(left_win);
    delwin(right_win);
    delwin(down_win);
    endwin();
    return 0;
}
