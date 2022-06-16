#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
//inet_ntop 사용으로, 추가함
#include <arpa/inet.h>

void errProc(const char*);
int main(int argc, char** argv)
{
	int mySock,readLen, nRecv, res;
	char buff[BUFSIZ];
	struct sockaddr_in srcAddr, destAddr;
	socklen_t addrLen;

	if(argc != 2) {
		fprintf(stderr,"Usage: %s Port",argv[0]);
		return 0;  
	}	
	mySock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(mySock == -1) errProc("socket");	
	memset(&srcAddr, 0, sizeof(srcAddr));
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_port = htons(atoi(argv[1]));

	res = bind(mySock,(struct sockaddr *) &srcAddr,sizeof(srcAddr));	
	if(res == -1) errProc("bind");
	addrLen = sizeof(destAddr);
	while(1)
	{
		nRecv = recvfrom(mySock, buff, BUFSIZ-1 , 0,(struct sockaddr *) &destAddr,&addrLen);
		if(nRecv == -1) errProc("recvfrom");

		// 이전 buff에 남아 있는 내용이 출력되지 않도록 하기 위해
		if(nRecv > 0) buff[nRecv]='\0';
		else buff[nRecv] = '\0';

		// 보낸 client의 host주소 알고 싶음
		char txt[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(destAddr.sin_addr),txt,sizeof(struct sockaddr_in));
		printf("%s:%d>%s\n",txt,ntohs(destAddr.sin_port),buff);
		nRecv = strlen(buff);	
		sendto(mySock, buff, nRecv, 0, (struct sockaddr *) &destAddr, addrLen);		
	}	
	close(mySock);//추가해야 함
	return 0;
}

void errProc(const char* str)
{
	fprintf(stderr,"%s: %s \n", str, strerror(errno));
	exit(1);
}
