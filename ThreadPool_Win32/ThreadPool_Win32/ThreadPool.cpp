#include "stdafx.h"
#include "ThreadPool.h"
#include "easylogging++.h"
#include <algorithm>

INITIALIZE_EASYLOGGINGPP
#define min(a, b) a < b ? a : b

#define FLAG_INIT 1
#define FLAG_WORK 2
#define FLAG_DONE 3

DWORD WINAPI ThreadPullManager(LPVOID);
DWORD WINAPI ThreadWork(LPVOID);

ThreadPool::ThreadPool(int N, int M)
{
	maxPoolSize = M;
	poolSize = min(N, M);

	// Create log file
	remove("ThreadLogs.log");
	el::Configurations conf("logconfig.conf");
	el::Loggers::reconfigureAllLoggers(conf);
	
	::InitializeCriticalSection(&csWorkWithTaskQueue);
	::InitializeCriticalSection(&csWriteToLog);


	LOG(INFO) << "Start the application.";

	DWORD id;
	// Create thread pool process
	hPoolThread = ::CreateThread(NULL, 0, ThreadPullManager, this, 0, &id);
}

void ThreadPool::AddTask(DWORD(*task)())
{
	::EnterCriticalSection(&csWorkWithTaskQueue);
		tasks.push(task);
		::EnterCriticalSection(&csWriteToLog);
			LOG(INFO) << "Task " << task << " is added in queue";
		::LeaveCriticalSection(&csWriteToLog);
	::LeaveCriticalSection(&csWorkWithTaskQueue);
}

DWORD WINAPI ThreadWork(LPVOID lpParameter)
{
	ThreadPool* tp = (ThreadPool*)(lpParameter);
	ThreadInPool* tip = NULL;

	DWORD id = ::GetCurrentThreadId();
	if (tp != NULL)
	{
		for (int i = 0; i < tp->workerThreads.size(); i++)
		{
			if (tp->workerThreads[i].id == id)
			{
				tip = &(tp->workerThreads[i]);
				break;
			}
		}

		if (tip != NULL)
		{
			while (1)
			{
				DWORD dwResult = ::WaitForSingleObject(tip->hEvent, INFINITE);

				if (dwResult == WAIT_OBJECT_0)
				{
					tip->flag = FLAG_WORK;
					tip->error = 0;

					DWORD _id;
					void* _task;

					try
					{

						if (tip->task != NULL)
						{
							::EnterCriticalSection(&tp->csWriteToLog);
							LOG(INFO) << "Task " << tip->task
								<< ", start in thread ¹" << tip->id << endl;
							::LeaveCriticalSection(&tp->csWriteToLog);

							_id = tip->id;
							_task = tip->task;

							// Start the task!
							tip->task();

							::EnterCriticalSection(&tp->csWriteToLog);
							LOG(INFO)
								<< "Thread ¹" << _id
								<< ", task " << _task
								<< " is done! ";
							::LeaveCriticalSection(&tp->csWriteToLog);
						}
					}
					catch (exception)
					{
						tip->error = 1;

						::EnterCriticalSection(&tp->csWriteToLog);
						LOG(INFO)
							<< "Thread ¹" << _id
							<< " has ERROR. Thread is stopped.";
						::LeaveCriticalSection(&tp->csWriteToLog);
					}

					tip->flag = FLAG_DONE;
				}

				::ResetEvent(tip->hEvent);
			}
		}
	}

	return 0;
}

void ThreadPool::AddThreadInPool()
{
	ThreadInPool _thread;
	
	// Add thread info
	workerThreads.push_back(_thread);

	// Create sinchronization object
	workerThreads.back().hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (workerThreads.back().hEvent == NULL)
	{
		workerThreads.pop_back();
		
		::EnterCriticalSection(&csWriteToLog);
		LOG(INFO) << "ERROR! CreateEvent failed!";
		::LeaveCriticalSection(&csWriteToLog);

	}
	else
	{
		// Create work thread
		workerThreads.back().error = 0;
		workerThreads.back().task = NULL;
		workerThreads.back().flag = FLAG_INIT;
		workerThreads.back().handle = ::CreateThread(NULL, 0, ThreadWork, this, 0, &workerThreads.back().id);

		if (workerThreads.back().handle != NULL)
		{
			::EnterCriticalSection(&csWriteToLog);
			LOG(INFO) << "Thread ¹" << workerThreads.back().id << " is added in thread pool";
			::LeaveCriticalSection(&csWriteToLog);
		}
		else
		{
			workerThreads.pop_back();
		}
	}
}

