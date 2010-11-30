#include "test_result.h"
#include "test_failure.h"
#include "base_c/nano.h"
using namespace indigo;
TestResult::TestResult() :
_testCount(0) ,
_secondsElapsed(0.0f) {
   _startTime = nanoClock();
}

TestResult::~TestResult() {
}

void TestResult::testWasRun() {
   _testCount++;
}

void TestResult::startTests() {
}

int TestResult::addFailure(Failure* failure) {
   _failures.add(failure);
   return _failures.size();
}

void TestResult::endTests() {
   _secondsElapsed = nanoHowManySeconds((nanoClock() - _startTime));
}

int TestResult::failureCount() const {
   return _failures.size();
}

int TestResult::testCount() const {
   return _testCount;
}

