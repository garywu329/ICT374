/**
 * ICT374 Major assignment Two
 * Author: Michael Bell & Gary Wu
 * Simple FTP Program
 * Server
 * 
 **/

#include  <signal.h>
#include  <string.h>
#include  <errno.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include  <sys/stat.h>
#include  <sys/socket.h>
#include  <netinet/in.h> 
 #include  <stdlib.h>
#include  <stdio.h>
#include  <fcntl.h>
#include  <dirent.h>
#include  <unistd.h>

#define SERV_TCP_PORT   40175
#define BUFSIZE         (1024*256)

void serve_a_client(int, struct sockaddr_in, int);
void claim_children();
void daemon_init(void);
void input_cd();
void input_pwd();
char* input_dir(char *);
char* input_cdpath(char *);
char* input_cddotdot(char* path);

int main() {
    int sd, nsd, cli_addr_len;  pid_t pid;
    struct sockaddr_in ser_addr, cli_addr;
	int opt;
    char buf[BUFSIZE];

    // Initialize daemon
    daemon_init();
        
    /* set up listening socket sd */
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server:socket");
        exit(1);
    } 
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
		printf("Error setting socket");
		exit(1);
	}
    /* build server Internet socket address */
    bzero((char *)&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(SERV_TCP_PORT);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    /* note: accept client request sent to any one of the
            network interface(s) on this host. 
    */

    /* bind server address to socket sd */
    if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0) {
        perror("server bind");
        exit(1);
    }

    /* become a listening socket */
    listen(sd, 5);

    while (1) {

        /* wait to accept a client request for connection */
        cli_addr_len = sizeof(cli_addr);
        nsd = accept(sd, (struct sockaddr *) &cli_addr, &cli_addr_len);

        // Handle interrupts
        if (nsd < 0){
            if (errno == EINTR)
                continue;
            perror("server:accept");
            exit(1);
        }
                
        /* create a child process to handle this client */
        if ((pid=fork()) <0) {
            perror("fork");
            exit(1);
        } else if (pid > 0) { 
            close(nsd);
            continue; /* parent to wait for next client */
        }

        /* now in child, serve the current client */
        close(sd); 
        serve_a_client(nsd, cli_addr, cli_addr_len);
    }
    exit(0);
}

void serve_a_client(int sd, struct sockaddr_in cli_addr, int cli_addr_len) {
    int nr, nw, length, i = 0;
    char buf1[BUFSIZE], buf2[BUFSIZE]; 
    char *temp;
    char tempcatcher[BUFSIZE];
	FILE *l;
	//l = fopen("log.txt", "w+");
	//if(l == NULL)
	//{
	//	perror("Unable to create log file.\n");
	//}

    while (++i) {
    /* read data from client */
        bzero(buf1, BUFSIZE);
        bzero(buf2, BUFSIZE);
        nr = recvfrom(sd, buf1, BUFSIZE, 0, (struct sockaddr *) &cli_addr, &cli_addr_len); 

	
        if (buf1[nr-1] == '\n') {
            buf1[nr-1] = '\0';
            --nr;
        }

        if (nr <= 0)
            exit(1);   /* receive error */

        printf("server[%d] start: receive %s\n", i, buf1);
	//fprintf(l, "server[%d] start: receive %s\n", i, buf1);
        if (strcmp(buf1, "cd")==0){
            input_cd();
            
            if(getcwd(buf2, sizeof(buf2)) != NULL) {
                strcpy(buf1,buf2);
            }
        } else if (strcmp(buf1, "pwd")==0) { 
            input_pwd(buf1);

            if(getcwd(buf2, sizeof(buf2)) != NULL) {
                strcpy(buf1,buf2);
            }
        } else if (strcmp(buf1, "dir")==0) {
            if(getcwd(buf2, sizeof(buf2)) != NULL) 		
            {
                strcpy(tempcatcher, input_dir(buf2));
                strcpy(temp, tempcatcher);
                strcpy(buf1, temp);
            }
            printf("%s\n", buf1);
		//fprintf(l, "%s\n", buf1);	
        } else if((nr > 2) && ((strncmp(buf1,"put", 3)==0)||(strncmp(buf1,"get", 3)==0)||(strncmp(buf1,"cd/", 3)==0)||(strncmp(buf1,"cd.", 3)==0))) {
            char *file, *ptr;
            char parentFile[BUFSIZE];

            /* function cd path */
            if(strncmp(buf1,"cd", 2)==0) {
                strcpy(file, input_cdpath(buf1)); //might have seg faults here

                /* path . */
                if (strcmp(file,".")==0) {
                    if(getcwd(buf2, sizeof(buf2)) != NULL) {
                        strcpy(buf1,buf2);
                    }
                }
                /* path .. */
                else if (strcmp(file,"..")== 0) {
                    strcpy(tempcatcher, input_cddotdot(buf2));
                    if(tempcatcher[0] != '\0')
                    {
                        strcpy(buf1, tempcatcher);
                    }
                }
                else {
                    /* path */
                    if((chdir(file)) == 0) {
                        strcpy(buf1,file);
                    }
                    else {
                        perror("Error in change directory\n");
			//fprintf(l, "Error in change of directory\n");
                    }
                }
            }
            /* function get */
            else if(strncmp(buf1,"get", 3)==0) {
                file = strtok(buf1,"get");
                printf("file %s\n", file);
		//fprintf(l, "file %s\n", file);
                int f1;
                char line[BUFSIZE];
		printf("Contents of file %s\n", file);
                f1 = open(file,O_RDONLY);
                if((f1 < 0)) { 
                    perror("Gets Error!\n"); 
                        continue; 
                }
                
                bzero(buf1, BUFSIZE);
                /* read from file */
		printf("Buf1 in server %s, nw value %d\n.", buf1, nw);
                while((nr = read(f1, line, sizeof(line))) > 0) { 
                    line[nr] = '\0';
                    fprintf(stdout,"%s\n",line);
			//fprintf(l, "%s\n", line);
                    length=strlen(line);
                    strncpy(buf1, line, length);
                    nw = send(sd, buf1, length, 0);	/* send to client */
		    printf("Buf1 in server %s, nw value %d\n.", buf1, nw);
                    if (nw <= 0)
			perror("send error");
			//fprintf(l, "send error");
                        exit(1);    /* send error */
                    printf("server[%d]: send %s\n", i,buf1);
			//fprintf(l, "server[%d]: send %s\n", i, buf1);	
                }
                close(f1);
		
                printf("server[%d]: file closed\n", i);
		//fprintf(l, "server[%d]: file closed\n", i);

                continue;
            }
            /* function put */
            else if(strncmp(buf1,"put", 3)==0) {
                int f2;
                file = strtok(buf1,"put");
                printf("file %s\n", file);
		//fprintf(l, "file %s\n", file);

                nw = send(sd, file, sizeof(file), 0);	/* send responese to client */
                if (nw <= 0)
                    exit(1);    /* send error */
                printf("server[%d]: send %s\n", i,file);
		//fprintf(l, "server[%d]: send %s\n", i,file);	
                f2 = open(file,O_WRONLY|O_CREAT,0666);
                if((f2 == -1)) { 
                    perror("Open Error!\n"); 
			//fprintf(l, "Open Error!\n");
                        continue; 
                }

                bzero(buf1, BUFSIZE);
                /* read from client */
                while((nr = recv(sd, buf1, sizeof(buf1), 0)) > 0) {
                    buf1[nr] = '\0';
                    fprintf(stdout,"%s\n",buf1);
			//fprintf(l, "%s\n", buf1);
                    length=strlen(buf1);

                    /* write into file */
                    if(nw = write(f2, buf1, length)<=0){
                        exit(1);
                    }
                    printf("server[%d] end: receive %s\n", i,buf1);
			//fprintf(l, "server[%d] end: receive %s\n", i,buf1);
                    break;
                }
                close(f2);
                continue;
            }
		fclose(l);
        }
        
        
        /* send results to client */
        nw = sendto(sd, buf1, sizeof(buf1), 0, (struct sockaddr *) &cli_addr, cli_addr_len);
        if (nw <= 0)
            exit(1);    /* send error */
        printf("server[%d]: send %s\n", i,buf1);
	//fprintf(l, "server[%d]: send %s\n", i,buf1);
    }
}

