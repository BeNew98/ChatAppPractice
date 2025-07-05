#include "Server.h"
#include <iostream>

Server::Server()
{
	strAccessName = "Server";
}

Server::~Server()
{
	DestroySocket();
	for (auto Iter : mapRecvThread)
	{
		Iter.second->join();
		delete Iter.second;
	}
}

void Server::AllowClient()
{
	while (true)
	{
		int Error = 0;

		SOCKADDR_IN ClientAddress = {};
		int AddressSize = sizeof(ClientAddress);

		//접속한 클라이언트 소켓
		//파라미터 -> 리슨 소켓, 클라이언트 주소, 클라이언트 주소 크기
		SOCKET ClientSocket = accept(Socket, (sockaddr*)&ClientAddress, &AddressSize);

		if (ClientSocket == INVALID_SOCKET)
		{
			std::cout << WSAGetLastError() << std::endl;
			return;
		}

		char strBuffer[PACKETSIZE] = {};

		int PacketSize = 0;
		//정보 수신
		//파라미터 ->클라이언트 소켓 , 버퍼, 버퍼사이즈 , 플래그
		PacketSize = recv(ClientSocket, strBuffer, PACKETSIZE, 0);

		std::cout << strBuffer << " Access Server" << std::endl;

		std::string ClientName;
		ClientName.resize(PacketSize);
		ClientName.assign(strBuffer, PacketSize);

		//접속 완료 메세지 전송
		//파라미터 -> 클라이언트 소켓, 버퍼, 버퍼사이즈, 플래그

		std::string strMessage = ClientName + " : Welcome Chat Server";
		Error = send(ClientSocket, strMessage.c_str(), static_cast<int>(strMessage.size()), 0);

		//0이면 연결 닫힘
		if (Error == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			std::cout << WSAGetLastError() << std::endl;
			return;
		}

		RegisterSocket(ClientSocket, ClientName);
	}
}

void Server::SetServerAddress(int Port)
{
	int Error = 0;

	//주소 정보
	Address.sin_family = AF_INET;
	Address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Address.sin_port = Port;

	//리슨 소켓에 주소 바인딩
	Error = bind(Socket, (sockaddr*)&Address, sizeof(Address));

	if (0 != Error)
	{
		std::cout << WSAGetLastError() << std::endl;
		return;
	}

	//SOMAXXCONN 최대 연결 승인수
	listen(Socket, SOMAXCONN);

	AllowThread = std::thread(std::bind(&Server::AllowClient,this));

	std::cout << "Chat Server Open"<<std::endl << std::endl;
}

void Server::RegisterSocket( SOCKET ClientSocket, std::string_view Name)
{
	mapUserList[ClientSocket] = Name.data();

	std::thread* NewThread = new std::thread(std::thread(std::bind(&Server::ReceiveMessage, this,ClientSocket)));
	mapRecvThread[Socket] = NewThread;
}

void Server::ReceiveMessage(SOCKET ClientSocket)
{
	while (true)
	{
		char strBuffer[PACKETSIZE] = {};

		int PacketSize = 0;
		PacketSize = recv(ClientSocket, strBuffer, PACKETSIZE, 0);

		if (PacketSize == -1)
		{
			continue;
		}

		std::string strTemp;
		strTemp = mapUserList[ClientSocket] + " : ";
		strTemp.append(strBuffer, PacketSize);

		queMessage.push(strTemp);
	}
}


void Server::SendAll()
{
	while (!queMessage.empty())
	{
		for (auto SendIter : mapUserList)
		{			
			SendMsg(queMessage.front(), SendIter.first);
		}

		std::cout << queMessage.front() << std::endl;
		queMessage.pop();
	}

}