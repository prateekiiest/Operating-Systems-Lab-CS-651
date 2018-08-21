#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<unistd.h>
#include<sys/stat.h>	// for open()
#include<sys/types.h>	
#include<fcntl.h>		// for open()
#include <sys/types.h>
#include <dirent.h>  // for DIR


// SOME DEFINITIONS USED

#define MAX_TERM_SIZE 30
#define MAX_NAME_WORDSIZE 30 // Maximum word size in each command
#define MAX_DATABLOCK_PER_FILE 16 // Per file the maximum no. of data blocks
#define permissible_command_size 10 // Maximum number of words in a command
#define SUPERBLOCK_BLOCK_COUNT 20 // Number of blocks required for Superblock (redundancy required)
#define MAX_INODE_COUNT 1024 // Maximum number of inode blocks that a FIle System can hold 
#define BLOCK_COUNT 20280 // (Total no. of blocks in FIle System)Inode Blocks + Data blocks(without superblock)
#define MAX_DATABLOCK_COUNT 1024*16-20	/* Maximum number of data blocks that a FIle System can hold 
					(max_inode_blocks*max_db_perfile) - no.of sb count */

#define IBM_SIZE 128	// Inode Bitmap maximum size
#define DBM_SIZE 2327  // Datablock BitMap maximum size
#define MAX_FOLDER_DBLOCK_CONTENT 10 // Maximum number of contents of folder
#define MAX_FS_COUNT 10	// Maximum number of FileSystem instances implemented
#define FILE_PATH_DEPTH_MAX 5	// Maximum Depth of path of a file

// Define Colors to be used during warnings 
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* First we need to create a structure for superblock, inode block and data blocks that will be stores
as File System and will contain necessary info about each files implemented in our FileSystem
*/

// INODE STRUCTURE FOR THE FILE SYSTEM
struct inode{
	int type; //  type :  whether its a file or a folder 
	int id_inode;	// the corresuper_blockinding id of the inode 				
	int size;	  // Indicates size of file/folder
	char *name;// Indicating name of file/folder							
	int dblock_count;// Indicating number of datablocks for file
	int dblock[MAX_DATABLOCK_PER_FILE];// Array of block numbers for datablocks of the file/folder
};

// SuperBlock Structure
struct SB{
	long int filesize;// File size of the osfile of the FileSystem
	long int blocksize;// Size of each block
	int inode_count;// Number of inodes						
	int dblock_count;// Number of data blocks
	int free_inode;// Number of free inodes
	int free_DB;// Number of free datablocks
	int inode_start;// Block number where inode starts
	int dblock_start;// Block number where data block starts
	int inode_root;	// Inode Number where information of root directory is stored	
	int IBM[IBM_SIZE];// Bitmap of inodes	( 1 for used, 0 for free ) 
	int DBM[DBM_SIZE];// Bitmap of data blocks ( 1 for used , 0 for free ) 
};


// Stucture to represent the datablock of a folder
struct dblock_folder{						
	int id_inode;	// Inode number of corresuper_blockonding inode to which the data block is related
	char *inode_name; // Name of Folder
	int dblock_no;	// Indicates which data block it is occupying
	int content_count; // Denotes number of contents
	char *content_names[MAX_FOLDER_DBLOCK_CONTENT];// Array of Names of Contents of Folder
	int content_inodes[MAX_FOLDER_DBLOCK_CONTENT];	// Array of Inode Numbers of Contents of folder
	long int content_sizes[MAX_FOLDER_DBLOCK_CONTENT];//Array of sizes of contents
};


// Filesystem structure
struct FS{
	char *FS_name;// Name of Mounted Drive							
	char *File_name;// Name of source file of filesystem
	long int size;							
	FILE *fd;								
	struct SB sb;// Superblock of OS file
};

struct FS FS_group[MAX_FS_COUNT]; // Creating an array of mounted File Systems


                                                  /* FUNCTION   DECLARATIONS */
							
