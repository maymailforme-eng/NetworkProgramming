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
#include <thread>
#include <unordered_map>
#include <mutex>


using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define MTU 1500

//Поля для хранения данных о клиенте
unordered_map<SOCKET, sockaddr_in> activeClients; //словарь подключенных клиентов
mutex activeClientsMutex; //токен доступа к общему ресурсу


//отображает всех подключенных пользователей к серверу
VOID ShowActiveClients();


//обработка сообщений клиента (вынесены пункты 7 и 8 в отдельную функцию которая будет открыта в отдельном потоке)
void ClientHandler(SOCKET client_socket)
{
	INT iRecivedBytes = 0;
	INT iSendBytes = 0;

	do
	{
		CHAR recv_buffer[MTU] = {};

		iRecivedBytes = recv(client_socket, recv_buffer, MTU, 0);

		if (iRecivedBytes > 0)
		{
			cout	<< "Recived " << iRecivedBytes
					<< " " << recv_buffer 
					<< endl;

			iSendBytes = send(client_socket, recv_buffer, iRecivedBytes, 0);

			if (iSendBytes == SOCKET_ERROR)
			{
				cout	<< "Send failed with error: "
						<< FormatLastError(WSAGetLastError()) 
						<< endl;
			}
		}
		else if (iRecivedBytes == 0)
		{
			cout << "Client disconnected" << endl;
		}
		else
		{
			cout	<< "recv failed with error: "
					<< FormatLastError(WSAGetLastError()) 
					<< endl;
		}

	} while (iRecivedBytes > 0);



	//удаляем данные о клиенте
	{
		lock_guard<mutex> lock(activeClientsMutex); // здесь поток остановиться и будет ждять выполнения освобждения токена управления 
		activeClients.erase(client_socket); // удаляе из словаря данные
	}

	ShowActiveClients(); //отображаем информацию 


	// Закрываем соединение именно с этим клиентом.
	// listen_socket не трогаем, потому что он нужен серверу для новых клиентов.
	shutdown(client_socket, SD_BOTH);
	closesocket(client_socket);
}






