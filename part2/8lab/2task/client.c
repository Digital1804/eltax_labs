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
#include <semaphore.h>

#include "info.h"

WINDOW *left_win, *right_win, *down_win;
bool running = TRUE;
mqd_t service_queue, client_queue;
sem_t sem;

void change_left_win(){
    if (left_win != NULL) {
        delwin(left_win);
    }
    left_win = newwin(10, 80, 0, 0);
    wbkgd(left_win, COLOR_PAIR(1));
    box(left_win, 0, 0);
    wrefresh(left_win);
}

void change_right_win(){
    if (right_win != NULL) {
        delwin(right_win);
    }
    right_win = newwin(10, 20, 0, 80);
    wbkgd(right_win, COLOR_PAIR(2));
    box(right_win, 0, 0);
    wrefresh(right_win);
}

void change_down_win(){
    if (down_win != NULL) {
        delwin(down_win);
    }
    down_win = newwin(5, 100, 10, 0);
    wbkgd(down_win, COLOR_PAIR(3));
    box(down_win, 0, 0);
    wrefresh(down_win);
}

void print_down_win(char text[]) {
    wclear(down_win);
    box(down_win, 0, 0);
    wrefresh(down_win);
    wmove(down_win, 1, 1);
    wprintw(down_win, "%s: ", text);
}

void print_right_win(char text[]) {
    wclear(right_win);
    char *temp = strdup(text);  // Создаем копию входной строки, чтобы не изменять оригинал
    char *token = strtok(temp, " ");
    int count = 0;
    char *lines[MAX_CLIENTS];
    while (token != NULL && count < MAX_CLIENTS) {
        lines[count] = strdup(token);  // Копируем токен в массив строк
        wmove(right_win, count+1, 1);
        wprintw(right_win, "%s", lines[count]);
        count++;
        token = strtok(NULL, " ");
    }
    free(temp);  // Освобождаем временную копию строки

    box(right_win, 0, 0);
    wrefresh(right_win);
}

void print_left_win(char text[]) {
    wclear(left_win);
    box(left_win, 0, 0);
    wrefresh(left_win);
    wmove(left_win, 1, 1);
    wprintw(left_win, "%s", text);
}

void create_windows(){
    left_win = newwin(10, 80, 0, 0);
    right_win = newwin(10, 20, 0, 80);
    down_win = newwin(5, 100, 10, 0);
    wbkgd(left_win, COLOR_PAIR(1));
    wbkgd(right_win, COLOR_PAIR(2));
    wbkgd(down_win, COLOR_PAIR(3));
    box(down_win, 0, 0);
    box(left_win, 0, 0);
    box(right_win, 0, 0);
    wrefresh(left_win);
    wrefresh(right_win);
    wrefresh(down_win);
    refresh();
}

void init_pairs(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
}

void name_screen(char name[], mqd_t service_queue){
    WINDOW * wnd;
    message_t msg;
    msg.type = NAME;
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
    FILE *fp;
    fp = fopen("log.txt", "a");
    fprintf(fp, "CLIENT send: %ld\t%s\t%s\n", msg.type, msg.text, msg.client_name);
    fclose(fp);
    if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
        exit(1);
    }
}

void *receive_messages(void *arg) {
    FILE *fp;
    while (running) {
        message_t msg;
        if (mq_receive(service_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        } 
        else {
            fp = fopen("log.txt", "a");
            fprintf(fp, "CLIENT recieve: %ld\t%s\t%s\n", msg.type, msg.text, msg.client_name);
            fclose(fp);
            sem_wait(&sem);
            switch (msg.type) {
                case CHAT:
                    break;
                case MEMBERS:
                    // print_right_win(msg.text);
                    break;
                default:
                    break;
            }
            sem_post(&sem);
        }
    }
    return NULL;
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        running = FALSE;
    }
}

void handle_resize(int sig) {
    // Не делать ничего, чтобы предотвратить изменение размера
}
int main() {
    char name[MAX_NAME_LEN + 1];
    memset(name, 0, sizeof(name));
    FILE *fp;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;
    service_queue = mq_open(QUEUE_NAME, O_RDWR);
    if (service_queue == -1) {
        perror("service mq_open");
        exit(1);
    }
    client_queue = mq_open(CLIENT_QUEUE_NAME, O_RDWR);
    if (client_queue == -1) {
        perror("client mq_open");
        exit(1);
    }
    sem_init(&sem, 0, 1);
    name_screen(name, service_queue);
    initscr();
    signal(SIGWINCH, handle_resize);
    clear();
    refresh();
    init_pairs();
    create_windows();
    cbreak();
    curs_set(FALSE);
    wmove(down_win, 1, 1);
    keypad(down_win, TRUE);
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, receive_messages, NULL);
    signal(SIGINT, sig_handler);
    while (1) {
        message_t msg;
        create_windows();
        print_down_win(name);
        wgetnstr(down_win, msg.text, MAX_SIZE);
        msg.type = TEXT;
        strncpy(msg.client_name, name, MAX_NAME_LEN+1);
        if ((strcmp(msg.text, "exit") == 0) || (strcmp(msg.text, "e") == 0)) {
            sem_wait(&sem);
            msg.type = QUIT;
            if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                sem_destroy(&sem);
                exit(1);
            }
            sem_post(&sem);
            endwin();
            exit(0);
        }
        if (strncmp(msg.text, "", 5) != 0) {
            sem_wait(&sem);
            if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                sem_destroy(&sem);
                exit(1);
            }
            sem_post(&sem);
        }
        fp = fopen("log.txt", "a");
        fprintf(fp, "CLIENT send: %ld\t%s\t%s\n", msg.type, msg.text, msg.client_name);
        fclose(fp);
        memset(msg.text, 0, sizeof(msg.text));
    }
    sem_destroy(&sem);
}
