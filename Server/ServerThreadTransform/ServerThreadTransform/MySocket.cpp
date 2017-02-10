#include "MySocket.h"
CRITICAL_SECTION CS;

DWORD WINAPI thAcceptFunc(PVOID pvParam)
{
	cout << "AcceptFunc������ ����" << endl;
	CMySocket* socket = (CMySocket*)pvParam;
	while (true)
	{
		socket->Accept();
		if (socket->isReturnAccept)
		{
			cout << "Accept������ ����" << endl;
			return 1;
		}
	}
}

DWORD WINAPI thSendFunc(PVOID pvParam)
{
	cout << "thSendFunc������ ����" << endl;
	ClntSocketAddrStruct* ClntInfo = (ClntSocketAddrStruct*)pvParam;
	CMySocket* CMySocketClass = ClntInfo->CMySocket;

	while (true)
	{
		int isReturn = CMySocketClass->sendn(ClntInfo, BUF_SIZE, 0);
		Sleep(1000);
		// recv�� ������ ���� ClntInfo->isReturn�� true�� ����ǰų� 
		// send�� ������ ���� isReturn�� -1�� ���� �Ǹ� �����带 �����Ѵ�.
		if (ClntInfo->isReturn == true || isReturn == -1) // ������ ����
		{
			cout << "Send������ ����" << endl;
			// send�����尡 ����Ǿ��ٰ� ǥ��
			ClntInfo->sendThreadReturn = true;
			return 1;
		}
	}
}

DWORD WINAPI thRecvFunc(PVOID pvParam)
{
	cout << "thRecvFunc������ ����" << endl;
	ClntSocketAddrStruct* ClntInfo = (ClntSocketAddrStruct*)pvParam;
	CMySocket* CMySocketClass = ClntInfo->CMySocket;
	//cout << "������ �� ���� �ּ� = " << sock << "|" << endl;
	list<ClntSocketAddrStruct*>::iterator iter;
	list<ClntSocketAddrStruct*>* infoList = CMySocketClass->getClntInfoList();

	//cout << ClntInfo->Buf << endl;

	while (true)
	{
		//char* Buf = new char[BUF_SIZE]; // �̷��ϱ� ���Ͽ��� �ȶ߳�//.
		int recvsize = 0;
		recvsize = CMySocketClass->recvn(ClntInfo, BUF_SIZE, 0);
		// recv�� ������ ������
		if (recvsize == -1) // ������ ����
		{
			// send�����尡 ���� �ɶ����� ��ٸ�.
			while (true)
			{
				if (ClntInfo->sendThreadReturn == true)
				{
					// 1. send�����带 �����Ų��.
					// 2. ����Ʈ���� ����ü�� �����Ѵ�.
					// 3. ������ �ݴ´�.
					// 4. ����ü �޸𸮸� �����Ѵ�.
					// 5. recv�����带 �����Ų��.
					cout << "sendn������ ����" << endl;
					break;
				}
			}
			//////////////
			// ����Ʈ���� ��������ü ����
			for (iter = infoList->begin(); iter != infoList->end(); ++iter)
			{
				if (*iter == ClntInfo)
				{
					infoList->erase(iter); // ����
					break;
				}
			}
			// ���� �ݱ�
			closesocket(*ClntInfo->hClntSock);
			// ����ü �޸� ����
			//(�����尡 send or recv�ϴ� ���߿� ����ü�� ����� ������ �߻��ϹǷ� �ݵ�� �����带 ���� �����Ŀ� �� �̻�
			// send or recv�ϴ� �����尡 ������ ���� �մϴ�.
			delete ClntInfo;
			//////////////
			cout << "recv������ ����" << endl;
			return 1;
		}
		cout << "���� ���� ������ ũ�� = " << recvsize << "|" << endl;
		//char* temp = new char[BUF_SIZE];
		//temp = *ClntInfo->Buf;
		for (iter = infoList->begin(); iter != infoList->end(); ++iter)
		{
			// �ڽ��� ���� ���� �ʴ´�.(�ּ� ��)
			if (&(*iter)->Buf == &ClntInfo->Buf)
			{
				cout << "���� ���� ����" << endl;
				continue;
			}
			// ()�� �ؾ� �ϴ±���... *iter != (*iter) 
			//strcat_s((*iter)->Buf, BUF_SIZE, ClntInfo->Buf);
			(*iter)->Buf = ClntInfo->Buf;
		}
		// ���ڿ� �ʱ�ȭ
		//ClntInfo->Buf = nullptr;
		Sleep(1000);
	}
}

