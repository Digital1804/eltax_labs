#include <ncurses.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "clientlib.h"

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
    for (int i = chat->count-chat_line_count; i < chat->count; i++) {
        wmove(left_win, i + 1, 1);
        wprintw(left_win, "%s", chat->text[i]);
    }
    wrefresh(left_win);
}

/** @brief Функция для создания окон интерфейса 
    @return Возвращает количество доступных строк чата
*/
int create_windows(WINDOW *down_win, WINDOW *left_win, WINDOW *right_win){
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

    int max_chat_lines = height*0.75-2;  // Максимальное количество строк чата
    left_win = newwin(height*0.75, width*0.8, 0, 0);  // Создание левого окна
    right_win = newwin(height*0.75, width*0.2, 0, width*0.8);  // Создание правого окна
    down_win = newwin(height-height*0.75, width, height*0.75, 0);  // Создание нижнего окна

    // Установка цветовых пар для окон
    init_pairs();
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

/** @brief Обработчик сигналов (в данной программе он не делает ничего) */
void signal_handler(int sig) {
    return;
}