//------USER INTERFACE FUNCTIONS------//
char **Command_getter(char* command); // Accepts the command and breaks into words
void exec_command(char **list_of_words); // Accepts the list of word and executes the command
void screen_design(void); // designs the screen
void help(void); // prints a list of implemented system calls

//-----MY SYSTEM CALLS-----//
void Make_FileSystem(char *filename,long int blockSize,long int fileSize);//Makes Filesystem on that OSFile
void mount(char *filename,char *drive);// Mounts FS on OSFile(filename) as drive(drive)
void cp(char *src,char *dest); // Copies file from src to dest
void mov(char *src,char *dest); // Moves file from src to dest								
void write_file(char **path,char *str); // Writes str into file denoted by file path
void readf(int Fsuper_blocks,int id_inode);
void ls_file(char *filename);

// -------------- AUXILLARY FUNCTIONS ------------------//
struct SB *sup_block_init(long int fileSize,long int blockSize); // Initializes the superblock structure of the FS on (filename)
struct SB *Read_SuperBlock(char *filename); // Reads the superblock structure of the FS on (filename)
int file_system_location(char *c);  // Finds the mounted FileSystem with drive name (c) from FS_group[]
char **file_Pathname(char *c1);		// Reads a path(c1) of a file and breaks it into "\" separated word array
struct inode *readInode(int Fsuper_blocks,int id_inode);	// Reads inode structure with inode no(id_inode) 
struct dblock_folder *readFolder(int Fsuper_blocks,int dblock_no);				    // Reads folder datablock given filename and data block number


typedef int File; // Storing integer return type of open() under the name of File
int FScount=0;	// Storing index of unused FS in FS array


// COMMAND LINE FUNCTIONS TO BE USED

// Storing the commands entered by the user from the command line
char **Command_getter(char *c1){
// Gets the command from the user 	
    // This will be used later for parsing the command
	char **command;//stores the list of words of the command
	int l;//Stores length of command
	int index;
	int i,j;
	// Allocating space for maximum possible command size
	command=(char **)malloc(permissible_command_size*sizeof(char *));
	index=0;
	j=0;
	l=strlen(c1);
	for(i=0;i<permissible_command_size;i++)
	{command[i]=(char *)malloc(MAX_TERM_SIZE*sizeof(char));
	}
	for(i=0;i<l;i++){ 									
			if(c1[i]!=' ')  // Stripping the whole command by space
			command[index][j++]=c1[i];											
		else{ 												
			command[index][j]='\0';	// array by a NULL and go to next entry in array to start
			j=0;																				
			index++;}
		}
	command[index][j]='\0';
	return command;
	}

//Takes the list of words and executes te command
void exec_command(char **command){												
	if(strcmp(command[0],"use")==0){ // Performing the Mounting operation using sys call "use"				
		printf(ANSI_COLOR_MAGENTA "Mounting FileSystem" ANSI_COLOR_RESET "\n");
		mount(command[1],command[3]);
		}
	else if(strcmp(command[0],"cp")==0){																
		cp(command[1],command[2]);	// Perform the copy function
		}
	else if(strcmp(command[0],"mkfs")==0){		
		long int fileSize;
		long int blockSize;
		fileSize=10*(command[3][0]-'0')+command[3][1]-'0';//Extracting filesize from command
		sscanf(command[2],"%ld",&blockSize); //Extracting blocksize from command
		printf(ANSI_COLOR_YELLOW "FILESYSTEM %s has been created" ANSI_COLOR_RESET "\n", command[1]);
		Make_FileSystem(command[1],blockSize,fileSize);	//Creating File System on filename
		}
	else if(strcmp(command[0], "mv") == 0){
		mov(command[1],command[2]); //  performing the mov operation. 
		}

	else if(strcmp(command[0], "ls") == 0){
		char *file_name = command[1];
		ls_file(*file_name) ; //  Printing the stat of the file_name 
		}
	
	else if(strcmp(command[0], "exit") == 0){	
		exit(1); //  exit the program gracefully (terminate normally)
		// remove the corresponding file name
		// close all files
		}
	}



