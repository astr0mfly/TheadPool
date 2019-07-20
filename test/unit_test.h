#ifndef _UNIT_TEST_HPP_
#define _UNIT_TEST_HPP_

#include <vector>
#include <typeinfo>
#include <cstring>
#include <cstdio>
#include <cmath>

#define TEST_NAME(test_name) test_name##_TEST

#define TEST(test_name) \
class TEST_NAME(test_name) : public TestCase \
{ \
public: \
	TEST_NAME(test_name)(const char *name):TestCase(name) { } \
	virtual void Run(); \
private: \
	static TestCase* const test_case_; \
}; \
TestCase* const TEST_NAME(test_name)::test_case_ = UnitTest::GetInstance()->RegisterTestCase( \
new TEST_NAME(test_name)(#test_name)); \
void TEST_NAME(test_name)::Run()

#define RUN_ALL_TESTS(str) UnitTest::GetInstance()->Run(str);

#define FORMAT_RED(_S)    "\033[0;31m"#_S"\033[0m"
#define FORMAT_GREEN(_S)    "\033[0;32m"#_S"\033[0m"
#define FORMAT_YELLOW(_S)    "\033[0;33m"#_S"\033[0m"
#define FORMAT_BULE(_S)    "\033[0;34m"#_S"\033[0m"
#define FORMAT_PURPLE(_S)    "\033[0;35m"#_S"\033[0m"

typedef enum OperatorType
{
    OPERATOR_TYPE_EQ,
    OPERATOR_TYPE_NE,
    OPERATOR_TYPE_GT,
    OPERATOR_TYPE_LT,
    OPERATOR_TYPE_GE,
    OPERATOR_TYPE_LE,

    OPERATOR_TYPE_BT
} OperatorType_E;


class TestCase
{
public:
	TestCase(const char *CaseName_F) :
        m_CaseName_(CaseName_F),
        m_TestResult_(false)
    { };
	virtual ~TestCase() { }
    virtual void Run() = 0;
    const char* getCaseName() const { return m_CaseName_; }
    bool getTestResult() { return m_TestResult_; }
    void setTestResult(bool &&result_F) { m_TestResult_= result_F; }

private:
    const char* m_CaseName_;
    bool        m_TestResult_;
};

class UnitTest
{
public:
	static UnitTest* GetInstance()
    {
		static UnitTest s_instUnitTest;
		return &s_instUnitTest;
	}

    ~UnitTest()
    {
        for (auto i = m_vecTestCases_.begin(); i != m_vecTestCases_.end(); ++i)
            delete* i;
    }

	TestCase* RegisterTestCase(TestCase *testCase_F)
    {
		m_vecTestCases_.push_back(testCase_F);
		return testCase_F;
	}

	bool Run(const char *str)
    {
		m_bTestResult_ = true;

		printf("\033[33m[ Start ]  Unit Tests\033[0m\n\n");

		m_iAll_ = 0;
		for (auto it = m_vecTestCases_.begin(); it != m_vecTestCases_.end(); ++it) {
			TestCase *test_case = *it;
            if (str && !strstr(test_case->getCaseName(), str)) {
                continue;
            }

			++m_iAll_;
			m_pinstCurrentTestCase_ = test_case;
			m_pinstCurrentTestCase_->setTestResult(true);

			printf("\033[34m[ Run  ] \033[0m%s\n", test_case->getCaseName());

			test_case->Run();

			if (test_case->getTestResult())
				printf("\033[32m[ Pass ] \033[0m%s\n", test_case->getCaseName());
			else
				printf("\033[31m[ Fail ] \033[0m%s\n", test_case->getCaseName());

			if (test_case->getTestResult()) {
				++m_iPassedNum_;
			} else {
				++m_iFailedNum_;
				m_bTestResult_ = false;
			}
		}

		printf("\n\033[33m[ ALL  ] \033[33;1m%d\033[0m\n", m_iAll_);
		printf("\033[32m[ PASS ] \033[32;1m%d\033[0m\n", m_iPassedNum_);
		printf("\033[31m[ FAIL ] \033[31;1m%d\033[0m\n", m_iFailedNum_);
		return !m_bTestResult_;
	}
    TestCase& getCrtTestCase(){ return (*m_pinstCurrentTestCase_); }

private:
    UnitTest():
        m_pinstCurrentTestCase_(nullptr),
        m_bTestResult_(false),
        m_iAll_(0),
        m_iPassedNum_(0),
        m_iFailedNum_(0)
    {}
    TestCase               *m_pinstCurrentTestCase_;
    bool                    m_bTestResult_;
    int                     m_iAll_;
    int                     m_iPassedNum_;
    int                     m_iFailedNum_;
    std::vector<TestCase*>  m_vecTestCases_;
};

template <class ElemType>
bool CheckNumericalData(ElemType left_value, ElemType right_value,
	                    const char *str_left_value, const char *str_right_value,
	                    const char *file_name, const unsigned long line_num,
                        OperatorType operator_type)
{
	bool condition = false;
	char str_operator[5] = {0};

	switch (operator_type) {
		case OPERATOR_TYPE_EQ:
			if (typeid(ElemType) == typeid(double))
				condition = !(std::fabs(left_value - right_value) < 1e-8);
			else if (typeid(ElemType) == typeid(float))
				condition = !(std::fabs(left_value - right_value) < 1e-6);
			else
				condition = !(left_value == right_value);
			strcpy(str_operator, " == ");
			break;

		case OPERATOR_TYPE_NE:
			if (typeid(ElemType) == typeid(double))
				condition = !(std::fabs(left_value - right_value) > 1e-8);
			else if (typeid(ElemType) == typeid(float))
				condition = !(std::fabs(left_value - right_value) > 1e-6);
			else
				condition = !(left_value != right_value);
			strcpy(str_operator, " != ");
			break;

		case OPERATOR_TYPE_GT:
			condition = !(left_value > right_value);
			strcpy(str_operator, " > ");
			break;

		case OPERATOR_TYPE_LT:
			condition = !(left_value < right_value);
			strcpy(str_operator, " < ");
			break;

		case OPERATOR_TYPE_LE:
			condition = !(left_value <= right_value);
			strcpy(str_operator, " <= ");
			break;

		case OPERATOR_TYPE_GE:
			condition = !(left_value >= right_value);
			strcpy(str_operator, " >= ");
			break;

        default:
            strcpy(str_operator, " no ");
            break;
	}

	if (condition) {
        UnitTest::GetInstance()->getCrtTestCase().setTestResult(false);
		printf("\033[36;1m%s\033[0m: \033[31;1m%lu\033[0m: ", file_name, line_num);
		printf("\033[33;1mExpect: \033[0m%s%s%s\n", str_left_value, str_operator, str_right_value);
	}
	return !condition;
}

bool CheckStrData(const char *left_value, const char *right_value,
	              const char *str_left_value, const char *str_right_value,
	              const char *file_name, const unsigned long line_num,
                  OperatorType operator_type)
{
	bool condition = false;
	char str_operator[5] = {0};

	if (operator_type == OPERATOR_TYPE_EQ) {
		condition = !((strcmp(left_value, right_value) == 0));
		strcpy(str_operator, " == ");
	}
	else if (operator_type == OPERATOR_TYPE_NE) {
		condition = !((strcmp(left_value, right_value) != 0));
		strcpy(str_operator, " != ");
	}

	if (condition) {
		UnitTest::GetInstance()->getCrtTestCase().setTestResult(false);
		printf("\033[36;1m%s\033[0m: \033[31;1m%lu\033[0m: ", file_name, line_num);
		printf("\033[33;1mExpect: \033[0m%s%s%s\n", str_left_value, str_operator, str_right_value);
	}

	return !condition;
}

#define CHECK_NUMERICAL_DATA(left_value, right_value, operator_type) \
CheckNumericalData(left_value, right_value, #left_value, #right_value, __FILE__, __LINE__, operator_type)
#define CHECK_STR_DATA(left_value, right_value, operator_type) \
CheckStrData(left_value, right_value, #left_value, #right_value, __FILE__, __LINE__, operator_type)

#define EXPECT_EQ(left_value, right_value) CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_EQ)
#define EXPECT_NE(left_value, right_value) CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_NE)
#define EXPECT_GT(left_value, right_value) CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_GT)
#define EXPECT_LT(left_value, right_value) CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_LT)
#define EXPECT_GE(left_value, right_value) CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_GE)
#define EXPECT_LE(left_value, right_value) CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_LE)

#define ASSERT_EQ(left_value, right_value) if (!CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_EQ)) return;
#define ASSERT_NE(left_value, right_value) if (!CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_NE)) return;
#define ASSERT_GT(left_value, right_value) if (!CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_GT)) return;
#define ASSERT_LT(left_value, right_value) if (!CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_LT)) return;
#define ASSERT_GE(left_value, right_value) if (!CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_GE)) return;
#define ASSERT_LE(left_value, right_value) if (!CHECK_NUMERICAL_DATA(left_value, right_value, OPERATOR_TYPE_LE)) return;

#define EXPECT_TRUE(condition) CHECK_NUMERICAL_DATA(static_cast<bool>(condition), true, OPERATOR_TYPE_EQ)
#define EXPECT_FALSE(condition) CHECK_NUMERICAL_DATA(static_cast<bool>(condition), false, OPERATOR_TYPE_EQ)
#define ASSERT_TRUE(condition) if (!CHECK_NUMERICAL_DATA(static_cast<bool>(condition), true, OPERATOR_TYPE_EQ)) return;
#define ASSERT_FALSE(condition) if (!CHECK_NUMERICAL_DATA(static_cast<bool>(condition), false, OPERATOR_TYPE_EQ)) return;
#define EXPECT_STREQ(left_value, right_value) CHECK_STR_DATA(left_value, right_value, OPERATOR_TYPE_EQ)
#define EXPECT_STRNE(left_value, right_value) CHECK_STR_DATA(left_value, right_value, OPERATOR_TYPE_NE)
#define ASSERT_STREQ(left_value, right_value) if (!CHECK_STR_DATA(left_value, right_value, OPERATOR_TYPE_EQ)) return;
#define ASSERT_STRNE(left_value, right_value) if (!CHECK_STR_DATA(left_value, right_value, OPERATOR_TYPE_NE)) return;

#endif /* _UNIT_TEST_HPP_ */