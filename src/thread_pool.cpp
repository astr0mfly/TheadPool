#include "thread_pool.h"

#include <stdio.h>
#include <stdarg.h>

#include <functional>
//Different definition in linux and WIN32
#ifdef __linux__
/** 
Function:	cleanup()
@brief      release the mutex when the thread is stopped
@param[in]  lock:mutex lock
@param[out] None
@return     None    
*/
void ThreadPool::cleanup(pthread_mutex_t* lock) {
	pthread_mutex_unlock(lock);
}
/** 
Function:	tp_thread_worker()
@brief      Every thread execute this function when working until being destroyed
@param[in]  None
@param[out] None
@return     None    
*/
void * ThreadPool::tp_thread_worker() {
	struct task *t;
	while (1) {
		//Triggering Mutex Locks when Preempting qTasks to Prevent Resource Occupation
		pthread_mutex_lock(&lock);	
		pthread_cleanup_push((void(*)(void*))cleanup, &lock);
		//the thread wait until the queue is not empty, wait can release the CPU resource
		while (qTasks->queue_IsEmpty()) {
			pthread_cond_wait(&task_ready, &lock);
			/*A condition wait is a cancellation point 
			a side-effect of acting upon a cancellation request 
			while in a condition wait is that the mutex is (in effect) re-acquired 
			before calling the first cancellation cleanup handler.*/
		}
		t = (struct task*)qTasks->queue_Dequeue();
		pthread_cleanup_pop(0);
		pthread_mutex_unlock(&lock);
		t->routine(t->arg);			//Deal with the task
		free(t);					//Free the pointer after finish the work
	}
	return NULL;
}
/** 
Function:	ThreadPool()
@brief      Constructor of ThreadPool. Initialize the member variables.
            Create a threadpool that contais severial thread
@param[in]  thread_count:the number of threads that needed to be created in advance
@param[out] None
@return     None    
*/
ThreadPool::ThreadPool(unsigned int m_thread_count) {
	unsigned int i;
	m_thread_count = m_thread_count;
	threads = (pthread_t*)malloc(sizeof(pthread_t)*m_thread_count);

	qTasks = new CQueue();

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&task_ready, NULL);

	for (i = 0; i<m_thread_count; i++) {
		//create threads and link the thread to worker function tp_thread_worker()
		pthread_create(threads + i, NULL, (void*(*)(void*))tp_thread_worker, this);
	}
}
/** 
Function:	addTask()
@brief      Add qTasks to the queue and wait to be dealt with. 
@param[in]  routine:the qTasks work in the function, arg:index of qTasks
@param[out] None
@return     None    
*/
void ThreadPool::addTask(void* (*routine)(void *arg), void *arg) {
	struct task *t;
	//Use mutex lock to ensure only one thread can be notified
	pthread_mutex_lock(&lock);
	t = (struct task*)qTasks->queue_Enqueue(sizeof(struct task));
	t->routine = routine;
	t->arg = arg;
	pthread_cond_signal(&task_ready);
	pthread_mutex_unlock(&lock);
}

/** 
Function:	~ThreadPool()
@brief      Destructor of ThreadPool.
@param[in]  None
@param[out] None
@return     None 
*/
ThreadPool::~ThreadPool() {
	unsigned int i;
	for (i = 0; i < m_thread_count; i++) {
		//destroy the threads
		pthread_cancel(threads[i]);
	}
	for (i = 0; i < m_thread_count; i++) {
		//thread wait other threads before being destroyed
		pthread_join(threads[i], NULL);
	}
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&task_ready);
	qTasks->~CQueue();
	free(threads);
}
#elif _WIN32