void help(void){
	
	printf("\n");
	printf(ANSI_COLOR_CYAN"============================================================================="ANSI_COLOR_RESET "\n");
	printf("The following are the list of implemented system calls for the File System : \n");
	printf(ANSI_COLOR_BLUE"mount : Mounting File System"ANSI_COLOR_RESET"\n");
	printf(ANSI_COLOR_BLUE"mkfs : Making a new FIle System"ANSI_COLOR_RESET"\n");	
	printf(ANSI_COLOR_BLUE"mov : moving file from one place to another"ANSI_COLOR_RESET"\n");
	printf(ANSI_COLOR_BLUE"cp : copying file from one place to another\n");
	printf("ls : Printing the contents of a folder\n");
	printf("exit : Exitting out from the current FIle System"ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_CYAN"============================================================================="ANSI_COLOR_RESET"\n");
				
}

void screen_design(void){
	system("clear");
	int i;
	for(i=0;i<8;i++){
		printf("\n");}
	printf("                                        ==================================================\n");
	printf("\n");	
	printf("\033[22;34m                                                     File System Implementation\033[0m\n");
	printf(ANSI_COLOR_MAGENTA"                                                   Maintainer : Prateek Chanda"ANSI_COLOR_RESET"\n");
	printf("\n");
//		printf("\n");
	printf("                                        ===================================================\n");
	for(i=0;i<10;i++){
		printf("\n");}
	}


/* Creating the File System with block size = block_Size with the OSfile sized file_Size
First we need to create the file , which will contain info about the initilized info 
about the Super block for the root directory inode. 
Secondly it will contain the inode and the data block of the root folder*/



void Make_FileSystem(char *filename,long int block_Size,long int file_Size){
	
	struct SB *sb,*sup_b; // creating the superblock
	struct inode *in; // inode instance created
	sup_b = (struct SB *)malloc(sizeof(struct SB));		//Allocating super_blockace to store SB
	sb=sup_block_init(file_Size,block_Size); // initialize the Super Block with the given parameters
	File fp; 						
	fp=open(filename,O_CREAT|O_RDWR);	// Open file descriptor of the OSfile
	chmod(filename,0777);	// Setting permissions for above file (Read, Write, Execute Permissions)
	
    // Handling Exceptions
	if(fp==-1){
		printf(ANSI_COLOR_RED"Warning ! Error opensing OSFile !!!"ANSI_COLOR_RESET"\n");
		return;
	}
	// else if file can be opened
	ftruncate(fp,file_Size);// Setting size of OSFile to file_Size (it may be different than file_Size)
	//Initialize SB for root directory inode
	sb->free_inode--; //  decrease free
	sb->free_DB--;// decrease  free
	sb->IBM[0]=128; // bitmap initialization
	sb->DBM[0]=128;

	lseek(fp,0,SEEK_SET); // FIle pointer positioning to write		
	write(fp,(void *)sb,sizeof(*sb)); // write into the file the initial information of superblock					
	//Initializing root directory inode
	in=(struct inode *)malloc(sizeof(struct inode));	
	in->type=0;										
	in->name="root";								
	in->id_inode=1;									
	in->dblock_count=0;								
	for(int i=0;i<in->dblock_count;i++){
		in->dblock[i]=0;
	}
	//Initializing the datablock for root folder
	struct dblock_folder f1;
	f1.id_inode=in->id_inode;
	f1.inode_name=(char *)malloc(MAX_NAME_WORDSIZE*sizeof(char));
	strcpy(f1.inode_name,in->name);   // copy the folder name of root inode ("root")
	f1.dblock_no=1;//Since this is the first dblock for the inode
	f1.content_count=0;//Root folder empty as of now
	for(int i=0;i<MAX_FOLDER_DBLOCK_CONTENT;i++){
		f1.content_names[i]=(char *)malloc(MAX_NAME_WORDSIZE*sizeof(char));
		f1.content_inodes[i]=0;
	}
	in->dblock_count++;
	in->dblock[in->dblock_count-1]=f1.dblock_no;

	//Writing root directory inode
	long int index_pos=sb->inode_start+(sb->inode_root-1)*block_Size;	//Setting offset for writing root folder 
	lseek(fp,index_pos,SEEK_SET);		// Setting file descriptor
	write(fp,(void *)in,sizeof(*in)); // Writing
	//Writing root directory folder datablock
	index_pos=sb->dblock_start+(f1.dblock_no-1)*block_Size;
	lseek(fp,index_pos,SEEK_SET);												// Setting file descriptor
	write(fp,(void *)&f1,sizeof(f1));	
	close(fp); // Close the file after everything has been written
}


