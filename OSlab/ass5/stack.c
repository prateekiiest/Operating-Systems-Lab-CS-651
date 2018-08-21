#include <stdio.h>
#include <unistd.h> 
#include <sys/ipc.h> /* for shmget() shmctl() */
#include <sys/shm.h> /* for shmget() shmctl() */
#include <sys/sem.h>
#include <errno.h> /* for perror */
#include <stdlib.h>
#include <stdbool.h> // for bool free
#include <malloc.h> 

#define MAX_STACK 2
#define NO_SEM	1
#define P(s) semop(s, &Pop, 1);
#define V(s) semop(s, &Vop, 1);

struct sembuf Pop,Vop;

// declaring all the keys globally
key_t mykey;


typedef struct
{
	key_t stackkey;  // key for creating shared stack
	int data_size; // data element size for each element in the stack
	int stack_size;  // size of the stack
	int stack_top;  // current stack_top
	int element_no; // present number of elements in the stack
	int *array; // = (int *)malloc(stack_size * sizeof(int));
	bool free;
	int semid;
}stackdesc;


union semun {
		int              val;    /* Value for SETVAL */
		struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
		unsigned short  *array;  /* Array for GETALL, SETALL */
		struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
	   } setvalArg;

/*
shstackget : Get stack from the shared stack
void *shstackget(key, element_size, stack_size)

	key : Used to check whether an existing stack is present with the same key. If existing stack is there, then the function
		returns only a pointer to the corresponding stack (void *stack_top).
		if new key, create a new stack
	

	element_size : Current number of the elements in the stack. This is used when no key is found for the existing stacks and 
			we need to create a new stack.

	stack_size : Denotes maximum number of elements that can be accomodated in the stack. This is used when no key is found for the 				existing stacks and we need to create a new stack.

*/

void shstackget(key_t key, int element_size, int stack_size){

	// mykey - already declared globally
	int stackid = shmget(mykey,sizeof(stackdesc) * MAX_STACK,IPC_CREAT | 0777);
	stackdesc *shared = shmat(stackid,NULL,0);  // attaching to the shared memory
	int status;		
	int c = 0;
	int i;
	// run through all the stacks to check for matching key
	for(i=0; i<MAX_STACK;i++){ 
		if((shared+i)->stackkey != key){
		c+=1;  // checking everytime, if stack key not found , so c gets incremented
		}
	 
		else{
		printf("Key already exists\n");
		break;
		}

	}

	// INTITIAL CASE - if the passed key is not found
	if(c ==  MAX_STACK)
	{	int j;
		for(int j=0; j<MAX_STACK;j++)  
		{ 
			if((shared+j)->stackkey == -1)  // stack key was initialized to -1 in main()
			{
			(shared+j)->stackkey = key;  // update key
			(shared+j)->data_size = element_size;  // update element size
			(shared+j)->stack_size = stack_size;  // update the current stack_size
			(shared+j)->array = (int *)malloc((shared+j)->stack_size * sizeof(int));  // allocate memory for the elements in the 	stack
			(shared+j)->semid= semget(key, NO_SEM, IPC_CREAT | 0777);  // creating  the semaphore with key = passed by the user
			if((shared+j)->semid == -1) {
				perror("semget() failed\n"); 
				exit(1);

			}

			// int semctl(int semid, int semnum, int cmd, ...);
			status = semctl((shared+j)->semid, 0, SETVAL, setvalArg);
			if(status == -1) {
				perror("semctl() failed\n");
				exit(1);
			}
			

			
			printf("New STack is created\n");
			printf("STack Key is %d\n",(shared+j)->stackkey);
			printf("STack Size created is  %d\n",(shared+j)->stack_size);

			break;
			}
	
		}

	}


}




/* Function for pushing an element into the stack

void shstackpush(key, element)

	key : Corresponding key of the stack where the element needs to be pushed. If key doesnot exist, throw error.
		key non-existent : perror("Stack does not exist. Create stack using shstackget") 

	element : element or object that needs to be pushed into the stack. (Assuming the stack is filled with elements of data_type integer.
			element to be pushed must also be of type integer) 

*/

void shstackpush(key_t key, int element)
{

	int stackid = shmget(mykey,sizeof(stackdesc) * MAX_STACK ,IPC_CREAT | 0777);
	stackdesc *shared = shmat(stackid,NULL,0);

	int i;
	int c= 0;
	for(i= 0;i< MAX_STACK; i++)
	{
		if((shared+i)->stackkey == key)  // check for the matching key
		{
		  // key exists
			printf("Stack key matched is %d\n",(shared+i)->stackkey);
		   // Push Function
			if((shared+i)->stack_top == (shared+i)->stack_size)  // check if stack is full
			{  
				printf("Stack Already full\n");   
			}

			else  // Push into the stack the element


			{	
				P((shared+i)->semid);				
				printf("Pushing element into the stack\n");
				(shared+i)->stack_top += 1;
				(shared+i)->array[(shared+i)->stack_top] = element;     // ***** CRITICAL SECTION ********
				(shared+i)->element_no += 1;
				(shared+i)->free = 0; // STack filled up , so its not free
				V((shared+i)->semid);

				int si;
				for(si=0;si<(shared+i)->stack_size;si++)
					printf("Stack element is %d\n",(shared+i)->array[si]);
				
				printf("Stack Pointer is now %d\n",(shared+i)->stack_top);
				
	
			}
	
			  
		}	

		
		else
		{	
		c += 1; // for each unmatched key counter is incremented until we get a stack for which key is matched
				
		}
		

	if(c== MAX_STACK)
		{
			printf("No such key exists\n");
			printf("Create a stack if possible by calling shstackget()\n");
			
		}

	}


}



