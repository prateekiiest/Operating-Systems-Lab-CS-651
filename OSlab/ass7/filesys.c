#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>	// for open()
#include <sys/types.h>	
#include <fcntl.h>		// for open()
#include <dirent.h>

typedef int File;
int FScount =0;


// inode structure

typedef struct{

	int type;  // denoting whether its a file or a folder
	int size; // size of file or folder
	char *file_name;   // NAME OF THE FILE OR THE FOLDER DEPENDING ON THE TYPE
	int id_inode; // the block id of the inode
	int db_count; // number of datablocks it corresponds to
	int DB[MAX_DATABLOCK_SIZE];  // the datablock array

}inode;



// Structure for the superblock

typedef struct{

	long int filesize; // size of the osfile in the filesystem
	long int block_size; // size of the block

	int inode_count;  // number of inodes currently
	int db_count; // no. of datablocks currently
	int inode_start;  // start block of inode
	int db_start; // start block of data block
	int free_inode; // no. of free inodes
	int free_DB; // no. of free data blocks currently
	int inode_root; // root inode number
	int IBM[INODE_MAP_SIZE]; // the inode bit map
	int DBM[DATABLOCK_MAP_SIZE]; // the datablock bit map


}SB;














// FUNCTIONS 

// Creating File System

void Create_FileSystem(char *filename, long int block_size, long int file_size)
{


	struct SB *sb, *sb2;
	File fp;
	struct inode *in;

	sb2 = (struct SB *)malloc(sizeof(struct SB));
	sb = initSB(block_size, file_size);



	fp = open(filename, O_CREAT|O_RDWR);
	chmod(filename, 0777);
	if(fp == -1)
	{

		printf("File can not be opened");
	}


	ftruncate(fp, file_size);

	sb->free_inode--;
	sb->free_DB--;
	sb->IBM[0] = 128;
	sb->DBM[0] = 128;

	lseek(fp,0,SEEK_SET);
	write(fp,(void *)sb,sizeof(*sb));

	in = (struct inode *)malloc(sizeof(struct inode));
	in->type= 0;  // folder means  = 0
	in->name = "root";
	in->id_inode =1; //  root inode havinf inode = 1
	in->dblock_count = 0;

	for(int i=0;i<in->dblock_count;i++)
	{
		in->DB[i] = 0;
	}

	struct dblock_folder


}



//initializing the superblock
struct SB *initSB(long int FileSize,long int BlockSize)
{
	

	struct SB *super_block;
	int i,j;

	super_block=(struct SB *)malloc(sizeof(struct SB));				//Allocating super_block for superblock
	

	
	super_block->filesize=FileSize*1048576;	// converting to Bytes from Megabytes 1 MB = 1048576 Bytes						
	super_block->blocksize=BlockSize;							
	super_block->inode_count=MAX_INODE_COUNT;								
	
	super_block->dblock_count=MAX_DATABLOCK_COUNT;						
	super_block->free_inode=super_block->inode_count;					
	super_block->free_DB=super_block->dblock_count;  					


	for(i=0;i<MAX_INODE_COUNT/8;i++)  
	{
		super_block->inodemap[i]=0;     //  initially the inodemap will contain all 0's , since everything is empty
	}
	for(j=0;j<MAX_DATABLOCK_COUNT/8;j++)
	{
		super_block->dblockmap[i]=0;   //  similarly initially the datablock_map will contain all 0's since everything is empty at first
	}		

	super_block->inode_start=SUPERBLOCK_BLOCK_COUNT*super_block->blocksize;  // super_blockecifying the position of first inode					 	
	super_block->dblock_start=(super_block->inode_count+SUPERBLOCK_BLOCK_COUNT)*super_block->blocksize; // super_blockecifying position of the first data block						
	super_block->inode_root=1;  // root inode set to a value of 1

 	// returning the intialized superblock  

	return super_block;
}




char **command_entry(char  *cl)

{
	char ** command;
	int ind;

	command = (char **)malloc(maximum_command_size*sizeof(char *));
	ind = 0;
	int len = strlen(command); //  length of the command

	int i,j;

	for(i=0;i<len;i++)
		{

			command[i] = (char *)malloc(word_size_max*sizeof(char));

		}

	for(i=0;i<len;i++)
	{

		if(cl[i] != ' ')
		{
			command[ind][j++] = cl[i];
		}
	
	else
		{																	
			command[index][j]='\0';											
			j=0;																														
			index++;
		}

	}
	command[index][j]='\0';

	return command;	

}



