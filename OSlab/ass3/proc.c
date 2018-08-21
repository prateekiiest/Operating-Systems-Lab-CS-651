#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <errno.h>


int main(int argc, char *argv[])
{
        int j;

        
	int n; // For creating 3 processes
	int i;
	int status;

        n = 2; //  number of executable files
	
       

	pid_t p = 1; // process id  
	       	

	for(i=1;i<=n;i++)

	{
         if(p !=0) //  if it is a parent process

		{
		p = fork();
		
		if(p == 0) // if it is a child process
	  	{	printf("proci: %d, pid: %d\n",i,p); //  print the process id and the iterator
                        char *argh[] = {argv[i], NULL};                	
			status = execve(argv[i],argh, NULL);
			if(status == -1){ 

                        perror("execve func() failed");// if status == -1 , its error
			}
			 
	
		}			 
				
		}	

	}




}