/*Function for popping element from stack given a correspoding key
int shstackpop(key)

	key : Corresponding key of the stack where the element needs to be pushed. If key doesnot exist, throw error.
		key non-existent : perror("Stack does not exist. Create stack using shstackget") 


	Returns: element on the top of the stack.


*/

int shstackpop(key_t key)
{

	int stackid = shmget(mykey,sizeof(stackdesc)* MAX_STACK ,IPC_CREAT | 0777);
	stackdesc *shared = shmat(stackid,NULL,0);   // attach to the shared memory containing all the stacks


	int value;
	int i;
	for(i = 0;i< MAX_STACK; i++)
	{
		if((shared+i)->stackkey = key)  // check for matching key
		{ // Key exists
		
			if((shared+i)->stack_top == -1)
				{
				(shared+i)->free = 1; // Stack is empty , hence its free
				printf("Stack is Already Empty\n");
				break;  //  break from the loop					
				}

			else

				{    // Pop element from the stack and print it

				printf("Popping element from the stack\n");
				//P((shared+i)->semid);
				value = (shared+i)->array[(shared+i)->stack_top];
				(shared+i)->array[(shared+i)->stack_top] = 0; // initial value  // ***** CRITICAL SECTION ********
				(shared+i)->stack_top = (shared+i)->stack_top - 1;
				//V((shared+i)->semid);
			
				int si;
				for(si=0;si<(shared+i)->stack_size;si++)
						printf("STack element now is %d\n",(shared+i)->array[si]);
	

				}

		break;
		}

	}

}


/*shstackrm(key): Removes the stack with the corresponding key. In this case we just initialize the stack key to -1, as if the stack doesnot exist
*/
void shstackrm(key_t key)
{
	int i;

	int stackid = shmget(mykey,sizeof(stackdesc)* MAX_STACK ,IPC_CREAT | 0777);
	stackdesc *shared = shmat(stackid,NULL,0);

	for(i = 0;i< MAX_STACK; i++)
	{
	if((shared+i)->stackkey = key)
		{
		(shared+i)->stackkey = -1; // stack key updated to -1
		(shared+i)->data_size = 0; // no data ->  no data size
		(shared+i)->stack_size = 0;  // size of the stack updated to 0
		(shared+i)->stack_top = -1;  // indicating empty stack
		(shared+i)->element_no = 0; // no elements
		(shared+i)->free = 1;  // stack is free
		}
		
	}

}


// Function Prototypes

void shstackget(key_t t,int x,int y);
void shstackpush(key_t t, int x);
int shstackpop(key_t t);
void shstackrm(key_t key);




int main()
{

	int sem_status;
	mykey = ftok("myfile.c",1);

	int shmid;
	shmid = shmget(mykey, sizeof(stackdesc) * MAX_STACK ,IPC_CREAT | 0777);  // used for the getting shared memory for the stacks



	int shmid2;
	shmid2 = shmget(10, sizeof(int) ,IPC_CREAT | 0777);
	int *count = shmat(shmid2, NULL, 0); // used for initializing all the stacks




	
	setvalArg.val = 1;

	/* struct sembuf has the following fields */
	//unsigned short sem_num;  /* semaphore number */
	//short          sem_op;   /* semaphore operation */
	//short          tsem_flg;  /* operation flags */

	Pop.sem_num = 0;
	Pop.sem_op = -1; // sem = sem -1
	Pop.sem_flg = SEM_UNDO;

	Vop.sem_num = 0;
	Vop.sem_op = 1;  // sem = sem+1
	Vop.sem_flg = SEM_UNDO;


	


	stackdesc *shared = shmat(shmid, NULL, 0);  // attaching to the list of shared stacks


	int ret;

	


	// Initializing all the stacks

	// only the first process initializes all the stacks for whom *count = 0.
	// Rest for all other process *count will be 1 and hence the stacks won't be reinitialized 


	if(*count == 0)

	{
		for(int i=0;i< MAX_STACK;i++)
		{

			(shared+i)->stackkey = -1; // initially no stack exists, so stack key set to -1
			(shared+i)->data_size = 0; // data element size for each element in the stack
			(shared+i)->stack_size = 0;  // size of the stack
			(shared+i)->stack_top = -1;  // current stack top
			(shared+i)->element_no = 0; // current number of elements = 0
			//(shared+i)->array = NULL;
			// present number of elements in the stack
			(shared+i)->free = 1;  // initially all stacks were free

		}

	*count += 1;

	}

	
	
	shstackget(2,sizeof(int),5); // Create a stack with key = 2 and with integer elements of size 5
	

	shstackpush(2,6); // push integer 6 into the corresponding stack

	

	shstackget(3,sizeof(int),3);  // create another stack with key = 3 with integer elements of size 3

	shstackpush(3,8);  //  push 8 into the stack having key = 3
	shstackpush(3,7);  // push 7 into the stack with key = 3

	shstackpop(3);  // pop from stack having key = 3   


	shstackrm(2);   // remove the stack having key 2
	shstackrm(3); // remove the stack having key 3
	
	//int shmdt(const void *shmaddr);
	ret = shmdt(NULL);
	if(ret!=0)
		printf("shmdt failed()");

	return 0;

}

