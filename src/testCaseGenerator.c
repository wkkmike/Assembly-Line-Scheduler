#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>  
#define AMOUNT 100
#define SEED 2324

int main(){
	FILE* output;
	output = fopen("testcase.txt", "w");
	int num=1;
	int k = time(NULL);
	for(num=1; num<=AMOUNT; num++){
		srand(k);
		k++;
		int startDate;
		int dueDate;
		int quantity;
		int productNum;
		startDate = rand() % 61 + 1;
		dueDate = rand() % (61 - startDate) + startDate;
		quantity = (rand() % 10 + 1) * 1000; 
		productNum = rand() % 5;
		char product = 'A' + productNum;
		fprintf(output,"R%03d D%03d D%03d Product_%c %d\n", num, startDate, dueDate, product, quantity);
	}
	return;
} 
