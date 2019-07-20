#include "unit_test.h"

#include <functional>
#include <chrono>
#include "..\src\thread_pool.h"

int testFunc(void* pvA)
{
    (*(int*)pvA)++;
    return 0;
}

TEST(normalFunction)
{
    std::function<int(void*)> fnProc = testFunc;
    int a = 1;
    ThreadPool::getInstance()->addTask(fnProc, &a);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(a, 2);
}

TEST(lambdaFunction)
{
    std::function<int(void*)> fnProc = [](void* pvA)->int { (*(int*)pvA)++; return 0; };
    int a=2;
    ThreadPool::getInstance()->addTask(fnProc, &a);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(a, 3);
}

class Functor
{
public:
    int operator()(void* pvA)
    {
        (*(int*)pvA)++;
        return 0;
    }
};

TEST(functor)
{
    Functor instFunctor;
    std::function<int(void*)> fnProc = instFunctor;

    int a = 2;
    ThreadPool::getInstance()->addTask(fnProc, &a);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(a, 3);
}
class TestClass
{
public:
    int ClassMember(void* pvA) { (*(int*)pvA)++;return 0; }
    static int StaticMember(void* pvA) { (*(int*)pvA)++; return 0; }
};

TEST(memberFunction)
{
    TestClass instTest;
    std::function<int(void*)> fnProc = std::bind(&TestClass::ClassMember, instTest, std::placeholders::_1);

    int a = 2;
    ThreadPool::getInstance()->addTask(fnProc, &a);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(a, 3);

    fnProc = TestClass::StaticMember;
    ThreadPool::getInstance()->addTask(fnProc, &a);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(a, 4);
}


int main(int argc, char **argv)
{


    return RUN_ALL_TESTS(argc == 2 ? argv[1] : '\0');
}