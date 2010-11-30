#ifndef TESTRESULTSTDERR_H
#define TESTRESULTSTDERR_H

#include "test_result.h"
namespace indigo
{
class TestResultStdErr : public TestResult {
   enum {
      TEST_WIN_RED = 4,
      TEST_WIN_GREEN = 2,
      TEST_WIN_WHITE = 15,
      TEST_RED = 31,
      TEST_GREEN = 32,
      TEST_WHITE = 38,
      TEST_BG_BLACK = 38
   };
   
public:

   virtual void appendInfoLine(const char* message) const;
   virtual void appendErrorLine(const char* message) const;
   virtual void appendSuccessLine(const char* message) const;
   
   virtual void endTests();
};
}

#endif

