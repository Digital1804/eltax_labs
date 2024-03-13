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
	int num, s_num;
	unsigned char *ptr;
	ptr = &num;
	ptr+=2;
	printf("Input positive number:");
	scanf("%d", &num);
	out_bin(num);
	printf("Input second positive number(less 256):");
	scanf("%hhd", ptr);
	out_bin(num);
	printf("\nResult num: %d\n", num);
	return 0;
}