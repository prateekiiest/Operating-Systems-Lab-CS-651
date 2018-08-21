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
    fptr = fopen(filename, "w");  // opening the file to write
    
fprintf (fptr, "This is a line  by parent\n");
int p;  
p =  fork();
if(p==0)
{    

 fprintf (fptr, "This is a line  by child\n");  // Child process is able to write into the file
 fclose(fptr);  // Now child closes the file and hence parent will not be able to write anything further.

}
//fclose(fptr);
}
