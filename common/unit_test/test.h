#ifndef TEST_H
#define TEST_H

#include "test_result.h"
#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/output.h"
namespace indigo
{
class AssertException: public Exception {
public:
   AssertException(const char* message, Failure* f):
   Exception(message),
   _failure(f) {
   }
   Failure* getFailure() const {
      return _failure;
   }
private:
   Failure* _failure;
};
 
class Test
{
public:
    Test (const char * testName, 
          const char * filename, int linenumber);
    virtual ~Test();

    virtual void run (TestResult& result) = 0;
 
protected:
    virtual void runTest (TestResult& result) = 0;
    
    const char * m_name;
    const char * m_filename;
    const int m_linenumber;

private:
    Test(const Test &);
    Test& operator=(const Test &);

};



class CurrentTest {

   enum {
      /*
       * Cut off number for double values
       */
      SIGNIFICANT_DOUBLE = 6
   };
public:
   CurrentTest(const char* test, const char* message, const TestResult& test_result);
   ~CurrentTest();

   void setFailureNum(int val) { _failure = val; }
   int getFailureNum() const { return _failure; }

   const char* getTestName() const {return _testName.ptr(); }

   void assertEquals(int expected, int actual,  const char* file, unsigned int line);
   void assertEquals(double expected, double actual,  const char* file, unsigned int line);
   void assertEquals(const char* expected, const char* actual,  const char* file, unsigned int line);
   
   void assertTrue(bool condition, const char* file, unsigned int line);
   
   void assertNull(int expected, const char* file, unsigned int line);
   void assertNull(const char* expected, const char* file, unsigned int line);

   void assertNotNull(int expected, const char* file, unsigned int line);
   void assertNotNull(const char* expected, const char* file, unsigned int line);

   void _throwAssertion(Array<char>& error, const char* file, unsigned int line);
private:
   Array<char> _message;
   Array<char> _testName;
   int _failure;
   const TestResult& _testResult;
};

#define BEGIN_TEST(description)                                                  \
   {                                                                             \
   CurrentTest current_test(_currentDescription.ptr(), description, test_result);\
   Failure* failure = 0;                                                         \
   test_result.testWasRun();                                                     \
   try {                                                                         \


#define END_TEST                                                                 \
   } catch(AssertException& e ) {                                                \
      failure = e.getFailure();                                                  \
   } catch(Exception& e) {                                                       \
      failure = new Failure("Exception", e.message(), current_test.getTestName(),\
            __FILE__, __LINE__);                                                 \
   } catch(...) {                                                                \
      failure = new Failure("Unknown exception", "unknown error",                \
           current_test.getTestName(), __FILE__, __LINE__);                      \
   }                                                                             \
   if(failure) {                                                                 \
      int fail_num = test_result.addFailure(failure);                            \
      current_test.setFailureNum(fail_num);                                      \
   }                                                                             \
   }                                                                             \


#define DESCRIBE(description)                                                    \
   bprintf(_currentDescription, "%s%s",m_name, description);                     \
   test_result.appendInfoLine(_currentDescription.ptr());                        \


#define ITEST(test_name, class_name)                                             \
    class test_name : public Test                                                \
    {                                                                            \
    public:                                                                      \
       test_name() : Test (#class_name, __FILE__, __LINE__){}                    \
    protected:                                                                   \
       virtual void runTest (TestResult& test_result);                           \
       virtual void run (TestResult& test_result);                               \
    private:                                                                     \
       Array<char> _currentDescription;                                          \
    };                                                                           \

#define TEST(test_name)                                                          \
   void test_name::run(TestResult& test_result) {                                \
      Failure* failure = 0;                                                      \
      try {                                                                      \
         runTest(test_result);                                                   \
      } catch(Exception& e) {                                                    \
      failure = new Failure("Exception while initializing: ", e.message(),       \
            #test_name, __FILE__, __LINE__);                                     \
      } catch(...) {                                                             \
         failure = new Failure("Exception while initializing:", "unknown error", \
           #test_name, __FILE__, __LINE__);                                      \
      }                                                                          \
      if(failure) {                                                              \
         test_result.addFailure(failure);                                        \
         test_result.appendErrorLine(failure->getMessage());                     \
      }                                                                          \
   }                                                                             \
   void test_name::runTest (TestResult& test_result)


#define ASSERT_TRUE(condition)                                                   \
   do {                                                                          \
      current_test.assertTrue(condition, __FILE__, __LINE__);                    \
   } while(0);                                                                   \


#define ASSERT_EQUALS(expected,actual)                                           \
   do {                                                                          \
      current_test.assertEquals(expected, actual, __FILE__, __LINE__);           \
   } while(0);                                                                   \

#define ASSERT_NOT_NULL(condition)                                               \
   do {                                                                          \
      current_test.assertNotNull(condition, __FILE__, __LINE__);                 \
   } while(0);                                                                   \

#define ASSERT_NULL(condition)                                                   \
   do {                                                                          \
      current_test.assertNull(condition, __FILE__, __LINE__);                    \
   } while(0);                                                                   \

#define ASSERT_THROW(expression)                                                 \
   do {                                                                          \
      bool throw_error = false;                                                  \
      try {                                                                      \
        expression;                                                              \
        throw_error = true;                                                      \
      } catch(Exception&) {                                                      \
      }                                                                          \
      if(throw_error) {                                                          \
        Array<char> error_message;                                               \
        bprintf(error_message, "expected exception but there wasn't any");       \
        current_test._throwAssertion(error_message, __FILE__, __LINE__);         \
      }                                                                          \
   } while(0);                                                                   \

}
#endif



