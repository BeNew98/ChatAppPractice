#pragma once

#include "../SocketInfo/SocketInfo.h"

class Client : SocketInfo
{
public:
	Client();
	~Client();

	bool AccessServer();

	void SetName(std::string_view Name)
	{
		strAccessName = Name;
	}

	void SetAccessAddress(int Port = 108, std::string_view IP = "127.0.0.1");

	void SendServer(std::string_view Message);


protected:
	void ReceiveMessage();

private:
	std::thread RecvThread;
};

