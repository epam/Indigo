#ifndef _BINGO_PG_CURSOR_H__
#define _BINGO_PG_CURSOR_H__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "bingo_postgres.h"

struct ItemPointerData;
class BingoPgText;

class BingoPgCursor
{
public:
    BingoPgCursor(const char* format, ...);
    BingoPgCursor(std::string& query_str);
    virtual ~BingoPgCursor();

    bool next();

    void getId(int arg_idx, ItemPointerData&);
    void getText(int arg_idx, BingoPgText&);
    uintptr_t getDatum(int arg_idx);
    unsigned int getArgOid(int arg_idx);

    DECL_ERROR;

private:
    BingoPgCursor(const BingoPgCursor&); // no implicit copy

    void _init(std::string& query_str);

    std::string _cursorName;
    PG_OBJECT _cursorPtr;
    bool _pushed;
};

#endif /* BINGO_PG_CURSOR_H */
