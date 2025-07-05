#include <iostream>
#include "Client.h"

int main()
{
	Client ClientStart;

	std::cout << "Set Client Name" << std::endl;
	std::cout << "Name : ";

	std::string strTemp;
	std::cin >> strTemp;

	ClientStart.SetName(strTemp);
	std::cout << std::endl;


	while (true)
	{
		std::cout << "Set Port Number" << std::endl;
		std::cout << "Port : ";

		int Port;
		std::cin >> Port;
		std::cout << std::endl;
		ClientStart.SetAccessAddress(Port);
		if (ClientStart.AccessServer())
		{
			break;
		}
	}

	while (true)
	{
		std::string Temp;
		std::getline(std::cin ,Temp);
		ClientStart.SendServer(Temp);
	}

	return 0;
}