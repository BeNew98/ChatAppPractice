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
	//iocp�ڵ�� ��Ʈ����� ����
	auto Error = CreateIoCompletionPort((HANDLE)client, IocpHandle, (ULONG_PTR)client, 0);

	if (IocpHandle != Error)
	{
		std::cout << "IocpHandle Error";
		std::cout << GetLastError();
		return;
	}

	mapIoContext[client] = new IOContext();

	IOContext* Context = mapIoContext[client];


	//io���ؽ�Ʈ�� ���� recv�־����� �������� ���
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

		//ó���� RegisterIocp���� ����� WSARecv�� ������ ������ ȣ���
		//�� �Ŀ� �� �Լ� ������ �ٽ� ����ѰͿ� ���� ȣ��
		//�Ķ���� -> iocp�ڵ�, ���� ��Ŷ ������, ����Ű(������ ����), io����
		BOOL Error = GetQueuedCompletionStatus(IocpHandle, &PacketSize, &Key, &Overlapped, INFINITE);
		if (!Error || Overlapped == nullptr)
		{
			std::cout << "QueueComplete Error: " << GetLastError() << std::endl;
			continue;
		}

		//Overlapped�� ���ؽ�Ʈ�� ����
		auto Context = CONTAINING_RECORD(Overlapped, IOContext, Overlapped);

		SOCKET ClientSocket = static_cast<SOCKET>(Key);

		if (PacketSize == 0)
		{
			closesocket(ClientSocket);
			delete Context;
			continue;
		}

		//���������� �Ѱ��༭ queue�� �ְ� �ϱ�
		std::string strTemp;
		strTemp.resize(PacketSize);
		strTemp.assign(Context->Buffer, PacketSize);

		Function(ClientSocket, strTemp);

		// ���������� recv�־����� �����ϰ� ��û
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