//initializing the superblock
struct SB *sup_block_init(long int FileSize,long int BlockSize){
	
	struct SB *super_block;
	int i,j;
	super_block=(struct SB *)malloc(sizeof(struct SB));				//Allocating super_block for superblock
	super_block->filesize=FileSize*1048576;   // 1 MB = 1048576 Bytes						
	super_block->blocksize=BlockSize;							
	super_block->inode_count=MAX_INODE_COUNT;			 // putting maximum inode count that it can hold for superblock					
	super_block->dblock_count=MAX_DATABLOCK_COUNT;						
	super_block->free_inode=super_block->inode_count; // since initially all are free	
	super_block->free_DB=super_block->dblock_count;  // since there are no data blocks initially					

	for(i=0;i<MAX_INODE_COUNT/8;i++){
		super_block->IBM[i]=0;     //  initially the IBM will contain all 0's , since everything is empty
	}
	for(j=0;j<MAX_DATABLOCK_COUNT/8;j++){
		super_block->DBM[i]=0;   //  similarly initially the datablock_map will contain all 0's since everything is empty at first
	}		
    // super_blockecifying the position of first inode	
	super_block->inode_start=SUPERBLOCK_BLOCK_COUNT*super_block->blocksize;  				 	
	// super_blockecifying position of the first data block
	super_block->dblock_start=(super_block->inode_count+SUPERBLOCK_BLOCK_COUNT)*super_block->blocksize;						
	super_block->inode_root=1;  // root inode set to a value of 1
 	// returning the intialized superblock  
	return super_block;
}


//Reads the superblock from the OSfile given by the filename
struct SB *Read_SuperBlock(char *filename){
	struct SB *super_block;
	super_block=(struct SB *)malloc(sizeof(struct SB));	//Allocating super_blockace to store SB
	File fd=open(filename,O_RDWR);
	lseek(fd,0,SEEK_SET); // position for reading
	read(fd,(void *)super_block,sizeof(*super_block)); //Reading SB from OSfile into super_block
	close(fd);
	return super_block;
}



// -----------------------------------------------------------------------

// IMPLEMENTATION OF MY SYSTEM CALLS


void ls_file(char *fsname){
	DIR *mydir;  // creating a directory file pointer
    struct dirent *myfile;
    struct stat fs_stat;  // will contain info about the directory
    char buf[512];
    mydir = opendir(fsname); // open the directory 
	if(mydir == NULL){
		printf(ANSI_COLOR_RED"File Entered : ls cannot be operated"ANSI_COLOR_RESET"\n");}
    while((myfile = readdir(mydir)) != NULL){  // read the directory
        sprintf(buf, "%s/%s", *fsname, myfile->d_name);
        stat(buf, &fs_stat);    
        printf("%zu",fs_stat.st_size); // printing the file sizes
        printf(" %s\n", myfile->d_name); // printing file name
	}
    closedir(mydir); // close the opened directory
}



