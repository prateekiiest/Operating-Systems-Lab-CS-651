#include <stdio.h>
#include <unistd.h> /* for using fork() */
#include <sys/wait.h> /* for using wait() */
#include <sys/types.h> /* using kill(2) from this library*/
#include <sys/ipc.h> /* using shmget() shmctl() from library*/
#include <sys/shm.h> /* using shmget() shmctl() from library */
#include <signal.h> 
#include <errno.h> /* for perror */
#include <stdlib.h>



#define TOTAL_STUDENTS 10 // Total Number of students = 10 


int shmid;  // Shared memory ID , declared globally



// Following function is used for releasing the shared memory when Ctrl+C is pressed on the command line. (Interrupt)
typedef void (*sighandler_t)(int);

void releaseSHM(int signum) {
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
                fprintf(stderr, "kill susccesful.\n");// The process is already killed and hence this line may not get printed :)

        } else if (status == -1) {
                perror("kill failed.\n");  // Cannot kill the given process
                fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
        } else {
                fprintf(stderr, "kill(2) returned wrong value.\n");
        }

}



// Main function

int main()
{
        // All Declarations
	int status;
	key_t mykey; // key defined to be used while defining the shared memory segment
	pid_t pid = 0; // initializing process id = 0, which is used later for checking wait() status.
	pid_t process = 0;   
	sighandler_t shandler;


        // CREATING SHARED MEMORY using the shared memory functions
	
	mykey = ftok("/home/OSlab/ass4/student.c", 1);  /* same key is being used by the teacher code also 
						      such that both the codes point to same shared memory location using identical
							keys*/

	// Getting the shared memory id. 0777 is used for granting read, write ,execution  public access to the file.
	shmid =  shmget(mykey, sizeof(int), IPC_CREAT | 0777);
	

	if (shmid == -1) {             
        	perror("shmget() failed: ");
		/* shmget() failed() */
		exit (1);  // exit the student process	
	}

	printf("shmget() returns shmid = %d.\n", shmid);
	// 	printing the shmid

       
	shandler =  signal(SIGINT, releaseSHM);
	int *roll = shmat(shmid, NULL, 0);
	
        if(*roll != -1)
	{  
		printf("Teacher didn't start taking !!\n");
		printf("You came to class early. Wait for teacher to come\n");	
		exit(1);  // exit the student process	
	}


	while (1) {
		if (*roll == -1) {	
			// Take student roll number as input and mark for attendance
			int rollno;
			printf("Enter student roll number :");
			scanf("%d", &rollno);
			printf ("Student having roll no %d is present", rollno);

				*roll = rollno;		/*  (*roll) = rollno that was entered as input for the roll number.
							This (*roll) value will be used to mark the attendance in the 
							teacher file corresponding to the teacher file.*/
								
			}
		
		

		
			exit(0);   // when one student process has enterred his roll number exit the student process.
		}
		
	
	
}

