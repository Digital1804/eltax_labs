#include <ncurses.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "clientlib.h"

/** @brief Функция для отображения окна с вводом имени 
    @param client структура клиента
*/
void start_screen(client_UI *client){
    WINDOW *wnd;
    // Создаем окно для ввода имени
    wnd = newwin(5, 23, 2, 2);
    wbkgd(wnd, COLOR_PAIR(4));
    wattron(wnd, A_BOLD);
    wprintw(wnd, "Enter your name...\n");
    char name[MAX_NAME_LEN] = "";

    while (strlen(name) == 0) {
        wclear(wnd);
        wprintw(wnd, "Enter your name...\n");
        wrefresh(wnd);
        wgetnstr(wnd, name, MAX_NAME_LEN);  // Получаем имя пользователя
    }

    // Копируем имя в структуру
    strncpy(client->name, name, MAX_NAME_LEN - 1);
    // client->name[MAX_NAME_LEN - 1] = '\0'; // На всякий случай добавляем нулевой символ
    sem_post(client->win_hold);
    wclear(wnd);
    wrefresh(wnd);
    delwin(wnd);
    refresh();
    return;
}

/** @brief Функция для отображения текста в нижнем окне 
    @param text текст для отображения
*/
void print_name(WINDOW *down_win, char text[]) {
    int height, width;
    getmaxyx(stdscr, height, width);  // Получаем размер терминала
    if (down_win != NULL) {
        delwin(down_win);
    }
    down_win = newwin(height-height*0.75, width, height*0.75, 0);  // Создание нижнего окна
    wbkgd(down_win, COLOR_PAIR(3));
    box(down_win, 0, 0);
    wmove(down_win, 1, 1); 
    wprintw(down_win, "%s: ", text);
    wrefresh(down_win);
}

/** @brief Функция для отображения текста в правом окне 
    @param members структура с именами участников чата
*/
void print_members(WINDOW *right_win, members_t *members) {
    int height, width;
    getmaxyx(stdscr, height, width);  // Получаем размер терминала
    if (right_win != NULL) {
        delwin(right_win);
    }
    right_win = newwin(height*0.75, width*0.2, 0, width*0.8);  // Создание правого окна
    wbkgd(right_win, COLOR_PAIR(2));
    box(right_win, 0, 0);
    wmove(right_win, 1, 1);
    wprintw(right_win, "Members:");
    for (int i = 0; i < members->count; i++) {
        wmove(right_win, i + 2, 1);
        wprintw(right_win, "%s", members->names[i]);
    }
    wrefresh(right_win);
}

/** @brief Функция для отображения чата в левом окне 
    @param chat структура с сообщениями чата
    @param chat_line_count количество строк чата
*/
void print_chat(WINDOW *left_win, chat_t *chat, int chat_line_count) {
    int height, width;
    getmaxyx(stdscr, height, width);  // Получаем размер терминала
    if (left_win != NULL) {
        delwin(left_win);
    }
    left_win = newwin(height*0.75, width*0.8, 0, 0);  // Создание левого окна
    wbkgd(left_win, COLOR_PAIR(1));
    wclear(left_win);
    box(left_win, 0, 0);
    sleep(1);
    for (int i = chat->count-getmaxy(left_win); i < chat->count; i++) {
        wmove(left_win, i + 1, 1);
        wprintw(left_win, "%s", chat->text[i]);
    }
    wrefresh(left_win);
}

/** @brief Функция для создания окон интерфейса 
    @return Возвращает количество доступных строк чата
*/
void create_windows(client_UI *client){
    int height, width;
    getmaxyx(stdscr, height, width);  // Получаем размер терминала
    printf("%d %d\n", height, width);
    client->max_chat_lines = height*0.75-2;  // Максимальное количество строк чата
    client->left_win = newwin(height*0.75, width*0.8, 0, 0);  // Создание левого окна
    client->right_win = newwin(height*0.75, width*0.2, 0, width*0.8);  // Создание правого окна
    client->down_win = newwin(height-height*0.75, width, height*0.75, 0);  // Создание нижнего окна

    // Установка цветовых пар для окон
    init_pairs();
    wbkgd(client->left_win, COLOR_PAIR(1));
    wbkgd(client->right_win, COLOR_PAIR(2));
    wbkgd(client->down_win, COLOR_PAIR(3));

    // Отрисовка рамок для окон
    box(client->down_win, 0, 0);
    box(client->left_win, 0, 0);
    box(client->right_win, 0, 0);

    // // Обновление окон
    wrefresh(client->left_win);
    wrefresh(client->right_win);
    wrefresh(client->down_win);
    refresh();
    return;
}

/** @brief Функция инициализации цветовых пар */
void init_pairs(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_GREEN);
}

/** @brief Обработчик сигналов (в данной программе он не делает ничего) */
void signal_handler(int sig) {
    return;
}