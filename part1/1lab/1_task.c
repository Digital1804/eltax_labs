#include "stdio.h"
#include "stdlib.h"

int main(){
	unsigned int num, count = 0;
	printf("Input positive number:");
	scanf("%d", &num);
	int *arr = (int*)calloc(1, sizeof(int));
	while (num>0){
		arr[count++] = num & 1;
		num = num >> 1;
		arr = (int*)realloc(arr, sizeof(int));
	}
	printf("\nResult:\t");
	for (count; count > 0 ; count--)
		printf("%d", arr[count-1]);
	printf("\n");
	free(arr);
	return 0;
}