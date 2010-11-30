
#ifndef TESTREGISTRY_H
#define TESTREGISTRY_H


#include "base_cpp/ptr_array.h"
namespace indigo
{

class Test;
class TestResult;

class TestRegistry {
public:

    TestRegistry();
    
    void add (Test* test);
    void run (TestResult& result);
        
private:
    PtrArray<Test> _tests;
};
}


#endif

