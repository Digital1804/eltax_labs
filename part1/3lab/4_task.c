#include "stdio.h"
#include "stdlib.h"

int main(){
	char *arr = (char*)calloc(1, sizeof(char));
	int *ptr;
	printf("Input string: ");
	int i = 0;
	while (1){
		scanf("%c", &arr[i]);
		if (arr[i] == '\n'){
			arr[i] = '\0';
			break;
		}
		arr = (char*)realloc(arr, sizeof(char));
		i++;
	}
	char  *arr2 = (char*)calloc(1, sizeof(char));
	int j = 0;
	printf("Input substring: ");
	while (1){
		scanf("%c", &arr2[j]);
		if (arr2[j] == '\n'){
			arr2[j] = '\0';
			break;
		}
		arr2 = (char*)realloc(arr2, sizeof(char));
		j++;
	}
	int flag = 0;
	for (int k = 0; k < i; k++){
		flag = k;
		for (int h = 0; h < j; h++){
			if (arr[k+h] != arr2[h]){
				flag = 0;
				break;
			}
		}
		if (flag)	break;
	}
	if (flag)	ptr = arr + flag;
	else	ptr = NULL;
	printf("Pointer: %p\n", ptr);
	free(arr);
	free(arr2);
	return 0;
}