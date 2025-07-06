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

		//접속한 클라이언트 소켓
		//파라미터 -> 리슨 소켓, 클라이언트 주소, 클라이언트 주소 크기
		SOCKET ClientSocket = accept(Socket, (sockaddr*)&ClientAddress, &AddressSize);

		if (ClientSocket == INVALID_SOCKET)
		{
			std::cout << WSAGetLastError() << std::endl;
			return;
		}


		char Buffer[PACKETSIZE] = {};

		int PacketSize = 0;
		//정보 수신
		//파라미터 ->클라이언트 소켓 , 버퍼, 버퍼사이즈 , 플래그
		PacketSize = recv(ClientSocket, Buffer, PACKETSIZE, 0);

		std::cout << Buffer << " Access Server" << std::endl;

		std::string ClientName;
		ClientName.resize(PacketSize);
		ClientName.assign(Buffer, PacketSize);

		mapUserList[ClientSocket] = ClientName;

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

		ThreadPool->RegisterIocp(ClientSocket);
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

	//SOMAXXCONN -> 최대 연결 승인수
	listen(Socket, SOMAXCONN);

	//스레드에 AllowClient를 계속 하게 요청
	AllowThread = std::thread([&]() {AllowClient(); });

	std::cout << "Chat Server Open" << std::endl << std::endl;
}

void Server::ReceiveMessage(SOCKET ClientSocket, std::string_view Message)
{
	std::string strTemp;
	strTemp = mapUserList[ClientSocket] + " : ";
	strTemp += Message;

	//임계영역 잠궈서 push
	std::unique_lock<std::mutex> UniqueLock(MsgQueueMutex);
	queMessage.push(strTemp);
	UniqueLock.unlock();
}

void Server::SendAll()
{
	//임계영역 잠궈서 pop
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

		//while문 돌거니까 다시 lock
		UniqueLock.lock();
	}

}