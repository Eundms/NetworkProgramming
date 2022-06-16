#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
// inet_ntop 사용으로, 추가함
#include <arpa/inet.h>

void errProc(const char *);
int main(int argc, char **argv)
{
	int mySock, readLen, nRecv, res, clntSd;
	char buff[BUFSIZ];
	struct sockaddr_in srcAddr, destAddr;
	socklen_t addrLen;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s Port", argv[0]);
		return 0;
	}
	// TCP 소켓 생성
	mySock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySock == -1)
		errProc("socket");

	// 도착지 주소 설정
	memset(&srcAddr, 0, sizeof(srcAddr));
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_port = htons(atoi(argv[1]));
	// bind() : mySocket에 주소 지정
	res = bind(mySock, (struct sockaddr *)&srcAddr, sizeof(srcAddr));
	if (res == -1)
		errProc("bind");
	addrLen = sizeof(destAddr);

	// listen()
	if (listen(mySock, 2) == -1)
	{
		printf("Listen Error");
		return -1;
	}
	while (1){

		// accpet()
		clntSd = accept(mySock, (struct sockaddr *)&destAddr, &addrLen);
		if (clntSd == -1)
		{
			printf("Accept Error");
			return -1;
		}
		// 하나의 클라이언트에 대한 요청이 모두 수행되어야 다음클라이언트 수행가능
		while (1){
			// recv()
			nRecv = recv(clntSd, buff, BUFSIZ - 1, 0);
			if (nRecv == -1){
				errProc("recv");
			}
			else if (nRecv > 0){// 이전 buff에 남아 있는 내용이 출력되지 않도록+문장의 끝 알려줌
				buff[nRecv] = '\0';
			}
			else{
				buff[nRecv] = '\0';
				break;
			}
			// 보낸 client의 host주소 알고 싶음
			char txt[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(destAddr.sin_addr), txt, sizeof(struct sockaddr_in));
			printf("%s:%d>%s\n", txt, ntohs(destAddr.sin_port), buff);
			nRecv = strlen(buff);

			// send()
			int nSend = send(clntSd, buff, nRecv, 0);
			if (nSend == -1)
				errProc("send");
		}
		close(clntSd);
	}
	close(mySock);

	return 0;
}

void errProc(const char *str)
{
	fprintf(stderr, "%s: %s \n", str, strerror(errno));
	exit(1);
}
