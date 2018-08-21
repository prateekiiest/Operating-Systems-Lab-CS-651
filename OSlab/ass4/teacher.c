#include <stdio.h>
#include <unistd.h> /* for fork() */
#include <sys/wait.h> /* for wait() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/ipc.h> /* for shmget() shmctl() */
#include <sys/shm.h> /* for shmget() shmctl() */
#include <signal.h> /* for signal(2) kill(2) */
#include <errno.h> /* for perror */
#include <stdlib.h>



#define TOTAL_STUDENTS 10


int shmid;  // the shared memory (shared) globally.
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

	shmid =  shmget(mykey, sizeof(int), IPC_CREAT | 0777);
	

	if (shmid == -1) {   
			perror("shmget() failed: ");
			/* shmget() failed() */
			exit (1);  
			}

	printf("shmget() returns shmid = %d.\n", shmid);

		
	shandler =  signal(SIGINT, releaseSHM);
	int *roll = shmat(shmid, NULL, 0);
	if (roll == (void *)-1) 
		{ /* shmat fails */
		perror("shmat() failed at parent: ");
		exit (1); // exit the teacher process	
		}

	int stud = 0;

	//attendace_sheet();
	char cd;
	*roll = -1;
	while (1) {
		if (*roll != -1) 
			{	
			class_roll[*roll-1]= (*roll);					
			printf("Roll No present is %d",(*roll));
			// again initialize roll back to -1
			*roll = -1;
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

