#include "stdafx.h"
#include "ThreadPool.h"
#include <iostream>
#include <windows.h>

void Print(int i)
{
	printf("task %d start\n", i);
	Sleep(i * 1000);
	printf("task %d end\n", i);
	// fflush(stdout);
}

DWORD func1()
{
	Print(1);
	return 0;
}

DWORD func2()
{
	Print(2);
	return 0;
}

DWORD func3()
{
	Print(3);
	return 0;
}

DWORD func4()
{
	Print(4);
	return 0;
}

DWORD func5()
{
	Print(5);
	return 0;
}

DWORD errorFunc()
{
	printf("Error task!\n");
	throw exception();
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

	pool->AddTask(&func1);
	pool->AddTask(&func2);
	pool->AddTask(&func3);
	pool->AddTask(&func4);
	pool->AddTask(&func5);
	//pool->AddTask(&errorFunc);

	Sleep(2000);
	//pool->AddTask(&func1);

	::WaitForSingleObject(pool->hPoolThread, INFINITE);
	return 0;
}

