#include"MySocket.h"

void main()
{
	CMySocket* socket = new CMySocket();
	socket->thAccept(socket);
	while (true)
	{
		cout << "���� list ����(���� Ŭ���̾�Ʈ ��) = " << socket->getClntInfoList()->size() << endl;
		Sleep(1000);
	}
}