void main()
{
	setlocale(LC_ALL, "Russian");

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	///CONST
	CONST CHAR* portNumbet = "27015";

	cout << "************************* SERVER *************************\n" << endl;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//1) Инициализация WinSOCK;
	WSAData wsaData;
	INT iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WinSOCK init failed with code" << iResult;
		return;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//2) Параметры подключения
	addrinfo hints;
	addrinfo* target = nullptr;

	ZeroMemory(&hints, sizeof(hints));	//обнуляем экземпляр струтуры 
	hints.ai_family = AF_INET;			// Стэк протоколов TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;	// потоковый сокет TCP
	hints.ai_protocol = IPPROTO_TCP;	//Определяет протокл транспортного уровня
	hints.ai_flags = AI_PASSIVE;		//соединенние будет работать в режиме 'LISTENING'

	// NULL означает: сервер будет слушать локальный компьютер.
	// AI_PASSIVE + NULL обычно дает адрес 0.0.0.0,
	// то есть сервер будет принимать подключения на всех сетевых интерфейсах:
	// Wi-Fi, Ethernet, localhost.
	iResult = getaddrinfo(NULL, portNumbet, &hints, &target);

	if (iResult != 0)
	{
		cout	<< "getadressinfo() failed with code " 
				<< iResult << endl;

		WSACleanup();
		return;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//3) Создаем сокет:
	SOCKET listen_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);

	if (listen_socket == INVALID_SOCKET)
	{
		cout	<< "SOCKET creation failde wuth error: " 
				<< FormatLastError(WSAGetLastError()) << endl;

		freeaddrinfo(target);
		WSACleanup();
		return;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//4) Привязываем сокет к интерфейсу и порту
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);
	if (iResult != 0)
	{
		cout	<< "bind failed with error: " 
				<< FormatLastError(WSAGetLastError()) << endl;

		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	// После bind структура target больше не нужна.
	// Все необходимые данные уже переданы сокету.
	freeaddrinfo(target);
	target = nullptr;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//5) Запускаем прослушивание порта
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) // SOMAXCONN — стандартное максимальное значение очереди подключений, которое доступно системе
	{
		cout	<< "Listen failed wity error: " 
				<< FormatLastError(WSAGetLastError()) << endl;

		closesocket(listen_socket);
		WSACleanup();
		return;
	}


	cout	<< "Сервер слушает порт " << portNumbet << "..."
			<< endl << endl;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//6) Принмаем подключение от клиента
	// Теперь accept находится внутри бесконечного цикла.
	// Главный поток сервера постоянно ждет новых клиентов.
	// Для каждого клиента создается отдельный поток ClientHandler.


	while (true)
	{
		sockaddr_in client_addr;		//стандартная структура хранящая адрес, его тип, порт (данные подключения)
										// sockaddr_in хранит адрес в бинарном виде, поле .sin_addr = 11000000 10101000 00000001 00001111
		int client_addr_size = sizeof(client_addr);

		
		

		SOCKET client_socket = accept	// client_socket — отдельный сокет именно для конкретного клиента.
										// accept блокирует главный поток до тех пор, пока не появится новое входящее подключение.
		(
			listen_socket,				// listen_socket остается серверным сокетом.	
			(sockaddr*)&client_addr,	//записываем данные в о подключении клиента в переменную 
			&client_addr_size
		);


		if (client_socket == INVALID_SOCKET)
		{
			cout << "Accept failed wity error: " << FormatLastError(WSAGetLastError()) << endl;

			// Не завершаем сервер из-за одной ошибки accept.
			// Просто продолжаем ждать следующих клиентов.
			continue;
		}

		//Подключение прошло успешно выводим инофомацию о клинете

		//char client_ip[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN - максимальная длина адреса IPv4

		//inet_ntop(
		//	AF_INET,					 //флаг IPv4
		//	&client_addr.sin_addr,		 //сохранненый в бинарном виде адрес подключения
		//	client_ip,					 //буфер обмена
		//	sizeof(client_ip)
		//);

		//int client_port = ntohs(client_addr.sin_port);	//порт хранится в network byte order - конвертируем из
		//												// сетевого формата в в формат процессора 

		//cout	<< "Client connected " 
		//		<< ">> IP: " << client_ip 
		//		<< "\n\t\t >> Port: " << client_port 
		//		<< endl << endl;;



		//записываем данные о клиенте 
		{
			lock_guard<mutex> lock(activeClientsMutex); // проверка возможности доступа
			activeClients[client_socket] = client_addr;
		}

		
		ShowActiveClients(); //отображаем информацию 


		// Создаем отдельный поток для обслуживания этого клиента.
		// Главный поток сразу возвращается к accept()  и может принимать следующего клиента.
		std::thread clientThread(ClientHandler, client_socket);


		// detach означает: поток работает самостоятельно - не уничтожеться при выходе из области видимости, аналог UniTask - Forget() 
		clientThread.detach();
	}




	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//7) Получаем данные от клиента
	
	//INT iRecivedBytes = 0;
	//INT iSendBytes = 0;
	//do
	//{
	//	CHAR recv_buffer[MTU] = {};
	//	iRecivedBytes = recv(client_socket, recv_buffer, MTU, 0);
	//	if (iRecivedBytes > 0)
	//	{
	//		cout << "Recived " << iRecivedBytes << " " << recv_buffer << endl;
	//		iSendBytes = send(client_socket, recv_buffer, iRecivedBytes, 0);
	//		if (iSendBytes == SOCKET_ERROR)
	//		{
	//			cout << "Send failed with error: " << FormatLastError(WSAGetLastError()) << endl;
	//		}
	//		else
	//		{
	//			cout << iSendBytes << "Bytes send" << endl;
	//		}
	//	}
	//	else if(iRecivedBytes == 0)  // получаем ноль при корректном shutdown на стороне клиента. 
	//	{
	//		cout << "Connection closing..." << endl;
	//	}
	//	else
	//	{
	//		cout << "recived failed with error: " << FormatLastError(WSAGetLastError()) << endl;
	//	}
	//}
	//while (iRecivedBytes > 0);


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//8) Разрываем TCP - соединение
	//iResult = shutdown(client_socket, SD_BOTH);
	//if (iResult == SOCKET_ERROR)
	//{
	//	cout << "Shutdown failed with error " << FormatLastError(WSAGetLastError()) << endl;
	//}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////\
	//9) освобождение ресурсов WinSOCK 
	closesocket(listen_socket);
	WSACleanup();
}




//отображает всех подключенных пользователей к серверу
VOID ShowActiveClients()
{
	lock_guard<mutex> lock(activeClientsMutex);

	cout << "\n===== ACTIVE CLIENTS =====" << endl;

	if (activeClients.empty())
	{
		cout << "No active clients" << endl;
		return;
	}

	for (const pair<const SOCKET, sockaddr_in>& client : activeClients)
	{
		SOCKET socket = client.first;
		sockaddr_in address = client.second;

		char ip[INET_ADDRSTRLEN];

		inet_ntop(
			AF_INET,
			&address.sin_addr,
			ip,
			sizeof(ip)
		);

		cout
			<< "Socket: " << socket
			<< " IP: " << ip
			<< " Port: " << ntohs(address.sin_port)
			<< endl;
	}

	cout << "==========================\n" << endl;
}