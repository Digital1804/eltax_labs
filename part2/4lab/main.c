#include <ncurses.h>
#include "library/fileslib.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


void list_files(const char *path) {
    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];

    // Открываем каталог
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем текущий и родительский каталоги
        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }

        // Формируем полный путь к файлу
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Получаем информацию о файле
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        // Выводим информацию о файле
        printf("Name: %s\n", entry->d_name);
        printf("Size: %ld bytes\n", file_stat.st_size);
        printf("Permissions: %o\n", file_stat.st_mode & 0777);
        printf("Last modified: %s", ctime(&file_stat.st_mtime));
        printf("\n");
    }

    closedir(dir);
}





void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

void end_ncurses() {
    endwin();
}

void display_directory(Directory *dir) {
    clear();
    printw("Directory: %s\n", dir->name);
    if (dir->files != NULL) {
        File *temp = dir->files;
        while (temp != NULL) {
            printw("  File: %s\n", temp->name);
            temp = temp->next;
        }
    }
    if (dir->child != NULL) {
        Directory *temp = dir->child;
        while (temp != NULL) {
            printw("  Dir: %s\n", temp->name);
            temp = temp->sibling;
        }
    }
    refresh();
}

int main() {
    init_ncurses();

    Directory *root = create_directory("root", NULL);
    add_directory(root, "dir1");
    add_directory(root, "dir2");
    add_file(root, "file1.txt");
    add_file(root, "file2.txt");

    display_directory(root);

    getch();
    end_ncurses();
    return 0;
}
