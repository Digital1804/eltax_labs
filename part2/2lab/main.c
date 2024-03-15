#include <stdio.h>
#include "library/calclib.h"

int main(){
    int choose;
    double a, b, res;
    while (1){
        printf("\nOptions:\n\t1) Adding two numbers\n\t2) Subtracting two numbers\n\t3) Multiplying two numbers\n\t4) Division of two numbers\n\t5) Exit\nInput your choice: ");
        scanf("%d", &choose);
        if (choose == 5)    break;
        printf("Input first number: ");
        scanf("%lf", &a);
        printf("Input second number: ");
        scanf("%lf", &b);
        switch (choose){
        case 1:
            res = my_addictive(a, b);
            printf("\nResult: %lf", res);
            break;
        case 2:
            printf("\nResult: %lf", my_subtractive(a, b));
            break;
        case 3:
            printf("\nResult: %lf", my_multiply(a, b));
            break;
        case 4:
            res = my_division(a, b);
            if (res == 0)   printf("\nDivision by zero is impossible");
            else            printf("\nResult: %lf", res);
            break;
        default:
            printf("\nPlease, input number in range 1-5\n\n");
            break;
        }
    }
}