#include "test_result_stderr.h"
#include "test_failure.h"
#include "base_cpp/array.h"
#include "base_cpp/output.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <iostream>
using namespace indigo;
void TestResultStdErr::appendInfoLine(const char* message) const {
   printf("%s\n", message);
}


void TestResultStdErr::appendErrorLine(const char* message) const {
    
#ifdef WIN32
   HANDLE hConsole;
   hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hConsole, TEST_WIN_RED);
#else
    printf("%c[%d;%d;%dm", 0x1B, 0, TEST_RED, TEST_BG_BLACK);
#endif
    

   printf("%s\n", message);

#ifdef WIN32
   SetConsoleTextAttribute(hConsole, TEST_WIN_WHITE);
#else
   printf("%c[%d;%d;%dm\n", 0x1B, 0, TEST_WHITE, TEST_BG_BLACK);
#endif
}

void TestResultStdErr::appendSuccessLine(const char* message) const {
#ifdef WIN32
   HANDLE hConsole;
   hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hConsole, TEST_WIN_GREEN);
#else
   printf("%c[%d;%d;%dm", 0x1B, 0, TEST_GREEN, TEST_BG_BLACK);
#endif
   printf("%s\n", message);
#ifdef WIN32
   SetConsoleTextAttribute(hConsole, TEST_WIN_WHITE);
#else
   printf("%c[%d;%d;%dm", 0x1B, 0, TEST_WHITE, TEST_BG_BLACK);
#endif
}

void TestResultStdErr::endTests ()
{
    TestResult::endTests();
    appendInfoLine("");
    Array<char> message;
    bprintf(message, "Finished in %f seconds", _secondsElapsed);
    appendInfoLine(message.ptr());
    appendInfoLine("");

    int failure_count = failureCount();

    bprintf(message, "%d examples, %d failures", _testCount, failure_count);

    if(failure_count > 0) {
       appendErrorLine("ERROR\n");
       appendErrorLine(message.ptr());
    } else
       appendSuccessLine(message.ptr());

    for (int i = 0; i < _failures.size(); ++i) {
      appendInfoLine("");
      bprintf(message, "%d) %s(%d):", i+1, _failures[i]->getFileName(), _failures[i]->getLineNumber());
      appendInfoLine(message.ptr());
      bprintf(message, "%s in %s\n%s",
              _failures[i]->getException(),
              _failures[i]->getTestName(), 
              _failures[i]->getMessage());
      appendErrorLine(message.ptr());
   }

}

