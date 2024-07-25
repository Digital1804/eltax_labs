#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <pthread.h>
#include "info.h"

WINDOW *left_win, *right_win, *down_win;
bool running = TRUE;
mqd_t mq;

void resize_windows(){
    int height, width;
    char text[3][MAX_SIZE] = {"", "", ""};
    getmaxyx(stdscr, height, width);
    if (left_win != NULL)   delwin(left_win);
    // else    winnstr(left_win, text[0], MAX_SIZE);
    if (right_win != NULL)  delwin(right_win);
    // else    winnstr(right_win, text[1], MAX_SIZE);
    if (down_win != NULL)   delwin(down_win);
    left_win = newwin(height*0.75, width*0.8, 0, 0);
    right_win = newwin(height*0.75, width*0.2, 0, width*0.8);
    down_win = newwin(height-height*0.75, width, height*0.75, 0);
    box(left_win, 0, 0);
    box(right_win, 0, 0);
    box(down_win, 0, 0);
    wbkgd(left_win, COLOR_PAIR(1));
    wbkgd(right_win, COLOR_PAIR(2));
    wbkgd(down_win, COLOR_PAIR(3));
    wmove(left_win, 1, 1);
    wprintw(left_win, "%s", text[0]);
    wmove(right_win, 1, 1);
    wprintw(right_win, "%s", text[1]);
    wrefresh(left_win);
    wrefresh(right_win);
    wrefresh(down_win);
}

void init_pairs(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
}

void name_screen(char name[], mqd_t mq){
    WINDOW * wnd;
    message_t msg;
    msg.type = NAME;
    // strncpy(" ", msg.text, MAX_SIZE);
    initscr();
    curs_set(TRUE);
    refresh();
    wnd = newwin(5, 23, 2, 2);
    wbkgd(wnd, COLOR_PAIR(1));
    wattron(wnd, A_BOLD);
    wprintw(wnd, "Enter your name...\n");
    wgetnstr(wnd, name, MAX_NAME_LEN);
    name[MAX_NAME_LEN] = 0;
    strncpy(msg.client_name, name, MAX_NAME_LEN+1);
    wrefresh(wnd);
    delwin(wnd);
    curs_set(FALSE);
    endwin();
    if (mq_send(mq, (char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
        exit(1);
    }
}

void *receive_messages(void *arg) {
    while (running) {
        message_t msg;
        if (mq_receive(mq, (char *)&msg, sizeof(msg), NULL) == -1) {
            
            perror("mq_receive");
            exit(1);
        }
        else{
            switch (msg.type){
                case CHAT:
                    break;
                case MEMBERS:
                    wclear(right_win);
                    box(right_win, 0, 0);
                    wmove(right_win, 1, 1);
                    wprintw(right_win, "%s", msg.text);
                    wrefresh(right_win);
                    refresh();
                    break;
                default:
                    break;
            }
        }
    }
    return NULL;
}

void main() {

    char name[MAX_NAME_LEN + 1];
    // Очистка буфера
    memset(name, 0, sizeof(name));

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;
    // Подключение к существующей очереди сообщений
    mq = mq_open(QUEUE_NAME, O_RDWR);
    if (mq == -1) {
        perror("mq_open");
        exit(1);
    }
    // Закрытие очереди
    name_screen(name, mq);
    initscr();
    clear();
    refresh();
    init_pairs();
    resize_windows();
    cbreak();
    curs_set(FALSE);
    // signal(SIGWINCH, sig_winch);
    wmove(down_win, 1, 1);
    keypad(stdscr, TRUE);
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, receive_messages, NULL);
    message_t msg;
    while (1){
        wprintw(down_win, "%s: ", name);
        wgetnstr(down_win, msg.text, MAX_SIZE);
        strncpy(msg.client_name, name, MAX_NAME_LEN+1);
        if ((strcmp(msg.text, "exit") == 0) || (strcmp(msg.text, "e") == 0)){
            msg.type = QUIT;
            if (mq_send(mq, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                exit(1);
            }
            endwin();
            exit(0);
        }
        msg.type = TEXT;
        if (strncmp(msg.text, "", 5) != 0)
            if (mq_send(mq, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                exit(1);
            }
        memset(msg.text, 0, sizeof(msg.text));
        wclear(down_win);
        resize_windows();
        wmove(down_win, 1, 1);
        wrefresh(down_win);
    }
}