/* Name: netproto.c
* Author: Gary Wu
* Date: 12-10-2019
* Revised: N/A
*
*/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int protocol(char opcode, int fnlength, char *filename)
{
	//Processing through the initial opcode, file length and filename
	if((opcode != 'a') && (opcode != 'A'))
	{
		printf("Request type error. Exiting...\n");
		exit(1);
	}
	else 
	{
		char file[fnlength + 1];
		strcpy(file, filename);
		file[strlen(file) + 1] = '\0'; //Adding null to make sure string terminates
		printf("A "); //Responding with one byte of opcode 'A'
		processfile(file); //Processing the rest of the file here
	}	
} 

void processfile(char *filename)
{
	int ackcode; //Acknowledgement byte to print out based on file.
	ackcode = fileexists(filename); //Checking if file already exists

	if(ackcode == 1)
	{
		printf("%d\n", ackcode); //File exists, return acknowledgement 1
		exit(1);
	}
}

int fileexist(char *filename)
{
	//Checking if file exists
	struct stat s = {0}

	if(!(stat(filename, &s)
	{
		if(ENOENT == errno)
		{
			return(0);
		}
	}
	else
	{
		return(1);
	}
}
