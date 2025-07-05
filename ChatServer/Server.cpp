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

		//������ Ŭ���̾�Ʈ ����
		//�Ķ���� -> ���� ����, Ŭ���̾�Ʈ �ּ�, Ŭ���̾�Ʈ �ּ� ũ��
		SOCKET ClientSocket = accept(Socket, (sockaddr*)&ClientAddress, &AddressSize);

		if (ClientSocket == INVALID_SOCKET)
		{
			std::cout << WSAGetLastError() << std::endl;
			return;
		}

		char strBuffer[PACKETSIZE] = {};

		int PacketSize = 0;
		//���� ����
		//�Ķ���� ->Ŭ���̾�Ʈ ���� , ����, ���ۻ����� , �÷���
		PacketSize = recv(ClientSocket, strBuffer, PACKETSIZE, 0);

		std::cout << strBuffer << " Access Server" << std::endl;

		std::string ClientName;
		ClientName.resize(PacketSize);
		ClientName.assign(strBuffer, PacketSize);

		//���� �Ϸ� �޼��� ����
		//�Ķ���� -> Ŭ���̾�Ʈ ����, ����, ���ۻ�����, �÷���

		std::string strMessage = ClientName + " : Welcome Chat Server";
		Error = send(ClientSocket, strMessage.c_str(), static_cast<int>(strMessage.size()), 0);

		//0�̸� ���� ����
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

	//�ּ� ����
	Address.sin_family = AF_INET;
	Address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Address.sin_port = Port;

	//���� ���Ͽ� �ּ� ���ε�
	Error = bind(Socket, (sockaddr*)&Address, sizeof(Address));

	if (0 != Error)
	{
		std::cout << WSAGetLastError() << std::endl;
		return;
	}

	//SOMAXXCONN �ִ� ���� ���μ�
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