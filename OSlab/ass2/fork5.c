#include<stdio.h>
#include<unistd.h>

long factorial(int n);

int main()
{

int n;
printf("Enter number");  // enter how many numbers for user
scanf("%d",&n);
int proc_no;
scanf("%d", &proc_no); //  Enter process no whose factorial is to be calculated

int a[n];
printf("Enter the numbers");
int j;
for(j=0;j<n;j++)
{
   scanf("%d", &a[j]); //  Enter the n numbers
}


int i;
i = 0;
int  p;

p = fork(); // first child created
int process = 1;
while(i< n-1)

{
        process += 1; 
	if(p!=0)
	{
    //  if its a parent process, create a new child process under it
   	p = fork();
        if(process == proc_no){
    		int z = factorial(a[proc_no]); // if process number = proc_no (ith process, print the factorial of that number)
    		printf("%d\n",z);
		break;

		}	




	}
	
	i+= 1;  // iterate till the end of all the processes

}


//printf("Hello\n");

return 0;
}


long factorial(int n)
{
  if (n == 0)
    return 1;
  else
    return(n * factorial(n-1));
}
