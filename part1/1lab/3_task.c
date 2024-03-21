#include "stdio.h"
#include "stdlib.h"

int main(){
	unsigned int num, count = 0, zeros = 0, ones = 0;
	printf("Input positive number:");
	scanf("%d", &num);
	int ch;
	while (num>0){
		ch = num & 1;
		num >>= 1;
		ch? ones++ : zeros++;
	}
	printf("\nzeros: %d\tones:%d\n", zeros, ones);
	return 0;
}