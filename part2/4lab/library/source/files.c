#include "../fileslib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Directory *create_directory(const char *name, Directory *parent) {
    Directory *dir = (Directory *)malloc(sizeof(Directory));
    strncpy(dir->name, name, 255);
    dir->parent = parent;
    dir->child = NULL;
    dir->sibling = NULL;
    dir->files = NULL;
    return dir;
}

File *create_file(const char *name) {
    File *file = (File *)malloc(sizeof(File));
    strncpy(file->name, name, 255);
    file->next = NULL;
    return file;
}
void add_file(Directory *dir, const char *name) {
    File *new_file = create_file(name);
    if (dir->files == NULL) {
        dir->files = new_file;
    } else {
        File *temp = dir->files;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_file;
    }
}

void add_directory(Directory *parent, const char *name) {
    Directory *new_dir = create_directory(name, parent);
    if (parent->child == NULL) {
        parent->child = new_dir;
    } else {
        Directory *temp = parent->child;
        while (temp->sibling != NULL) {
            temp = temp->sibling;
        }
        temp->sibling = new_dir;
    }
}
