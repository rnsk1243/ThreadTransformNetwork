#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "MySocket.h"



CMySocket::CMySocket()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		err_display("WSAStartup() error!");

	hClntSock = new SOCKET();

	*hClntSock = socket(PF_INET, SOCK_STREAM, 0);
	if (*hClntSock == INVALID_SOCKET)
		err_display("socket() error");
	cout << "생성자에서의 주소 = " << hClntSock << endl;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(IP);
	servAddr.sin_port = htons(PORT);

	if (connect(*hClntSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		err_display("connect() error!");
	else
		puts("Connected.............");
}


CMySocket::~CMySocket()
{
	closesocket(*hClntSock);
	delete &dataStruct;
}

void CMySocket::err_display(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int CMySocket::sendn(SOCKET * ClntSock, const void * data, int dataSize, int flags)
{
	if (data == nullptr)
	{
		return -1;
	}
	int temp = 0;
	while (true)
	{
		temp += send(*ClntSock, (char*)data, dataSize, 0);
		cout << "temp = " << temp << endl;
		if (temp >= dataSize)
			break;
	}
	return temp;
}

int CMySocket::recvn(SOCKET * ClntSock, void * data, int dataSize, int flags)
{
	// 받은 데이터 양(바이트)
	int receivedSize;
	// 남은 데이터 양(바이트)
	int left = dataSize;

	while (left > 0)
	{
		receivedSize = recv(*ClntSock, (char*)data, dataSize, flags);
		if (receivedSize == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (receivedSize == 0)
			break;
		// 남은 데이터 양
		left -= receivedSize;
	}
	// 전체 - 남은양 = 현재 받은 양
	return dataSize - left;
}
