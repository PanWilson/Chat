#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <list>
#include <iterator>
#include <minwinbase.h>
#include <minwinbase.h>

#pragma comment (lib, "ws2_32.lib")

std::list<SOCKET> clients;
std::list<std::thread> threads;

void connection(SOCKET socket, bool *running)
{
	SOCKET SClient = socket;
	char buf[4096];

	send(SClient, "Welcom to chat",14 , 0);

	while (*running)
	{
		ZeroMemory(buf, 4096);

		int bytesRecived = recv(SClient, buf, 4096, 0);
		if (bytesRecived == SOCKET_ERROR)
		{
			break;
		}
		if (bytesRecived == 0)
		{
			std::cout << "client disconnected\n";
			break;
		}

		std::cout << std::string(buf, 0, bytesRecived) << std::endl;

		for (std::list<SOCKET>::iterator it = clients.begin(); it != clients.end(); it++) {
			if(*it!=SClient)send(*it, buf, bytesRecived + 1, 0);
		}

	}
	// Close winsock
	clients.remove(SClient);
	closesocket(SClient);
}

void listening(SOCKET SListen, bool *running)
{
	while (*running) {
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET SClient = accept(SListen, (sockaddr*)&client, &clientSize);

		char host[NI_MAXHOST];
		char service[NI_MAXSERV];

		ZeroMemory(host, NI_MAXHOST);
		ZeroMemory(service, NI_MAXSERV);

		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			std::cout << host << " connected\nPort: " << service << std::endl;
		}
		else
		{
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			std::cout << host << " connected\nPort: " << ntohs(client.sin_port) << std::endl;
		}
		clients.push_front(SClient);
		threads.push_front(std::thread(connection, SClient, running));
	}
}


void main()
{

														// Init winsock

	WSADATA WSdata;
	WORD ver = MAKEWORD(2, 2);

	int wsok = WSAStartup(ver, &WSdata);
	if(wsok != 0)
	{
		return;
	}

														// Create socket

	SOCKET SListen = socket(AF_INET, SOCK_STREAM, 0);
	if(SListen == INVALID_SOCKET)
	{
		return;
	}

														// Give IP and port a socket

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(SListen, (sockaddr*)&hint, sizeof(hint));

														// Set socket to listen

	listen(SListen, SOMAXCONN);

														// Wait for connection
	bool running = true;

	std::thread TListen(listening, SListen, &running);


	std::string CInput;
	std::string name = "(Serwer)";

	do
	{
		std::getline(std::cin, CInput);

		if (CInput.size() > 0 && CInput != "/E") {

			for (std::list<SOCKET>::iterator it = clients.begin(); it != clients.end(); it++) {
				send(*it,(name+CInput).c_str(), (name + CInput).size() + 1, 0);
			}
		}

	} while (CInput != "/E");

														// Close listen socket
	running = false;

	closesocket(SListen);

	for (std::list<SOCKET>::iterator it = clients.begin(); it != clients.end(); it++) {
		closesocket(*it);
	}

	TListen.join();

	for (std::list<std::thread>::iterator it = threads.begin(); it != threads.end(); it++) {
		it->join();
	}
	
														// Clean winsock
	WSACleanup();
}