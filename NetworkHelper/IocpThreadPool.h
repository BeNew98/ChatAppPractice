#pragma once

#include <winsock2.h>
#include <vector>
#include <functional>
#include <string>
#include <thread>
#include <map>

class IocpThreadPool
{
public:
	struct IOContext 
	{
		OVERLAPPED	Overlapped;
		WSABUF        WsaBuf;
		char          Buffer[1024];
	};

	IocpThreadPool();
	~IocpThreadPool();

	void RegisterIocp(SOCKET client);
	void SetFunction(std::function<void(SOCKET ClientSocket, std::string_view Message)> Func)
	{
		Function = Func;
	}
protected:
	void Init();
	void RunThread();

private:

	int ThreadSize = 0;
	std::vector<std::thread> vecThreadPool;
	std::function<void(SOCKET ClientSocket, std::string_view Message)> Function;
	std::map<SOCKET, IOContext*> mapIoContext;

	HANDLE IocpHandle;
};

