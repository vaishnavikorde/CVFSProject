#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include<io.h>

#define MAXINODE 5   // No of inodes in file system

#define READ 1
#define WRITE 2

#define MAXFILESIZE 2048   // maximum size of file

#define REGULAR 1
#define SPECIAL 2

#define START 0 
#define CURRENT 1
#define END 2

// structure of superblock
typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;

// structure of inode
typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferanceCount;
    int Permission; // 1  23
    struct inode *next;
}INODE, *PINODE, **PPINODE;

// structure of filetable
typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode; // 1  2  3
    PINODE ptrinode;
}FILETABLE, *PFILETABLE;

// structure of user file description table
typedef struct ufdt
{
    PFILETABLE ptrfiletable;
}UFDT;

// Global variables
UFDT UFDTArr[MAXINODE];    // array of UFDT
SUPERBLOCK SUPERBLOCKobj;  // object of superblock
PINODE head= NULL;        // head pointer of type PINODE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  man function
// Description    :  This function is used as mannual to display the use of each function and parameters to be passed to the function
// Parameters     :  commands
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void man(char *name)
{
    if(name == NULL)
    {
        return;
    }

    if(strcmp(name,"create") == 0)
    {
        printf("Description: Used to create new regular file\n");
        printf("Usage: Create File_Name permission\n");
    }
    else if(strcmp(name,"read") == 0)
    {
        printf("Description: Used to read data from regular file\n");
        printf("Usage: Read File_Nmae, No_Of_Bytes_to_Read\n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description: Used to write into regular file\n");
        printf("Usage: Write File_Name\n After this enetr the data that we want to write\n");
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description: Used to list all inforamtion of files\n");
        printf("Usage: ls\n");
    }
    else if(strcmp(name,"stat") == 0)
    {
        printf("Description: Used to display information of file\n");
        printf("Usage: stat File_Name\n");
    }
    else if(strcmp(name,"fstat") == 0)
    {
        printf("Description: Used to display information of file\n");
        printf("Usage: stat file_desriptor\n");
    }
    else if(strcmp(name,"truncate") == 0)
    {
        printf("Description: Used to remove the data fromfile\n");
        printf("Usage: Truncate File_Name\n");
    }
    else if(strcmp(name,"open") == 0)
    {
        printf("Description: Used to open existing file\n");
        printf("Usage: Open File_Name\n");
    }
    else if(strcmp(name,"close") == 0)
    {
        printf("Description: Used to close opened file\n");
        printf("Usage: Close File_Name\n");
    }
    else if(strcmp(name,"closeall") == 0)
    {
        printf("Description: Used to close all opened files\n");
        printf("Usage: CloseAll\n");
    }
    else if(strcmp(name,"lseek") == 0)
    {
        printf("Description: Used to change file offset\n");
        printf("Usage: Lseek File_Name ChangeInOffset StartPoint\n");
    }
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description: Used to delete the file\n");
        printf("Usage: rm File_Name\n");
    }
    else
    {
        printf("Error: No mannual entry availabe\n");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  DisplayHelp
// Description    :  Display the use of commands
// Parameters     :  --
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayHelp()
{
    printf("ls: To list out all files\n");
    printf("clear: To clear console\n");
    printf("open: To open the file\n");
    printf("close: To close the file\n");
    printf("closeall: To close all files\n");
    printf("read: To read the contents from the file\n");
    printf("write: To write the contents in the file\n");
    printf("exit: to teminate file system\n");
    printf("stat: To display inforamtion of file using name\n");
    printf("fstat: To display the information of file using file descriptor\\n");
    printf("truncate: To remove all the data from the file\n");
    printf("rm: To delete the file\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  InitializeSuperBlock
// Description    :  Initializes the superblock which contains Total Nodes and Free Nodes
// Parameters     :  --
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InitializeSuperBlock()
{
    int i= 0;
    while(i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable= NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes= MAXINODE;
    SUPERBLOCKobj.FreeInode= MAXINODE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  CreateDIBL
// Description    :  Create the initialization of Disk Inode List Block(DILB)
// Parameters     :  --
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateDILB()
{
    int i= 1;   // inode number
    PINODE newn= NULL;
    PINODE temp= head;

    while(i <= MAXINODE)
    {
        newn= (PINODE)malloc(sizeof(INODE));

        newn->LinkCount= 0;
        newn->ReferanceCount= 0;
        newn->FileType= 0;
        newn->FileSize= 0;

        newn->Buffer= NULL;
        newn->next= NULL;

        newn->InodeNumber= i;

        if(temp == NULL)
        {
            head= newn;
            temp= head;
        }
        else
        {
            temp->next= newn;
            temp= temp->next;
        }
        i++;
    }
    printf("DILB created Successfully\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  Get_Inode
// Description    :  fetch the inode
// Parameters     :  name of file
// Return value   :  PINODE(pointer of Inode)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PINODE Get_Inode(char *name)  // file_name
{
    PINODE temp = head;

    int i= 0;  // Inode_Number  

    if(name == NULL)
    {
        return NULL;
    }
    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
        {
            break;
        }

        temp= temp->next;
    }

    return temp;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  GetFdFromName
// Description    :  Used to fetch the file descriptor with the help of file name
// Parameters     :  File name
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetFDFromName(char *name)
{
    int i= 0;  // fd of file

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0) 
            {
                break;
            }
        }
        i++;
    }

    if(i == MAXINODE)
    {
        return -1;
    }
    else 
    {
        return i;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  CreateFile
// Description    :  create the new file
// Parameters     :  name of file and the Permission ie the purpose of file creation(Read or write)
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CreateFile(char *name, int Permission)
{
    int i = 0;

    PINODE temp= head;

    if((name == NULL) || (Permission == 0) || (Permission > 3))
    {
        return -1;
    }

    if(SUPERBLOCKobj.FreeInode == 0)
    {
        return -2;
    }
    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name) != NULL)
    {
        return -3;
    }

    while(temp != NULL)
    {
        if(temp->FileType == 0)
        {
            break;
        }
        temp= temp->next;
    }

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE)); 
    UFDTArr[i].ptrfiletable->count = 1; 
    UFDTArr[i].ptrfiletable->mode = Permission; 
    UFDTArr[i].ptrfiletable->readoffset = 0; 
    UFDTArr[i].ptrfiletable->writeoffset = 0; 
    UFDTArr[i].ptrfiletable->ptrinode = temp; 

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name); 

    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR; 
    UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount = 1; 
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1; 
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0; 
    UFDTArr[i].ptrfiletable->ptrinode->Permission = Permission; 
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char*)malloc(MAXFILESIZE); 

    return i;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  rm_file
// Description    :  remove the file
// Parameters     :  name of file
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int rm_file(char *name)
{
    int fd= 0;

    fd= GetFDFromName(name);
    if(fd == -1)
    {
        return -1;
    }

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        //Free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable= NULL;
    (SUPERBLOCKobj.FreeInode)++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  ReadFile
// Description    :  Read the acual data of file
// Parameters     :  file descriptor,  string data, length of data
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ReadFile(int fd, char *arr, int iSize)
{
    int read_size= 0;

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ+WRITE)
    {
        return -2;
    }

    if(UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ &&  UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE)
    {
        return -2;
    }

    if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
    {
        return -3;
    }

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
    {
        return -4;
    }

    read_size= (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

    if(read_size < iSize)
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);

        UFDTArr[fd].ptrfiletable->readoffset= UFDTArr[fd].ptrfiletable->readoffset + read_size;
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),iSize);

        UFDTArr[fd].ptrfiletable->readoffset= UFDTArr[fd].ptrfiletable->readoffset + iSize;
    }

    return iSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  WriteFile
