#include "Client.h"
#include <Ws2tcpip.h>
#include "iostream"

Client::Client()
{
}

Client::~Client()
{
	DestroySocket();
}

bool Client::AccessServer()
{
	int Error = 0;

	//서버에 접속 시도
	Error = connect(Socket, (sockaddr*)&Address, sizeof(Address));

	if (SOCKET_ERROR == Error)
	{
		std::cout << "Connect Fail" << std::endl;
		return false;
	}

	//이름전송
	Error = send(Socket, strAccessName.c_str(), sizeof(strAccessName.c_str()), 0);

	//0이면 연결 닫힘
	if (Error == 0)
	{
		std::cout << "Connect Close" << std::endl;
		return false;
	}

	char strBuffer[PACKETSIZE] = {};
	int PacketSize = recv(Socket, strBuffer, PACKETSIZE, 0);

	std::string strTemp;
	strTemp.resize(PacketSize);
	strTemp.assign(strBuffer, PacketSize);

	std::cout << strTemp << std::endl;

	RecvThread = std::thread(std::bind(&Client::ReceiveMessage, this));

	return true;
}

void Client::SetAccessAddress(int Port, std::string_view IP)
{
	//주소 정보
	Address.sin_port = Port;
	Address.sin_family = AF_INET;

	if (1 != inet_pton(AF_INET, IP.data(), &Address.sin_addr.S_un.S_addr))
	{
		closesocket(Socket);
		WSACleanup();
		std::cout << WSAGetLastError() << std::endl;
		return;
	}
}

void Client::SendServer(std::string_view Message)
{
	SendMsg(Message, Socket);
}

void Client::ReceiveMessage()
{
	while (true)
	{
		char strBuffer[PACKETSIZE] = {};

		int PacketSize = 0;
		PacketSize = recv(Socket, strBuffer, PACKETSIZE, 0);

		if (PacketSize == -1)
		{
			continue;
		}

		std::string strTemp;
		strTemp.resize(PacketSize);
		strTemp.assign(strBuffer, PacketSize);

		std::cout << strTemp << std::endl;
	}
}
