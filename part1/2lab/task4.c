#include "stdio.h"
#include "stdlib.h"

int main(){
	unsigned int N;
	printf("Input positive number:");
	scanf("%d", &N);
	int **arr = (int**)calloc(N, sizeof(int*));
	for (int i = 0; i < N ; i++){
        arr[i] = (int*)calloc(N, sizeof(int));
	}
	int up_row = 0, down_row = N-1, left_column = 0, right_column = N-1, value = 1;
	while (value <= N*N){
		for (int i = left_column; i <= right_column; i++){
			arr[up_row][i] = value++;
		}
		up_row++;
		for (int i = up_row; i <= down_row; i++){
			arr[i][right_column] = value++;
		}
		right_column--;
		for (int i = right_column; i >= left_column; i--){
			arr[down_row][i] = value++;
		}
		down_row--;
		for (int i = down_row; i >= up_row; i--){
			arr[i][left_column] = value++;
		}
		left_column++;
	}
	printf("\nResult:\n");
	for (int i = 0; i < N ; i++){
		for (int j = 0; j < N ; j++)
			printf("%d\t", arr[i][j]);
		printf("\n");
        free(arr[i]);
	}
	free(arr);
	return 0;
}