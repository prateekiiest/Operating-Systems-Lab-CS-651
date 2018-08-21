#include <stdio.h>
#include <sys/ipc.h> /* for shmget() shmctl() */
#include <sys/shm.h> /* for shmget() shmctl() */
#include <sys/sem.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h> /* for creating input and output array */
#include <pthread.h> /* for thread functions */


/* Following function (runner) will be executed by each thread */
/* This signature of runner() is fixed as imposed by pthread_create().
   That is, runner() must take 'void *' as parameter and return 'void *' */

void *runner( void *param);  // the runner function where we will be executing our program
   
struct v {   // the structure that will be passed to the runner function

	int *input_data;  // the input data that we will be working on (the given array)
	int *output_data;  // output data (performing operations and storing the output)
	int stage; //at which stage the current operation is performing 
	int thread;  // the current thread 
	};

/* Defining mutex */
pthread_mutex_t p_mutex;  // we will be using this later

int *data;
int *global_output;  // define an output function which will be constant throughout (all elements set to 0)
int *tdat;

int main()
{
	int size;
	printf("Enter Size of the Array : ");  // enter the array size
	scanf("%d",&size);

	int *input_data =  (int *)malloc(size*sizeof(int)); // input data
	int *output_data = (int *)malloc(size*sizeof(int)); // output data
	
	int j;
	for(j=0;j<size;j++)
	{	int value;
		printf("\nEnter array element for data[%d] :",j);
		scanf("%d",&value);
		input_data[j] = value;    // TAKING USER INPUT FOR THE INPUT DATA (ARRAY)
	}
	global_output = output_data;
	
	int dynamic_size = (size);  // size must be a power of 2 in this case (ASSUMPTION)
 	int stag = 1; // (Initial stage, starting with stag = 1)
 	// after each iteration stag will be doubled 
	while(dynamic_size != 1 )  // while dynamic_size != 1   at the last position there will be only one element
	{
		int i = 0;
		pthread_t tid[dynamic_size];  //Thread ID
		pthread_attr_t attr; /*Set of thread attributes required to be passed in pthread_create() */
       		 /* Set the default attributes */
		/* int pthread_attr_init(pthread_attr_t *attr); */
        pthread_attr_init(&attr);  /* same attr will be used for creating all the threads */

		int mutex_status;
		/* Initialize the mutexes */
		//int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
		mutex_status = pthread_mutex_init(&p_mutex, NULL);

		if (mutex_status != 0) { /* pthread_mutex_init() failed */
			printf("pthread_mutex_init() failed:");
			exit(1);
		}
					
		for(i=0;i<dynamic_size;i++)   // loop through the whole array having size = dynamic_size
		{	int status;
			struct v *dat; /* each thread will be passed with i and j in 'struct v' */
	 
			/* Allocate a structure where values of i and j is put for passing it to the thread */
			dat = (struct v *) malloc(sizeof(struct v));
         		/* Assign a row and column for each thread */
			dat->input_data = input_data;  // assigning input data to the input array
			dat->output_data = global_output;  // assigning output data to the output array     		
			dat->thread = i; // the thread that will be executing
			dat->stage =stag; // at stage we are computing the array
	
			status = pthread_create(&tid[i], &attr, runner, dat);
			if (status != 0) { /* pthread_create() failed */
				/* Do not use perror since pthreads functions  do  not  set  errno. */
				// perror("pthread_create() Failed: "); Cannot use this here 
				//printf("%d\n",*dat->output_data);
				/* Consult pthread_create() manual for information on return value for failure */
				exit(1);
				}	
			int status2;
         		/* The parent thread waits for all the threads to complete */
			/* int pthread_join(pthread_t thread, void **retval) */
         	status2 = pthread_join(tid[i], NULL);
			if (status2 != 0) { /* pthread_join() failed */
				/* Do not use perror since pthreads functions  do  not  set  errno. */
				// perror("pthread_join() Failed: "); Cannot use this here
				/* Consult pthread_join() manual for information on return value for failure */
				exit(1);
			}
			data = dat->input_data;   // input data
			tdat = dat->output_data;  // current modified output data
	
	}
		int k;
		printf("Input data is ");
		for(k=0;k<dynamic_size;k++)
		{
			printf("%d ",data[k]);
		}   // printing the output data at each stage
		printf("\n");
		printf("Output data is ");
		for(k=0;k<dynamic_size;k++)
			{
			printf("%d ",tdat[k]);}   // printing the output data at each stage
		printf("Stage is %d\n",stag);  // at which stage we are operating
		stag = stag*2; // for each stage we are multiplying by 2
		dynamic_size = dynamic_size/2;  // after each stage size gets halved
	}	
	return 0;
}

int status;
void *runner(void *param) {

    struct v *dat; /* the structure that holds our data */
    int l, sum = 0;
    dat = (struct v *)param; /* the structure that holds our data */
	//printf("Stage %d\n", (dat->stage)/2);
	int *inp = dat->input_data;
	int *oup = dat->output_data;
	//printf("Input Data is %d\n", *inp);
	int st = dat->stage;
        /*Row multiplied by column */
    int thr = dat->thread;
	/* int pthread_mutex_lock(pthread_mutex_t *mutex); */
	status =  pthread_mutex_lock(&p_mutex); /* check status for error */
	// A[2i] = A[2i] + A[2i+1]
	// At each stage this range gets multiplied by 2 
    oup[2*st*thr] = inp[2*st*thr] + inp[st*((2*thr)+1)];
	oup[st*((2*thr)+1)] = 0;             // Critical Section 
	//getchar(); /* for waiting user response */
	dat->input_data = oup;
	dat->output_data = global_output;
	status = pthread_mutex_unlock(&p_mutex); /* check status for error */
	//printf("%d\n",*dat->output_data);
	/* Free the data malloced in the main thread */
	//free(param);
        /* Exit the thread */
    pthread_exit(0);

}

  