// Our SYSTEM CALL FOR USE
void mount(char *filename,char *drive){
	//This function mounts File System on OSFile filename on drive(drive)
	struct SB *sb;
	int l=strlen(drive);
	FILE *fp;
	//FS_group = array of mounted FS
	//Setting drive name of mounted FS
	FS_group[FScount].FS_name=(char *)malloc(MAX_NAME_WORDSIZE*sizeof(char));
	strcpy(FS_group[FScount].FS_name,drive);
	printf(ANSI_COLOR_BLUE"New FIle System Created = %s"ANSI_COLOR_RESET"\n",FS_group[FScount].FS_name);
	FS_group[FScount].File_name=(char *)malloc(MAX_NAME_WORDSIZE*sizeof(char));
	strcpy(FS_group[FScount].File_name,filename); 
	fp = open(filename,O_RDWR);                              
	FS_group[FScount].fd=fp;	//Setting fileDescriptor of mounted FS
	FS_group[FScount].sb=*(Read_SuperBlock(filename));  //Setting superblock of mounted FS
	FS_group[FScount].size=FS_group[FScount].sb.filesize; //Setting size of mounted FS
	FScount++;			//Increasing the index to the next unused location in FS_group
}

void cp(char *src,char *dest){
	// src file to be moved over to the dest file
	char *p1,*p2;  // will be used for searching for character occurence in a string
	int mode;  // kernel / user mode
	char **pathr; // path of the file to be read 
	char **pathw; // path of the file to be written into
	
	//strchr return char pointer to first occurence of a character in a string, returns NULL if not found
	p1=strchr(src,':');   
	p2=strchr(dest,':');
	/*Searches both file for presence of ':'. 
	OS FIle will not have ':' since its a simple osfile
	*/
	if(p1==NULL && p2==NULL)  //  incase of os file hence p1 or p2 will be null
		mode =0;  // kernel mode 
	else if(p1==NULL && p2!=NULL) // src is an OSFile but dest is not one OSFile
		mode =1;  //  user mode
	else if(p1!=NULL && p2 == NULL) // if src is an user file and dest is an OSFile
		mode = 2;  
	if(mode == 0){
		//Both pointers = NULL, Thereby both OS files
		printf(ANSI_COLOR_RED"Permission Denied : Both are OS Files"ANSI_COLOR_RESET"\n");
		exit(1);}
	if(mode == 2){
		printf(ANSI_COLOR_RED"Permission Denied : Cannot Write into an OS File"ANSI_COLOR_RESET"\n");}
	if(mode == 1){
		int size; 
		int fds;
		fds=open(src,O_RDWR); // open file with name src in read/write mode
		size=lseek(fds,0,SEEK_END);	//Size of source OS file
		char *reads=(char *)malloc(size*sizeof(char));
		lseek(fds,0,SEEK_SET);
		read(fds,(void *)reads,size);//Reads from source OS file
		pathw=file_Pathname(dest); //get the destination file path_name
		write_file(pathw,reads); // write into the file	
	}
}





// SYSTEM call FOR MOVING FILE FROM ONE LOCATION TO ANOTHER
void mov(char *src,char *dest){
	// src file to be moved over to the dest file
	char *p1,*p2;  // will be used for searching for character occurence in a string
	int mode;  // kernel / user mode
	char **pathr; // path of the file to be read 
	char **pathw; // path of the file to be written into
	
	//strchr return char pointer to first occurence of a character in a string, returns NULL if not found
	p1=strchr(src,':');   
	p2=strchr(dest,':');
	/*Searches both file for presence of ':'. 
	OS FIle will not have ':' since its a simple osfile
	*/
	if(p1==NULL && p2==NULL)  //  incase of os file hence p1 or p2 will be null
		mode =0;  // kernel mode 
	else if(p1==NULL && p2!=NULL) // src is an OSFile but dest is not one OSFile
		mode =1;  //  user mode
	else if(p1!=NULL && p2 == NULL) // if src is an user file and dest is an OSFile
		mode = 2;  
	if(mode == 0){
		//Both pointers = NULL, Thereby both OS files
		printf(ANSI_COLOR_RED"Permission Denied : Both are OS Files"ANSI_COLOR_RESET"\n");
		exit(0);}
	if(mode == 2){
		printf(ANSI_COLOR_RED"Permission Denied : Cannot Write into an OS File"ANSI_COLOR_RESET"\n");}
	if(mode == 1){
		int size; 
		int fds;
		fds=open(src,O_RDWR); // open file with name src in read/write mode
		size=lseek(fds,0,SEEK_END);	//Size of source OS file
		char *reads=(char *)malloc(size*sizeof(char));
		lseek(fds,0,SEEK_SET);
		read(fds,(void *)reads,size);//Reads from source OS file
		pathw=file_Pathname(dest); //get the destination file path_name
		write_file(pathw,reads); // write into the file	
		//sup_block_init(file_Size,size); // delete the first file after moving(re-initialize)	
	}
}


