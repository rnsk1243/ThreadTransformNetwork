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
	// send�����尡 ���� �Ǿ����� Ȯ�ο�
	// send�����尡 ���� ���� �� �Ŀ� 
	// recv�����带 �����Ѵ�.
	bool sendThreadReturn = false;
	// ������
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
	// Accept������ �ڵ�
	HANDLE hAcceptThread;
	// ����Ǵ� ����ü (���� �� ����ü ���� nullptr�� �ƴϸ� �� ����ü�� �ش��ϴ� �����带 �����Ų��)
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
	// Ŭ���̾�Ʈ ���� �Ҷ� ȣ��Ǵ� �Լ�(������ ����, ���ϴݱ�)
	//void closeSocket();
};

