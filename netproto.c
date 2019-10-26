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
#include <netinet/in.h>
#include <netinet/ip.h>
#define PORT 8080
/* Protocol Return States
* -1 = Error making the socket
* 0 = Everythings fine
* 1 = File pointer is null
*
*/
// TCP/IP domain in call to socket() is AF_INET. PF_INET can be used in socket() but only socket()
int protocol(FILE *fp, int portnumb)
{
	int sock, reader, new_sock, opt;
	char buffer[1024] = {0};
	struct sockaddr_in servaddr;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Creation Error\n");
		exit(-1);
	}
	//Helps reuse address
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		printf("Error setting socket");
		exit(-2);
	}
	//Setting initial socket based on port number
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(portnumb);
	//Binding the socket
	if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("Bind failure");
		exit(-3);
	}
	//Sets socket to passive, waits for connection
	if(listen(socket, 3) < 0)
	{
		printf("Listen failed");
		exit(-4);
	}
	//Extracts connection request
	if((new_sock = accept(socket, (struct sockaddr *)&servaddr, (socklen_t*)&servaddr)) < 0)
	{
		printf("New socket creation failed");
		exit(-5);
	}
	
	while(!feof(fp))
	{
		reader = fread(buffer, 1, sizeof(buffer), fp);
		send(new_sock, buffer, reader, 0);
	}
	fclose(fp);
	return(0);
} 


