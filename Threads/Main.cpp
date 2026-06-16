#include<iostream>
#include<Windows.h>
#include<thread>

using namespace std;

bool finish = false;

VOID Function()
{
	while (!finish)
	{
		cout << "Hello Threads from " << GetCurrentThreadId() << endl;
		//system("PAUSE");
	}
}



struct Point
{

	Point(INT X, INT Y) : x (X), y (Y){}

	INT x;
	INT y;




};


VOID Collision(Point &point)
{
	while (point.x != point.y)
	{
		cout << "X = " << point.x++ << "\tY = " << point.y-- << endl;
		Sleep(10);
	}
}


VOID Decrement(int i)
{
	while (i) cout << i-- << "\t";
}


VOID Plus()
{
	while (!finish)
	{
		cout << "+ ";
	}

}

VOID Minus()
{
	while (!finish)
	{
		cout << "- ";
	}

}


//#define WINDOWS_THREADS_1
//#define WINDOWS_THREADS_2

void main()
{
	setlocale(LC_ALL, "");

#ifdef WINDOWS_THREADS_1
	DWORD dwID = 0;
	HANDLE hThread = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Function,
		NULL,
		NULL,
		&dwID
	);

	cin.get();
	finish = true;


	cout << "Thread ID from main(): " << dwID << endl;
	WaitForSingleObject(hThread, INFINITE);
#endif // WINDOWS_THREADS_1


#ifndef WINDOWS_THREADS_2
	Point A(0, 100);
	int i = 1000;

	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread
	(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)Decrement,
		(LPVOID)i,
		NULL,
		&dwThreadID
	);


	WaitForSingleObject(hThread, INFINITE);
#endif // !WINDOWS_THREADS_2


	thread plus_thread = thread(Plus);
	thread minus_thread = thread(Minus);


	cin.get();
	finish = true;


	if (plus_thread.joinable()) plus_thread.join();
	if (minus_thread.joinable()) minus_thread.join();


}