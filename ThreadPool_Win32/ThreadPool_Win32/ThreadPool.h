#pragma once

#include <windows.h>
#include <vector>
#include <queue>

using namespace std;
typedef DWORD(*fnTask)();

struct ThreadInPool
{
	DWORD id;
	HANDLE handle;
	HANDLE hEvent;
	int error;
	int flag;
	fnTask task;
};

class ThreadPool
{
public:
	vector<ThreadInPool> workerThreads;
	queue<fnTask> tasks;
	int poolSize = 0;
	int maxPoolSize = 0;
	//int m_freeThread = 0;
	HANDLE hPoolThread;

	CRITICAL_SECTION csWorkWithTaskQueue;
	CRITICAL_SECTION csWriteToLog;

	ThreadPool(int, int);

	void AddThreadInPool();
	void InitializeThreadPool();
	void AddTask(DWORD(*task)());
};

