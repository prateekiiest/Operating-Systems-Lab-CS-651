#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<unistd.h>
#include<sys/stat.h>	// for open()
#include<sys/types.h>	
#include<fcntl.h>		// for open()


	
#define permissible_command_size 10					// Maximum number of words in a command
#define MAX_TERM_SIZE 20				// Maximum size of a word of a command
#define MAX_DATABLOCK_PER_FILE 10 			// Indicating maximum number of data blocks per file
#define BLOCK_COUNT 20280					// Total number of blocks in a FS except for superblock
#define SUPERBLOCK_BLOCK_COUNT 25			// Number of blocks required for Superblock
#define MAX_INODE_COUNT 1024				// Maximum number of inodes for every FS

#define MAX_DATABLOCK_COUNT 1024*10-25		// Maximum number of data blocks for every FS
#define IBM_SIZE 128					// Maximum size of IBM of Superblock
#define DBM_SIZE 2429					// Maximum size of Data Block Map of Superblock
#define MAX_FS_COUNT 10						// Maximum number of FileSystem instances implemented





typedef int File;							// Storing integer return type of open() under the name of File
int FScount=0;							// Storing index of unused FS in FS array
                                 // FS => File System


// Inode Structure
typedef struct
{
	int type;	//  type :  whether its a file or a folder 
	int id_inode;	// the corresuper_blockinding id of the inode 				
	int size;	  // Indicates size of file/folder
	char *file_name;								// Indicating name of file/folder							
	int dblock_count;						// Indicating numbr of datablocks for file
	int dblock[MAX_DATABLOCK_PER_FILE];		// Array of block numbers for datablocks of the file/folder

}inode;


// SuperBlock Structure

typedef struct {

	long int filesize;						// File size of the osfile of the FileSystem
	long int block_size;						// Size of each block
	int inode_count;						// Number of inodes						
	int DB_count;						// Number of data blocks
	int free_inode;					// Number of free inodes
	int free_DB;					// Number of free datablocks
	int inode_start;						// Block number where inode starts
	int dblock_start;						// Block number where data block starts
	int inode_root;						// Inode Number where information of root directory is stored	
	int IBM[IBM_SIZE];			// Bitmap of inodes	( 1 for used, 0 for free ) 
	int DBM[DBM_SIZE];			// Bitmap of data blocks ( 1 for used , 0 for free ) 


}SB;




// Filesystem structure
struct FS
{
 	char *FS_name;							// Name of Mounted Drive							
	char *File_name;						// Name of source file of filesystem, that is the OSfile the filesystem was made on
	long int size;							// Size of OSfile/source file
	File fd;								// File descriptor of OSfile
	struct SB sb;						// Superblock of OS file

}	array_filesystem[MAX_FS_COUNT];
// Array of mounted File Systems



struct DB_folder						// Structure to represent datablock of a folder
{
	int id_inode;							// Inode number of corresuper_blockonding inode to which the data block is related
	char * inode_name;						// Name of Folder
	int dblock_no;							// Indicates which data block it is occupying
	int content_count;						// Denotes number of contents
	char *content_names[MAX_FOLDER_DBLOCK_CONTENT];// Array of Names of Contents of Folder
	int content_inodes[MAX_FOLDER_DBLOCK_CONTENT];	// Array of Inode Numbers of Contents of folder
	long int content_sizes[MAX_FOLDER_DBLOCK_CONTENT]//Array of sizes of contents
};





							

// The main interface for getting commands from the user side
char **Command_getter(char* command);				
void exec_command(char **list_of_commands);				

// Defined SYSTEM CALLS in our file implementation
void Create_FileSystem(char *filename,long int blockSize,long int fileSize);			//Makes Filesystem on that OSFile
void mount(char *filename,char *drive);										// Mounts FS on OSFile(filename) as drive(drive)
void cp(char *src,char *dest);												// Copies file from src to dest						


