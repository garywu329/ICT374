/* Name: netproto.c
* Author: Gary Wu
* Date: 12-10-2019
* Revised: 21-10-2019
* Removed opcode, fnlength, filename from the original SFTP. Changed everything to work off of a single file pointer.
*/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

/* Protocol Return States
* -1 = Error making the socket
* 0 = Everythings fine
* 1 = File pointer is null
*
*/
// TCP/IP domain in call to socket() is AF_INET. PF_INET can be used in socket() but only socket()
int protocol(FILE *fp, char* domain, int portnumb, struct in_addr netadd)
{
	int sock;
	struct sockaddr_in servaddr
	if((sock = socket(domain, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Creation Error\n");
		return(-1);
	}
	
	int acknumb; //number determines file state.
	acknumb = fileTransfer(fp);

} 


int ftpTransfer(FILE* fp)
{
	int i, length;
	
	if(fp == NULL)
	{
		return(1);
	}
	
	fp = fopen
	
	
}

