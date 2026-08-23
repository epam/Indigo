#ifndef _BINGO_PG_BUFFER_H__
#define _BINGO_PG_BUFFER_H__

#include <climits>

#include "base_cpp/exception.h"
#include "bingo_postgres.h"
/*
 * Class for postgres buffers handling
 */
class BingoPgBuffer
{
public:
    /*
     * Empty buffer constructor
     */
    BingoPgBuffer();
    /*
     * New buffer constructor
     */
    BingoPgBuffer(PG_OBJECT rel, unsigned int block_num);
    /*
     * Existing buffer constructor
     */
    BingoPgBuffer(PG_OBJECT rel, unsigned int block_num, int lock);
    /*
     * Destructor
     */
    ~BingoPgBuffer();

    /*
     * Changes an access for the buffer
     */
    void changeAccess(int lock);
    /*
     * Buffer getter
     */
    int getBuffer() const
    {
        return _buffer;
    }

    void* getPage() const;

    void setWalEnabled(bool enabled);
    void setRawPageWal(bool enabled);

    /*
     * Set the first block that higher-level Bingo metadata proves is not yet
     * published. Existing pages at or after this boundary may be overwritten
     * while recovering an interrupted tail extension. Existing pages before
     * it remain protected from PageInit().
     */
    void setReusableTailStart(unsigned int block_num);

    /*
     * Writes a new buffer with WRITE lock
     */
    int writeNewBuffer(PG_OBJECT rel, unsigned int block_num);
    /*
     * Reads a buffer
     */
    int readBuffer(PG_OBJECT rel, unsigned int block_num, int lock);

    /*
     * Clears and releases the buffer
     */
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
    unsigned int _reusableTailStart;
};

#endif /* BINGO_PG_BUFFER_H */