void exec_command(char **command)												//Takes the list of words and executes te command
{
	/*First it reads the list of words of the command starting with the first word. It detects which operation has to be performed
	and uses one of mySystemCalls passing the required arguments as given in corresuper_blockonding  words in the word array*/

	if(strcmp(command[0],"use")==0)					
	{													//Word array = ["use" , "(filename)" , "as" , "(Drive Name)"]
		printf("Mounting FileSystem on file = %s as drive %s\n",command[1],command[3]);
		mount(command[1],command[3]);							// Calling function for mounting passing filename,drivename as parameters
	}

	else if(strcmp(command[0],"cp")==0)						
	{													
		cp(command[1],command[2]);			// Perform the copy function
	}


	//mkfs osfile1 512 10MB
	else if(strcmp(command[0],"mkfs")==0)						
	{								//Command from user = "mkfs" , "(filename)" , "(block_size)" , "(file_size) MB"
		long int fileSize;
		long int blockSize;


		fileSize=10*(command[3][0]-'0')+command[3][1]-'0';		//Extracting filesize from word array
		sscanf(command[2],"%ld",&blockSize);				//Extracting blocksize from word array
		printf("Making FILESYSTEM with %s file of size %ld with blocksize = %ldB\n",command[1],fileSize,blockSize);
		Create_FileSystem(command[1],blockSize,fileSize);			// Calling function for FS creation on file(filename)

	}

	else if(strcmp(command[0], "mv") == 0)
	{


		// perform the move operation 
	}

	else if(strcmp(command[0], "ls") == 0)
	{42

		// list all the files under that directory.   
	}


	else if(strcmp(command[0], "rm") == 0)
	{

		// remove the corresponding file name
	}

	else if(strcmp(command[0], "exit") == 0)
	{

		exit(1); //  exit the program gracefully (terminate normally)
		// remove the corresponding file name
		// close all files
	}




}






void cp(char *src,char *dest)
{
	//Copies file src to dest
	char *d1,*d2;  
	int mode;
	char **pathr;
	char **pathw;
	int size;
	int fds,fdd;

	char *reads;

	//strchr return char pointer to first occurence of a character in a string, returns NULL if not found
	d1=strchr(src,':');   
	d2=strchr(dest,':');
	/*Searches both file for presence of ':'. 
	OS FIle will not have :. since its a simple osfile
	*/
	if(d1==NULL && d2==NULL)  //  incase of os file hence d1 or d2 will be null
		mode =0;  // kernel mode 
	else if(d1==NULL && d2!=NULL)
		mode =1;  //  user mode
	

	switch(mode)
	{
		case 0:					//Both pointers = NULL, Thereby both OS files
			
			printf("Permission Denied because they are OS files\n");
			break;
		
		case 1:					// Only first pointer NULL, Thereby copy from OSfile to a virtual file on our FS
			
			fds=open(src,O_RDWR); // open file with name src in read/write mode
			size=lseek(fds,0,SEEK_END);									//Finds size of source OS file
			reads=(char *)malloc(size*sizeof(char));
			lseek(fds,0,SEEK_SET);
			read(fds,(void *)reads,size);								//Reads from source OS file
			pathw=return_Path(dest);
			writef(pathw,reads); // write to the file denoted by the path file name			

			

	}

}



// Listing all the files under a file system
void ls(char *fsname)  
{


	DIR *mydir;
    struct dirent *myfile;
    struct stat fs_stat;

    char buf[512];
    mydir = opendir(fsname);
    while((myfile = readdir(mydir)) != NULL)
    {
        sprintf(buf, "%s/%s", *fsname, myfile->d_name);
        stat(buf, &fs_stat);
        printf("%zu",fs_stat.st_size);
        printf(" %s\n", myfile->d_name);
    }
    closedir(mydir);


}





int main()
{

	char *c=(char *)malloc(30*sizeof(char));

	printf("  ---->  IMPLEMENTING FILESYSTEM   \n");
		while(1)
	{
		printf("MY_FS > ");
		
		gets(c);   // gets each command as input from the user
		char **cm=getCommand(c);
		
		for(int i=0;i<permissible_command_size;i++)
		{
			if(strcmp(cm[i],"")!=0)
				printf("%s\n",cm[i]);  // prints the corresponding command out into the terminal
			else
				break;
		}
			
		exec_command(cm);  // execute the particular command

		// if user enters, Ctrl + C break from the loop
	}
}