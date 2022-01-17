#include "bingo_error.h"

using namespace indigo;

BingoError::BingoError(const char* format, ...) : Exception("bingo: ")
{
    va_list args;
    va_start(args, format);
    const size_t len = strlen(_message);
    vsnprintf(_message + len, sizeof(_message) - len, format, args);
    va_end(args);
}
