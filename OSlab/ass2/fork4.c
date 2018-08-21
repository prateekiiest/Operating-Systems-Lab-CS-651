#include<stdio.h>
#include<unistd.h>


int main()
{

int n;
int a[n];
printf("Enter the numbers");
int j;
for(j=0;j<n;j++)
{
   scanf("%d", &a[i]); //  Enter the n numbers
}


int i;
i = 0;
int  p;

p = fork(); // first child created

while(i< n-1)
{
	if(p!=0)
	{
    //  if its a parent process, create a new child process under it
   	p = fork();
	}
	
	i+= 1;

}


printf("Hello\n");

return 0;
}
