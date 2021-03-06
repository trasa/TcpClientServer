#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>

unsigned long nonBlockingEnabled = 1;
unsigned long blockingEnabled = 0;

int serverNonBlockingWithThreads(void);
int serverNonBlockingWithSelect(void);

void printError(const char* msg)
{
	std::cout << "Error from " << msg << " " << WSAGetLastError() << std::endl;;
}


int main()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2), &wsadata) != 0)
	{
		printError("startup");
		return 1;
	}

	if (serverNonBlockingWithThreads() != 0)
	{
		return 1;
	}

	// if (serverNonBlockingWithSelect() != 0)
	// {
		// return 1;
	// }

	if (WSACleanup() != 0)
	{
		printError("cleanup");
		return 1;
	}
}

void handleClientThread(SOCKET clientSocket)
{
	const int RECV_BUF_LEN = 10000;
	char recvBuf[RECV_BUF_LEN];

	ZeroMemory(recvBuf, sizeof(char) * RECV_BUF_LEN);
	int recvBufferRemainingLength = RECV_BUF_LEN;
	char* recvBufActive = recvBuf;
	int recvLength = 0;
	int res = 0;

	// receive
	do
	{
		res = recv(clientSocket, recvBufActive, recvBufferRemainingLength, 0);
		if (res == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				printError("Error from recv from client : ");
				closesocket(clientSocket);
			}
		}
		else
		{
			recvLength += res;
			recvBufActive += res;
			recvBufferRemainingLength -= res;
		}
	} while (res != 0);

	std::cout << "Received: " << recvBuf << std::endl;

	// send
	res = send(clientSocket, recvBuf, recvLength, 0);
	if (res == SOCKET_ERROR)
	{
		printError("send to client failed");
		closesocket(clientSocket);
	}
	shutdown(clientSocket, SD_BOTH);
	closesocket(clientSocket);
}


int serverNonBlockingWithThreads(void)
{
	const SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		printError("create listen socket");
		return 1;
	}

	// address
	sockaddr_in serverAddress = {};
	memset(&serverAddress.sin_zero, 0, 8);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(12345);

	// bind
	if (bind(listenSock, (sockaddr*)(&serverAddress), sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		printError("bind fail");
		return 1;
	}

	// nonblocking
	ioctlsocket(listenSock, FIONBIO, &nonBlockingEnabled);

	// listen
	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		printError("listen fail");
		return 1;
	}
	std::cout << "I'm listening using threads..." << std::endl;

	while(true)
	{
		SOCKET clientSock = accept(listenSock, NULL, NULL);
		if (clientSock == INVALID_SOCKET)
		{
			int res = WSAGetLastError();
			if (res != WSAEWOULDBLOCK)
			{
				printError("Error in accept");
			}
			// std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		// this will be a blocking socket:
		ioctlsocket(clientSock, FIONBIO, &blockingEnabled);
		std::thread clientThread = std::thread(handleClientThread, clientSock);
		clientThread.detach();
	}
}

int serverNonBlockingWithSelect(void)
{
	const SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		printError("create listen socket");
		return 1;
	}

	// address
	sockaddr_in serverAddress = {};
	memset(&serverAddress.sin_zero, 0, 8);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(12345);

	// bind
	if (bind(listenSock, (sockaddr*)(&serverAddress), sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		printError("bind fail");
		return 1;
	}

	// nonblocking
	ioctlsocket(listenSock, FIONBIO, &nonBlockingEnabled);


	// listen
	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		printError("listen fail");
		return 1;
	}
	std::cout << "I'm listening using select..." << std::endl;

	constexpr int MAX_CLIENT_SOCKETS = 50;
	SOCKET clientSockets[MAX_CLIENT_SOCKETS];

	for (unsigned long long& clientSocket : clientSockets)
	{
		clientSocket = 0;
	}

	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);

		FD_SET(listenSock, &readfds);
		for(int i=0; i < MAX_CLIENT_SOCKETS ; i++)
		{
			SOCKET s = clientSockets[i];
			if (s > 0)
			{
				FD_SET(s, &readfds);
			}
		}

		int activity = select(0, &readfds, NULL, NULL, NULL);
		if (activity == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}
			printError("select fail");
			return 1;
		}
		if (FD_ISSET(listenSock, &readfds))
		{
			// incoming connection
			SOCKET newSock;
			struct sockaddr_in clientAddress;
			int addrLen = sizeof(sockaddr_in);

			if ((newSock = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddress), &addrLen)) < 0)
			{
				printError("accept");
				return 1;
			}
			if (ioctlsocket(newSock, FIONBIO, &nonBlockingEnabled) == SOCKET_ERROR)
			{
				printError("fail to make newSock nonblocking");
				return 1;
			}

			for (int i=0; i < MAX_CLIENT_SOCKETS; i++)
			{
				if (clientSockets[i] == 0)
				{
					clientSockets[i] = newSock;
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < MAX_CLIENT_SOCKETS; i++)
			{
				SOCKET s = clientSockets[i];
				if (FD_ISSET(s, &readfds))
				{
					// it's readable
					handleClientThread(s);
					FD_CLR(s, &readfds);
					clientSockets[i] = 0;
				}
			}
		}
	}
}


