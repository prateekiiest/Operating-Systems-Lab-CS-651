#include<stdio.h>
#include<unistd.h>

void main()
{
    FILE *fptr;
    char filename[15];
    char ch;
 
    printf("Enter the filename to be opened \n");
    scanf("%s", filename);
    /*  open the file for reading */
    fptr = fopen(filename, "r`");
    // parent process opens the file

 int p;  
p =  fork();
if(p==0)
{    

while (ch != EOF)
    {
        printf ("%c", ch);
        ch = fgetc(fptr);  //  child process tries to read
    }
 fclose(fptr); // file closed by child
}

//fclose(fptr);
}
