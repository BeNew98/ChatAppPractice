#include "Server.h"
#include <iostream>
#include "../NetworkHelper/IocpThreadPool.h"

Server::Server()
{
	strAccessName = "Server";

	ThreadPool = new IocpThreadPool();
	ThreadPool->SetFunction([this](SOCKET Client, std::string_view Msg) {ReceiveMessage(Client, Msg); });
}

Server::~Server()
{
	DestroySocket();

	delete ThreadPool;
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


		char Buffer[PACKETSIZE] = {};

		int PacketSize = 0;
		//���� ����
		//�Ķ���� ->Ŭ���̾�Ʈ ���� , ����, ���ۻ����� , �÷���
		PacketSize = recv(ClientSocket, Buffer, PACKETSIZE, 0);

		std::cout << Buffer << " Access Server" << std::endl;

		std::string ClientName;
		ClientName.resize(PacketSize);
		ClientName.assign(Buffer, PacketSize);

		mapUserList[ClientSocket] = ClientName;

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

		ThreadPool->RegisterIocp(ClientSocket);
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

	//SOMAXXCONN -> �ִ� ���� ���μ�
	listen(Socket, SOMAXCONN);

	//�����忡 AllowClient�� ��� �ϰ� ��û
	AllowThread = std::thread([&]() {AllowClient(); });

	std::cout << "Chat Server Open" << std::endl << std::endl;
}

void Server::ReceiveMessage(SOCKET ClientSocket, std::string_view Message)
{
	std::string strTemp;
	strTemp = mapUserList[ClientSocket] + " : ";
	strTemp += Message;

	//�Ӱ迵�� ��ż� push
	std::unique_lock<std::mutex> UniqueLock(MsgQueueMutex);
	queMessage.push(strTemp);
	UniqueLock.unlock();
}

void Server::SendAll()
{
	//�Ӱ迵�� ��ż� pop
	std::unique_lock<std::mutex> UniqueLock(MsgQueueMutex);
	while (!queMessage.empty())
	{
		std::string strSendMsg = queMessage.front();
		queMessage.pop();

		UniqueLock.unlock();

		for (auto SendIter : mapUserList)
		{
			SendMsg(strSendMsg, SendIter.first);
		}

		std::cout << strSendMsg << std::endl;

		//while�� ���Ŵϱ� �ٽ� lock
		UniqueLock.lock();
	}

}