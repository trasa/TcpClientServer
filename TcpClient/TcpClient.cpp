#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>


void printError(const char* msg)
{
	std::cout << "Error from " << msg << " " << WSAGetLastError() << std::endl;;
}

int main()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		printError("startup");
		return 1;
	}
	const SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printError("create socket");
		return 1;
	}

	// address
	sockaddr_in serverAddress = {};
	memset(&serverAddress.sin_zero, 0, 8);
	serverAddress.sin_family = AF_INET;
	if (inet_pton(AF_INET, "10.0.0.185", &serverAddress.sin_addr) != 1) {
		printError("inet pton");
	}
	serverAddress.sin_port = htons(12345);

	std::cout << "Connecting...";
	if (connect(sock, (sockaddr*)&serverAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		printError("connect");
		return 1;
	}
	std::cout << "Connected!" << std::endl;

	// send something
	const char* msg = "Hello, World";
	if (send(sock, msg, static_cast<int>(strlen(msg)), 0) == SOCKET_ERROR)
	{
		printError("send");
		return 1;
	}
	std::cout << "sent!" << std::endl;
	shutdown(sock, SD_SEND);

	// blocking receive
	int buflen = 1000;
	char* buf = new char[buflen];
	std::string result = "";
	memset(buf, 0, buflen);
	while(true)
	{
		int bytes = recv(sock, buf, buflen, 0);
		if (bytes == SOCKET_ERROR)
		{
			printError("receive");
			return 1;
		}
		else if (bytes == 0)
		{
			break;
		}
		else
		{
			result.append(buf);
		}
	}
	std::cout << "received: " << result << std::endl;
	closesocket(sock);
	delete[] buf;
	if (WSACleanup() != 0)
	{
		printError("cleanup");
		return 1;
	}
}