char **file_Pathname(char *cm){

	char **command;//stores the list of words of the path
	int len;//Stores length of file path
	int ind;
	int i,j;
	command=(char **)malloc(FILE_PATH_DEPTH_MAX*sizeof(char *));// Allocating memory for entire word array
	ind=0;
	j=0;
	len=strlen(cm);
	for(i=0;i<FILE_PATH_DEPTH_MAX;i++)	{
		command[i]=(char *)malloc(MAX_NAME_WORDSIZE*sizeof(char));		//Allocating memory for each word of the word array
	}
	for(i=0;i<len;i++){			// Storing the words character wise into the array of words
		if(cm[i]!='\\'){
			command[ind][j++]=cm[i];											
		}
		else{						// If we encounter a '\', we end the present word of the
			command[ind][j]='\0';	// array by a NULL and go to next entry in array to start
			j=0;																													
			ind++;
		}
	}
	command[ind][j]='\0';

	return command;
}


int file_system_location(char *c){  // Finds the filesystem in the FS array with name = "C:"
	int fs_pos;
	for(fs_pos=1;fs_pos<=FScount;fs_pos++){  // search through all the filesystems
		if(strcmp(FS_group[fs_pos].FS_name,c)==0)
			return fs_pos+1;  
	}
	return -1;  // if the corresponding file_system is not found
}



void write_file(char **path,char *str){
	/*This function will write contents as given in str into the file mentioned by filepath=path*/
	int Fsuper_blocks;
	struct FS f;
	struct inode rooti;
	Fsuper_blocks=file_system_location(path[0]);// Finds index of mounted FS with given drive name as in pathw in FS_group
	f=FS_group[Fsuper_blocks-1];// Gets corresuper_blockonding FileSystem structure at that index
	rooti=*(readInode(Fsuper_blocks,1));//Reads the 1st Inode(Root Directory Inode)

}


// from the FS arrays it will read the corresponding inode
struct inode *readInode(int Fsuper_blocks,int id_inode){
	//This function will read Inode(id_inode) from FS present in Fsuper_blocks position in FS array
	struct inode *i=(struct inode *)malloc(sizeof(struct inode));			
	long int offset=FS_group[Fsuper_blocks-1].sb.inode_start+(id_inode-1)*FS_group[Fsuper_blocks-1].sb.blocksize;	//Calculating offset
	//Setting offset for writing root folder
	lseek(FS_group[Fsuper_blocks-1].fd,offset,SEEK_SET);
	read(FS_group[Fsuper_blocks-1].fd,(void *)i,sizeof(*i));
	return i;
}


int main(){

	char *c=(char *)malloc(30*sizeof(char)); // allocate space for the command string
	system("clear");
	//screen_design();
	while(1){
		printf(ANSI_COLOR_BLUE"MY_FS $ "ANSI_COLOR_RESET);
		gets(c);   // gets the command as input from the user and will be stored in `c`- 
		char **cm=Command_getter(c);
		for(int i=0;i<permissible_command_size;i++){
			if(strcmp(cm[i],"")!=0) // if the command is not empty
				printf("%s\n",cm[i]);  // prints the corresponding command out into the terminal
			else
				break;
		}
			
		exec_command(cm);  // execute the particular command
	}
}
