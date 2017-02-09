#pragma once
#include<WinSock2.h>
#include<list>
#include<iostream>
using namespace std;

#define BUF_SIZE sizeof(TransformStruct)

struct Vector3
{
	float x = 0;
	float y = 0;
	float z = 0;
	Vector3() {}
	Vector3(float x_, float y_, float z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}
};

struct TransformStruct
{
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
};

class CMySocket
{
	WSADATA wsaData;
	SOCKET* hClntSock;
	SOCKADDR_IN servAddr;
	TransformStruct dataStruct;
public:
	CMySocket();
	~CMySocket();
	void err_display(char *msg);
	int sendn(SOCKET* ClntSock, const void *data, int dataSize, int flags);
	int recvn(SOCKET* ClntSock, void* data, int dataSize, int flags);
	SOCKET* getSocket() { return hClntSock; }
};

