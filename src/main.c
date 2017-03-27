#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PRODUCTAMOUNT 5
#define EQUIPMENTAMOUNT 8
#define MAXORDER 200


struct Order{
	int num;
	int startDate;
	int dueDate;
	char product;
	int quantity;
	int remainQty;
};
typedef struct Order Order;

struct AssemblyLine{
	int ordernum;
};

struct OrderList{
	struct Order* head;
	struct Order* tail;
};
typedef struct OrderList OrderList;

struct DueNode{
	int key;
	int dueDate;
	struct DueNode* next;
};
typedef struct DueNode DueNode;

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
int addBatchOrder(int count, Order* order, char *fileName){
	char orderNum[10];
	char startDate[10];
	char dueDate[10];
	char product[10];
	char quantity[10];
	//char buf[80];
	FILE* name;
	name = fopen(fileName, "r");
	if(name == NULL){
		printf("file open failed\n");
		return 0; 
	}
	while(fscanf(name, "R%s D%s D%s Product_%s %s\n", orderNum, startDate, dueDate, product, quantity) != EOF){
		// Indicate the start of a new order.
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

/*
 * This is input module for keyboard input
 * writePipe for
 */ 
int addOrder(int count, Order* order){
    char orderNum[10];
    char startDate[10];
    char dueDate[10];
    char product[10];
    char quantity[10];
    scanf(" R%s D%s D%s Product_%s %s", orderNum, startDate, dueDate, product, quantity);
    order[count].num = atoi(orderNum);
    order[count].startDate = atoi(startDate);
    order[count].dueDate = atoi(dueDate);
    order[count].product = product[0];
    order[count].quantity = atoi(quantity);
    order[count].remainQty = atoi(quantity);
    printf("%s %s %s %s %s\n", orderNum, startDate, dueDate, product, quantity);
    count++;
    return count;
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
void inputProductInfo(char* input, int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT]){
    FILE* in = fopen(input, "r");
	int i,k;
	for(i=0; i<10; i++){
		for(k=0; k<10; k++){
			productInfo[i][k] = 0;
		}
	}
	char* result;
	int productNum = 0;
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
	return;
}

/*
 * qsort Compare function for FCFS
 * a: Order pointer
 * b: Order pointer
 * return a positive number if a start latter than b
 */ 
int cmpFCFS(const void *a, const void *b){
	Order* aa = (Order*)a;
	Order* bb = (Order*)b;
	return (aa->startDate - bb->startDate);
}

int cmpOrder(const void *a, const void *b){
    Order* aa = (Order*)a;
    Order* bb = (Order*)b;
    return (aa->num - bb->num);
}

/*
 * exam whether the line and the equipment is available for the production
 * lineState: current line state
 * equipState: current equipment state
 * productInfo: product equipment relationship
 * product: the product order needed
 * return 1 if line1 is available, 2 line2, 3 line3, 0 if not
 */
int qulifyIn(int lineState[3], int equipState[EQUIPMENTAMOUNT], int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT], char product, int startDate, int date){
	int productNum = product - 'A';
	int i;
	if((date+1) < startDate) return 0;
	// if the equipment product need is not available
	for(i=0; i<EQUIPMENTAMOUNT; i++){
		if(equipState[i] == 1 && productInfo[productNum][i] == 1) return 0;	
	}
	// if productline i+1 is available
	for(i=0; i<3; i++){
		if(lineState[i] == 0)
			return (i+1);
	}
	return 0;	
}


/*
 * Method for link list, delete the head and return the key;
 */
int deleteHead(DueNode** head){
	if(*head == NULL) return -1;
	int key = (*head)->key;
	DueNode* previous = *head;
	*head = (*head)->next;
	free(previous);
	return key;
}

/*
 * Add node to a appropriate position, according to their duedate. 
 */
int addNodeEDF(DueNode** head, int key, int dueDate){
	if(*head == NULL){
		*head = (DueNode*) malloc(sizeof(DueNode));
		(*head)->key = key;
		(*head)->dueDate = dueDate;
		(*head)->next = NULL;
	}	
	DueNode* current = *head;
	DueNode* previous = NULL;
	DueNode* my = (DueNode*) malloc(sizeof(DueNode));
	my->key = key;
	my->dueDate = dueDate;
	my->next = NULL;
	if((*head)->dueDate > dueDate){
		my->next = *head;
		*head = my;
		return 1;
	}
	
	while(current != NULL){
		if(current->dueDate > dueDate){
			previous->next = my;
			my->next = current;
			return 1;
		}
		previous = current;
		current = current->next;
	}
	current = my;
	return 1;
}

/* test whether the time is sufficient to finish this job
 * order: the order need to be tested
 * date: current date
 * return 1 if sufficient, 0 if not
 */
int canFinish(Order order, int date){
	int requireDate = order.remainQty / 1000;
	//printf("requireDate:%d date:%d duedate:%d \n", requireDate, date+1, order.dueDate);
	if((order.dueDate - date) >= requireDate) return 1;
	return 0;
}

/* Transmit orderlist and reject list to parent
 * 
 */
void transResult(int line[3][60], int rejectList[MAXORDER], int rejectNum, int writePipe){
	//communicate to the parent
	char aBuf[10];
	char bBuf[10];
	char cBuf[10];
	int i;
	for(i=0; i<60; i++){
		sprintf(aBuf, "%d", line[0][i]);
		sprintf(bBuf, "%d", line[1][i]);
		sprintf(cBuf, "%d", line[2][i]);
		write(writePipe, aBuf, 10);
		write(writePipe, bBuf, 10);
		write(writePipe, cBuf, 10);
	}
	
	char rejectBuf[10];
	sprintf(rejectBuf, "%d", rejectNum);
	write(writePipe, rejectBuf, 10);
	for(i=0; i<rejectNum; i++){
		sprintf(rejectBuf, "%d", rejectList[i]);
		write(writePipe, rejectBuf, 10);
	}
	return;
}

/* receive and store result of scheduler
 * line: the shedule list
 * rejectList: rejectList, fill null with 0
 */
void storeSchedule(int line[3][60], int rejectList[MAXORDER], int readPipe){
	char aBuf[10];
	char bBuf[10];
	char cBuf[10];
	int i;
	for(i=0; i<60; i++){
		read(readPipe, aBuf, 10);
		read(readPipe, bBuf, 10);
		read(readPipe, cBuf, 10);
		line[0][i] = atoi(aBuf);
		line[1][i] = atoi(bBuf);
		line[2][i] = atoi(cBuf);
	}
	char reject[10];
	int rejectNum;
	read(readPipe, reject, 10);
	rejectNum = atoi(reject);
	for(i=0; i<rejectNum; i++){
		read(readPipe, reject, 10);
		rejectList[i] = atoi(reject);
	}
	for(i=rejectNum; i<MAXORDER; i++){
		rejectList[i] = 0;
	}
	return;
}

/* Schedule core for FCFS
 * orderList: a list contain the order
 * orderNum: the total order Num.
 * 
 */ 

void EDF(Order orderList[MAXORDER], int orderNum, int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT], int writePipe){
	int static line[3][60]; //store the order number each assembly line produce. 0 represent out of work
	int lineState[3]; // state of a line. 0: available 1: occupied
	int lineP[3];
	int rejectList[MAXORDER];
	int i, k;
	int equipState[EQUIPMENTAMOUNT]; //state of each equipment. 0: available 1: occupied
	int orderEDF[MAXORDER];
	int pointer=0; 
	int date = 0; //day counter.
	int rejectNum = 0; // Number of reject order
	DueNode* head=NULL; // linked list store the order available in ascending of the duedate.
	int productLine;
	for(i=0; i<MAXORDER; i++){
		orderList[i].remainQty = orderList[i].quantity;
	}
	for(i=0; i<3; i++){
		lineState[i] = 0;
		lineP[i] = 0;
	}
	for(i=0; i<MAXORDER; i++){
		rejectList[i] = 0;
	}
	for(i=0; i<3; i++){
		for(k=0; k<60; k++){
			line[i][k] = 0;
		}
	}
	for(i=0; i<EQUIPMENTAMOUNT; i++){
		equipState[i] = 0;
	}
	int key = -1;
	qsort(orderList, orderNum, sizeof(Order), cmpFCFS); //sort the orderlist in ascending order of start date.
	while(date < 60){
		//add new order to EDF list
		while(orderList[pointer].startDate <= (date+1) && pointer < orderNum){
			addNodeEDF(&head, pointer, orderList[pointer].dueDate); //enqueue
			pointer++;
		}	
		// accept new order
		while(1){	
			if(key == -1){ // if no key has been assigned
				key = deleteHead(&head);
				if(key == -1) break; //queue is empty, no new work can be assigned
			}
			while(!canFinish(orderList[key], date)){
				//printf("==%d==%d==%d\n", orderList[key].num, rejectNum, key); 
				rejectList[rejectNum] = orderList[key].num;
				rejectNum++;
				key = deleteHead(&head);
				if(key == -1) break; // if queue is empty
			}
			
			if(key == -1) break;
 			productLine = qulifyIn(lineState, equipState, productInfo, orderList[key].product, orderList[key].startDate, date);
			if(productLine == 0) break; //the current order need to be product is not available now, we need to wait,
			else{
				for(i=0; i<EQUIPMENTAMOUNT; i++){
					int productNum = orderList[key].product - 'A';
					if(productInfo[productNum][i] == 1) equipState[i] = 1; 
				}	
				lineState[productLine-1] = 1;
				line[productLine-1][date] = orderList[key].num;
				lineP[productLine-1] = key;
				key = deleteHead(&head); //dequeue
			}
		}
		//working
		for(i=0; i<3; i++){
			if(lineState[i] != 0){
				orderList[lineP[i]].remainQty -= 1000; // reduce remain amount
				//if finish, change line state
				if(orderList[lineP[i]].remainQty == 0){
					lineState[i] = 0; 

					//change equipmentstate
					for(k=0; k<EQUIPMENTAMOUNT; k++){
						int productNum = orderList[lineP[i]].product -'A';
						if(productInfo[productNum][k] == 1) equipState[k] = 0;
					}
				}
				
				else line[i][date+1] = line[i][date]; // if not finish, do the same job next day.
			}
		}
		//printf("%d date: %d %d %d\n", date+1, line[0][date], line[1][date], line[2][date]);
		date++;
	}
	// put all remaining job to reject list.
	while(head != NULL){
		int key=deleteHead(&head); 
		rejectList[rejectNum++] = orderList[key].num; 
	}
	transResult(line, rejectList, rejectNum, writePipe);
    return;
}

