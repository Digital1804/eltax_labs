#include "stdio.h"
#include "string.h"
#include "stdlib.h"

typedef struct abonent{
    char name[10];
    char second_name[10];
    char tel[10];
} abonent;

void add_abonent(abonent *abonents, int count){
    printf("\nInput abonent's name: ");
    scanf("%9s", abonents[count].name);
    printf("\nInput abonent's second name: ");
    scanf("%9s", abonents[count].second_name);
    printf("\nInput abonent's phone number: ");
    scanf("%9s", abonents[count].tel);
}

void del_abonent(abonent *abonents, int count){
    char name[10], second_name[10];
    printf("Input abonent's name: ");
    scanf("%9s", name);
    printf("\nInput abonent's second name: ");
    scanf("%9s", second_name);
    for (int i = 0; i < count; i++)
        if ((strcmp(name, abonents[i].name) == 0) & (strcmp(second_name, abonents[i].second_name) == 0)){
            for (int j = i; j < count; j++)
                abonents[j] = abonents[j+1];
            break;
        }
}

void find_abonent(abonent *abonents, int count){
    char name[10];
    printf("Input abonent's name: ");
    scanf("%9s", name);
    for (int i = 0; i < count; i++)
        if (strcmp(name, abonents[i].name) == 0)
            printf("\n%s %s %s\n", abonents[i].name, abonents[i].second_name, abonents[i].tel);
}
int main(){
    abonent *abonents = (abonent*)calloc(1, sizeof(abonent));
    int choose, count = 0;
    char name[10], second_name[10], ch;
    while (1){
        printf("\nOptions:\n\t1) Add abonent\n\t2) Delete abonent\n\t3) Search by abonent's name\n\t4) Print all records\n\t5) Exit\nInput your choice: ");
        scanf("%d", &choose);
        if (choose == 5)    break;
        switch (choose){
        case 1:
            abonents = (abonent*)realloc(abonents, sizeof(abonent));
            add_abonent(abonents, count);
            count++;
            break;
        case 2:
            del_abonent(abonents, count);
            count--;
            break;
        case 3:
            find_abonent(abonents, count);
            break;
        case 4:
            for (int i = 0; i < count; i++)
                printf("\n%s %s %s\n", abonents[i].name, abonents[i].second_name, abonents[i].tel);
            break;
        default:
            printf("\nPlease, input number in range 1-5\n\n");
            break;
        }
    }
    free(abonents);
}