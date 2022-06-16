#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#define PORT 9001

int main()		
{		
	int clntSd;	
	struct sockaddr_in clntAddr;	
	int clntAddrLen, readLen;	
	char wBuff[] = "How old are you?";	
	char rBuff[BUFSIZ];	
		
		
	/*1) socket()*/
	clntSd = socket(PF_INET, SOCK_STREAM, 0);	
	if(clntSd == -1)	
	{	
		printf("Socket Creation Error");
		return -1;
	}	
	clntAddr.sin_family = AF_INET;	
	clntAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	
	clntAddr.sin_port = htons(PORT);	
	/*2) connect()*/
	if(connect(clntSd, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) == -1)	
	{	
		printf("Connect Error");
		close(clntSd);
		return -1;
	}	
		
	/*3) send() : flags가 0이면 일반 데이터 송신*/	
	//write(clntSd, wBuff, sizeof(wBuff));
	send(clntSd,wBuff,sizeof(wBuff),0);
	printf("Client: %s\n",wBuff);	
	/*4) recv() : flags가 0이면 일반 데이터 수신*/
	//readLen = read(clntSd, rBuff,sizeof(rBuff)-1);	
	readLen=recv(clntSd,rBuff,sizeof(rBuff)-1,0);
	if(readLen == -1)	
	{	
		printf("Read Error");
		return -1;
	}	
	rBuff[readLen] = '\0';
	printf("Server: %s\n", rBuff);
	close(clntSd);
	return 0;
}	