/* Schedule core for FCFS
 * orderList: a list contain the order
 * orderNum: the total order Num.
 * 
 */ 
void FCFS(Order orderList[MAXORDER], int orderNum, int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT], int writePipe){
	int static line[3][60]; //store the order number each assembly line produce. 0 represent out of work
	int lineP[3]; //store the pointer
	int lineState[3]; // state of a line. 0: available 1: occupied
	int rejectList[MAXORDER];
	int i, k;
	for(i=0; i<MAXORDER; i++){
		orderList[i].remainQty = orderList[i].quantity;
	}
	
	for(i=0; i<3; i++){
		lineState[i] = 0;
		lineP[i] = 0;
	}
	
	for(i=0; i<3; i++){
		for(k=0; k<60; k++){
			line[i][k] = 0;
		}
	}
	
	for(i=0; i<MAXORDER; i++){
		rejectList[i] = 0;
	}
	
	int equipState[EQUIPMENTAMOUNT]; //state of each equipment. 0: available 1: occupied
	for(i=0; i<EQUIPMENTAMOUNT; i++){
		equipState[i] = 0;
	}

	qsort(orderList, orderNum, sizeof(Order), cmpFCFS); //sort the orderlist in ascending order of start date.
	int date = 0; //day counter.
	int rejectNum = 0; // Number of reject order
	int pointer = 0; // the next order need to be execute.
	while(date < 60){
		int productLine;
		// accept new order
		while(1){
			// if the order at the beginning of the queue can't be finish, put it to the rejectlist.
			while(!canFinish(orderList[pointer], date) && pointer < orderNum){
				rejectList[rejectNum] = orderList[pointer].num;
				rejectNum++;
				pointer++;
			}
			// if no more order can be add to the queue
			if(pointer >= orderNum) break;
 			productLine = qulifyIn(lineState, equipState, productInfo, orderList[pointer].product, orderList[pointer].startDate, date); 
			if(productLine == 0) break; //the current order need to be product is not available now, we need to wait
			for(i=0; i<EQUIPMENTAMOUNT; i++){
				int productNum = orderList[pointer].product - 'A';
				if(productInfo[productNum][i] == 1) equipState[i] = 1; 
			}
			lineState[productLine-1] = 1;
			line[productLine-1][date] = orderList[pointer].num; 
			lineP[productLine-1] = pointer;
			pointer++;
		}
		//working
		for(i=0; i<3; i++){
			if(lineState[i] != 0){
				orderList[lineP[i]].remainQty -= 1000; // reduce remain amount
				//printf("==%d %d==\n", line[i][date], orderList[lineP[i]].remainQty);
				//if finish, change line state
				if(orderList[lineP[i]].remainQty == 0){
					lineState[i] = 0; 
					
					//change equipmentstate
					for(k=0; k<EQUIPMENTAMOUNT; k++){
						int productNum = orderList[lineP[i]].product -'A';
						if(productInfo[productNum][k] == 1) equipState[k] = 0;
					}
				}
				else{
					line[i][date+1] = line[i][date]; // if not finish, do the same job next day.
				}
			}
		}
		//printf("%d date: %d %d %d\n", date+1, line[0][date], line[1][date], line[2][date]);
		date++;
	}
	
	// put all remaining job to reject list.
	for(i=pointer; i<orderNum; i++){
		rejectList[rejectNum++] = orderList[pointer].num;
		pointer++;
	}

	transResult(line, rejectList, rejectNum, writePipe);
    return;
} 

