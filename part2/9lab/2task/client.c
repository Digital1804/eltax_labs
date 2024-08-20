#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>


#include "clientlib/clientlib.h"


int main() {
    initscr();
    system("clear");  // Очистка экрана
    cbreak();  // Включаем режим cbreak (не буферизированный ввод)
    refresh();
    init_pairs();  // Инициализация цветовых пар
    curs_set(TRUE);  // Отключаем отображение курсора
    // char name[MAX_NAME_LEN];
    // start_screen(name);
    
    client_UI client;
    client.win_hold = sem_open(SEM_HOLD, O_CREAT, 0644, 0);
    
    start_screen(&client);
    printf("Hello!\n");
    int height, width;
    getmaxyx(stdscr, height, width);
    create_windows(&client);  // Создаем окна интерфейса
    signal(SIGWINCH, signal_handler);  // Назначаем обработчик сигнала изменения размера окна

    pthread_t write_thread;
    pthread_create(&write_thread, NULL, write_to_shm, &client);

    pthread_t members_thread;
    pthread_create(&members_thread, NULL, members_control, &client);

    pthread_t chat_thread;
    pthread_create(&chat_thread, NULL, chat_control, &client);
    
    pthread_join(write_thread, NULL);
    pthread_join(members_thread, NULL);
    pthread_join(chat_thread, NULL);
    pthread_cancel(write_thread);
    pthread_cancel(members_thread);
    pthread_cancel(chat_thread);
    return 0;
}