// Description    :  Write the data in the file
// Parameters     :  file descriptor, input data, length of data
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WriteFile(int fd, char *arr, int iSize)
{
    if(((UFDTArr[fd].ptrfiletable->mode) != WRITE) && (UFDTArr[fd].ptrfiletable->mode != READ + WRITE))
    {
        return -1;
    }

    if(((UFDTArr[fd].ptrfiletable->ptrinode->Permission) != WRITE) && (UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE))
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
    {
        return -2;
    }

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
    {
        return -3;
    }

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr,iSize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;

    return iSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  OpenFile
// Description    :  open the desired file
// Parameters     :  file name, mode in which you want to open the file ie read, write or both read and write 
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int OpenFile(char *name, int mode)
{
    int i= 0;
    PINODE temp= NULL;

    if((name == NULL) || (mode <= 0))
    {
        return -1;
    }

    temp= Get_Inode(name);
    if(temp == NULL)
    {
        return -2;
    }

    if(temp->Permission < mode)
    {
        return -3;
    }

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL)
    {
        return -1;
    }
    UFDTArr[i].ptrfiletable->count= 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset= 0;
        UFDTArr[i].ptrfiletable->writeoffset= 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset= 0;
    }
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }

    UFDTArr[i].ptrfiletable->ptrinode= temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount)++;

    return i;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  CloseFileByName
