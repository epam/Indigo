#ifndef _BINGO_PG_TEXT_H__
#define _BINGO_PG_TEXT_H__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "bingo_postgres.h"

class BingoPgText
{
public:
    BingoPgText();
    BingoPgText(uintptr_t text_datum);
    ~BingoPgText();

    void clear();
    void init(uintptr_t text_datum);
    void initFromString(const char* str);
    void initFromArray(indigo::std::string& str);
    void initFromBuffer(const char* buf, int buf_len);

    const char* getText(int& size);
    const char* getString();

    uintptr_t getDatum();
    PG_OBJECT release();

    DECL_ERROR;

private:
    BingoPgText(const BingoPgText&); // no implicit copy

    PG_OBJECT _text;
    indigo::std::string _cstr;
};

#endif /* BINGO_PG_TEXT_H */
