#ifndef SINGLETON_H_
#define SINGLETON_H_
/*
Thread Safe:
If control enters the declaration concurrently 
while the variable is being initialized,
the concurrent execution shall wait for completion of the initialization.
--------------HOW TO USE IT
//--------step0:
#include "singleton.h"

class TestSingle : public Singleton<TestSingle>
{
//--------step1: must to be friend
	friend class Singleton<TestSingle>;
public:
	TestSingle(const TestSingle&)=delete;
	TestSingle& operator= (const TestSingle&)=delete;

private:
	TestSingle()=default;
}

//--------step2: Test&Verify
int foo(int arg, char* argv[])
{
	TestSingle& instanceA = TestSingle::getInstance();
	TestSingle& instanceB = TestSingle::getInstance();
}
*/
template <typename T>
class Singleton
{
public:
	static T* getInstance()
	{
        if (sm_pinstObj == nullptr) {
            sm_pinstObj = new T;
        }
 
		return sm_pinstObj;
	}
	Singleton(const Singleton&)=delete;
	Singleton& operator= (const Singleton&)=delete;
	virtual ~Singleton(){}

    void releseInst()
    {
        if (sm_pinstObj != nullptr) {
            delete sm_pinstObj;
            sm_pinstObj = nullptr;
        }
    }

protected:
	Singleton(){}

private:
    static T* sm_pinstObj;
};

template <typename T>
T* Singleton<T>::sm_pinstObj=nullptr;

#endif //SINGLETON_H_