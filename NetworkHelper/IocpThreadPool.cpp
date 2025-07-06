#include "IocpThreadPool.h"
#include <iostream>

IocpThreadPool::IocpThreadPool()
{
	Init();
}

IocpThreadPool::~IocpThreadPool()
{
}

void IocpThreadPool::Init()
{
	ThreadSize = std::thread::hardware_concurrency() * 2 + 1;

	IocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, ThreadSize);

	vecThreadPool.resize(ThreadSize);
	for (size_t i = 0; i < ThreadSize; i++)
	{
		vecThreadPool[i] = std::thread([&]() {RunThread();});
	}

}
void IocpThreadPool::RegisterIocp(SOCKET client)
{
	//iocp핸들로 포트만들어 연결
	auto Error = CreateIoCompletionPort((HANDLE)client, IocpHandle, (ULONG_PTR)client, 0);

	if (IocpHandle != Error)
	{
		std::cout << "IocpHandle Error";
		std::cout << GetLastError();
		return;
	}

	mapIoContext[client] = new IOContext();

	IOContext* Context = mapIoContext[client];


	//io컨텍스트를 통해 recv있었으면 반응오게 등록
	DWORD PacketSize = 0;
	DWORD flag = 0;

	ZeroMemory(&Context->Overlapped, sizeof(Context->Overlapped));
	Context->WsaBuf.buf = Context->Buffer;
	Context->WsaBuf.len = sizeof(Context->Buffer);
	int RecvError = WSARecv(client, &Context->WsaBuf, 1, &PacketSize, &flag, &Context->Overlapped, nullptr);

	if (RecvError == SOCKET_ERROR&& WSAGetLastError()!=WSA_IO_PENDING)
	{
		std::cout << "Recv Error : " << WSAGetLastError() << std::endl;
		closesocket(client);
		delete Context;
	}
}

void IocpThreadPool::RunThread()
{
	while (true)
	{
		DWORD PacketSize = 0;
		ULONG_PTR Key = 0;
		OVERLAPPED* Overlapped = nullptr;

		//처음엔 RegisterIocp에서 등록한 WSARecv에 반응이 왔으면 호출됨
		//그 후엔 이 함수 끝날때 다시 등록한것에 의해 호출
		//파라미터 -> iocp핸들, 받은 패킷 사이즈, 넣은키(소켓을 넣음), io상태
		BOOL Error = GetQueuedCompletionStatus(IocpHandle, &PacketSize, &Key, &Overlapped, INFINITE);
		if (!Error || Overlapped == nullptr)
		{
			std::cout << "QueueComplete Error: " << GetLastError() << std::endl;
			continue;
		}

		//Overlapped로 컨텍스트를 복구
		auto Context = CONTAINING_RECORD(Overlapped, IOContext, Overlapped);

		SOCKET ClientSocket = static_cast<SOCKET>(Key);

		if (PacketSize == 0)
		{
			closesocket(ClientSocket);
			delete Context;
			continue;
		}

		//받은데이터 넘겨줘서 queue에 넣게 하기
		std::string strTemp;
		strTemp.resize(PacketSize);
		strTemp.assign(Context->Buffer, PacketSize);

		Function(ClientSocket, strTemp);

		// 다음번에도 recv있었으면 반응하게 요청
		ZeroMemory(&Context->Overlapped, sizeof(OVERLAPPED));
		Context->WsaBuf.len = sizeof(Context->Buffer);
		DWORD Flags = 0;
		DWORD Bytes = 0;

		Error = WSARecv(ClientSocket, &Context->WsaBuf, 1, &Bytes, &Flags, &Context->Overlapped, nullptr);
		
		if (Error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "Recv Error : " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			delete Context;
		}
	}
}