CMySocket::CMySocket()
{
	ClntInfoList = new list<ClntSocketAddrStruct*>();
	//closeSocketStruct = nullptr;

	::InitializeCriticalSection(&CS);

	hAcceptThread = NULL;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		err_display("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		err_display("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(PORT);

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		err_display("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		err_display("listen() error");
	else
		printf("listen...");
}


CMySocket::~CMySocket()
{
	// ��������� �����ָ� �ǰ���??
	::DeleteCriticalSection(&CS);
	isReturnAccept = true;
	closesocket(hServSock);
	delete ClntInfoList;
	delete &servAddr;
	//delete closeSocketStruct;
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


void CMySocket::Accept()
{
	// ����&�ּ� �޸� �Ҵ�
	SOCKET* hClntSock = new SOCKET();
	SOCKADDR_IN* hClntAddr = new SOCKADDR_IN();
	// accept
	szClntAddr = sizeof(*hClntAddr);
	*hClntSock = accept(hServSock, (SOCKADDR*)hClntAddr, &szClntAddr);// list�� ���Ϳ� ����
	//cout << "accept ������ �ּ� = " << hClntSock << endl;
	if (*hClntSock == INVALID_SOCKET)
		err_display("accept() error");
	else
		printf("Connected client");
	// ������ �ڵ� ������ ����
	HANDLE newThreadSend = NULL;
	HANDLE newThreadRecv = NULL;
	// Ŭ���̾�Ʈ ���� ��� ��(������ �ڵ� ����)
	ClntSocketAddrStruct* ClntInfo = new ClntSocketAddrStruct();
	ClntInfo->CMySocket = this;
	ClntInfo->hClntSock = hClntSock;
	//cout << "accept ������ �ּ� = " << hClntSock << endl;
	ClntInfo->clntAddr = hClntAddr;
	ClntInfo->hThreadRecv = newThreadRecv;
	ClntInfo->hThreadSend = newThreadSend;
	// ����Ʈ�� ���ϱ���ü(�������ڵ�����) ����.
	ClntInfoList->push_back(ClntInfo);

	// ������ ����� (sendn��) // ����ü�� ���� �ѱ�
	newThreadSend = CreateThread(NULL, 0, thSendFunc, ClntInfo, 0, NULL);
	// ������ ����� (recvn��)
	newThreadRecv = CreateThread(NULL, 0, thRecvFunc, ClntInfo, 0, NULL);

}

int CMySocket::sendn(ClntSocketAddrStruct* sockInfo, int dataSize, int flags)
{
	if (sockInfo == nullptr || sockInfo->Buf == nullptr)
	{
		return -1;
	}
	int temp = 0;
	while (true)
	{
		temp += send(*sockInfo->hClntSock, (char*)sockInfo->Buf, dataSize, 0);
		if (temp >= dataSize)
			break;
	}
	cout << "���� Byteũ�� = " << temp << endl;
	return temp;
}

int CMySocket::recvn(ClntSocketAddrStruct* sockInfo, int dataSize, int flags)
{
	// ���� ������ ��(����Ʈ)
	int receivedSize;
	// ���� ������ ��(����Ʈ)
	int left = dataSize;

	while (left > 0)
	{
		receivedSize = recv(*sockInfo->hClntSock, (char*)sockInfo->Buf, dataSize, flags);
		if (receivedSize == SOCKET_ERROR)
		{
			cout << "����" << endl;
			sockInfo->isReturn = true;
			//=====================================
			//::EnterCriticalSection(&CS);
			//// ���� ������ ����
			//closeSocketStruct = sockInfo;
			//while (true)// closeSocket() �Լ��� �Ϸ� �ɶ����� ��� 
			//{
			//	if (closeSocketStruct == nullptr) // �Լ��� �Ϸ�Ǹ�
			//		break; // while���� �������´�.
			//}
			//::LeaveCriticalSection(&CS);
			//======================================
			return SOCKET_ERROR;
		}
		else if (receivedSize == 0)
			break;
		// ���� ������ ��
		left -= receivedSize;
	}
	// ��ü - ������ = ���� ���� ��
	return dataSize - left;
}

void CMySocket::thAccept(CMySocket * socket)
{
	hAcceptThread = CreateThread(NULL, 0, thAcceptFunc, this, 0, NULL);
	if (hAcceptThread == NULL)
		cout << "Accept�����尡 NULL �̴�..." << endl;
	else
		cout << "hAcceptThread = " << hAcceptThread << endl;
}

//void CMySocket::closeSocket()
//{
//	// main�����尡 �� ���� �ʿ���� ���ϰ� �����带 ó���ϴ� ��
//	if (closeSocketStruct == nullptr)
//		return;
//
//	// ����Ʈ���� ��������ü ����
//	list<ClntSocketAddrStruct*>::iterator iter;
//
//	for (iter = ClntInfoList->begin(); iter != ClntInfoList->end(); ++iter)
//	{
//		if (*iter == closeSocketStruct)
//		{
//			ClntInfoList->erase(iter); // ����
//			break;
//		}
//	}
//	// ���� �ݱ�
//	closesocket(*closeSocketStruct->hClntSock);
//	// ����ü �޸� ����
//	delete closeSocketStruct;
//	closeSocketStruct = nullptr;
//}


