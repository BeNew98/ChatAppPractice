#pragma once

#include "../SocketInfo/SocketInfo.h"
#include "map"


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
	void RegisterSocket(SOCKET ClientSocket, std::string_view Name);

	void ReceiveMessage(SOCKET ClientSocket);

private:
	bool bServerClose = false;
	std::map<SOCKET,std::string> mapUserList;
	std::map<SOCKET,std::thread*> mapRecvThread;

	std::thread AllowThread;
	std::queue<std::string> queMessage;
};

