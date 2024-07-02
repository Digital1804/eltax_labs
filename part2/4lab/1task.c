#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file;
    file = fopen("output.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file output.txt to write.\n");
        return 1;
    }
    fprintf(file, "String from file");
    fclose(file); 
    file = fopen("output.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file output.txt to read.\n");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long pos = ftell(file);
    char ch;
    while (pos >= 0) {
        fseek(file, pos, SEEK_SET);
        ch = fgetc(file);
        if (ch != '\n' && ch != EOF) {
            putchar(ch);
        }
        pos--;
    }
    fclose(file);
    putchar('\n');
    return 0;
}