void ThreadPool::InitializeThreadPool()
{
	workerThreads.reserve(poolSize);
	for (int i = 0; i < poolSize; i++)
	{
		AddThreadInPool();
	}
}

BOOL threadInProgress(HANDLE hEvent)
{
	return ::WaitForSingleObject(
		hEvent, WAIT_OBJECT_0) == WAIT_OBJECT_0;
}

DWORD WINAPI ThreadPullManager(LPVOID lpParameter)
{
	ThreadPool * threadPool = (ThreadPool*)(lpParameter);
	
	threadPool->InitializeThreadPool();
	while (true)
	{
		Sleep(10);

		::EnterCriticalSection(&threadPool->csWorkWithTaskQueue);

		// If we heave new task
		if (!threadPool->tasks.empty())
		{		
			bool createNewThread = true;
			
			// Find complited thread
			for (int i = 0; i < threadPool->workerThreads.size(); i++)
			{
				if (threadPool->workerThreads[i].flag != FLAG_WORK)
				{
					createNewThread = false;
					threadPool->workerThreads[i].flag = FLAG_INIT;
					threadPool->workerThreads[i].task = threadPool->tasks.front();
					threadPool->tasks.pop();
									
					// Do the task
					if (!::SetEvent(threadPool->workerThreads[i].hEvent))
					{
						::EnterCriticalSection(&threadPool->csWriteToLog);
						LOG(INFO) << "Error! Can't start suspended thread ¹" 
							<< threadPool->workerThreads[i].id << endl;
						::LeaveCriticalSection(&threadPool->csWriteToLog);
					}

					break;
				}
			}

			// Create new thread
			if (createNewThread)
			{
				ThreadInPool _thread;

				// Add thread info
				threadPool->workerThreads.push_back(_thread);

				threadPool->workerThreads.back().error = 0;
				threadPool->workerThreads.back().flag = FLAG_INIT;
				threadPool->workerThreads.back().hEvent = 
						::CreateEvent(NULL, TRUE, FALSE, NULL);

				if (threadPool->workerThreads.back().hEvent == NULL)
				{
					threadPool->workerThreads.pop_back();
					::EnterCriticalSection(&threadPool->csWriteToLog);
					LOG(INFO) << "ERROR! CreateEvent failed!";
					::LeaveCriticalSection(&threadPool->csWriteToLog);
				}
				else
				{
					threadPool->workerThreads.back().task = threadPool->tasks.front();
					threadPool->tasks.pop();

					threadPool->workerThreads.back().handle = ::CreateThread(NULL, 0,
						ThreadWork, (void*)threadPool,
						0, &(threadPool->workerThreads.back().id));

					if (threadPool->workerThreads.back().handle != NULL)
					{
						if (threadPool->workerThreads.size() > threadPool->maxPoolSize)
						{
							::EnterCriticalSection(&threadPool->csWriteToLog);
								LOG(INFO) << "Overflow thread limit!!!";
								LOG(INFO) << "Thread ¹" << threadPool->workerThreads.back().id << " is added in thread pool";
							::LeaveCriticalSection(&threadPool->csWriteToLog);
						}

						::SetEvent(threadPool->workerThreads.back().hEvent);
					}
					else
					{
						::CloseHandle(threadPool->workerThreads.back().hEvent);
						threadPool->workerThreads.pop_back();
						::EnterCriticalSection(&threadPool->csWriteToLog);
						LOG(INFO) << "Error! Can't create thread." << endl;
						::LeaveCriticalSection(&threadPool->csWriteToLog);
					}
				}
			}
		}
		::LeaveCriticalSection(&threadPool->csWorkWithTaskQueue);
	}
}

