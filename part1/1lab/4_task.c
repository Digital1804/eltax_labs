#include "stdio.h"
#include "stdlib.h"

void out_bin(int num){
	int *arr = (int*)calloc(32, sizeof(int));
    for (int i = 0; i < 32; i++){
		arr[i] = num & 1;
		num = num >> 1;
    }
	printf("\nResult:\t");
	for (int i = 31; i >= 0 ; i--)
		printf("%d", arr[i]);
	printf("\n");
	free(arr);
}

int main(){
	int num, s_num, var;
	printf("Input positive number:");
	scanf("%d", &num);
	printf("Input second positive number(less 256):");
	scanf("%d", &s_num);
    out_bin(num);
    num &= 4278255615;
    s_num <<= 16;
    num |= s_num;
    out_bin(num);
	return 0;
}