void claim_children() {
    pid_t pid = 1;
    while (pid>0) { 
        pid = waitpid(0, (int *)0, WNOHANG); 
    } 
}

/**
 *  daemon_init(void)
 * 
 *  Turn the program into a daemon process then kill
 *  the parent.
 * 
 *  The child is then set to the session leader, changes
 *  the current working directory and then clears the file
 *  mode creation mask.
 **/
void daemon_init(void) {       
    pid_t   pid;
    struct sigaction act;
    if ( (pid = fork()) < 0) {
        perror("fork");
        exit(1); 
    } else if (pid > 0) {
        printf("Server PID: %d\n", pid);
        exit(0);
    }

    setsid();
    chdir("/");
    umask(0);

    /* catch SIGCHLD to remove zombies from system */
    act.sa_handler = claim_children;	/* use reliable signal */
    sigemptyset(&act.sa_mask);	/* not to block other signals */
    act.sa_flags   = SA_NOCLDSTOP;	/* not catch stopped children */
    sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);
}

void input_cd()
{
    char *env;
    env = getenv("HOME");
    chdir(env);
}

void input_pwd(char *path)
{
    getcwd(path, sizeof(path));
    printf("Present working directory: %s\n", path);
}

char* input_dir(char *path)
{
    DIR *dp;
    struct dirent *file;
    char fileName[BUFSIZE], temp[BUFSIZE];
    int length, n = 0;

    /* open directory */
    if((dp = opendir(path)) == NULL) {
        printf("Error in opening directory %s\n", path);
        exit(1);
    }
    if((file = readdir(dp)) < 0) {
        perror("lstat error");
        exit(1);
    }
    //read all files inside the directory
    while((file =readdir(dp)) != NULL) {
        bzero(fileName, BUFSIZE);
        strcpy(fileName, file->d_name);//get file name
        length = strlen(fileName);
        if( n < 1) {    
            bzero(temp, BUFSIZE);
            strcpy(temp,"");
            strncpy(temp,fileName, length);
            n++;
        }
        else {
            strcat(temp," ");
            strncat(temp,fileName, length);
        } 
    }
    closedir(dp);

    int stringlength;
    stringlength = strlen(temp);
    char bufreturn[stringlength];
    strcpy(bufreturn, temp);

    return(bufreturn);
}

char* input_cdpath(char* path)
{
    char *file;

    strtok(path,"d");
    file = strtok(NULL,"d");
    printf("file %s\n", file);

    int stringlength;
    stringlength = strlen(file);
    char filereturn[stringlength];
    strcpy(filereturn, file);

    return(filereturn);
}

char* input_cddotdot(char* path)
{
	char *ptr;
    char parentFile[BUFSIZE];
    char bufreturn[BUFSIZE] = "\0";

    if(getcwd(path, sizeof(path)) != NULL) {
        ptr = strrchr(path,'/');
        
        strncpy(parentFile, path, ptr - path);
        parentFile[ptr - path]='\0';
        if((chdir(parentFile)) == 0) {
            strcpy(bufreturn,parentFile);
            return(bufreturn);
        }   
        else {
            printf("Error in change directory\n");
            return(bufreturn);
        }
    } 
}
