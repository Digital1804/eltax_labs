#include <stdio.h>

int main(int argc, char const *argv[])
{
    printf("It's a program %s\nArguments:", argv[0]);
    if (argc == 1){
        printf("No arguments");
    }
    else{
        for (int i = 1; i < argc; i++){
            printf("%d)%s\t", i, argv[i]);
        }
    }
    printf("\n");
    return 0;
}