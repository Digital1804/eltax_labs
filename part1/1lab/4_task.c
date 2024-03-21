#include "stdio.h"
#include "stdlib.h"

int main(){
	int num, s_num, var;
	printf("Input positive number:");
	scanf("%d", &num);
	printf("Input second positive number(less 256):");
	scanf("%d", &s_num);
	num = (num & 0xFFFF00FF) | (s_num << 8);
	printf("Result: %d\n", num);
	return 0;
}