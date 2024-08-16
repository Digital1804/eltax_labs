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


/** @brief Структура для хранения строки чата
    @param chat_line::text Текст сообщения
    @param chat_line::name Имя отправителя
*/
typedef struct chat_line{
    char *text;
    char *name;
} chat_line;

WINDOW *left_win, *right_win, *down_win;  // Глобальная переменные для окна интерфейса
bool running = TRUE;  // Флаг для работы основного цикла
mqd_t service_queue, client_queue;  // Дескриптор очереди сообщений

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;  // Мьютекс для синхронизации
chat_line **chat = NULL;  // Указатель на массив строк чата

/** @brief Функция для отображения текста в нижнем окне 
    @param text[]: текст для отображения
*/
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

/** @brief Функция для отображения текста в правом окне 
    @param text текст для отображения
*/
void print_right_win(char text[]) {
    if (right_win == NULL) {
        return;
    }
    wclear(right_win);
    box(right_win, 0, 0);
    wrefresh(right_win);

    // Разбиваем входной текст на слова и выводим их построчно
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

/** @brief Функция для отображения чата в левом окне 
    @param type HISTORY: отображение чата, CHAT: отображение сообщения
    @param chat_line_count количество строк чата
*/
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

/** @brief Функция для создания окон интерфейса 
    @return Возвращает количество доступных строк чата
*/
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
    getmaxyx(stdscr, height, width);  // Получаем размер терминала

    // Выделяем память для массива строк чата
    chat = (chat_line**)calloc(height*0.75, sizeof(chat_line*));
    for (int i = 0; i < height; i++) {
        chat[i] = (chat_line*)calloc(MAX_SIZE, sizeof(chat_line));
    }

    int max_chat_lines = height*0.75-2;  // Максимальное количество строк чата
    left_win = newwin(height*0.75, width*0.8, 0, 0);  // Создание левого окна
    right_win = newwin(height*0.75, width*0.2, 0, width*0.8);  // Создание правого окна
    down_win = newwin(height-height*0.75, width, height*0.75, 0);  // Создание нижнего окна

    // Установка цветовых пар для окон
    wbkgd(left_win, COLOR_PAIR(1));
    wbkgd(right_win, COLOR_PAIR(2));
    wbkgd(down_win, COLOR_PAIR(3));

    // Отрисовка рамок для окон
    box(down_win, 0, 0);
    box(left_win, 0, 0);
    box(right_win, 0, 0);

    // Обновление окон
    wrefresh(left_win);
    wrefresh(right_win);
    wrefresh(down_win);
    refresh();

    return max_chat_lines;
}

/** @brief Функция инициализации цветовых пар */
void init_pairs(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
}

/** @brief Начальная функция для получения имени клиента 
    @param name строка для хранения имени клиента
    @param client_queue_name строка для хранения уникального имени очереди клиента
*/
void start_screen(char name[], char client_queue_name[MAX_NAME_LEN + 15]){
    WINDOW *wnd;
    message_t msg;
    msg.type = NAME;
    memset(msg.text, 0, MAX_SIZE);
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
        start_screen(name, client_queue_name);  // Если имя пустое, повторяем ввод
        return;
    }
    name[MAX_NAME_LEN] = 0;
    strncpy(msg.client_name, name, MAX_NAME_LEN+1);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_t);
    attr.mq_curmsgs = 0;

    // Создаем уникальное имя очереди для клиента
    snprintf(client_queue_name, MAX_NAME_LEN + 15, "/client_queue_%s", name);
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

    // Логируем отправку имени клиента
    FILE *fp = fopen("log.txt", "a");
    fprintf(fp, "CLIENT %s send: %ld\t%s\n", name, msg.type, msg.text);
    fclose(fp);
    return;
}

/** @brief Поток для отправки сообщений на сервер */
void *send_messages(void *arg) {
    FILE *fp;
    char *name = (char *)arg;  // Имя клиента
    while (running){
        message_t msg;
        memset(msg.text, 0, sizeof(msg.text));

        print_down_win(name);
        wgetnstr(down_win, msg.text, MAX_SIZE);  // Ввод сообщения пользователем
        msg.type = TEXT;
        strncpy(msg.client_name, name, MAX_NAME_LEN+1);

        // Если пользователь ввел команду выхода, отправляем сообщение о выходе
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

        // Отправляем сообщение на сервер
        if (strncmp(msg.text, "", 1) != 0) {
            pthread_mutex_lock(&m1);
            if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
                perror("mq_send");
                pthread_mutex_unlock(&m1);
                exit(1);
            }
            pthread_mutex_unlock(&m1);
        }

        // Логируем отправленное сообщение
        fp = fopen("log.txt", "a");
        fprintf(fp, "CLIENT %s send: %ld\t%s\n", name, msg.type, msg.text);
        fclose(fp);
    }
    return NULL;
}

