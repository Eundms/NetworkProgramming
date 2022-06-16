#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <errno.h>
#include <time.h> //서버 시간 확인

#define MAX_EVENTS 100
#define MAX_CLIENT 100

void errProc(const char *);
void setData(char *, char *);
void setTime(char *, time_t *);
void setSender(char *, int);
void sendToAll(int, char *);
void setMsg(char *, char *);
int findId(int);
struct clntInfo
{
	int eventfd; // epoll에서 사용
	struct sockaddr_in clntAddr;
};
struct clntInfo clnt_socks[MAX_CLIENT];
int clnt_cnt = 0;
int main(int argc, char **argv)
{
	int listenSd, connectSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, readLen;
	char rBuff[BUFSIZ - 50];
	int i;

	int epfd, ready, readfd;
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];

	time_t timer; //서버 시간 확인

	if (argc != 2)
	{
		printf("Usage: %s [Port Number]\n", argv[0]);
		return -1;
	}

	printf(">> Server Started\n");

	epfd = epoll_create(1);
	if (epfd == -1)
		errProc("epoll_create");

	listenSd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSd == -1)
		errProc("socket");

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(atoi(argv[1]));

	if (bind(listenSd, (struct sockaddr *)&srvAddr, sizeof(srvAddr)) == -1)
		errProc("bind");
	if (listen(listenSd, 5) < 0)
		errProc("listen");

	ev.events = EPOLLIN; 
	ev.data.fd = listenSd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenSd, &ev) == -1)
		errProc("epoll_ctl");

	clntAddrLen = sizeof(clntAddr);
	while (1)
	{
		ready = epoll_wait(epfd, events, MAX_EVENTS, -1); //발생한 이벤트 개수
		if (ready == -1)
		{
			if (errno == EINTR)
				continue;
			else
				errProc("epoll_wait");
		}
		for (i = 0; i < ready; i++)
		{
			if (events[i].data.fd == listenSd)
			{ // accept a client
				connectSd = accept(listenSd, (struct sockaddr *)&clntAddr, &clntAddrLen);
				if (connectSd == -1)
				{
					fprintf(stderr,"Accept Error");
					continue;
				}
				fprintf(stderr, ">> Accept connection from client\n");
				/*서버에 사용자 입장*/
				ev.data.fd = connectSd;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, connectSd, &ev) == -1)
					errProc("epoll_ctl");
				// 소켓 관리 위해서
				clnt_socks[clnt_cnt].eventfd = connectSd; 
				clnt_socks[clnt_cnt].clntAddr = clntAddr;
				clnt_cnt++;
				/*모든 연결된 사용자에게 메시지 전송*/
				char sBuff[BUFSIZ];
				memset(sBuff, 0, sizeof(sBuff));
				setSender(sBuff, ntohs(clntAddr.sin_port));
				setMsg(sBuff, "가 입장하셨습니다.\n");
				sendToAll(clnt_cnt, sBuff);

				continue;
			}
			else
			{ // IO
				readfd = events[i].data.fd;
				readLen = read(readfd, rBuff, sizeof(rBuff) - 1);
				if (readLen == -1)
				{
					if (errno != EAGAIN)
					{
						fprintf(stderr, "Read Error \n");
					}
					printf("data unavailable\n");
					break;
				}
				/*채팅방 나감*/
				if (readLen == 0)
				{	int leaver=findId(readfd);
					fprintf(stderr, "leave user : 사용자%d\n", leaver);

					if (epoll_ctl(epfd, EPOLL_CTL_DEL, readfd, &ev) == -1)
						errProc("epoll_ctl");
					for (int j = 0; j < clnt_cnt; ++j)
					{
						if (clnt_socks[j].eventfd == readfd)
						{
							while (j < clnt_cnt - 1)
							{
								clnt_socks[j] = clnt_socks[j + 1];
								++j;
							}
							break;
						}
					}
					--clnt_cnt;
					//다른 모든 client에게 퇴장을 알림
					char sBuff[BUFSIZ]; // 클라이언트에게 전달할 정보를 담는 버퍼
					memset(sBuff, 0, sizeof(sBuff));
					setSender(sBuff, leaver); // 퇴장한 사람 정보 설정
					setMsg(sBuff, "님이 대화방을 나갔습니다.\n");// 메시지 설정
					sendToAll(clnt_cnt, sBuff);// 모든 클라이언트에게 전달

					close(readfd);
					continue;
				}

				/*사용자가 메시지 보냄*/
				rBuff[readLen] = '\0';
				printf("From client : 사용자(%d) : %s", findId(events[i].data.fd), rBuff);

				char sBuff[BUFSIZ]; // [2022.05.07. 14:10:00] 사용자 n : rBuff내용
				memset(sBuff, 0, sizeof(sBuff));
				setTime(sBuff, &timer);
				setSender(sBuff, findId(events[i].data.fd));
				setData(sBuff, rBuff);

				/*모든 연결된 사용자에게 메시지 전송(echo)*/
				sendToAll(clnt_cnt, sBuff);
			}
		}
	}
	close(listenSd);
	close(epfd);
	return 0;
}
int findId(int eventfd)
{
	for (int i = 0; i < clnt_cnt; i++)
	{
		if (clnt_socks[i].eventfd == eventfd)
		{ //찾음!
			return ntohs(clnt_socks[i].clntAddr.sin_port);
		}
	}
}
void setMsg(char *sBuff, char *msg)
{
	strncat(sBuff, msg, strlen(msg));
}
void sendToAll(int clnt_cnt, char *sBuff)
{
	for (int i = 0; i < clnt_cnt; i++)
	{
		write(clnt_socks[i].eventfd, sBuff, strlen(sBuff));
	}
}
void setSender(char *sBuff, int clientNo) // 넘길 clientNo생각해볼것
{
	char clientInfo[15];
	sprintf(clientInfo, "사용자%d", clientNo);
	strncat(sBuff, clientInfo, strlen(clientInfo));
}
void setTime(char *sBuff, time_t *timer)
{
	time(timer); //현재 시간
	struct tm *t = localtime(timer);
	sprintf(sBuff, "[ %d.%02d.%02d. %02d:%02d:%02d ] ", 1900 + t->tm_year, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}
void setData(char *sBuff, char *rBuff)
{
	char connecter[5] = " : ";
	strncat(sBuff, connecter, strlen(connecter));
	strncat(sBuff, rBuff, strlen(rBuff));
}
void errProc(const char *str)
{
	fprintf(stderr, "%s: %s", str, strerror(errno));
	exit(1);
}
