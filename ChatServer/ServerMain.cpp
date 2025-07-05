#include <iostream>
#include "Server.h"

int main()
{
	Server ServerStart;

	int Port;
	std::cout << "Set Port Number"<<std::endl;
	std::cout << "Port : ";
	std::cin >> Port;
	std::cout <<std::endl;
	ServerStart.SetServerAddress(Port);

	while (true)
	{
		ServerStart.SendAll();
	}	

	return 0;
}