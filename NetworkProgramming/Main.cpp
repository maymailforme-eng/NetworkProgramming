//CLIENT
 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#include "FormatLastError.h"

#include <iostream>;
#include <Windows.h>;
#include <WinSock2.h>;
#include <WS2tcpip.h>;
#include <iphlpapi.h>;

#pragma comment(lib, "WS2_32.lib") //встраиваем статическую библиотеку, для заголовка <WS2TCPIP.h>

using namespace std;

#define MTU 1500
void main()
{
	setlocale(LC_ALL, "");
	cout << "********************** CLIENT *************************\n" << endl;

	//1) Инициализация WinSOCK;
	WSAData wsaData;
	int iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WinSOCK init failed with code" << iResult;
		return;
	}

	//2) Определяем параметры подключения
	addrinfo hints;
	addrinfo* target;
	 
	ZeroMemory(&hints, sizeof(hints)); //обнуляем экземпляр струтуры 
	hints.ai_family = AF_INET; // Стэк протоколов TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; //Определяет протокл транспортного уровня
	iResult = getaddrinfo("127.0.0.1", "27015", &hints, &target);
	//"127.0.0.1" - IP самого компьютера.

	if (iResult != 0)
	{
		cout << "getadressinfo() failed with code " << iResult << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3) Создаем сокет:
	SOCKET connect_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);

	if (connect_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error:\t" << FormatLastError(WSAGetLastError())<< endl;
		WSACleanup();
		return;
	}



	//4) Подключаемся к узлу:
	iResult = connect(connect_socket, target->ai_addr, target->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to connect to server " << endl;
		closesocket(connect_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;

	}

	//freeaddrinfo(target);

	//5) Отправка данных:
	CHAR send_buffer[MTU] = "Hello Server";

	iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Send failed with error: " << FormatLastError(WSAGetLastError()) << endl;
		closesocket(connect_socket);
		WSACleanup();
		return;
	}
	
	//6) Получение данных:
	CHAR recv_buffer[MTU] = {};
	do
	{
		iResult = recv(connect_socket, recv_buffer, MTU, 0);
		if (iResult > 0)
		{
			cout << "Bytes recevived: " << iResult << "Message: " << recv_buffer << endl;
		}
		else if (iResult == 0) cout << "Connection closed" << endl; 
		else cout << "Receive failed with error " << FormatLastError(WSAGetLastError()) << endl;

	} while (iResult > 0);


	iResult = shutdown(connect_socket, SD_BOTH); //Закрывааем сокет на получение и отправку данных (разрыв TCP-соединения)

	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown failed with error " << FormatLastError(WSAGetLastError()) << endl;
	}

	
	
	//7) освобождение ресурсов WinSOCK 
	closesocket(connect_socket);
	WSACleanup();
}