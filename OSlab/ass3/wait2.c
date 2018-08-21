#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
        int j;

        
	int n; // For creating n processes
	int i;
	int status;


        n = 2; //  number of executable files
	
        int stat;
        pid_t pid;

	pid_t p = 1; // process id  
	       	

	for(i=1;i<=n;i++)

	{
         if(p !=0) //  if it is a parent process

		{
		p = fork();
		
		if(p == 0) // if it is a child process
	  	{  //childpid = p;

	printf("proci: %d, pid: %d\n",i,p); //  print the process id and the iterator
                        char *arg[] = {argv[i], NULL};                	
			stat = execve(argv[i],arg, NULL);
			if(stat == -1){

 perror("execve func() failed");// if stat == -1 , its error


			}

		abort();
	
	}

}

    if(wait(&status) == p)
      { if (WIFEXITED(status))
	 printf("normal termination, exit status = %d\n",WEXITSTATUS(status));

    else if (WIFSIGNALED(status))

	 printf("abnormal termination, signal number = %d\n",WTERMSIG(status));

}
			 
				
		
     
 	

}









}





