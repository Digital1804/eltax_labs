#include <ncurses.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "clientlib.h"

/** @brief Функция для отображения текста в правом окне 
    @param client структура клиента чата
    @param members структура с именами участников чата
*/
void print_members(client_UI *client, members_t *members_ptr) {
    wclear(client->right_win);
    wbkgd(client->right_win, COLOR_PAIR(2));
    box(client->right_win, 0, 0);
    mvwprintw(client->right_win, 1, 1, "Members:");
    for (int i = 0; i < members_ptr->count; i++) {
        mvwprintw(client->right_win, i + 2, 1, "%s", members_ptr->clients[i].name);
    }
    wrefresh(client->right_win);
}

/** @brief Функция для отображения чата в левом окне 
    @param client структура клиента чата
    @param chat_line_count количество строк чата
*/
void print_chat(client_UI *client, chat_t *chat_ptr) {
    wclear(client->left_win);
    box(client->left_win, 0, 0);
    sleep(1);
    chat_ptr->max_chat_lines = client->max_chat_lines;
    int start = (chat_ptr->count > client->max_chat_lines) ? chat_ptr->count - client->max_chat_lines : 0;
    for (int i = start; i < chat_ptr->count; i++) {
        mvwprintw(client->left_win, i - start + 1, 1, "%s", chat_ptr->text[i % client->max_chat_lines]);
    }
    wrefresh(client->left_win);
}

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

    // char sem_name[32];
    // snprintf(sem_name, sizeof(sem_name), "/sem_hold_%s", client->name);
    // client->win_hold = sem_open(sem_name, O_CREAT, 0644, 0);
    // Копируем имя в структуру
    strncpy(client->name, name, MAX_NAME_LEN);
    client->running = 1;
    // client->name[MAX_NAME_LEN - 1] = '\0'; // На всякий случай добавляем нулевой символ
    wclear(wnd);
    wrefresh(wnd);
    delwin(wnd);
    refresh();
    return;
}

/** @brief Функция для создания окон интерфейса 
    @return Возвращает количество доступных строк чата
*/
void create_windows(client_UI *client){
    int height, width;
    getmaxyx(stdscr, height, width);  // Получаем размер терминала
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