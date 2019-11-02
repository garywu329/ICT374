/**
 * ICT374 Major assignment Two
 * Author: Michael Bell & Gary Wu
 * Simple FTP Program
 * Client
 * 
 **/
#include  <stdlib.h>
#include  <stdio.h>
#include  <sys/types.h>        
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netdb.h>
#include  <string.h>
#include  <sys/types.h> 
#include  <unistd.h>
#include  <dirent.h>
#include  <fcntl.h>
     

#define   SERV_TCP_PORT  40175 /* port number */
#define   BUFSIZE        (1024*256)

void ldir(char* path) {
	DIR *dp;
	struct dirent *file;
	char fileName[BUFSIZE];

	/* Open the directory */
	if((dp = opendir(path)) == NULL) {
		printf("Error in opening directory %s\n", path);
		exit(1);
	}

	if((file = readdir(dp)) < 0) {
		perror("lstat error");
		exit(1);
	}

	/* Read every file in the directory */
	while((file =readdir(dp)) != NULL) {
		strncpy(fileName, file->d_name, BUFSIZE);
		fileName[BUFSIZE] = '\0';
		printf("%s ", fileName);
	}

	printf("\n");
	closedir(dp);
}

int main(int argc, char *argv[]) {

	int sd, nr, nw, i = 0, length;
	char buf1[BUFSIZE], buf2[BUFSIZE], host[60];
	char *str;
	struct sockaddr_in ser_addr; 
	struct hostent *hp;

	/** Get the host name of the server 
	 * 	If the argument counter is 1 then use localhost
	 * 	Else use supplied host
	*/
	if (argc==1)
		gethostname(host, sizeof(host));
	else if (argc == 2)
		strcpy(host, argv[1]);
	else {
		printf("Usage: %s [<server_host_name>]\n", argv[0]);
		exit(1); 
	}

	/* get host address, & build a server socket address */
	bzero((char *) &ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(SERV_TCP_PORT);
	if ((hp = gethostbyname(host)) == NULL) {
		printf("host %s not found\n", host);
		exit(1);   
	}
	ser_addr.sin_addr.s_addr = * (u_long *) hp->h_addr;

	/* create TCP socket & connect socket to server address */
	sd = socket(PF_INET, SOCK_STREAM, 0);
	if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0) { 
		perror("client connect");
		exit(1);
	}

	while (++i) {
		int index = 0;
		char tempbuf[BUFSIZE] = "\0";
		bzero(buf1, BUFSIZE);
		bzero(buf2, BUFSIZE);

		printf("client[%d]: > ", i);
		fgets(buf1, BUFSIZE, stdin);
		nr = strlen(buf1);
		
		if(buf1[nr - 1] == '\n')
		{
			buf1[nr-1] == '\0';
			--nr;
		}

		for(int i = 0; i < strlen(buf1); ++i)
		{
			if(buf1[i] != ' ')
			{
				tempbuf[index] = buf1[i];
				index++;
			}	
		}
		
		index = strlen(tempbuf);
		tempbuf[index] = '\0';
		
		if(tempbuf[0] != '\n')
		{
			strcpy(buf1, tempbuf);
		}
		
		index = 0;
		str = strchr(buf1,' ');
		printf("Buf1 in client %s, nw value %d\n.", buf1, nw);
		if (strcmp(buf1, "quit") == 0) {
			printf("Bye from client\n");
			exit(0);
		} else if (strcmp(buf1, "lpwd") == 0) {
			if(getcwd(buf2, sizeof(buf2)) != NULL) {
				printf("Current working directory : %s\n",buf2);
			}
			continue;
		} else if (strcmp(buf1, "ldir") == 0) {
			if(getcwd(buf2, sizeof(buf2)) != NULL) {
				ldir(buf2);
			}
			continue;
		} else if(str!=NULL) {
			char * function, *file, *ptr;
			char parentFile[BUFSIZE];
			
			function = strtok(buf1," ");
			file = strtok(NULL," ");
			if(strcmp(function,"lcd")==0) {
				if (strcmp(file,".")==0) {
					if(getcwd(buf2, sizeof(buf2)) != NULL){
						printf("Current directory : %s\n",buf2);
					}
					continue;
				} else if (strcmp(file,"..") == 0) {
					if(getcwd(buf2, sizeof(buf2)) != NULL) {
						ptr = strrchr(buf2,'/');
						strncpy(parentFile, buf2, ptr-buf2);
						parentFile[ptr-buf2]='\0';
						if(chdir(parentFile) == 0) {
							printf("Current directory : %s\n",parentFile);
						} else {
							printf("Error in change directory\n");
						}
					}
					continue;	
				} else {
					if((chdir(file)) == 0) {
						printf("Current directory : %s\n",file);
					}
					else {
						printf("Error in change directory\n");
					}
					continue;
				}
			} else if(strcmp(function,"cd") == 0) {
				strcat(function, file);
				strcpy(buf1,function);
			} else if(strcmp(function,"get") == 0) {
				int f2;
				f2 = open(file, O_WRONLY|O_CREAT, 0666);
				
				if(f2 < 0) { 
					
					perror("Open Error!\n"); 
					continue; 
				}

				strcat(function, file);
				strcpy(buf1,function);
				nw = send(sd, buf1, sizeof(buf1), 0);
				printf("Buf1 in client %s, nw value %d\n.", buf1, nw);
				if (nw <= 0) {
					exit(1);
				}
				
				printf("client[%d]: send %s\n", i, buf1);

				bzero(buf1, BUFSIZE);
				while((nr = recv(sd, buf1, sizeof(buf1), 0)) > 0) {
					buf1[nr] = '\0';
					fprintf(stdout,"%s\n",buf1);
					length = strlen(buf1);

					if(nw = write(f2, buf1, length)<0) {
						exit(1);
					}
					
					printf("client[%d]: receive %s\n", i,buf1);
					break;
				}
				
				close(f2);
				
				printf("client[%d]: file closed\n", i);
				continue;
			} else if(strcmp(function,"put") == 0) {
				int f1;
				char line[BUFSIZE];

				strcat(function, file);
				strcpy(buf1,function);
				nw = send(sd, buf1, sizeof(buf1), 0);

				bzero(buf1, BUFSIZE);
				nr = read(sd, buf1, sizeof(buf1));
				
				file = buf1;
				
				printf("file %s\n", file);
				f1 = open(file,O_RDONLY);
				if((f1 < 0)) { 
					perror("Open Error!\n"); 
					continue; 
				}

				bzero(buf1, BUFSIZE);
				while((nr = read(f1, line, sizeof(line))) > 0) {
					printf("nr %d\n", nr);
					line[nr] = '\0';
					fprintf(stdout,"%s\n",line);
					length = strlen(line);
					
					strncpy(buf1, line, length);
					nw = send(sd, buf1, length, 0);
					if (nw <= 0) {
						exit(1);
					}
					printf("client[%d]: send %s\n", i,buf1);
				}
				close(f1);
				printf("file closed\n");

				continue;
			}
		} else if (strcmp(buf1, "lcd") == 0) {
			char *env;
			env = getenv("HOME");
			chdir(env);
			
			if(getcwd(buf2, sizeof(buf2)) != NULL) {
				printf("%s\n",buf2);
			}

			continue;
		}
			
		nw = sendto(sd, buf1, sizeof(buf1), 0, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
		if (nw <= 0) {
			exit(1);
		}
		printf("client[%d]: send %s\n", i, buf1);

		bzero(buf1, BUFSIZE);
		int server_address_length = sizeof(ser_addr);
		nr = recvfrom(sd, buf1, BUFSIZE, 0, (struct sockaddr *) &ser_addr, &server_address_length);
		if (buf1[nr-1] == '\n') {
			buf1[nr-1] = '\0';
			--nr;
		}

		if (nr <= 0) {
			exit(1);
		}
		printf("client[%d]: receive %s\n", i, buf1);
	}
}
