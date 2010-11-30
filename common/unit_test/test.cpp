#include <math.h>

#include "test.h"
#include "test_registry.h"
#include "test_result.h"
#include "test_failure.h"
#include "base_cpp/output.h"

using namespace indigo;

Test::Test (const char * testName, 
            const char * filename, int linenumber)
    : m_name (testName)
    , m_filename (filename)
    , m_linenumber (linenumber)
{
}

Test::~Test()
{
}

CurrentTest::CurrentTest(const char* test, const char* message, const TestResult& test_result) :
_failure(0),
_testResult(test_result) {
   _message.appendString("- ", true);
   _message.appendString(message, true);
   bprintf(_testName, "%s %s", test, message);
}

CurrentTest::~CurrentTest() {
   if (_failure) {
      bprintf(_message, "%s (FAILED - %d)", _message.ptr(), _failure);
      _testResult.appendErrorLine(_message.ptr());
   } else {
      _testResult.appendSuccessLine(_message.ptr());
   }
}

void CurrentTest::assertEquals(const char* expected, const char* actual,  const char* file, unsigned int line) {
   if((expected == 0 || actual == 0) && !(expected == 0 && actual == 0)) {
      Array<char> error_message;
      bprintf(error_message, "one of the strings was null");
      _throwAssertion(error_message, file, line);
   }
   if(strcmp(expected, actual) != 0) {
      Array<char> error_message;
      bprintf(error_message, "expected: '%s' but was: '%s'", expected, actual);
      _throwAssertion(error_message, file, line);
   }
}

void CurrentTest::assertEquals(int expected, int actual,  const char* file, unsigned int line) {
   if(expected != actual) {
      Array<char> error_message;
      bprintf(error_message, "expected: '%d' but was: '%d'", expected, actual);
      _throwAssertion(error_message, file, line);
   }
}
void CurrentTest::assertEquals(double expected, double actual,  const char* file, unsigned int line) {
   /*
    * Cut off decimicals
    */
   expected *= pow(10.0,SIGNIFICANT_DOUBLE-1);
   expected = (int)float(expected);
   expected /= pow(10.0, SIGNIFICANT_DOUBLE-1);

   actual *= pow(10.0,SIGNIFICANT_DOUBLE-1);
   actual = (int)float(actual);
   actual /= pow(10.0,SIGNIFICANT_DOUBLE-1);
   if (expected != actual) {
      Array<char> error_message;
      bprintf(error_message, "expected: '%.8f' but was: '%.8f'", expected, actual);
      _throwAssertion(error_message, file, line);
   }
}

void CurrentTest::assertTrue(bool condition, const char* file, unsigned int line) {
   if(!condition) {
      Array<char> error_message;
      bprintf(error_message, "expected: 'true' but was: 'false'");
      _throwAssertion(error_message, file, line);
   }
}

void CurrentTest::assertNull(int expected, const char* file, unsigned int line) {
   if(expected != 0) {
      Array<char> error_message;
      bprintf(error_message, "expected null but was not null");
      _throwAssertion(error_message, file, line);
   }
}
void CurrentTest::assertNull(const char* expected, const char* file, unsigned int line) {
   if(expected != 0) {
      Array<char> error_message;
      bprintf(error_message, "expected null but was not null");
      _throwAssertion(error_message, file, line);
   }
}

void CurrentTest::assertNotNull(const char* expected, const char* file, unsigned int line) {
   if(expected == 0) {
      Array<char> error_message;
      bprintf(error_message, "expected not null but was null");
      _throwAssertion(error_message, file, line);
   }
}

void CurrentTest::assertNotNull(int expected, const char* file, unsigned int line) {
   if(expected == 0) {
      Array<char> error_message;
      bprintf(error_message, "expected not null but was null");
      _throwAssertion(error_message, file, line);
   }
}

void CurrentTest::_throwAssertion(Array<char>& error, const char* file, unsigned int line) {
   throw AssertException("Assert error",
              new Failure("AssertException", error.ptr(), _testName.ptr(), file, line));
}




