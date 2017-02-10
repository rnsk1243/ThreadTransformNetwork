#include "MySocket.h"
CRITICAL_SECTION CS;

DWORD WINAPI thAcceptFunc(PVOID pvParam)
{
	cout << "AcceptFunc스레드 시작" << endl;
	CMySocket* socket = (CMySocket*)pvParam;
	while (true)
	{
		socket->Accept();
		if (socket->isReturnAccept)
		{
			cout << "Accept스레드 종료" << endl;
			return 1;
		}
	}
}

DWORD WINAPI thSendFunc(PVOID pvParam)
{
	cout << "thSendFunc스레드 시작" << endl;
	ClntSocketAddrStruct* ClntInfo = (ClntSocketAddrStruct*)pvParam;
	CMySocket* CMySocketClass = ClntInfo->CMySocket;

	while (true)
	{
		int isReturn = CMySocketClass->sendn(ClntInfo, BUF_SIZE, 0);
		Sleep(1000);
		// recv에 문제가 생겨 ClntInfo->isReturn에 true가 저장되거나 
		// send에 문제가 생겨 isReturn에 -1이 저장 되면 스레드를 종료한다.
		if (ClntInfo->isReturn == true || isReturn == -1) // 스레드 종료
		{
			cout << "Send스레드 종료" << endl;
			// send스레드가 종료되었다고 표시
			ClntInfo->sendThreadReturn = true;
			return 1;
		}
	}
}

