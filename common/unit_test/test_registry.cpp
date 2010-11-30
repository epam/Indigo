#include "test_registry.h"
#include "test.h"
#include "test_result.h"
using namespace indigo;
TestRegistry::TestRegistry() {
}

void TestRegistry::add (Test* test) {
   _tests.add(test);
}

void TestRegistry::run (TestResult& result) {
    result.startTests();
    for(int i = 0; i < _tests.size(); ++i)
       _tests[i]->run (result);
    result.endTests();
}



