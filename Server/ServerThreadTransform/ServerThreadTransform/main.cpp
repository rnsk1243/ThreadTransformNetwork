#include"MySocket.h"

void main()
{
	CMySocket* socket = new CMySocket();
	socket->thAccept(socket);
	while (true)
	{
		cout << "현재 list 개수(붙은 클라이언트 수) = " << socket->getClntInfoList()->size() << endl;
		Sleep(1000);
	}
}