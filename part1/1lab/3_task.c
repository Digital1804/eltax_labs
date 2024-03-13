#include "stdio.h"
#include "stdlib.h"

int main(){
	unsigned int num, count = 0, zeros = 0, ones = 0;
	printf("Input positive number:");
	scanf("%d", &num);
	int *arr = (int*)calloc(1, sizeof(int));
	int ch;
	while (num>0){
		ch = num & 1;
		arr[count++] = ch;
		num = num >> 1;
		arr = (int*)realloc(arr, sizeof(int));
		if (ch) 	ones++;
		else		zeros++;
	}
	printf("\nzeros: %d\tones:%d\n", zeros, ones);
	free(arr);
	return 0;
}