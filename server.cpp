#include <stdio.h>
#include <string.h>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32")

sockaddr_in loc;
sockaddr_in cli;
SOCKET s = -1;
SOCKET s1= -1;
char ip[16];
char loc_ip[16];
const unsigned short port = 7500;

bool init(void);
bool serv(void);
bool cmd_1(void);
bool cmd_2(void);
void cmd_3(void);
void cmd_4(void);

int main(void)
{
	bool bRes = init();
	if(!bRes)
		return 0;
start:
	bRes = serv();
	if(bRes)
		goto start;

	return 0;
}

bool init(void)
{
	printf("Write IP to start from (def:ANY): ");
	gets_s(loc_ip);

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	loc.sin_family = AF_INET;
	loc.sin_port = htons(port);
	if(strlen(loc_ip) == 0)
		loc.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		loc.sin_addr.s_addr = inet_addr(loc_ip);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == s)
	{
		perror("socket error");
		return false;
	}
	int rc = bind(s, (struct sockaddr*)&loc, sizeof(loc));
	if(rc == SOCKET_ERROR)
	{
		closesocket(s);
		s = -1;
		perror("bind error");
		return false;
	}

	char hello_msg[100];
	char *tmp = inet_ntoa(loc.sin_addr);
	if(tmp != 0)
		strcpy(loc_ip, tmp);
	sprintf(hello_msg, "Server started on %s:%hu", loc_ip, port);
	puts(hello_msg);

	rc = listen(s, 5);
	if(rc == SOCKET_ERROR)
	{
		closesocket(s);
		s = -1;
		perror("listen error");
		return false;
	}
	return true;
}
bool serv(void)
{
	int cli_len = sizeof(cli);
//	if(s1 = accept(s, (struct sockaddr*)&cli, &cli_len) == INVALID_SOCKET)
	s1 = accept(s, (struct sockaddr*)&cli, &cli_len);
	if(INVALID_SOCKET == s1)
	{
		perror("accept error");
		return true;
	}
	char *tmp = inet_ntoa(cli.sin_addr);
	if(tmp != 0)
		strcpy(ip, tmp);

	printf("%s", ip);
	puts(" connected");
	while(1)
	{
		int cmd = 0;
		int iRes = recv(s1, (char*)&cmd, sizeof(cmd), 0);
		if(-1 == iRes)
		{
			closesocket(s1);
			s1 = -1;
			perror("recv error");
			return true;
		}
		bool bRes = true;
		if(cmd == 1)
		{
			bRes = cmd_1();
		}
		else if(cmd == 2)
		{
			bRes = cmd_2();
		}
		else if(cmd == 3)
		{
			cmd_3();
			return true;
		}
		else if(cmd == 4)
		{
			cmd_4();
			return false;
		}
		if(!bRes)
		{
			closesocket(s1);
			s1 = -1;
			return true;
		}
	}
	return true;
}

bool cmd_1(void)
{
	char answer[10000];
	char clientmsg[10000];
	int climsglen;
	int iRes;
	int iCmdID = 1;
	int answerlen;

	iRes = recv(s1, (char*)&climsglen, sizeof(climsglen), 0);
	if(iRes == -1)
		return false;
	iRes = recv(s1, clientmsg, climsglen, 0);
	if(iRes == -1)
		return false;
	printf("C: ");
	puts(clientmsg);
	printf("S: ");
	gets_s(answer);

	iRes = send(s1, (const char*)&iCmdID, sizeof(iCmdID), 0);
	if(iRes == -1)
		return false;
	answerlen = (int)strlen(answer)+1;
	iRes = send(s1, (const char*)&answerlen, sizeof(answerlen), 0);
	if(iRes == -1)
		return false;
	iRes = send(s1, answer, answerlen, 0);
	if(iRes == -1)
		return false;

	return true;
}
bool cmd_2(void)
{
	char path_to_app[1000];
	int len;
	int iRes;
	int msglen;
	char *msg;
	int iCmdID = 2;
	int iTmp;

	iRes = recv(s1, (char*)&len, sizeof(len), 0);
	if(iRes == -1)
		return false;
	if(len == 1)
		return true;
	iRes = recv(s1, path_to_app, len, 0);
	if(iRes == -1)
		return false;
	printf("launching \"%s\"...\n", path_to_app);
	iTmp = system(path_to_app);

	iRes = send(s1, (const char*)&iCmdID, sizeof(iCmdID), 0);
	if(iRes == -1)
		return false;
	iRes = send(s1, (const char*)&iTmp, sizeof(iTmp), 0);
	if(iRes == -1)
		return false;

	return true;
}
void cmd_3(void)
{
	printf("client disconnected.\n");
	closesocket(s1);
	s1 = -1;
}
void cmd_4(void)
{
	printf("closing...\n");
	shutdown(s, SD_BOTH);
	closesocket(s);
	s = -1;
	closesocket(s1);
	s1 = -1;
}