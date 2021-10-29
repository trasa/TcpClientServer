#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

int main()
{

	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		return 1;
	}



	// our destination
	// const char* host = "www.digipen.edu";
	const char* host = "4.2.2.2";

	sockaddr_in destAddr;
	memset(&destAddr.sin_zero, 0, 8);
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(80); // port 80


	int res = inet_pton(AF_INET, host, &destAddr.sin_addr);


	// is this an IP address?
	if (res != 1)
	{
		// nope, doesn't appear to be an IP address
		// let's see if we can parse it
		ADDRINFOA hint;
		PADDRINFOA pAddr;
		memset(&hint, 0, sizeof(ADDRINFOA));
		hint.ai_family = AF_INET;

		res = getaddrinfo(host, NULL, &hint, &pAddr);

		if ((res != 0) || (pAddr == NULL))
		{
			std::cout << "Unable to parse " << host << " as either a hostname (getaddrinfo) or IP address (inet_pton).  " << WSAGetLastError() << std::endl;
			return 1;
		}
		else
		{
			destAddr.sin_addr = ((SOCKADDR_IN*)pAddr->ai_addr)->sin_addr;
			freeaddrinfo(pAddr);
		}
	}
	char result[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &destAddr.sin_addr, result, INET_ADDRSTRLEN);
	std::cout << "Resulting IP Address is " << result << std::endl;


	if (WSACleanup() != 0)
	{
		return 1;
	}
}