/** @brief Обработчик сигналов (в данной программе он не делает ничего) */
void signal_handler(int sig) {
    return;
}

int main() {
    FILE *fp;
    initscr();
    system("clear");  // Очистка экрана
    cbreak();  // Включаем режим cbreak (не буферизированный ввод)
    nodelay(left_win, TRUE);  // Включаем неблокирующий режим для окон
    nodelay(right_win, TRUE);
    curs_set(FALSE);  // Отключаем отображение курсора

    char name[MAX_NAME_LEN + 1];
    memset(name, 0, sizeof(name));

    // Открытие очереди сообщений с сервером
    service_queue = mq_open(QUEUE_NAME, O_RDWR);
    if (service_queue == -1) {
        perror("service mq_open");
        exit(1);
    }

    int chat_line_count = 0;
    char client_queue_name[MAX_NAME_LEN + 15];

    start_screen(name, client_queue_name);  // Запрашиваем имя пользователя
    init_pairs();  // Инициализация цветовых пар
    
    int max_chat_lines = create_windows(&left_win, &right_win, &down_win);  // Создаем окна интерфейса
    signal(SIGWINCH, signal_handler);  // Назначаем обработчик сигнала изменения размера окна

    wmove(down_win, 1, 1);
    keypad(down_win, TRUE);

    // Создаем поток для отправки сообщений
    pthread_t sender_thread;
    pthread_create(&sender_thread, NULL, send_messages, name);

    sleep(1);  // Задержка перед началом получения сообщений

    message_t msg;
    msg.type = HISTORY;
    sprintf(msg.text, "%d", max_chat_lines);
    snprintf(msg.client_name, MAX_NAME_LEN+1, "%s", name);

    // Отправляем запрос на получение истории сообщений
    if (mq_send(service_queue, (char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
        exit(1);
    }

    // Основной цикл получения сообщений
    while (1) {
        if (mq_receive(client_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            exit(1);
        } else {
            // Логируем полученное сообщение
            fp = fopen("log.txt", "a");
            fprintf(fp, "CLIENT %s receive: %ld\t%s\tFROM %s\n", name, msg.type, msg.text, msg.client_name);
            fclose(fp);

            switch (msg.type) {
                case CHAT:
                    // Если чат заполнен, удаляем самое старое сообщение
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

                    // Добавляем новое сообщение в чат
                    chat[chat_line_count] = (chat_line *)malloc(sizeof(chat_line));
                    chat[chat_line_count]->text = strdup(msg.text);
                    chat[chat_line_count]->name = strdup(msg.client_name);
                    if (chat_line_count < max_chat_lines)  chat_line_count++;

                    print_left_win(CHAT, chat_line_count);
                    break;

                case MEMBERS:
                    print_right_win(msg.text);  // Отображаем список участников в правом окне
                    break;

                case HISTORY:
                    // Обработка сообщений истории чата
                    if (chat_line_count == max_chat_lines) {
                        for (int i = 0; i < chat_line_count - 1; i++) {
                            chat[i] = chat[i + 1];
                        }
                        chat_line_count--;
                    }

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

    // Освобождаем память и закрываем окна
    delwin(left_win);
    delwin(right_win);
    delwin(down_win);
    endwin();

    for (int i = 0; i < chat_line_count; i++) {
        free(chat[i]->text);
        free(chat[i]->name);
        free(chat[i]);
    }

    // Закрытие очередей сообщений
    if (mq_close(service_queue) == -1) {
        perror("service mq_close");
        exit(1);
    }
    if (mq_close(client_queue) == -1) {
        perror("client mq_close");
        exit(1);
    }
    if (mq_unlink(client_queue_name) == -1) {
        perror("client mq_unlink");
        exit(1);
    }

    // Завершение потока отправки сообщений
    pthread_cancel(sender_thread);
}