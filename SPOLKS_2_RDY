#include <iostream>
#include <stdio.h>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
class Server
{
public:
	Server(int port)
	{
		CheckDll();
		CreateSocket(port);
	};
	SOCKADDR_IN addr;
	WSADATA wsaData;
	SOCKET listenSocket;
	SOCKET acceptSocket;
	int socketResult;
	int CheckDll();
	int CreateSocket(int port);
	struct in_addr getMyIP();
	void NetWork();
	char recvbuff[1024];
	char mas[1024];
	int ByteReceived, nlen, count;
};


void Server::NetWork()
{
	while(1)
	{		int count = 0;
			do
			{
				ByteReceived =  recv(acceptSocket, recvbuff, sizeof(recvbuff), 0);
				if(ByteReceived == SOCKET_ERROR)
				{
					printf("\nConnection closed!\n");
					return;
				}
				mas[count] = recvbuff[0];
				count++;
			} while (recvbuff[0]!=13);
			if ( count > 0 )
			{
				for(int i=0;i < count-1;i++)
				{	
					printf("%c", mas[i]);
				}
				printf("\n");
				socketResult = send( acceptSocket, mas, int(count), 0);
			}
	}
}
int Server::CheckDll()
{
	   // int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
       if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
		{
			wprintf(L"WSAStartup failed with error: %ld\n", WSAGetLastError());
			Sleep(3000);
			exit(1);
		}
		return 0;
};
struct in_addr Server::getMyIP()
{
	char ac[80];
	gethostname(ac, sizeof(ac));
	struct hostent *phe = gethostbyname(ac);
    struct in_addr addr;
    memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
     //memcpy(&addr, argv[1], sizeof(struct in_addr));
    cout << "Address : " << inet_ntoa(addr) << endl;
	return addr;
};
int Server::CreateSocket(int port)
{
		CheckDll();
		struct in_addr address = getMyIP();
		listenSocket = INVALID_SOCKET;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port); //htons(port);
		addr.sin_addr.s_addr = address.S_un.S_addr; //inet_addr("127.0.0.1"); htonl(INADDR_ANY); htonl(address.S_un.S_addr); //address.S_un.S_addr

		if((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET ) //SOCK_STREAM (for UDP - SOCK_DGRAM)
		{

			wprintf(L"Error! Socket Failed. Decription: %d\n", WSAGetLastError());
			return 1;
		}
		if((socketResult = bind(listenSocket,(struct sockaddr*)&addr, sizeof(addr))) == SOCKET_ERROR) 
		{
		    wprintf(L"Error! Bind Failed. Decription: %d\n", WSAGetLastError());
			if((socketResult = closesocket(listenSocket)) == SOCKET_ERROR)
				wprintf(L"Error! Can not close socket. Decription: %d\n", WSAGetLastError());
			return 1;
		}
		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) // 1 = SOMAXCONN (для многопользователей)
			{
				wprintf(L"Error! Listen failed: %d\n", WSAGetLastError());
				return 1;
			}

		while(1)
		{
			wprintf(L"\nListening on a socket...\n");
			if((acceptSocket = accept(listenSocket, NULL, NULL)) == SOCKET_ERROR)//INVALID_SOCKET
			{
				wprintf(L"Error! Accept Failed. Decription: %d\n",WSAGetLastError());
				WSACleanup();
				return 1;
			}
			else
				wprintf(L"Connected...\n");
			NetWork();
			closesocket(acceptSocket);
		}
		closesocket(listenSocket);
		WSACleanup();
		return 0;
};
int main(int argc, char* argv[])
{
	int port = atoi(argv[2]);
	Server _server(port);
	return 1;
}
