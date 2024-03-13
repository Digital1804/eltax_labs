#include "stdio.h"
#include "stdlib.h"

int main(){
	int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	printf("Array:\t");
	for (int i=0; i <10; i++)
		printf("%d ", *(arr+i));
	printf("\n");
	return 0;
}