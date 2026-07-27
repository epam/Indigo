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
    BingoPgCursor(indigo::Array<char>& query_str);
    virtual ~BingoPgCursor();

    bool next();

    void getId(int arg_idx, ItemPointerData&);
    void getText(int arg_idx, BingoPgText&);
    uintptr_t getDatum(int arg_idx);
    unsigned int getArgOid(int arg_idx);

    /**
     * Forces deferred SPI_finish at transaction end instead of immediately
     * when the cursor is destroyed. This is needed to avoid invoking
     * inside the index code. SPI_finish will be called once
     * the transaction has fully ended.
     */
    void finishOnTransactionEnd()
    {
        _finishOnTransactionEnd = true;
    }

    DECL_ERROR;

private:
    BingoPgCursor(const BingoPgCursor&); // no implicit copy

    void _init(indigo::Array<char>& query_str);
    bool _finishOnTransactionEnd;
    indigo::Array<char> _cursorName;
    PG_OBJECT _cursorPtr;
    bool _pushed;
};

#endif /* BINGO_PG_CURSOR_H */
