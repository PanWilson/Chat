#include <iostream>
#include <thread>
#include <string>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


void recive(SOCKET sock, bool *running)
{

	char buf[4096];
	SOCKET SClient = sock;
	while (*running)
	{
		ZeroMemory(buf, 4096);
		int bytesRecived = recv(SClient, buf, 4096, 0);

		if (bytesRecived > 0)
		{
			std::cout << std::string(buf, 0, bytesRecived) << std::endl;
		}
	}
	std::cout << "elo";
}

void main()
{
	std::string ipAddress = "127.0.0.1";
	int port = 54000;


														// Init winsock

	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		std::cerr << "Can't start Winsock, Err #" << wsResult << std::endl;
		return;
	}

														// Create socket

	SOCKET SClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SClient == INVALID_SOCKET)
	{
		std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}
														// Give IP and port a socket
	std::cout << "(def 127.0.0.1)IP address:\n";
	std::cin >> ipAddress;
	std::cout << "(def 54000)Port:\n";
	std::cin >> port;


	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

														// Connect to server

	int connResult = connect(SClient, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		std::cout << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
		closesocket(SClient);
		WSACleanup();
		return;
	}

	bool running = true;

	std::thread TRecive(recive, SClient, &running);

														// Answer connection
	std::cout << "/E to Exit\n";

	std::string CInput;
	getchar();

	do
	{
		std::getline(std::cin, CInput);

		if (CInput.size() > 0 && CInput != "/E") {

			int sendResult = send(SClient, CInput.c_str(), CInput.size() + 1, 0);

		}

	} while (CInput!="/E");

														// Close winsock

	running = false;
	closesocket(SClient);

	TRecive.join();

	WSACleanup();
}
