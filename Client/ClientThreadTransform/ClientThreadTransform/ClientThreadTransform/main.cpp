#include"MySocket.h"
#include<iostream>
using namespace std;

void main()
{
	CMySocket* socket = new CMySocket();
	TransformStruct data; // 보낼 구조체

	data.position = Vector3(9, 9, 9);
	data.rotation = Vector3(9, 9, 9);
	data.scale = Vector3(9, 9, 9);

	char* recvData = new char[BUF_SIZE];
	// 클라이언트 데이터 보내기
	cout << "보낸 데이터 크기 = " << socket->sendn(socket->getSocket(), &data, BUF_SIZE, 0) << "|" << endl;
	while (true)
	{
		socket->recvn(socket->getSocket(), recvData, BUF_SIZE, 0);

		TransformStruct* recvTransform = (TransformStruct*)recvData;
		cout << "position = " << recvTransform->position.x << ", " << recvTransform->position.y << ", " << recvTransform->position.z << endl;
		cout << "rotation = " << recvTransform->rotation.x << ", " << recvTransform->rotation.y << ", " << recvTransform->rotation.z << endl;
		cout << "scale = " << recvTransform->scale.x << ", " << recvTransform->scale.y << ", " << recvTransform->scale.z << endl;
	}
	
}