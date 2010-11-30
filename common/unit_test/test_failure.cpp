#include "test_failure.h"
using namespace indigo;
Failure::Failure(const char* exception, const char* message, const char* test_name,
        const char* file_name, int line_number) :
_lineNumber(line_number) {
   _message.appendString(message, true);
   _fileName.appendString(file_name, true);
   _testName.appendString(test_name, true);
   _exception.appendString(exception, true);
}



int Failure::getLineNumber() const {
   return _lineNumber;
}

const char* Failure::getMessage() const {
   return _message.ptr();
}
const char* Failure::getException() const {
   return _exception.ptr();
}

const char* Failure::getTestName() const {
   return _testName.ptr();
}

const char* Failure::getFileName() const {
   return _fileName.ptr();
}



