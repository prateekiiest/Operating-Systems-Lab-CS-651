#include <stdio.h>
#include <unistd.h> /* for using fork() */
#include <sys/wait.h> /* for using wait() */
#include <sys/types.h> /* using kill(2) from this library*/
#include <sys/ipc.h> /* using shmget() shmctl() from library*/
#include <sys/shm.h> /* using shmget() shmctl() from library */
#include <signal.h> 
#include <errno.h> /* for perror */
#include <stdlib.h> // for using abs()


#define rows 2
#define cols 2

int d; // value (used to check if the difference between two elements of the matrix fall below this value)

int A[rows][cols];

int r;
int shmid;

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


int avg(int A[rows][cols], int row, int col)
{
    int sum = 0;
	int count = 0;
	int av;
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            
            // ignore rows which are out of range.
            if ( (i + row) < 0 || (i + row >= rows) )
		continue;
            // ignore columns  which are out of range.
            if ( (j + col) < 0 || (j + col >= cols) )
		continue;
            // add to sum
		
            sum += A[i + row][j + col];
		count+=1;
		
        }
    }

	av = sum/(count);
	
    return av;         
		  
}

void check(int A[rows][cols])
{
int row, col;

for(int i = 0;i< row;i++)
{
	for(int j = 0;j<col;j++)
		{ for(int k = i+1;k<row;k++)
		{ for(int h = j+1;h<col;h++)
		if (abs(A[i][j] - A[k][h]) < d)
			{printf("Difference falls below given value of d");
			exit(0);
			}
}


}

}

}


int avg(int A[rows][cols], int row, int col);
void check(int A[rows][cols]);

int main()
{




int ic, jc; 
int noproc; //  Number of child processes to be created 
int ia, ja;
   for(ia=0; ia<rows; ia++) {
      for(ja=0;ja<cols;ja++) {
         printf("Enter value for A[%d][%d]:", ia, ja);  // user input for the array
         scanf("%d", &A[ia][ja]);
      }
 }


printf("Enter the value of d :\n");
scanf("%d",&d);  // reading user input as `d`





int ib,jb;


sighandler_t shandler;

shandler =  signal(SIGINT, releaseSHM);

while(1)
{ check(A[ib][jb]);
}



shmid = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT | 0777);
printf("shmget() returns shmid = %d.\n", shmid);  // print outputs the shared memory id

pid_t pid = 1;
pid = fork();

int *pi_child = shmat(shmid,NULL,0);


for(ib=0;ib<rows;ib ++)
{ 	
	for(jb =0;jb<cols;jb ++)
{

	if(pid !=0)  // if its a parent process
	{	
		pid = fork();
		if(pid == 0)  // if its a child process
		
		{ 
		
	     r =  *(pi_child)+ (ib*jb) + 1;
	     
	     r = avg(A,ib,jb);

		A[ib][jb] = avg(A,ib,jb);

		
		

		printf("%d\n",r); // printing each element of the computed matrix
		//printf("process: %d, pid: %d\n",ic,pid);  // process id of the child process
 		}

               else{
			// parent process
			
 		   wait(NULL);  // parent waits for the child to exit

			
		}
	}

}
}



return 0;
}


