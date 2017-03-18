#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct Order{
	int num;
	int startDate;
	int dueDate;
	char product;
	int quantity;
	int remainQty;
};
typedef struct Order Order;

struct OrderList{
	struct Order* head;
	struct Order* tail;
};
typedef struct OrderList OrderList;

/*
 * It will read a block of text from a file
 * name: the name of the file
 * it will return a string with the first element set to -1 if encounter a EOF
 */
char* readContent(FILE *name){
	char static content[50];
	char letter = fgetc(name);
	int count=0;
	if(letter == EOF){
		content[0] = -1;
		content[1] = 0;
		return content;
	}
	while(letter != ',' && letter != ' ' && letter != '\n' && letter != EOF){
		content[count] = letter;
		letter = fgetc(name);
		count++;
	}
	content[count] = 0;
	return content;
}

/*
 * This is input module for file input.
 * fileName: the name of the file contain the input.
 * writePipe: pipe No for write to the main process.
 */

void addBatchOrder(char *fileName, int writePipe){
	char orderNum[10];
	char startDate[10];
	char dueDate[10];
	char product[10];
	char quantity[10];
	char buf[80];
	FILE* name;
	name = fopen(fileName, "r");
	if(name == NULL){
		printf("file open failed\n");
		exit(1);
	}
	while(fscanf(name, "R%s D%s D%s Product_%s %s\n", orderNum, startDate, dueDate, product, quantity) != EOF){
		// Indicate the start of a new order.
		buf[0] = 'C';
		buf[1] = 'O';
		buf[2] = 0;
		write(writePipe, buf, 3);
		
		write(writePipe, orderNum, 10);
		write(writePipe, startDate,10);
		write(writePipe, dueDate, 10);
		write(writePipe, product, 10);
		write(writePipe, quantity, 10);
	}
	buf[0] = EOF;
	write(writePipe, buf, 2);
	return;
}

/*
 * This is input module for keyboard input
 * writePipe for
 */ 
void addOrder(int writePipe, int readPipe){
	
}

/*
 * This function is used by parent process to get input order
 * from input module and store it.
 * orderList: a link list store all order
 * readPipe: pipe for read
 * return: number of order
 */
int storeOrder(Order* order, int readPipe){
	int count = 0;
	char orderNum[10];
	char startDate[10];
	char dueDate[10];
	char product[20];
	char quantity[10];
	char buf[10];
	while(1){
		//check start
		read(readPipe, buf, 3);
		if(buf[0] != 'C' || buf[1] != 'O') break;

		// read from input module
		read(readPipe, orderNum, 10);
		read(readPipe, startDate, 10);
		read(readPipe, dueDate, 10);
		read(readPipe, product, 10);
		read(readPipe, quantity, 10);
		
		//store in the order array
		order[count].num = atoi(orderNum);
		order[count].startDate = atoi(startDate);
		order[count].dueDate = atoi(dueDate);
		order[count].product = product[0];
		order[count].quantity = atoi(quantity);
		order[count].remainQty = atoi(quantity);
		printf("%s %s %s %s %s\n", orderNum, startDate, dueDate, product, quantity);
		count++;
	}
	return count;
}

/* transfor a input command to corresponding int.
 * 1: addOrder
 * 2: addBatchOrder
 * 3: runALS
 * 4: printReport
 * 5: endProgram
 * -1: wrong input
 */
int commandChoose(char* input){
	if(strcmp(input, "addOrder") == 0) return 1;
	if(strcmp(input, "addBatchOrder") == 0) return 2;
	if(strcmp(input, "runALS") == 0) return 3;
	if(strcmp(input, "printReport") == 0) return 4;
	if(strcmp(input, "endProgram") == 0) return 5;
	return -1;
}

/*
 * A module to input the equipment a product need from configuration file
 * input: name of file contain the configuration info
 * It will return a 2-d array,
 * first dimension represent product
 * second dimension represent equipment
 * int[product][equipment]
 */
int** inputProductInfo(char* input){
	FILE* in = fopen(input, "r");
	int static productInfo[10][10];
	int i,k;
	for(i=0; i<10; i++){
		for(k=0; k<10; k++){
			productInfo[i][k] = 0;
		}
	}
	char* result;
	int productNum;
	while(1){
		result = readContent(in);
		if(result[0] == -1) break;
		if(strstr(result, "Product_") != NULL)
			productNum = result[8] -'A';
		else if(strstr(result, "Equipment_") != NULL){
			char* startOfNum = strchr(result, '_');
			startOfNum++;
			int equipmentNum;
			equipmentNum = atoi(startOfNum);
			productInfo[productNum][equipmentNum] = 1;
		}
	}
}
