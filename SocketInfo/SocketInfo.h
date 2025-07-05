#pragma once

#include "WinSock2.h"
#include <string>
#include <queue>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")
#include <functional>


#define PACKETSIZE 1024

class SocketInfo
{
public:
	SocketInfo();
	virtual ~SocketInfo();

protected:
	virtual void SendMsg(std::string_view Msg,SOCKET socket);
	virtual void DestroySocket();
	virtual void Init();

	std::string_view GetRecvPacket()
	{
		return strRecvPacket;
	};


	WSADATA wsaData = {};
	SOCKET Socket = INVALID_SOCKET;
	sockaddr_in Address = {};
	std::string strAccessName = "";
	std::queue<std::string> queMessage;
	
	std::string strRecvPacket;
	
private:
};