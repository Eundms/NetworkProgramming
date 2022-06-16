#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>// 헤더추가
#define PORT 9001

int main()
{
	int srvSd, clntSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, readLen;
	char rBuff[BUFSIZ];
	char wBuff[] = "I am 20 years old.";
	
	/*1) socket()*/
	srvSd = socket(AF_INET, SOCK_STREAM, 0);
	if(srvSd == -1)
	{
		printf("Socket Error\n");
		return -1;
	}	
	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_port = htons(PORT);
	/*2) bind() - srvSd에 서버 주소/포트 할당*/
	if(bind(srvSd, (struct sockaddr *) &srvAddr, sizeof(srvAddr)) == -1)
	{
		printf("Bind Error");
		return -1;
	}
	/*3) listen()*/
	if(listen(srvSd, 5) == -1)
	{
		printf("Listen Error");
		return -1;	
	}
	clntAddrLen = sizeof(clntAddr);
	/*4) accept() -client의 연결 요청을 받는다*/
	clntSd = accept(srvSd, (struct sockaddr*)&clntAddr, &clntAddrLen);
	if(clntSd == -1)
	{
		printf("Accept Error");
		return -1;
	}
	/*5) recv()*/
	//readLen = read(clntSd, rBuff, sizeof(rBuff)-1);
	readLen=recv(clntSd,rBuff,sizeof(rBuff)-1,0);
	if(readLen == -1) 
	{
		printf("Read Error");
		return -1;
	}
	rBuff[readLen] = '\0';
	printf("Client: %s \n", rBuff);
	
	/*6) send()*/
	//write(clntSd, wBuff, sizeof(wBuff));
	send(clntSd,wBuff,sizeof(wBuff),0);
	close(clntSd);
	close(srvSd);	
	
	return 0;	
}
