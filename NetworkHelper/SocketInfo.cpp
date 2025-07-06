#include "SocketInfo.h"
#include <iostream>

SocketInfo::SocketInfo()
{
	Init();
}

SocketInfo::~SocketInfo()
{
}

void SocketInfo::SendMsg(std::string_view Msg, SOCKET socket)
{
	if (SOCKET_ERROR == send(socket, Msg.data(), static_cast<int>(Msg.size()), 0))
	{
		return;
	}
}


void SocketInfo::DestroySocket()
{
	closesocket(Socket);
	WSACleanup();

	std::cout << "SocketError";
}

void SocketInfo::Init()
{
	int Error = 0;

	//���� �ʱ�ȭ
	Error = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (0 != Error)
	{
		return;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		return;
	}

	//���� ���� ����
	//�Ķ���� -> ip4 , ����� ��� , tcp
	//Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	Socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP,nullptr,0,WSA_FLAG_OVERLAPPED);

	if (Socket == INVALID_SOCKET)
	{
		std::cout << WSAGetLastError() << "\n";
		return;
	}

}
