#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <functional>
#include <atomic>
#define MAX_THREADS (20)	//The max number of threads that can be created.

/*Linux has <pthread.h> to support multithread while WIN32 has <thread>*/
#ifdef __linux__
#include <stdlib.h>
#include <pthread.h>
static pthread_mutex_t lock;		//lock--mutex used in linux
static pthread_cond_t task_ready;	//task_ready--condition variable in linux
#pragma comment(lib,"x86/pthreadVC2.lib")
using namespace std;
#elif _WIN32
#include <thread>
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#endif

#include "..\include\singleton.h"

#ifdef _MYDEBUG
#define DP (printf("%s:%u %s:%s:\t", __FILE__, __LINE__, __DATE__, __TIME__), printf) 
#else
#define DP(...)
#endif

/*
it's a function adptor for trans lambda function to CallBack_T function 
with the way that use MACRO.
!!!attention:
"_PV_IN", "_RET" can't be nullptr or NULL, and it could be "pvArg".

example:
			VOID* pvArg = (VOID*)&(this->m_vecServiceTbl[iIndex]);
			this->m_ThreadPool->addTask(TP_ADP_LBD_TASK(this, this->__toSendHB(pvArg), pvArg, 0), nullptr);
*/
#define TP_ADP_LBD_TASK(_CAT, _FUNC, _PV_IN, _RET) ([_CAT](void* _PV_IN)->int {_FUNC; return _RET; })
#define TP_ADP_MBR_TASK(_CLASS_NAME, _MEMBER_FUNC_NAME, _INSTANCE) (std::bind(&_CLASS_NAME::_MEMBER_FUNC, _INSTANCE, std::placeholders::_1))

//std::bind(&TestClass::ClassMember, instTest, std::placeholders::_1)
using namespace std;
/**
@class	ThreadPool
@brief	Functions include: 1.create a thread pool, 
						   2.add qTasks to the queue,
                           3.deal with 
						   in multithread environment, 
						   4.notice the main program qTasks have finished
                           5.destroy the thread pool and threads
@return	None
-----------------HOW TO USE IT
	1. Need HeadFile : #include <functional>

	2. assign Func
#Normal Function
	int testFunc(void* pvA){ return pvA; }

	std::function<int(void*)> fnProc = testFunc;

#lambda
	std::function<int(void*)> fnProc = [](void* pvA)->int{ return pvA; };

#functor
	class Functor
	{
	public:
		int operator()(void* pvA)
		{
			return pvA;
		}
	}
	Functor instFunctor;
	std::function<int(void*)> fnProc = instFunctor;

#Function of Class
	class TestClass
	{
	public:
		int ClassMember(void* pvA) { return pvA; }
		static int StaticMember(void* pvA) { return pvA; }
	};

	TestClass instTest;
	std::function<int(void*)> fnProc = std::bind(&TestClass::ClassMember, instTest, std::placeholders::_1);

	std::function<int(void*)> fnProc = TestClass::StaticMember;

	3. use "addTask"
	addTask(fnProc, pvA);
*/
class ThreadPool : public Singleton<ThreadPool>
{
public:
	/*define a struct for a task which include the pointer of working function and index*/
	typedef std::function<int(VOID*)> CallBack_T;
	typedef VOID* (*GENERAL_FUNC)(VOID*);
	typedef DWORD (WINAPI *THREAD_PROC_FUNC)(LPVOID pvParam);
	typedef DWORD (WINAPI ThreadPool::*MEMBER_PROC_FUNC)(LPVOID pvParam);
	typedef struct tagTask
	{
		CallBack_T pfnProc;
		VOID* pvArg;
		tagTask(CallBack_T pfnInput=nullptr, VOID* pvArgInput=nullptr):
			pfnProc(pfnInput),
			pvArg(pvArgInput)
		{
		}
	}Task, *PTask;
	typedef struct tagWorkerParam
	{
		ThreadPool *pinstThreadPool;
		DWORD dwThreadId;
		tagWorkerParam(ThreadPool *pThis=nullptr, DWORD dwThreadNo=0)
		{
			pinstThreadPool = pThis;
			dwThreadId = dwThreadNo;
		}
	}WorkerParam, *PWorkerParam;
	typedef union tagProc
	{
		THREAD_PROC_FUNC pfnThreadProc;
		MEMBER_PROC_FUNC pfnMemberProc;
		tagProc()
		{
			pfnThreadProc = nullptr;
		}
	}Proc, *PProc;

	ThreadPool();
	explicit ThreadPool(UINT);
	~ThreadPool();

	VOID addTask(CallBack_T pfnProcess, VOID* pvArgInput);//addTask--add task to the queue and notify a thread to work

#ifdef __linux__
	static void cleanup(pthread_mutex_t* lock);			//cleanup--release the mutex when the thread is stopped
#elif _WIN32
#endif

	friend class Singleton<ThreadPool>;	// friend class for adapt Abstract Singletion
private:
	UINT m_uThreadCount;	//m_thread_count in need

/*Ensuring portability of programs by macro definition*/
#ifdef __linux__
	static void* tp_thread_worker();					//tp_thread_worker--thread worker function in linux

	pthread_t* threads;			//pointer of threads
#elif _WIN32
	DWORD WINAPI __threadWorker(LPVOID pvParam);//__threadWorker--thread worker function in windows
	DWORD WINAPI __runWorker(MEMBER_PROC_FUNC pfnMemberProc);

	vector<HANDLE> *m_pThreadHandleTbl;
	DWORD m_dwThreadId;	//m_dwThreadId--thread have an id
	Proc m_unProc;
#endif
	mutex               m_mtxTask;	//sg_mtxTask--mutex used in WIN32
	condition_variable  m_condTaskReady; //sg_condTaskReady--condition variable in WIN32
	std::atomic<int>    m_iTaskNum;//sg_iTaskNum--the number of qTasks that haven't been dealt with
	std::atomic<bool>   m_bStoped;
	std::queue<PTask>   m_qTasks;//qTasks--the queue that qTasks are waiting for worker thread
};

#endif //THREAD_POOL_H_