DWORD WINAPI thRecvFunc(PVOID pvParam)
{
	cout << "thRecvFunc스레드 시작" << endl;
	ClntSocketAddrStruct* ClntInfo = (ClntSocketAddrStruct*)pvParam;
	CMySocket* CMySocketClass = ClntInfo->CMySocket;
	//cout << "스레드 내 소켓 주소 = " << sock << "|" << endl;
	list<ClntSocketAddrStruct*>::iterator iter;
	list<ClntSocketAddrStruct*>* infoList = CMySocketClass->getClntInfoList();

	//cout << ClntInfo->Buf << endl;

	while (true)
	{
		//char* Buf = new char[BUF_SIZE]; // 이러니까 소켓에러 안뜨네//.
		int recvsize = 0;
		recvsize = CMySocketClass->recvn(ClntInfo, BUF_SIZE, 0);
		// recv에 문제가 생길경우
		if (recvsize == -1) // 스레드 종료
		{
			// send스레드가 종료 될때까지 기다림.
			while (true)
			{
				if (ClntInfo->sendThreadReturn == true)
				{
					// 1. send스레드를 종료시킨다.
					// 2. 리스트에서 구조체를 제거한다.
					// 3. 소켓을 닫는다.
					// 4. 구조체 메모리를 해제한다.
					// 5. recv스레드를 종료시킨다.
					cout << "sendn스레드 종료" << endl;
					break;
				}
			}
			//////////////
			// 리스트에서 정보구조체 제거
			for (iter = infoList->begin(); iter != infoList->end(); ++iter)
			{
				if (*iter == ClntInfo)
				{
					infoList->erase(iter); // 삭제
					break;
				}
			}
			// 소켓 닫기
			closesocket(*ClntInfo->hClntSock);
			// 구조체 메모리 해제
			//(스레드가 send or recv하는 도중에 구조체를 지우면 문제가 발생하므로 반드시 스레드를 먼저 죽인후에 더 이상
			// send or recv하는 스레드가 없을때 제거 합니다.
			delete ClntInfo;
			//////////////
			cout << "recv스레드 종료" << endl;
			return 1;
		}
		cout << "서버 받은 데이터 크기 = " << recvsize << "|" << endl;
		//char* temp = new char[BUF_SIZE];
		//temp = *ClntInfo->Buf;
		for (iter = infoList->begin(); iter != infoList->end(); ++iter)
		{
			// 자신은 복사 하지 않는다.(주소 비교)
			if (&(*iter)->Buf == &ClntInfo->Buf)
			{
				cout << "같다 복사 안해" << endl;
				continue;
			}
			// ()을 해야 하는구나... *iter != (*iter) 
			//strcat_s((*iter)->Buf, BUF_SIZE, ClntInfo->Buf);
			(*iter)->Buf = ClntInfo->Buf;
		}
		// 문자열 초기화
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
	// 멤버변수만 지워주면 되겠죠??
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
	// 소켓&주소 메모리 할당
	SOCKET* hClntSock = new SOCKET();
	SOCKADDR_IN* hClntAddr = new SOCKADDR_IN();
	// accept
	szClntAddr = sizeof(*hClntAddr);
	*hClntSock = accept(hServSock, (SOCKADDR*)hClntAddr, &szClntAddr);// list나 벡터에 저장
	//cout << "accept 에서의 주소 = " << hClntSock << endl;
	if (*hClntSock == INVALID_SOCKET)
		err_display("accept() error");
	else
		printf("Connected client");
	// 스레드 핸들 보관할 공간
	HANDLE newThreadSend = NULL;
	HANDLE newThreadRecv = NULL;
	// 클라이언트 정보 담는 곳(스레드 핸들 포함)
	ClntSocketAddrStruct* ClntInfo = new ClntSocketAddrStruct();
	ClntInfo->CMySocket = this;
	ClntInfo->hClntSock = hClntSock;
	//cout << "accept 에서의 주소 = " << hClntSock << endl;
	ClntInfo->clntAddr = hClntAddr;
	ClntInfo->hThreadRecv = newThreadRecv;
	ClntInfo->hThreadSend = newThreadSend;
	// 리스트에 소켓구조체(스레드핸들포함) 담음.
	ClntInfoList->push_back(ClntInfo);

	// 스레드 만들기 (sendn용) // 구조체도 같이 넘김
	newThreadSend = CreateThread(NULL, 0, thSendFunc, ClntInfo, 0, NULL);
	// 스레드 만들기 (recvn용)
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
	cout << "보낸 Byte크기 = " << temp << endl;
	return temp;
}

int CMySocket::recvn(ClntSocketAddrStruct* sockInfo, int dataSize, int flags)
{
	// 받은 데이터 양(바이트)
	int receivedSize;
	// 남은 데이터 양(바이트)
	int left = dataSize;

	while (left > 0)
	{
		receivedSize = recv(*sockInfo->hClntSock, (char*)sockInfo->Buf, dataSize, flags);
		if (receivedSize == SOCKET_ERROR)
		{
			cout << "종료" << endl;
			sockInfo->isReturn = true;
			//=====================================
			//::EnterCriticalSection(&CS);
			//// 공용 포인터 변수
			//closeSocketStruct = sockInfo;
			//while (true)// closeSocket() 함수가 완료 될때까지 대기 
			//{
			//	if (closeSocketStruct == nullptr) // 함수가 완료되면
			//		break; // while문을 빠져나온다.
			//}
			//::LeaveCriticalSection(&CS);
			//======================================
			return SOCKET_ERROR;
		}
		else if (receivedSize == 0)
			break;
		// 남은 데이터 양
		left -= receivedSize;
	}
	// 전체 - 남은양 = 현재 받은 양
	return dataSize - left;
}

void CMySocket::thAccept(CMySocket * socket)
{
	hAcceptThread = CreateThread(NULL, 0, thAcceptFunc, this, 0, NULL);
	if (hAcceptThread == NULL)
		cout << "Accept스레드가 NULL 이다..." << endl;
	else
		cout << "hAcceptThread = " << hAcceptThread << endl;
}

//void CMySocket::closeSocket()
//{
//	// main스레드가 할 일은 필요없는 소켓과 스레드를 처분하는 것
//	if (closeSocketStruct == nullptr)
//		return;
//
//	// 리스트에서 정보구조체 제거
//	list<ClntSocketAddrStruct*>::iterator iter;
//
//	for (iter = ClntInfoList->begin(); iter != ClntInfoList->end(); ++iter)
//	{
//		if (*iter == closeSocketStruct)
//		{
//			ClntInfoList->erase(iter); // 삭제
//			break;
//		}
//	}
//	// 소켓 닫기
//	closesocket(*closeSocketStruct->hClntSock);
//	// 구조체 메모리 해제
//	delete closeSocketStruct;
//	closeSocketStruct = nullptr;
//}


