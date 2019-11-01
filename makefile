# makefile for myftpd
# the filename must be either Makefile or makefile
myftpd: myftpd.c
	gcc	myftpd.c	-o	myftpd
myftp: myftp.c
	gcc	myftp.c	-o	myftp
clean:
	rm *.o
