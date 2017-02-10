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
	cout << "�����ڿ����� �ּ� = " << hClntSock << endl;
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
	// ���� ������ ��(����Ʈ)
	int receivedSize;
	// ���� ������ ��(����Ʈ)
	int left = dataSize;

	while (left > 0)
	{
		receivedSize = recv(*ClntSock, (char*)data, dataSize, flags);
		if (receivedSize == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (receivedSize == 0)
			break;
		// ���� ������ ��
		left -= receivedSize;
	}
	// ��ü - ������ = ���� ���� ��
	return dataSize - left;
}
