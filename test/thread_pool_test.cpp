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

int main(int argc, char **argv)
{


    return RUN_ALL_TESTS(argc == 2 ? argv[1] : '\0');
}