// Description    :  close the desired file
// Parameters     :  file descriptor
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset= 0;
    UFDTArr[fd].ptrfiletable->writeoffset= 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferanceCount)--;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  CloseFileByName
// Description    :  close the desired file by using its name
// Parameters     :  file name
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CloseFileByName(char *name)
{
    int i= 0;
    i= GetFDFromName(name);

    if(i == -1)
    {
        return -1;
    }

    UFDTArr[i].ptrfiletable->readoffset= 0;
    UFDTArr[i].ptrfiletable->writeoffset= 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount)--;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  CloseAllFile
// Description    :  close the all created files
// Parameters     :  --
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CloseAllFile()
{
    int i= 0;
    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset= 0;
            UFDTArr[i].ptrfiletable->writeoffset= 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount)--;
            break;
        }
        i++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  LseekFile
// Description    :  
// Parameters     :  file descriptor, length of file, position from which you want to seek the file
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LseekFile(int fd, int iSize, int from)
{
    if((fd < 0) || (from > 2))
    {
        return -1;
    }

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset) + iSize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }

            if(((UFDTArr[fd].ptrfiletable->readoffset) + iSize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset)+iSize;

        }
        else if(from == START)
        {
            if(iSize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }

            if(iSize < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset)= iSize;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize > MAXFILESIZE)
            {
                return -1;
            }

            if(((UFDTArr[fd].ptrfiletable->readoffset) + iSize) < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;
        }
    }
    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize) > MAXFILESIZE)
            {
                return -1;
            }

            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize) < 0)
            {
                return -1; 
            }

            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;
            }
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;
        }
        else if(from == START)
        {
            if(iSize > MAXFILESIZE)
            {
                return -1;
            }

            if(iSize < 0)
            {
                return -1;
            }

            if(iSize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)= iSize;
            }

            (UFDTArr[fd].ptrfiletable->writeoffset)= iSize;
        }
        else if(from == END)
        {
            if(iSize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize > MAXFILESIZE)
            {
                return -1;
            }

            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize) < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  ls_file
