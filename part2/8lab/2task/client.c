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
#include <malloc.h>

#include "info.h"

WINDOW *left_win, *right_win, *down_win;
char name[MAX_NAME_LEN + 1];
bool running = TRUE;
mqd_t service_queue, client_queue;
sem_t sem;

void print_down_win(char text[]) {
    if (down_win == NULL) {
        return;
    }
    wclear(down_win);
    box(down_win, 0, 0);
    wmove(down_win, 1, 1);
    wprintw(down_win, "%s: ", text);
    wrefresh(down_win);
}

void print_right_win(char text[]) {
    if (right_win == NULL) {
        return;
    }
    wclear(right_win);
    box(right_win, 0, 0);
    wrefresh(right_win);
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
    // Освобождение памяти для каждого токена
    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
    wrefresh(right_win);
}

void print_left_win(char text[]) {
    if (left_win == NULL) {
        return;
    }
    wclear(left_win);
    box(left_win, 0, 0);
    wmove(left_win, 1, 1);
    wprintw(left_win, "%s", text);
    wrefresh(left_win);
}

char** create_windows(){
    char **chat;
    if (left_win != NULL) {
        delwin(left_win);
    }
    if (right_win != NULL) {
        delwin(right_win);
    }
    if (down_win != NULL) {
        delwin(down_win);
    }
    int height, width;
    getmaxyx(stdscr, height, width);
    chat = (char**)calloc(height, sizeof(char*));
    for (int i = 0; i < height; i++) {
        chat[i] = (char*)calloc(width*0.8, sizeof(char));
    }
    left_win = newwin(height*0.75, width*0.8, 0, 0);
    right_win = newwin(height*0.75, width*0.2, 0, width*0.8);
    down_win = newwin(height-height*0.75, width, height*0.75, 0);
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
    return chat;
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
    memset(msg.text, 0, MAX_SIZE);
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
    fprintf(fp, "CLIENT %s send: %ld\t%s\n", name, msg.type, msg.text);
    fclose(fp);
    if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
        exit(1);
    }
}

void *send_messages(void *arg) {
    FILE *fp;
    while (running){
        message_t msg;
        memset(msg.text, 0, sizeof(msg.text));
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
        if (strncmp(msg.text, "", 1) != 0) {
            sem_wait(&sem);
            if (mq_send(client_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                sem_destroy(&sem);
                exit(1);
            }
            sem_post(&sem);
        }
        fp = fopen("log.txt", "a");
        fprintf(fp, "CLIENT %s send: %ld\t%s\n", name, msg.type, msg.text);
        fclose(fp);
    }
    return NULL;
}

void signal_handler(int sig) {
    return;
}

int main() {
    initscr();
    system("clear");
    // refresh();
    cbreak();
    curs_set(FALSE);
    memset(name, 0, sizeof(name));
    FILE *fp;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_CLIENTS;
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
    name_screen(name, service_queue);
    init_pairs();
    char **chat = create_windows(&left_win, &right_win, &down_win);
    signal(SIGWINCH, signal_handler);
    wmove(down_win, 1, 1);
    keypad(down_win, TRUE);
    sem_init(&sem, 0, 1);
    pthread_t sender_thread;
    pthread_create(&sender_thread, NULL, send_messages, down_win);
    sleep(1);
    while (1) {
        message_t msg;
        if (mq_receive(service_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        } 
        else {
            fp = fopen("log.txt", "a");
            fprintf(fp, "CLIENT %s recieve: %ld\t%s\n", name, msg.type, msg.text);
            fclose(fp);
            switch (msg.type) {
                case CHAT:
                    print_left_win(msg.text);
                    break;
                case MEMBERS:
                    print_right_win(msg.text);
                    break;
                default:
                    break;
            }
            sleep(1);
        }
    }
    delwin(left_win);
    delwin(right_win);
    delwin(down_win);
    endwin();
    sem_destroy(&sem);
    pthread_cancel(sender_thread);
}
