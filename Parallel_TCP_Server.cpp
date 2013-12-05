#include <iostream>
#include <stdio.h>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
#include <stdlib.h>
#include <process.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
DWORD WINAPI TCPWork(LPVOID _acceptSocket);
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
     //   SOCKET acceptSocket;
        int socketResult;
        int CheckDll();
        int CreateSocket(int port);
		//LPTHREAD_START_ROUTINE TCPWork(LPVOID _acceptSocket);
        struct in_addr getMyIP();
};


int Server::CheckDll()
{
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
				SOCKET client_socket; // сокет для клиента
				sockaddr_in client_addr; // адрес клиента
				int client_addr_size=sizeof(client_addr);
				while((client_socket=accept(listenSocket, (sockaddr *)&client_addr, &client_addr_size)))
                {
					if(client_socket == SOCKET_ERROR)
                    {
                        wprintf(L"Error! Accept Failed. Decription: %d\n",WSAGetLastError());
                        WSACleanup();
                        return 1;
                    }
                    else
					{
					    // пытаемся получить имя хоста
					    HOSTENT *hst;
					    hst=gethostbyaddr((char *)&client_addr.sin_addr.s_addr,4, AF_INET);
						printf("+%s [%s] new connect!\n",(hst)?hst->h_name:"",inet_ntoa(client_addr.sin_addr));
						DWORD thID;
						CreateThread(NULL,NULL, (LPTHREAD_START_ROUTINE) TCPWork,&client_socket,NULL,&thID);
                       // TCPWork();
                      //  closesocket(acceptSocket);
					}
                }
                closesocket(listenSocket);
                WSACleanup();
                return 0;
};
DWORD WINAPI TCPWork(LPVOID _acceptSocket)
{
        ///получаем имя файла
        ///есил не существует - создать
        ///если существует - высылаем смещение
        ///"если смещение = размеру, то клиент просто разорвёт соединение"
        ///иначе - ждём отправки
		int socketResult;
		SOCKET acceptSocket;
		acceptSocket=((SOCKET *) _acceptSocket)[0];

        char recvbuff[1024];
        int ByteReceived = 1;
		ByteReceived = recv(acceptSocket, (char*)recvbuff, sizeof(recvbuff), 0); 
        if(ByteReceived == SOCKET_ERROR)
        {
            printf("\nConnection closed!\n");
            return 1;
        }       
		
		string filename = string((char*)recvbuff).substr(0, ByteReceived);
		string disk = "D:\\" + filename;
		//const char * rez = strcat(disk, filename.c_str());
        FILE *_file;
        char path[256];
        int serverFileLength;
		if(_file = fopen(disk.c_str(), "r+b")) //если файл существует
        {
                fseek(_file, 0, SEEK_END);
                serverFileLength = ftell(_file); //смотрим, сколько уже есть на сервере        
        }
        else
        {
			if((_file = fopen(disk.c_str(), "wb")) == NULL) //если не получилось создать файл - досвидания
                {
                        printf("Error");
                        fclose(_file);
                        return 1;
                }
                serverFileLength = 0; //файл же пустой
        }
        char sl[1024];
        _itoa(serverFileLength, sl, 10);
        if((socketResult = send( acceptSocket, sl, (int)strlen(sl), 0)) == SOCKET_ERROR) //посылаем сколько уже есть
        {
                wprintf(L"Error! Socket Failed. Decription: %d\n", WSAGetLastError());
                fclose(_file);
                return 1;
        }
        
        while(ByteReceived != SOCKET_ERROR)
        {
                byte buf [1024]; //чистый приёмник
                ByteReceived = recv(acceptSocket, (char*)buf, sizeof(buf), 0); 
                if((ByteReceived == SOCKET_ERROR) || (ByteReceived == 0))
                {
                        printf("\nConnection closed!\n");
                        fclose(_file);
                        return 1;
                }                
                int wrlength = (int)ARRAYSIZE(buf);
                if((fwrite(buf, 1, wrlength, _file)) == 0 ) //если ошибка возникла strlen((char*)buf)
                {
                        printf("/nError File Write!/n");
                        fclose(_file);
                        return 1;
                }
                //внеполосный приём
                fd_set rfd;
                FD_ZERO(&rfd);
                FD_SET(acceptSocket, &rfd);
                timeval tv = { 0 };
                tv.tv_sec = 0; tv.tv_usec = 0;
                if(select(0,NULL,NULL,&rfd,&tv) == 1) //проверка на приход внеполосных
                {
                        char num [20];
                        byte _msg;
                        if((recv(acceptSocket, (char *)&_msg, 1, MSG_OOB)) == SOCKET_ERROR)
                        {
                                printf("\nCannot get byte info, sorry:(\n");
                        }
                        int _bytessend;
                        if((_bytessend = send(acceptSocket, (char*)&_msg, 1, MSG_OOB)) == SOCKET_ERROR) //_sl, (int)strlen(_sl)
                        {
                                wprintf(L"\nCannot send byte info, sorry:( %d\n", WSAGetLastError());
                        }
                }
                ZeroMemory(buf, sizeof(buf));
        }
                //файл докачался
                fclose(_file);
                return 1;
};
int main(int argc, char* argv[])
{
        int port = atoi(argv[2]);
        Server _server(port);
        Sleep(3000);
        return 1;
}
