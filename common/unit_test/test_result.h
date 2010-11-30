
#ifndef TEST_RESULT_H
#define TEST_RESULT_H

#include "base_cpp/ptr_array.h"
#include "test_failure.h"

namespace indigo
{

class TestResult {
public:
    TestResult ();
    virtual ~TestResult();

    virtual void testWasRun ();
    virtual void startTests ();
    virtual void endTests ();

    virtual int addFailure (Failure* failure);

    virtual void appendInfoLine(const char* message) const = 0;
    virtual void appendErrorLine(const char* message) const = 0;
    virtual void appendSuccessLine(const char* message) const = 0;


    int failureCount() const;
    int testCount() const;

protected:
    qword _startTime;
    int _testCount;
    float _secondsElapsed;

    PtrArray<Failure> _failures;
};
}

#endif
