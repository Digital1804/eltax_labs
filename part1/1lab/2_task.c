#include "stdio.h"
#include "stdlib.h"

int main(){
	int num, count = 0;
	printf("Input negative number:");
	scanf("%d", &num);
	int *arr = (int*)calloc(32, sizeof(int));
	while (count<32){
		arr[count++] = num & 1;
		num = num >> 1;
	}
	printf("\nResult:\t");
	for (count; count > 0 ; count--)
		printf("%d", arr[count-1]);
	printf("\n");
	free(arr);
	return 0;
}