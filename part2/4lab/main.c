#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX_FILES 1024
int name_col_width, size_col_width, time_col_width;
int height, width;
char left_path[1024], right_path[1024];

typedef struct {
    char name[256];
    uint64_t size;
    time_t mtime;
    __mode_t mode;
} FileInfo;

WINDOW *left_win, *right_win;

int compare_filenames(const void *a, const void *b) {
    const FileInfo *fileA = (const FileInfo *)a;
    const FileInfo *fileB = (const FileInfo *)b;
    return strcmp(fileA->name, fileB->name);
}

void list_directory(const char *path, FileInfo files[MAX_FILES], int *file_count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    *file_count = 0;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL && *file_count < MAX_FILES) {
        if (strcmp(entry->d_name,".") == 0) {
            continue;
        }
        snprintf(files[*file_count].name, sizeof(files[*file_count].name), "%s", entry->d_name);
        stat(entry->d_name, &file_stat);
        files[*file_count].size = file_stat.st_size;
        files[*file_count].mtime = file_stat.st_mtime;
        files[*file_count].mode = file_stat.st_mode;
        (*file_count)++;
    }

    closedir(dir);
    qsort(files, *file_count, sizeof(FileInfo), compare_filenames);
}
void display_files(WINDOW *win, FileInfo files[MAX_FILES], int file_count, int highlight, char current_path[1024]) {
    int x = 2, y = 3;
    int max_lines = height - 4;

    box(win, 0, 0);
    mvwprintw(win, 2, 2, "%-*s %*s %-*s", name_col_width, "Name", size_col_width, "Size", time_col_width, "Last Modified");

    for (int i = 0; i < max_lines && i < file_count; i++) {
        char mtime_str[20];
        struct tm *mtime_tm = localtime(&files[i].mtime);
        strftime(mtime_str, sizeof(mtime_str), "%m-%d %H:%M", mtime_tm);
        if (S_ISDIR(files[i].mode)) {
            if (i == highlight) {
                wattron(win, A_REVERSE);
                mvwprintw(win, y + i, x, "/%-*s %*ld %-*s", name_col_width - 1, files[i].name, size_col_width, files[i].size, time_col_width, mtime_str);
                wattroff(win, A_REVERSE);
            }
            else {
                mvwprintw(win, y + i, x, "/%-*s %*ld %-*s", name_col_width - 1, files[i].name, size_col_width, files[i].size, time_col_width, mtime_str);
            }
        } else {
            if (i == highlight) {
                wattron(win, A_REVERSE);
                mvwprintw(win, y + i, x, "%-*s %*ld %-*s", name_col_width, files[i].name, size_col_width, files[i].size, time_col_width, mtime_str);
                wattroff(win, A_REVERSE);
            }
            else {
                mvwprintw(win, y + i, x, "%-*s %*ld %-*s", name_col_width, files[i].name, size_col_width, files[i].size, time_col_width, mtime_str);
            }
        }
    }
    mvwvline(win, 2, name_col_width+3, 0, height-3);
    mvwvline(win, 2, name_col_width+3+size_col_width, 0, height-3);
    wrefresh(win);
    mvwprintw(win, 1, 1, "Current directory: %s", current_path);
}

void resize_windows() {
    getmaxyx(stdscr, height, width);

    if (left_win != NULL) {
        delwin(left_win);
    }
    if (right_win != NULL) {
        delwin(right_win);
    }

    left_win = newwin(height, width / 2, 0, 0);
    right_win = newwin(height, width / 2, 0, width / 2);

    name_col_width = (width / 2 - 6) * 0.7;
    size_col_width = (width / 2 - 6) * 0.1+1;
    time_col_width = (width / 2 - 6) * 0.2;

    keypad(left_win, TRUE);
    keypad(right_win, TRUE);
}

int main() {
    initscr();
    cbreak();
    curs_set(FALSE);
    keypad(stdscr, TRUE);


    int highlight_left = 0, highlight_right = 0;
    int choice;
    int file_count_left, file_count_right;
    FileInfo files_left[MAX_FILES], files_right[MAX_FILES];
    bool is_left_active = TRUE;
    int start_left = 0, start_right = 0;
    getcwd(left_path, sizeof(left_path));
    getcwd(right_path, sizeof(right_path));
    list_directory(right_path, files_right, &file_count_right);  
    list_directory(left_path, files_left, &file_count_left);
    while (1) {
        clear();
        resize_windows();
        display_files(left_win, files_left, file_count_left, highlight_left, left_path);
        display_files(right_win, files_right, file_count_right, highlight_right, right_path);
        wrefresh(left_win);
        wrefresh(right_win);


        choice = wgetch(is_left_active ? left_win : right_win);

        switch (choice) {
            case '\t':
                if (is_left_active) {
                    getcwd(left_path, sizeof(left_path));
                } else {
                    getcwd(right_path, sizeof(right_path));
                }
                is_left_active = !is_left_active;
                if (is_left_active) {
                    chdir(left_path);
                } else {
                    chdir(right_path);
                }
                break;
            case KEY_UP:
                if (is_left_active) {
                    if (highlight_left > 0)
                        highlight_left--;
                } else {
                    if (highlight_right > 0)
                        highlight_right--;
                }
                break;
            case KEY_DOWN:
                if (is_left_active) {
                    if (highlight_left < file_count_left - 1)
                        highlight_left++;
                } else {
                    if (highlight_right < file_count_right - 1)
                        highlight_right++;
                }
                break;
            case '\n':
                if (is_left_active) {
                    struct stat file_stat;
                        stat(files_left[highlight_left].name, &file_stat);
                        if (S_ISDIR(file_stat.st_mode)) {
                            chdir(files_left[highlight_left].name);
                        }
                    getcwd(left_path, sizeof(left_path));
                    list_directory(left_path, files_left, &file_count_left);
                    highlight_left = 0;
                } 
                else {
                    struct stat file_stat;
                        stat(files_right[highlight_right].name, &file_stat);
                        if (S_ISDIR(file_stat.st_mode)) {
                            chdir(files_right[highlight_right].name);
                        }
                    getcwd(right_path, sizeof(right_path));
                    list_directory(right_path, files_right, &file_count_right);
                    highlight_right = 0;
                }
                break;
            case 'q':
                endwin();
                return 0;
        }
    }
    endwin();
    return 0;
}
