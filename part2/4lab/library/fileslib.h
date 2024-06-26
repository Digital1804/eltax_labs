#ifndef FILES_H
#define FILES_H

typedef struct File {
    char name[256];
    struct File *next;
} File;

typedef struct Directory {
    char name[256];
    struct Directory *parent;
    struct Directory *child;
    struct Directory *sibling;
    File *files;
} Directory;

Directory *create_directory(const char *name, Directory *parent);
File *create_file(const char *name);
void add_file(Directory *dir, const char *name);
void add_directory(Directory *parent, const char *name);

#endif