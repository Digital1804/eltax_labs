#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define QUEUE_NAME  "/chat_queue"
#define MAX_SIZE 512
#define MSG_BUFFER_SIZE (MAX_SIZE + 10)
#define MAX_NAME_LEN 15
#define QUIT 0
#define NAME 1
#define TEXT 2
#define CHAT 3
#define MEMBERS 4

WINDOW *left_win, *right_win, *down_win;
int height, width;
char name[MAX_NAME_LEN + 1];

typedef struct {
    long type;
    char text[MAX_SIZE];
    char client_name[MAX_NAME_LEN+1];
} message_t;

void resize_windows(){
    getmaxyx(stdscr, height, width);
    if (left_win != NULL) {
        delwin(left_win);
    }
    if (right_win != NULL) {
        delwin(right_win);
    }
    if (down_win != NULL) {
        delwin(right_win);
    }
    left_win = newwin(height*0.75, width*0.8, 0, 0);
    right_win = newwin(height*0.75, width*0.2, 0, width*0.8);
    down_win = newwin(height-height*0.75, width, height*0.75, 0);
    box(left_win, 0, 0);
    box(right_win, 0, 0);
    box(down_win, 0, 0);
    wbkgd(left_win, COLOR_PAIR(1));
    wbkgd(right_win, COLOR_PAIR(2));
    wbkgd(down_win, COLOR_PAIR(3));
    wrefresh(left_win);
    wrefresh(right_win);
    wrefresh(down_win);
}

void sig_winch(int signo){
    endwin();
    refresh();
    clear();
    resize_windows();
    wprintw(down_win, "%s: ", name);
}

void init_pairs(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

void name_screen(char name[], mqd_t mq){
    WINDOW * wnd;
    message_t msg;
    msg.type = NAME;
    // strncpy(" ", msg.text, MAX_SIZE);
    initscr();
    signal(SIGWINCH, sig_winch);
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

void main() {
    int ch;
    mqd_t mq;
    char buffer[MSG_BUFFER_SIZE];

    // Очистка буфера
    memset(buffer, 0, sizeof(buffer));
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
    signal(SIGWINCH, sig_winch);
    wmove(down_win, 1, 1);
    keypad(stdscr, TRUE);
    message_t msg;
    while (1){
        wprintw(down_win, "%s: ", name);
        wgetnstr(down_win, buffer, MAX_SIZE);
        if ((strcmp(buffer, "exit") == 0) || (strcmp(buffer, "e") == 0)){
            msg.type = QUIT;
            strncpy(msg.client_name, name, MAX_NAME_LEN+1);
            if (mq_send(mq, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                exit(1);
            }
            endwin();
            exit(0);
        }
        if (mq_send(mq, buffer, strlen(buffer) + 1, 0) == -1) {
            perror("mq_send");
            exit(1);
        }
        memset(buffer, 0, sizeof(buffer));
        wclear(down_win);
        resize_windows();
        wmove(down_win, 1, 1);
        wrefresh(down_win);
    }
}