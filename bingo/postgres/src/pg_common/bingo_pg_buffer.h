#ifndef _BINGO_PG_BUFFER_H__
#define _BINGO_PG_BUFFER_H__

#include "base_cpp/exception.h"
#include "bingo_postgres.h"
/*
 * Class for postgres buffers handling
 */
class BingoPgBuffer
{
public:
    BingoPgBuffer();
    BingoPgBuffer(PG_OBJECT rel, unsigned int block_num);
    BingoPgBuffer(PG_OBJECT rel, unsigned int block_num, int lock);
    ~BingoPgBuffer();

    void changeAccess(int lock);
    int getBuffer() const
    {
        return _buffer;
    }

    void* getPage() const;

    void setWalEnabled(bool enabled);
    /*
     * Bingo's metapage stores useful bytes in the area PostgreSQL considers
     * the standard-page free-space hole. Generic WAL deliberately zeros that
     * area, so that page must be WAL-logged as a non-standard full image.
     */
    void setRawPageWal(bool enabled);

    int writeNewBuffer(PG_OBJECT rel, unsigned int block_num);
    int readBuffer(PG_OBJECT rel, unsigned int block_num, int lock);

    void clear();

    void* getIndexData(int& data_len);
    void formIndexTuple(void* map_data, int size);
    void formEmptyIndexTuple(int size);
    void replaceIndexData(const void* data, int size);

    bool isReady() const;

    DECL_ERROR;

private:
    BingoPgBuffer(const BingoPgBuffer&); // no implicit copy

    int _getAccess(int lock);
    void _beginWrite(bool full_image);
    void _finishWrite();
    void _abortWrite();
    void* _getPage() const;

    int _buffer;
    int _lock;
    unsigned int _blockIdx;
    PG_OBJECT _relation;
    void* _walState;
    void* _writePage;
    bool _walEnabled;
    bool _rawPageWal;
    bool _writeAborted;
};

#endif /* BINGO_PG_BUFFER_H */