struct SB *initSB(long int fileSize,long int blockSize);				// Initializes the superblock structure of the FS on (filename)
struct SB *Read_SuperBlock(char *filename);										// Reads the superblock structure of the FS on (filename)




char **Command_getter(char *cl)
{
	
// Gets the command from the user 	
    // This will be used later for parsing the command

	char **command;//stores the list of words of the command
	int l;//Stores length of command
	int index;// Stores index of word array(list of words)
	int i,j;// Indexing/loop variables


	command=(char **)malloc(permissible_command_size*sizeof(char *));// Allocating space for maximum possible command size
	index=0;
	j=0;
	l=strlen(c1);

	for(i=0;i<permissible_command_size;i++)
	{command[i]=(char *)malloc(MAX_TERM_SIZE*sizeof(char));
	}

	for(i=0;i<l;i++)														// Storing the words character wise into the array of words
	{
		if(c1[i]!=' ')  // Stripping the whole command by space
			command[index][j++]=c1[i];											
		
		else
		{ 												
			command[index][j]='\0';												// array by a NULL and go to next entry in array to start
			j=0;															// storing next word															
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

	if(strcmp(command[0],"use")==0)					//If command is use
	{													//Word array = ["use" , "(filename)" , "as" , "(Drive Name)"]
		printf("Mounting FileSystem on file = %s as drive %s\n",command[1],command[3]);
		mount(command[1],command[3]);							// Calling function for mounting
	}

	else if(strcmp(command[0],"cp")==0)						
	{													
		cp(command[1],command[2]);			// Perform the copy function using the cp command
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

		
	else
	{

		printf("Command function not implemented till now");
	}


}

void Create_FileSystem(char *filename,long int blockSize1,long int fileSize1)
{
	//Makes Filesystem with size of block=blockSize1 on that OSFile(filename) of size fileSize1

	struct SB *sb,*sb2; // creating the superblovk
	File fp;
	struct inode *rt;
	sb2=(struct SB *)malloc(sizeof(struct SB));		//Allocating super_blockace to store SB
	sb=initSB(fileSize1,blockSize1);						// Initialization fo SB
	fp=open(filename,O_CREAT|O_RDWR);						// Opening file descriptor
	chmod(filename,0777);									// Setting permissions for above file
	if(fp==-1)
	{
		fprintf(stderr,"File cannot be opened\n");
		return;
	}
	
	
	
	ftruncate(fp,fileSize1);							// Setting size of OSFile to fileSize1

	//Initialize SB for root directory inode
	sb->free_inode--;
	sb->free_DB--;
	sb->IBM[0]=128;
	sb->DBM[0]=128;								// We use the first inode as root directory inode
	
	lseek(fp,0,SEEK_SET);								
	write(fp,(void *)sb,sizeof(*sb));					
	//Initializing root directory inode
	rt=(struct inode *)malloc(sizeof(struct inode));	//Allocating  for root directory inode
	rt->type=0;										// root is a folder , hence we print (0) 
	rt->name="root";								// name of root folder = "root"
	rt->id_inode=1;									// inode number of root folder   
	rt->dblock_count=0;								
	for(int i=0;i<rt->dblock_count;i++)
	{
		rt->dblock[i]=0;
	}

	//Initializing the first datablock for root folder
	struct DB_folder f1;
	f1.id_inode=rt->id_inode;
	f1.inode_name=(char *)malloc(MAX_TERM_SIZE*sizeof(char));
	strcpy(f1.inode_name,rt->name);
	f1.dblock_no=1;//Since this is the first dblock for the inode
	f1.content_count=0;//Root folder empty as of now
	for(int i=0;i<MAX_FOLDER_DBLOCK_CONTENT;i++)
	{
		f1.content_names[i]=(char *)malloc(MAX_TERM_SIZE*sizeof(char));
		f1.content_inodes[i]=0;
	}
	rt->dblock_count++;
	rt->dblock[rt->dblock_count-1]=f1.dblock_no;


	//Writing root directory inode
	long int offset=sb->inode_start+(sb->inode_root-1)*blockSize1;		//Setting offset for writing root folder 
	lseek(fp,offset,SEEK_SET);												// Setting file descriptor
	write(fp,(void *)rt,sizeof(*rt));										// Writing to file
	//Writing root directory folder datablock
	offset=sb->dblock_start+(f1.dblock_no-1)*blockSize1;
	lseek(fp,offset,SEEK_SET);												// Setting file descriptor
	write(fp,(void *)&f1,sizeof(f1));	
	
	
	lseek(fp,0,SEEK_SET);				
	read(fp,(void *)sb2,sizeof(*sb));
	printf("FileSize of SuperBlock:= %ld\n",sb2->filesize);
	printf("BlockSize of SuperBlock:= %ld\n",sb2->blocksize);
	close(fp);




	

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
		super_block->IBM[i]=0;     //  initially the IBM will contain all 0's , since everything is empty
	}
	for(j=0;j<MAX_DATABLOCK_COUNT/8;j++)
	{
		super_block->DBM[i]=0;   //  similarly initially the datablock_map will contain all 0's since everything is empty at first
	}		

	super_block->inode_start=SUPERBLOCK_BLOCK_COUNT*super_block->blocksize;  // super_blockecifying the position of first inode					 	
	super_block->dblock_start=(super_block->inode_count+SUPERBLOCK_BLOCK_COUNT)*super_block->blocksize; // super_blockecifying position of the first data block						
	super_block->inode_root=1;  // root inode set to a value of 1

 	// returning the intialized superblock  

	return super_block;
}


//Reads the superblock from the OSfile the filesystem was implemented on
struct SB *Read_SuperBlock(char *filename)
{
	

	struct SB *super_block;
	super_block=(struct SB *)malloc(sizeof(struct SB));			//Allocating super_blockace to store SB
	File fd=open(filename,O_RDWR);
	lseek(fd,0,SEEK_SET);
	read(fd,(void *)super_block,sizeof(*super_block));							//Reading SB from OSfile into super_block
	close(fd);
	return super_block;
}



/* Functionf for Mounting the FileSystem

Corresponding command -> "Use"
*/

void mount(char *filename,char *drive)
{
	//This function mounts File System on OSFile filename on drive(drive)

	struct SB *sb;
	int l=strlen(drive);
	//array_filesystem = array of mounted FS
	//Setting drive name of mounted FS
	array_filesystem[FScount].FS_name=(char *)malloc(MAX_TERM_SIZE*sizeof(char));
	strcpy(array_filesystem[FScount].FS_name,drive);
	printf("array_filesystem[FScount].FS_name = %s\n",array_filesystem[FScount].FS_name);
	array_filesystem[FScount].File_name=(char *)malloc(MAX_TERM_SIZE*sizeof(char));
	strcpy(array_filesystem[FScount].File_name,filename);                                                             
	array_filesystem[FScount].fd=open(filename,O_RDWR);	//Setting fileDescriptor of mounted FS

	
	array_filesystem[FScount].sb=*(Read_SuperBlock(filename));  //Setting superblock of mounted FS
	array_filesystem[FScount].size=array_filesystem[FScount].sb.filesize; //Setting size of mounted FS


	printf("Superblock filesize:= %ld\n",array_filesystem[FScount].sb.filesize);
	printf("Superblock blocksize:= %ld\n",array_filesystem[FScount].sb.blocksize);

	FScount++;			//Increasing the index to the next unused location in array_filesystem

	
	/*array_filesystem[FScount]
	char *FS_name;
	char *File_name;
	long int size;
	File *fd;
	struct SB sb;*/
}







int main()
{

	char *c=(char *)malloc(30*sizeof(char));

	printf("  ---->  IMPLEMENTING FILESYSTEM   \n");
		while(1)
	{
		printf("MY_FS > ");
		
		gets(c);   // gets each command as input from the user
		char **cm=Command_getter(c);
		
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