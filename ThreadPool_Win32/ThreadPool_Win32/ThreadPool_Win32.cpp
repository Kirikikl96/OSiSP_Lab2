#include "stdafx.h"
#include "ThreadPool.h"
#include <iostream>
#include <windows.h>

void PrintResult(int i)
{
	printf("Task %d start\n", i);
	Sleep(i * 1000);
	printf("Task %d end\n", i);
}

DWORD Task1()
{
	PrintResult(1);
	return 0;
}

DWORD Task2()
{
	PrintResult(2);
	return 0;
}

DWORD Task3()
{
	PrintResult(3);
	return 0;
}

DWORD Task4()
{
	PrintResult(4);
	return 0;
}

DWORD Task5()
{
	PrintResult(5);
	return 0;
}

DWORD Task6()
{
	PrintResult(6);
	return 0;
}

DWORD Task7()
{
	PrintResult(7);
	return 0;
}

int main()
{
	int poolSize = 0, maxPoolSize = 0;
	cout << "Thread pool size: ";
	cin >> poolSize;
	cout << "Maximum thread pool size: ";
	cin >> maxPoolSize;

	ThreadPool *pool = new ThreadPool(poolSize, maxPoolSize);

	pool->AddTask(&Task1);
	pool->AddTask(&Task2);
	pool->AddTask(&Task3);
	pool->AddTask(&Task4);
	pool->AddTask(&Task5);
	pool->AddTask(&Task6);
	pool->AddTask(&Task7);
	Sleep(2000);
	::WaitForSingleObject(pool->hPoolThread, INFINITE);
	return 0;
}

