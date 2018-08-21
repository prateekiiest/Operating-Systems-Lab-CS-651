#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
        int j;

        
	int n; // For creating 3 processes
	int i;
	int status;
	pid_t childpid;

        n = 2; //  number of executable files
	
       

	pid_t p = 1; // process id  
	       	

	for(i=1;i<=n;i++)

	{
         if(p !=0) //  if it is a parent process

		{
		p = fork();
		
		if(p == 0) // if it is a child process
	  	{  childpid = p;

	printf("proci: %d, pid: %d\n",i,p); //  print the process id and the iterator
                        char *arg[] = {argv[i], NULL};                	
			status = execve(argv[i],arg, NULL);
			if(status == -1){

 perror("execve func() failed");// if status == -1 , its error


			}
			 
	
		}			 
				
		}
 int returnStatus;    
    waitpid(childpid, &returnStatus, 0);  // Parent process waits here for child to terminate.

    if (returnStatus == 0)  // Verify child process terminated without error.  
    {
       printf("The child process terminated normally.");    
    }

    if (returnStatus == 1)      
    {
       printf("The child process terminated with an error!.");    
    }	

	}




}





