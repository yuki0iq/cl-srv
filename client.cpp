#include <stdio.h>
#include <string>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32")

sockaddr_in loc;
SOCKET s = -1;
char ip[16];

bool init(void);
void cli(void);
bool cmd_1(void);
bool cmd_2(void);
void cmd_3(void);

int main(void)
{
	bool bRes = init();
	if(bRes)
		cli();

	return 0;
}

bool init(void)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	printf("Enter ip to connect (def:localhost): ");
	gets_s(ip);
	if(strlen(ip) == 0)
		strcpy(ip, "127.0.0.1");

	loc.sin_family = AF_INET;
	loc.sin_port = htons(7500);
	loc.sin_addr.s_addr = inet_addr(ip);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
	{
		perror("socket error");
		return false;
	}
	int rc = connect(s, (struct sockaddr*)&loc, sizeof(loc));
	if(rc)
	{
		closesocket(s);
		s = -1;
		perror("connect error");
		return false;
	}
	return true;
}
void cli(void)
{
	while(1)
	{
		printf("actions:\n");
		printf("1 - message\n");
		printf("2 - prog_start\n");
		printf("3 - close\n");
		printf("4 - close all\n");
		int cmd = 0;
		scanf("%i", &cmd);
		getchar();
		if((cmd == 3) || (cmd == 4))
		{
			int iRes = send(s, (const char*)&cmd, sizeof(cmd), 0);
			if(iRes == -1)
			{
				closesocket(s);
				break;
			}
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
		else if((cmd == 3) || (cmd == 4))
		{
			cmd_3();
			break;
		}
		if(!bRes)
		{
			closesocket(s);
			break;
		}
	}
}

bool cmd_1(void)
{
	char local_msg[10000];
	char answer[10000];
	int locmsglen;
	int iRes;
	int iCmdID = 1;
	int answerlen;

cmd1_loop:
	printf("C: ");
	gets_s(local_msg);
	locmsglen = (int)strlen(local_msg)+1;
	if(strlen(local_msg) == 0)
		goto cmd1_loop_end;
	iRes = send(s, (const char*)&iCmdID, sizeof(iCmdID), 0);
	if(iRes == -1)
		return false;
	iRes = 0;
	iRes = send(s, (const char*)&locmsglen, sizeof(locmsglen), 0);
	if(iRes == -1)
		return false;
	iRes = send(s, local_msg, locmsglen, 0);
	if(iRes == -1)
		return false;

	iRes = recv(s, (char*)&iCmdID, sizeof(iCmdID), 0);
	if(iRes == -1)
		return false;
	if(iCmdID != 1)
		goto cmd1_loop_end;
	answerlen = 0;
	iRes = recv(s, (char*)&answerlen, sizeof(answerlen), 0);
	if(iRes == -1)
		return false;
	iRes = recv(s, answer, answerlen, 0);
	if(iRes == -1)
		return false;
	printf("S: ");
	puts(answer);
	goto cmd1_loop;

cmd1_loop_end:
	return true;
}
bool cmd_2(void)
{
	char path_to_app[1000];
	int ptalen;
	int iRes;
	char msg[1000];
	int iTmp;
	int iCmdID = 2;

cmd2_loop:
	gets_s(path_to_app);
	ptalen = (int)strlen(path_to_app)+1;
	if(strlen(path_to_app) == 0)
		goto cmd2_loop_end;
	iRes = send(s, (const char*)&iCmdID, sizeof(iCmdID), 0);
	if(iRes == -1)
		return false;
	iRes = 0;
	iRes = send(s, (const char*)&ptalen, sizeof(ptalen), 0);
	if(iRes == -1)
		return false;
	iRes = send(s, path_to_app, ptalen, 0);
	if(iRes == -1)
		return false;

	iRes = recv(s, (char*)&iCmdID, sizeof(iCmdID), 0);
	if(iRes == -1)
		return false;
	if(iCmdID != 2)
		goto cmd2_loop_end;
	iRes = recv(s, (char*)&iTmp, sizeof(iTmp), 0);
	if(iRes == -1)
		return false;
	sprintf(msg, "Program \"%s\" returned \"%i\"", path_to_app, iTmp);
	puts(msg);
	goto cmd2_loop;

cmd2_loop_end:
	return true;
}
void cmd_3(void)
{
	printf("closing...");
	shutdown(s, SD_BOTH);
	closesocket(s);
}