#include "stdio.h"
#include "stdlib.h"

int main(){
	int *arr = (int*)calloc(1, sizeof(int)), local_num, i = 0;
	char ch;
	printf("Input array:");
	while (1){
		scanf("%d%c", &local_num, &ch);
		arr[i++] = local_num;
		if (ch == '\n'){
			break;
		}
		arr = (int*)realloc(arr, sizeof(int));
    }
	printf("\nResult:\t");
	for (--i; i>=0; i--){
		printf("%d ", arr[i]);
	}
	printf("\n");
	free(arr);
	return 0;
}