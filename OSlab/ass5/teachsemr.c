#include <stdio.h>
#include <unistd.h> /* for fork() */
#include <sys/wait.h> /* for wait() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/ipc.h> /* for shmget() shmctl() */
#include <sys/shm.h> /* for shmget() shmctl() */
#include <sys/sem.h>
#include <signal.h> /* for signal(2) kill(2) */
#include <errno.h> /* for perror */
#include <stdlib.h>



#define TOTAL_STUDENTS 10


#define NO_SEM	1

#define P(s) semop(s, &Pop, 1);
#define V(s) semop(s, &Vop, 1);

struct sembuf Pop;
struct sembuf Vop;


int semid;

int student_ids[TOTAL_STUDENTS] = {0};



int shmid;  // the shared memory (shared) globally.

int shmid2;
int class_roll[TOTAL_STUDENTS] = {0};  //  teacher maintains the attendance registry.  

void attendance_sheet();

// Following function is used for releasing the shared memory when Ctrl+C is pressed on the command line. (Interrupt)
typedef void (*sighandler_t)(int);
void releaseSHM(int signum) 
{
        int status;
        // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
        status = shmctl(shmid, IPC_RMID, NULL); /* IPC_RMID is the command for destroying the shared memory*/
        if (status == 0) {
                fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
        } else if (status == -1) {
                fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
        } else {
                fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);
        }

        // int kill(pid_t pid, int sig);
        status = kill(0, SIGKILL);
        if (status == 0) {
                fprintf(stderr, "kill susccesful.\n"); // The process is already killed and hence this line may not get printed :)

        } else if (status == -1) {
                perror("kill failed.\n");
                fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
        } else {
                fprintf(stderr, "kill(2) returned wrong value.\n");
        }
}



void attendace_sheet()
	{
	int i;

	int count=0;
	for (i=0; i<TOTAL_STUDENTS; i++) 
		{  
		if (class_roll[i] != 1)
			count += 1;  // as more students present themselves, count increases.
		}

		printf("Number of students present is %d out of %d students\n", count,TOTAL_STUDENTS);
		// number of students present out of the total number of students present in the class.

	}




int main()
{
	int status;
	key_t mykey;

	pid_t pid = 0;
	pid_t p1 = 0; 


	sighandler_t shandler;

	mykey = ftok("/home/OSlab/ass4/student.c", 1);

	mykey2 = ftok("Process ID",1);

	shmid =  shmget(mykey, sizeof(int), IPC_CREAT | 0777);
	shmid2 =  shmget(mykey2, sizeof(int), IPC_CREAT | 0777);

	if (shmid == -1) {   
			perror("shmget() failed: ");
			/* shmget() failed() */
			exit (1);  
			}

	printf("shmget() returns shmid = %d.\n", shmid);

	union semun 
	{
	int	val; /* Value for SETVAL */ 
	struct semid_ds *buf; /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
	} setvalArg;

	setvalArg.val = 1;


	/* struct sembuf has the following fields */
	//unsigned short sem_num;  /* semaphore number */
        //short          sem_op;   /* semaphore operation */
        //short          sem_flg;  /* operation flags */

	Pop.sem_num = 0;
	Pop.sem_op = -1;
	Pop.sem_flg = SEM_UNDO;

	Vop.sem_num = 0;
	Vop.sem_op = 1;
	Vop.sem_flg = SEM_UNDO;
	

	semid = semget(mykey, NO_SEM, IPC_CREAT | 0777);
	if(semid == -1) {
		perror("semget() failed");
		exit(1);
	}

	// int semctl(int semid, int semnum, int cmd, ...);
	status = semctl(semid, 0, SETVAL, setvalArg);
	if(status == -1) {
		perror("semctl() failed");
		exit(1);
	}

		
	shandler =  signal(SIGINT, releaseSHM);
	int *roll = shmat(shmid, NULL, 0);
	int *process_id = shmat(shmid2, NULL, 0);

	if (roll == (void *)-1) 
		{ /* shmat fails */
		perror("shmat() failed at parent: ");
		exit (1); // exit the teacher process	
		}

	int stud = 0;

	//attendace_sheet();
	char cd;
	*roll = -1;
	* process_id = -1;
	while (1) {
		if (*roll != -1) 
			{	
			for(int k=0;k<TOTAL_STUDENTS;k++)
			{ if(class_roll[k] ==  *roll)
				printf("Roll number already exists. Enter new roll number");
				break;
			}
			class_roll[*roll-1]= (*roll);	
			student_ids[*process_id -1 ] = (*process_id);	  // Updating process id for each student			
			printf("Roll No present is %d",(*roll));
			// again initialize roll back to -1
			P(semid);
			
			*roll = -1;
			*process_id = -1;

			V(semid);
			
			
			}
			getchar();  // enter Ctrl+C to exit the teacher process.
			// Otherwise the teacher waits for each student to come.
			//attendace_sheet();
			
			
			//attendace_sheet();
			

			stud +=1;

			if(stud == TOTAL_STUDENTS+1)
				{ printf("Total attendance list filled up\n");
				printf("---------ATTENDANCE REPORT--------------");
				attendace_sheet();
				printf("----------------------------------------");
					exit(1);
				}
			
			}
			
}

