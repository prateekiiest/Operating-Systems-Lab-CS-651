#include<stdio.h>
#include<malloc.h>

#define size 5

int main()
{

int *arr = (int*)malloc(size*sizeof(int));

arr[0] = 1;
arr[1] = 2;

printf("%d", arr[1]);


return 0;
}



