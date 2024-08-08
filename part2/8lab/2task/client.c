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

typedef struct chat_line{
    char *text;
    char *name;
} chat_line;

WINDOW *left_win, *right_win, *down_win;
bool running = TRUE;
mqd_t service_queue, client_queue;
// Создаем уникальное имя очереди для клиента
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
chat_line **chat = NULL;

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

void print_left_win(int type, int chat_line_count) {
    if (left_win == NULL) {
        return;
    }
    wclear(left_win);
    box(left_win, 0, 0);
    sleep(1);
    for (int i = 0; i < chat_line_count; i++) {
        wmove(left_win, i + 1, 1);
        if (type == CHAT)
            wprintw(left_win, "%s: %s", chat[i]->name, chat[i]->text);
        else{
            wprintw(left_win, "%s", chat[i]->text);
        }
    }
    wrefresh(left_win);
}

int create_windows(){
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
    chat = (chat_line**)calloc(height*0.75, sizeof(chat_line*));
    for (int i = 0; i < height; i++) {
        chat[i] = (chat_line*)calloc(MAX_SIZE, sizeof(chat_line));
    }
    int max_chat_lines = height*0.75-2;
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
    return max_chat_lines;
}

void init_pairs(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
}

void start_screen(char name[], char client_queue_name[MAX_NAME_LEN + 15]){
    WINDOW *wnd;
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
    if (strlen(name) == 0) {
        start_screen(name, client_queue_name);
        return;
    }
    name[MAX_NAME_LEN] = 0;
    strncpy(msg.client_name, name, MAX_NAME_LEN+1);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;

    snprintf(client_queue_name, MAX_NAME_LEN + 15, "/client_queue_%s", name);
    // Открываем уникальную очередь клиента
    client_queue = mq_open(client_queue_name, O_CREAT | O_RDWR, QUEUE_PERMISSIONS, &attr);
    if (client_queue == -1) {
        perror("client queue mq_open");
        exit(1);
    }

    // Отправляем имя клиента в общую очередь
    msg.type = NAME;
    snprintf(msg.text, MAX_SIZE, "%s", client_queue_name);
    if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
        exit(1);
    }

    wrefresh(wnd);
    delwin(wnd);
    curs_set(FALSE);
    endwin();
    FILE *fp = fopen("log.txt", "a");
    fprintf(fp, "CLIENT %s send: %ld\t%s\n", name, msg.type, msg.text);
    fclose(fp);
    return;
}

void *send_messages(void *arg) {
    FILE *fp;
    char *name = (char *)arg;
    while (running){
        message_t msg;
        memset(msg.text, 0, sizeof(msg.text));
        print_down_win(name);
        wgetnstr(down_win, msg.text, MAX_SIZE);
        msg.type = TEXT;
        strncpy(msg.client_name, name, MAX_NAME_LEN+1);
        if ((strcmp(msg.text, "exit") == 0) || (strcmp(msg.text, "e") == 0)) {
            pthread_mutex_lock(&m1);
            msg.type = QUIT;
            if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                pthread_mutex_unlock(&m1);
                exit(1);
            }
            pthread_mutex_unlock(&m1);
            endwin();
            exit(0);
        }
        if (strncmp(msg.text, "", 1) != 0) {
            pthread_mutex_lock(&m1);
            if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                pthread_mutex_unlock(&m1);
                exit(1);
            }
            pthread_mutex_unlock(&m1);
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
    FILE *fp;
    initscr();
    system("clear");
    cbreak();
    nodelay(left_win, TRUE);
    nodelay(right_win, TRUE);
    curs_set(FALSE);
    char name[MAX_NAME_LEN + 1];
    memset(name, 0, sizeof(name));
    service_queue = mq_open(QUEUE_NAME, O_RDWR);
    if (service_queue == -1) {
        perror("service mq_open");
        exit(1);
    }
    int chat_line_count = 0;
    char client_queue_name[MAX_NAME_LEN + 15];
    start_screen(name, client_queue_name);
    init_pairs();
    
    int max_chat_lines = create_windows(&left_win, &right_win, &down_win);
    signal(SIGWINCH, signal_handler);
    wmove(down_win, 1, 1);
    keypad(down_win, TRUE);
    pthread_t sender_thread;
    pthread_create(&sender_thread, NULL, send_messages, name);
    sleep(1);
    message_t msg;
    msg.type = HISTORY;
    sprintf(msg.text, "%d", max_chat_lines);
    snprintf(msg.client_name, MAX_NAME_LEN+1, "%s", name);
    if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
        exit(1);
    }
    while (1) {
        if (mq_receive(client_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        } 
        else {
            fp = fopen("log.txt", "a");
            fprintf(fp, "CLIENT %s recieve: %ld\t%s\tFROM %s\n", name, msg.type, msg.text, msg.client_name);
            fclose(fp);
            switch (msg.type) {
                case CHAT:
                    if (chat_line_count == max_chat_lines) {
                        free(chat[0]->text);
                        free(chat[0]->name);
                        free(chat[0]);         
                        // Сдвиг элементов массива влево
                        for (int i = 0; i < chat_line_count - 1; i++) {
                            chat[i] = chat[i + 1];
                        }
                        chat_line_count--;

                    }
                    // Добавление новой строки в массив
                    chat[chat_line_count] = (chat_line *)malloc(sizeof(chat_line));
                    chat[chat_line_count]->text = strdup(msg.text);
                    chat[chat_line_count]->name = strdup(msg.client_name);
                    if (chat_line_count < max_chat_lines)  chat_line_count++;

                    print_left_win(CHAT, chat_line_count);
                    break;
                case MEMBERS:
                    print_right_win(msg.text);
                    break;
                case HISTORY:
                    if (chat_line_count == max_chat_lines) {
                                             
                        // Сдвиг элементов массива влево
                        for (int i = 0; i < chat_line_count - 1; i++) {
                            chat[i] = chat[i + 1];
                        }
                        chat_line_count--;

                    }
                    // Добавление новой строки в массив
                    chat[chat_line_count]->text = strdup(msg.text);
                    chat[chat_line_count]->name = strdup(msg.client_name);
                    if (chat_line_count < max_chat_lines)  chat_line_count++;
                    if (strncmp(msg.client_name, name, MAX_NAME_LEN + 1) == 0) {
                        print_left_win(HISTORY, chat_line_count);
                        break;
                    }
                default:
                    break;
            }
        }
    }
    delwin(left_win);
    delwin(right_win);
    delwin(down_win);
    endwin();
    for (int i = 0; i < chat_line_count; i++) {
        free(chat[i]->text);
        free(chat[i]->name);
        free(chat[i]);
    }
    if (mq_close(service_queue) == -1) {
        perror("service mq_close");
        exit(1);
    }
    if (mq_close(client_queue) == -1) {
        perror("service mq_close");
        exit(1);
    }
    if (mq_unlink(client_queue_name) == -1) {
        perror("service mq_unlink");
        exit(1);
    }
    pthread_cancel(sender_thread);
}