int main(){
    printf("\n\n   ~~Welcome to ALS~~\n\n");
    Order order[MAXORDER];
    int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT];
    char input[] = "product_configuration.txt";
    inputProductInfo(input, productInfo);
    int count = 0;
    int i,j;
    int line[3][60];
    int rejectList[MAXORDER];
    while(1){
        printf("Please enter:\n>");
        char buffer[20];
        scanf("%19s", buffer);
        int command = commandChoose(buffer);
        if(command == 1){
            count = addOrder(count, order);
        }
        if (command == 2) {
            char file[20];
            scanf(" %s", file);
            count = addBatchOrder(count, order, file);
        }
        if(command == 3){
            char scheduler[10];
            scanf(" %s", scheduler);
            int pid_run;
            int fd_reject[2];
            
            if(pipe(fd_reject) < 0){
                printf("Pipe error\n");
                exit(1);
            }
            
            pid_run = fork();   //fork a process for runALS
            
            if ( pid_run < 0)
            {
                printf("Fork Failed\n");
                exit(1);
            }
            
            else if ( pid_run == 0 )		//process for runALS
            {
                close(fd_reject[0]);
                int pid_report, fd[2];
                
                if(pipe(fd) < 0){
                    printf("Pipe pc error\n");
                    exit(1);
                }
                pid_report = fork();
                
                if ( pid_report < 0)
                {
                    printf("Fork Failed\n");
                    exit(1);
                }

                else if ( pid_report == 0){	//process for output
                    //printf("sp");
                    close(fd[1]);
                    int i;
                    int line[3][60];
                    int reject[MAXORDER];  //just store, no use.
                    storeSchedule(line, reject, fd[0]);
                    for(i = 0; i<60; i++){
                        printf("DATE:%d 1:%d 2:%d 3:%d\n", i+1, line[0][i], line[1][i], line[2][i]);
                    }
                    close(fd[0]);
                    exit(0);
                }
                else{
                    close(fd[0]);
                    if(strcmp(scheduler, "-FCFS") == 0){
                        FCFS(order, count, productInfo, fd[1]);		//write line to output
                        FCFS(order, count, productInfo, fd_reject[1]);  //wrire rejectlist and line to main process 
                    }
                    if(strcmp(scheduler, "-EDF") == 0){
                        EDF(order, count, productInfo, fd[1]);
                    }
                    
                    wait(NULL);
                    close(fd[1]);
                    close(fd_reject[1]);
                    exit(0);
                }
            }
            wait(NULL);
            close(fd_reject[1]);
            storeSchedule(line, rejectList, fd_reject[0]);  //read from line and runALS for rejectlist.
            close(fd_reject[0]);
        }
      
        if (command == 4) {			//printReport
            int pid;
            pid = fork();
            
            if ( pid < 0)
            {
                printf("Fork Failed\n");
                exit(1);
            }
            
            else if ( pid == 0 )
            {
                qsort(order, count, sizeof(Order), cmpOrder);
                int c = 0;
                for(i = 0; i < count; i++){
                    for(j = 0; j < count; j++){
                        if(order[j].num == rejectList[i]){
                            printf("R%d D%d D%d Product_%c %d\n", order[j].num, order[j].startDate, order[j].dueDate, order[j].product, order[j].quantity);
                            c++;
                        }
                    }
                }
                printf("%d", c);
                exit(0);
            }
            
            wait(NULL);
            exit(0);

        }
        if (command == 5) {
            break;
        }
    }
    
    return 0;
}