/** 
Function:	ThreadPool()
@brief      Constructor of ThreadPool. Initialize the member variables.
@param[in]  None
@param[out] None
@return     None    b
*/
ThreadPool::ThreadPool() :
	m_dwThreadId(NULL),
	m_iTaskNum(0),
	m_bStoped(false)
{
	m_pThreadHandleTbl = new std::vector<HANDLE>(MAX_THREADS);
	//create threads and link the thread to worker function __threadWorker
	__runWorker(&ThreadPool::__threadWorker);
}
/** 
Function:	ThreadPool()
@brief      Constructor of ThreadPool. Initialize the member variables.
@param[in]  None
@param[out] None
@return     None    b  
*/
ThreadPool::ThreadPool(UINT uThreadCount):
	m_dwThreadId(NULL),
	m_iTaskNum(0),
	m_bStoped(false)
{
	m_pThreadHandleTbl = new std::vector<HANDLE>(uThreadCount);
	DP("createPool and create thread\n");
	//create threads and link the thread to worker function __threadWorker
	__runWorker(&ThreadPool::__threadWorker);
}
/** 
Function:	__threadWorker()
@brief      Every thread execute this function when working until being destroyed
@param[in]  None
@param[out] None
@return     None    
*/
DWORD ThreadPool::__threadWorker(LPVOID pvParam) {
	PTask pTmpTask;
	while (!this->m_bStoped.load()) {

		std::unique_lock<std::mutex> uLocker(this->m_mtxTask);
		this->m_condTaskReady.wait(uLocker, [this] { return (this->m_bStoped.load() ||
															 !this->m_qTasks.empty()); });

		if (this->m_bStoped.load() && this->m_qTasks.empty()) {
			return 0;
		}

		pTmpTask = this->m_qTasks.front();
		this->m_qTasks.pop();
		uLocker.unlock();

		ULONGLONG Start = GetTickCount64();
		pTmpTask->pfnProc(pTmpTask->pvArg);
		ULONGLONG End = GetTickCount64();

		//some test print to ensure the thread pool work well
		DP("Finish time of task %d running in thread %d is %d\n ", (int)pTmpTask->pvArg, GetCurrentThreadId(), (int)(End - Start));
		DP("---Thread %d returns to wait.---\n", GetCurrentThreadId());
		DP("the number of left qTasks wait in the queue is %d\n", this->m_qTasks.size());
		this->m_iTaskNum--;

		delete pTmpTask;
	}
	return 0;
}

/** 
Function:	__runWorker()
@brief      run memberFunction and create thread
@param[in]  pfnMemberProc
@param[out] None
@return     None    
*/
DWORD ThreadPool::__runWorker(MEMBER_PROC_FUNC pfnMemberProc)
{
	/* Entry is guaranteed and no checks needs made */

	m_unProc.pfnMemberProc = pfnMemberProc;
	for (UINT uIndex = 0; uIndex < m_pThreadHandleTbl->size(); ++uIndex) {

		m_pThreadHandleTbl->at(uIndex) = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(m_unProc.pfnThreadProc),
			                                         (LPVOID)this, 0, &m_dwThreadId);

		DP("creat thread id: %d \n", m_dwThreadId);
	}

	return true;
}

/** 
Function:	addTask()
@brief      Add qTasks to the queue and wait to be dealt with.
@param[in]  routine:the qTasks work in the function, arg:index of qTasks
@param[out] None
@return     None    

*/
VOID ThreadPool::addTask(CallBack_T pfnProcess, VOID *pvArgInput)
{
	m_iTaskNum++;

	PTask pTmp = new Task(pfnProcess, pvArgInput);
	//Use mutex lock to ensure only one thread can be notified
	m_mtxTask.lock();
	m_qTasks.emplace(pTmp);
    m_condTaskReady.notify_one();
	m_mtxTask.unlock();
}

/** 
Function:	~ThreadPool()
@brief      Destructor of ThreadPool.
@param[in]  None
@param[out] None
@return     None 
*/
ThreadPool::~ThreadPool() {
	DP("Destroying...wait...\n");
	unsigned int i;

	m_bStoped.store(true);
	m_condTaskReady.notify_all();

	for (i = 0; i < m_pThreadHandleTbl->size(); i++) {
        if (i > 0) {
            WaitForSingleObject(m_pThreadHandleTbl->at(i - 1), INFINITE);
        }
		CloseHandle(m_pThreadHandleTbl->at(i));
	}
	delete m_pThreadHandleTbl;
}
#endif
