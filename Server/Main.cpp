//SERVER 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "FormatLastError.h"

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>


using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define MTU 1500

void main()
{
	setlocale(LC_ALL, "Russian");
	cout << "************************* SERVER *************************\n" << endl;

	//1) Инициализация WinSOCK;
	WSAData wsaData;
	INT iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WinSOCK init failed with code" << iResult;
		return;
	}

	//2) Параметры подключения
	addrinfo hints;
	addrinfo* target;

	ZeroMemory(&hints, sizeof(hints)); //обнуляем экземпляр струтуры 
	hints.ai_family = AF_INET; // Стэк протоколов TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; //Определяет протокл транспортного уровня
	hints.ai_flags = AI_PASSIVE; //соединенние будет работать в режиме 'LISTENING'

	iResult = getaddrinfo(NULL, "27015", &hints, &target);

	if (iResult != 0)
	{
		cout << "getadressinfo() failed with code " << iResult << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}


	//3) Создаем сокет:
	SOCKET listen_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);

	//listen_socket = INVALID_SOCKET; //проверка 
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failde wuth error: " << FormatLastError(WSAGetLastError()) << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//4) Привязываем сокет к интерфейсу и порту
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);
	if (iResult != 0)
	{
		cout << "bind failed with error: " << FormatLastError(WSAGetLastError()) << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//5) Запускаем прослушивание порта
	if (listen(listen_socket, 1) == SOCKET_ERROR) // 1 - Максимальное количество одновременно подключенных клиентов.
	{
		cout << "Listen failed wity error: " << FormatLastError(WSAGetLastError()) << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//6) Принмаем подключение от клиента
	SOCKET client_socket = accept(listen_socket, NULL, NULL); //для каждого клиента создаеться свое подключение
	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept failed wity error: " << FormatLastError(WSAGetLastError()) << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//7) Получаем данные от клиента
	CHAR recv_buffer[MTU] = {};
	INT iRecivedBytes = 0;
	INT iSendBytes = 0;
	do
	{
		iRecivedBytes = recv(client_socket, recv_buffer, MTU, 0);
		if (iRecivedBytes > 0)
		{
			cout << "Recived " << iRecivedBytes << " " << recv_buffer << endl;
			iSendBytes = send(client_socket, recv_buffer, iRecivedBytes, 0);
			if (iSendBytes == SOCKET_ERROR)
			{
				cout << "Send failed with error: " << FormatLastError(WSAGetLastError()) << endl;
			}
			else
			{
				cout << iSendBytes << "Bytes send" << endl;
			}
		}
		else if(iRecivedBytes == 0)  // получаем ноль при корректном shutdown на стороне клиента. 
		{
			cout << "Connection closing..." << endl;
		}
		else
		{
			cout << "recived failed with error: " << FormatLastError(WSAGetLastError()) << endl;
		}
	}
	while (iRecivedBytes > 0);


	//8) Разрываем TCP - соединение
	iResult = shutdown(client_socket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown failed with error " << FormatLastError(WSAGetLastError()) << endl;
	}



	//?) освобождение ресурсов WinSOCK 
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();
}