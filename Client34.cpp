#include <iostream>
#include <stdio.h>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
#include <errno.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
class Client
{
public:
	Client(string addrr, int port)
	{
		CheckDll();
		struct in_addr address;
		const char * pop = addrr.c_str();
		address.S_un.S_addr = inet_addr(pop);
		CreateSocket(port, address);
	};
	struct sockaddr_in addr;
	WSADATA wsaData;
	SOCKET connectSocket;
	//SOCKET acceptSocket;
	int socketResult;
	int CheckDll();
	int CreateSocket(int port, struct in_addr address);
	
	//void FindServer();
//	void Rghost();
	void NetWork();
};

int Client::CheckDll()
{
	    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != NO_ERROR) 
		{
			wprintf(L"WSAStartup failed with error: %ld\n", iResult);
			Sleep(3000);
			exit(1);
		}
		return 1;
};
int Client::CreateSocket(int port, struct in_addr address)
{
		CheckDll();
		//struct in_addr address = getMyIP();
		connectSocket = INVALID_SOCKET;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = address.S_un.S_addr; //inet_addr("127.0.0.1"); htonl(INADDR_ANY); address.S_un.S_addr;

		if((connectSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET ) //SOCK_STREAM (for UDP - SOCK_DGRAM)
		{
			int errrr = WSAGetLastError();			
			wprintf(L"Error! Socket Failed. Decription: %d\n", WSAGetLastError());
			return 1;
		}
		if((socketResult=connect(connectSocket, (sockaddr *)& addr, sizeof (addr))) == SOCKET_ERROR)
		{
			wprintf(L"Error! Connection Failed. Decription: %d\n", WSAGetLastError());
			if((socketResult = closesocket(connectSocket)) == SOCKET_ERROR)
				wprintf(L"Error! Can not close socket. Decription: %d\n", WSAGetLastError());
			return 1;
		}
		printf("Connected to server!\n");

		NetWork();
		return 0;
};
void Client::NetWork()
{
	///шлём имя файла
	///получаем смещение
	///"если смещение = размеру, то клиент просто разорвёт соединение"
	///иначе - отправка
	
	//добавить ввод
	bool keyPress = false;
	char * fileName = "readme.txt";
	if((socketResult = send(connectSocket, fileName, (int)strlen(fileName), 0)) == SOCKET_ERROR) //посылаем имя файла
	{
		wprintf(L"Error! Socket Failed. Decription: %d\n", WSAGetLastError());
		return;
	}
	int serverByteCount = 0;
	char buf[1024];
	int ByteReceived = recv(connectSocket, buf, sizeof(buf), 0);  //кол-во байт у сервера
	if(ByteReceived == SOCKET_ERROR)
	{

		printf("\nConnection closed!\n");
		Sleep(5000);
		return;
	}
	serverByteCount = atoi(buf);
	int clientFileLength = 0;
	FILE *_file;
	char path[256];
	sprintf (path, "C:\\send.rar");
	if(_file = fopen(path, "rb")) //если файл существует
	{
		fseek(_file, 0, SEEK_END);
		clientFileLength = ftell(_file);
		if(serverByteCount>clientFileLength)
		{
			closesocket(connectSocket);
			return;
		}
		if(serverByteCount == clientFileLength)
		{
			printf("File has already been uploaded!");
			closesocket(connectSocket);
			return;
		}
		fseek(_file, serverByteCount, SEEK_SET);	//ставим откуда надо посылать
	}
	else
		return;
	int sendCounter = clientFileLength - serverByteCount;
	int _bytes;
	RegisterHotKey(NULL, 1, NULL, VK_SPACE); //регистрируем пробел для внеполосных
	while(sendCounter>1024) //пока надо отправить больше чем буфер
	{
		byte buf [1024];
		if((_bytes = fread(buf, sizeof(byte), 1024, _file)) <= 0)
		{
			printf("Error While Reading From File! %d\n", errno);
			fclose(_file);
			closesocket(connectSocket);
			return;
		}
		if((socketResult = send(connectSocket,  (char*)buf, _bytes, 0)) == SOCKET_ERROR) //посылаем порцию
		{
			wprintf(L"Error! Socket Failed. Decription: %d\n", WSAGetLastError());
			return;
		}
		//printf("\nSEND COUNTER %d", sendCounter);
		HWND hwnd = NULL; 
		MSG msg;
		//внеполосный запрос
		while (PeekMessage(&msg, hwnd,  0, 0, PM_REMOVE)) 
		{ 
			switch(msg.message) 
			{ 
				case WM_HOTKEY: 
					{
						int _bytessend;
						byte _msg = 13;
						if((_bytessend = send(connectSocket, (char*)&_msg, 1, MSG_OOB)) == SOCKET_ERROR) //запрос на кол-во байт
						{
							wprintf(L"\nCannot send byte info request, sorry:( %d\n", WSAGetLastError());
						}
					}
			} 
		} 
		//внеполосный приём
		fd_set rfd;
		FD_ZERO(&rfd);
		FD_SET(connectSocket, &rfd);
		timeval tv = { 0 };
		tv.tv_sec = 0; tv.tv_usec = 0;
		if(select(0,NULL,NULL,&rfd,&tv) == 1) //проверка на приход внеполосных
		{

			byte _b;
			int brc;
			if((brc =(recv(connectSocket, (char*)&_b, 1, MSG_OOB))) == SOCKET_ERROR)
			{
				printf("\nCannot get byte info, sorry:( %d\n", errno);
			}
			else
			{
			//(int)ARRAYSIZE(buf);
			//printf("\nNUM: %d", int(_b));
				double ghhh = (double)ftell(_file) / (double)clientFileLength *100;
				system("cls");
				printf("\n%d %%\n", (int)ghhh); //выводим сколько загружено
			}
		}
	//	Sleep(100);//тестовая пауза типа отправляется
		ZeroMemory(buf, sizeof(buf));
		sendCounter = sendCounter - _bytes;
	}
	//последняя посылка
	if(sendCounter > 0)
	{
		byte buf[1024];
		if((_bytes = fread(buf, sizeof(byte), (int)sizeof(buf), _file)) <= 0)
		{
			printf("Error While Reading From File! %d\n", errno);
			fclose(_file);
			closesocket(connectSocket);
			return;
		}
		if((socketResult = send(connectSocket, (char*)buf, _bytes, 0)) == SOCKET_ERROR) //посылаем порцию
		{
			wprintf(L"Error! Socket Failed. Decription: %d\n", WSAGetLastError());
			return;
		}
	//	for(int i = 0; i <_bytes; i++)
		//	printf("%c", buf[i]);
	}

	printf("/nFile has been succesfully sent to the server!/n");
	fclose(_file);
	closesocket(connectSocket);
	return;

};
int main(int argc, char* argv[])
{
	string addrr = string(argv[1]);
	int port = atoi(argv[2]);
	Client _client(addrr, port);
	Sleep(20000);
	return 1;
}