// Description    :  display the list of created files
// Parameters     :  --
// Return value   :  void
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ls_file()
{
    int i = 0;
    PINODE temp= head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error: There are no files\n");
        return;
    }
    
    printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
    printf("--------------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->InodeNumber,temp->FileActualSize ,temp->LinkCount);
        }
        temp= temp->next;
    }
    printf("--------------------------------------------------------\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  stat_file
// Description    :  display the stastical information of the file
// Parameters     :  file name
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int stat_file(char *name)
{
    PINODE temp= head;
    int i= 0;

    if(name == NULL) 
    {
        return -1;
    }

    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
        {
            break;
        }
        temp= temp->next;
    }

    if(temp == NULL)
    {
        return -2;
    }

    printf("\n-------------------Statistical information about file----------------------------\n");
    printf("File name: %s\n", temp->FileName);
    printf("Inode Number %d\n", temp->InodeNumber);
    printf("File Size: %d\n", temp->FileSize);
    printf("Actual File Size: %d\n", temp->FileActualSize);
    printf("Link count: %d\n", temp->LinkCount);
    printf("Referance count: %d\n", temp->ReferanceCount);

    if(temp->Permission == 1)
    {
        printf("File Permission: Read only\n");
    }
    else if(temp->Permission == 2)
    {
        printf("File Permission: Write\n");
    }
    else if(temp->Permission == 3)
    {
        printf("File Permission: Read and Write\n");
    }
    printf("------------------------------------------------------\n\n");

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  stat_file
// Description    :  display the stastical information of the file with the help of file descriptor
// Parameters     :  file descriptor
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fstat_file(int fd)
{
    PINODE temp= head;
    int i= 0;

    if(fd < 0)
    {
        return -1;
    }

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -2;
    }

    temp= UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n-------------------Statistical information about file----------------------------\n");
    printf("File name: %s\n", temp->FileName);
    printf("Inode Number %d\n", temp->InodeNumber);
    printf("File Size: %d\n", temp->FileSize);
    printf("Actual File Size: %d\n", temp->FileActualSize);
    printf("Link count: %d\n", temp->LinkCount);
    printf("Referance count: %d\n", temp->ReferanceCount);

    if(temp->Permission == 1)
    {
        printf("File Permission: Read only\n");
    }
    else if(temp->Permission == 2)
    {
        printf("File Permission: Write\n");
    }
    else if(temp->Permission == 3)
    {
        printf("File Permission: Read and Write\n");
    }
    printf("------------------------------------------------------\n\n");

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function name  :  Truncate_File
// Description    :  Delete the file
// Parameters     :  file name
// Return value   :  integer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Truncate_File(char *name)
{
    int fd= GetFDFromName(name);
    if(fd == -1)
    {
        return -1;
    }

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
    UFDTArr[fd].ptrfiletable->readoffset= 0;
    UFDTArr[fd].ptrfiletable->writeoffset= 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize= 0;
}

int main()
{
    char *ptr= NULL;
    int ret= 0, fd= 0, count= 0;
    char command[4][80], str[80], arr[1024];

    InitializeSuperBlock();
    CreateDILB();

    while(1)   
    {
        fflush(stdin);  
        strcpy(str, "");  

        printf("\nMarvellous VFS : >");   

        fgets(str, 80, stdin);  

        count= sscanf(str,"%s %s %s %s", command[0], command[1], command[2], command[3]);
        
        if(count == 1)
        {
            if(strcmp(command[0],"ls") == 0)
            {
                ls_file();
                continue;
            }
            else if(strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                printf("All files are closed successfully\n");
                continue;
            }
            else if(strcmp(command[0],"clear") == 0)
            {
                system("cls");
                continue;
            }
            else if(strcmp(command[0],"help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0], "exit") == 0)
            {
                printf("Terminatting the Marvellous Virtual File System\n");
                break;
            }
            else
            {
                printf("\nError: Command not found !!!\n");
                continue;
            }
        }
        else if(count == 2)
        {
            if(strcmp(command[0],"stat") == 0)
            {
                ret= stat_file(command[1]); // file_name
                if(ret == -1)
                {
                    printf("Error: Incorrect parameters\n");
                }
                if(ret == -2)
                {
                    printf("Error: There is no such file\n");
                }
                continue;
            }
            else if(strcmp(command[0],"stat") == 0)
            {
                ret = fstat_file(atoi(command[1]));
                if(ret == -1)
                {
                    printf("Error: Incorrect parameters\n");
                }
                if(ret == -2)
                {
                    printf("Error: There is no such file\n");
                }              
                continue;
            }
            else if(strcmp(command[0],"close") == 0)
            {
                ret= CloseFileByName(command[1]);
                if(ret == -1)
                {
                    printf("Error: There is no such file\n");
                }
                continue;
            }
            else if(strcmp(command[0],"rm") == 0)
            {
                ret= rm_file(command[1]);
                if(ret == -1)
                {
                    printf("Error: There is no such file\n");
                }
                continue;
            }
            else if(strcmp(command[0],"man") == 0)
            {
                man(command[1]);
            }
            else if(strcmp(command[0], "write") == 0)
            {
                fd= GetFDFromName(command[1]);
                if(fd == -1)
                {
                    printf("Error: Incorrect parameter\n");
                    continue;
                }
                printf("Enter the data: \n");
                scanf("%[^'\n']", arr);

                ret= strlen(arr);
                if(ret == 0)
                {
                    printf("Error: Incorrect parameters\n");
                    continue;
                }

                ret= WriteFile(fd, arr, ret);  // file_descriptor, data, length
                if(ret == -1)
                {
                    printf("Error: Permission denied\n");
                }
                if(ret == -2)
                {
                    printf("Error: There is no sufficient memory to write\n");
                }
                if(ret == -3)
                {
                    printf("Error: It is not regular file\n");
                } 
            }
            else if(strcmp(command[0],"truncate") == 0)
            {
                ret= Truncate_File(command[1]);
                if(ret == -1)
                {
                    printf("Error: Incorrect parameters\n");
                }
            }
            else
            {
                printf("\nError: Command not found !!!\n");
                continue;
            }
        }
        else if(count == 3)
        {
            if(strcmp(command[0],"create") == 0)
            {
                ret= CreateFile(command[1], atoi(command[2]));  // file_name, Permission
                if(ret >= 0)
                {
                    printf("File is successfully created with File Descriptor %d\n", ret);
                }
                if(ret == -1)
                {
                    printf("Error: Incorrect Parameters\n");
                }
                if(ret == -2)
                {
                    printf("Error: There are no nodes\n");
                }
                if(ret == -3)
                {
                    printf("Error: File alredy exist\n");
                }
                if(ret == -4)
                {
                    printf("Error: Memory alloaction failure\n");
                }
                continue;
            }
            else if(strcmp(command[0],"open") == 0)
            {
                ret= OpenFile(command[1], atoi(command[2]));
                if(ret >= 0)
                {
                    printf("File is Successfully opened with Fd %d\n");
                }
                if(ret == -1)
                {
                    printf("Error: Incorrect parameters\n");
                }
                if(ret == -2)
                {
                    printf("Error: File not present\n");
                }
                if(ret == -3)
                {
                    printf("Error: Permission denied\n");
                }
            }
            else if(strcmp(command[0],"read") == 0)
            {
                fd= GetFDFromName(command[1]);
                if(fd == -1)
                {
                    printf("Error: Incoret parameters\n");
                    continue;
                }

                ptr= (char *)malloc(sizeof(atoi(command[2]))+1);

                if(ptr == NULL)
                {
                    printf("Error: Memory allocation failure\n");
                    continue;
                }

                ret= ReadFile(fd, ptr, atoi(command[2]));
                if(ret == -1)
                {
                    printf("Error: File not exist\n");
                }
                if(ret == -2)
                {
                    printf("Error: Permission denied\n");
                }
                if(ret == -3)
                {
                    printf("Error: Reached at the end file\n");
                }
                if(ret == -4)
                {
                    printf("Error: It is not regukar file\n");
                }
                if(ret == 0)
                {
                    printf("Error: File empty\n");
                }

                if(ret > 0)
                {
                    write(2,ptr,ret);
                }
                continue;
            }
            else
            {
                printf("\nError: Commnad not found!!!!\n");
                continue;
            }   
        }
        else if(count == 4)
        {
            if(strcmp(command[0],"lseek") == 0)
            {
                if(fd == -1)
                {
                    printf("Error: Incorrect parameters\n");
                    continue;
                }

                ret= LseekFile(fd, atoi(command[2]), atoi(command[3]));
                if(ret == -1)
                {
                    printf("Error: Unable to perform lseek\n");
                }
            }
            else
            {
                printf("\nError: Command not found!!!\n");
                continue;
            }
        }
        else
        {
            printf("\nError: Command not found\n");
        }

    }    

    return 0;

}