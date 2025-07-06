#pragma once

#include "../NetworkHelper/SocketInfo.h"
#include "map"
#include <mutex>


class Server : SocketInfo
{
public:
	Server();
	~Server();

	bool IsServerClose()
	{
		return bServerClose;
	}
	void SetServerAddress(int Port = 108);
	void SendAll();

protected:
	void AllowClient();
	void ReceiveMessage(SOCKET ClientSocket, std::string_view Message);

private:
	bool bServerClose = false;
	std::map<SOCKET, std::string> mapUserList;

	std::mutex MsgQueueMutex;
	std::thread AllowThread;
	std::queue<std::string> queMessage;

	class IocpThreadPool* ThreadPool;
	
};

