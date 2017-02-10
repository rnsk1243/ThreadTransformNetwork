#pragma once
#include<WinSock2.h>
#include<list>
#include<iostream>
using namespace std;

#define BUF_SIZE sizeof(TransformStruct)
#define PORT 9000

struct Vector3
{
	float x = 0;
	float y = 0;
	float z = 0;
};

struct TransformStruct
{
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
};
class CMySocket;

struct ClntSocketAddrStruct
{
	CMySocket* CMySocket;
	HANDLE hThreadSend;
	HANDLE hThreadRecv;
	SOCKET* hClntSock;
	SOCKADDR_IN* clntAddr;
	char* Buf = new char[BUF_SIZE];
	bool isReturn = false;
	// send스레드가 종료 되었는지 확인용
	// send스레드가 먼저 종료 된 후에 
	// recv스레드를 종료한다.
	bool sendThreadReturn = false;
	// 생성자
	ClntSocketAddrStruct() 
	{
	}
};

class CMySocket
{
	list<ClntSocketAddrStruct*>* ClntInfoList;
	WSADATA wsaData;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	int szClntAddr;
	// Accept스레드 핸들
	HANDLE hAcceptThread;
	// 종료되는 구조체 (만약 이 구조체 값이 nullptr이 아니면 이 구조체에 해당하는 스레드를 종료시킨다)
public:
	bool isReturnAccept = false;
	//ClntSocketAddrStruct* closeSocketStruct;
	CMySocket();
	~CMySocket();
	void err_display(char *msg);
	void Accept();
	int sendn(ClntSocketAddrStruct* sockInfo, int dataSize, int flags);
	int recvn(ClntSocketAddrStruct* sockInfo, int dataSize, int flags);
	void thAccept(CMySocket* socket);
	list<ClntSocketAddrStruct*>* getClntInfoList() { return ClntInfoList; }
	// 클라이언트 종료 할때 호출되는 함수(스레드 종료, 소켓닫기)
	//void closeSocket();
};

