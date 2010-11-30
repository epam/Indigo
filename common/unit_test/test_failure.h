
#ifndef TEST_FAILURE_H
#define TEST_FAILURE_H
#include "base_cpp/array.h"
namespace indigo
{
class Failure
{
public:
    Failure (const char* exception, const char* message, const char* testName,
             const char* fileName, int lineNumber);

    int getLineNumber() const;
    const char* getMessage() const;
    const char* getTestName() const;
    const char* getFileName() const;
    const char* getException() const;

    ~Failure(){}



private:
    
    Array<char> _message;
    Array<char> _testName;
    Array<char> _fileName;
    Array<char> _exception;
    int _lineNumber;
};

}
#endif

