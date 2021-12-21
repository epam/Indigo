#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "access/itup.h"
#include "fmgr.h"
#include "postgres.h"
#include "storage/bufmgr.h"
#include "storage/lock.h"
#include "utils/rel.h"
#include "utils/relcache.h"
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_common.h"

using namespace indigo;

IMPL_ERROR(BingoPgBuffer, "bingo buffer");

/*
 * Empty buffer constructor
 */
BingoPgBuffer::BingoPgBuffer() : _buffer(InvalidBuffer), _lock(BINGO_PG_NOLOCK), _blockIdx(0)
{
}
/*
 * New buffer constructor
 */
BingoPgBuffer::BingoPgBuffer(PG_OBJECT rel_ptr, unsigned int block_num) : _buffer(InvalidBuffer), _lock(BINGO_PG_NOLOCK), _blockIdx(0)
{
    writeNewBuffer(rel_ptr, block_num);
}
/*
 * Existing buffer constructor
 */
BingoPgBuffer::BingoPgBuffer(PG_OBJECT rel_ptr, unsigned int block_num, int lock) : _buffer(InvalidBuffer), _lock(BINGO_PG_NOLOCK), _blockIdx(0)
{
    readBuffer(rel_ptr, block_num, lock);
}
/*
 * Destructor
 */
BingoPgBuffer::~BingoPgBuffer()
{
    clear();
}
/*
 * Changes an access for the buffer
 */
void BingoPgBuffer::changeAccess(int lock)
{
    if (_buffer == InvalidBuffer)
        return;
    if (_lock == BINGO_PG_WRITE)
    {
        BINGO_PG_TRY
        {
            MarkBufferDirty(_buffer);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not set buffer dirty %d: %s", _buffer, message));
    }
    BINGO_PG_TRY
    {
        if (_lock != BINGO_PG_NOLOCK)
        {
            LockBuffer(_buffer, BUFFER_LOCK_UNLOCK);
        }
        if (lock != BINGO_PG_NOLOCK)
        {
            LockBuffer(_buffer, _getAccess(lock));
        }
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not lock the buffer %d: %s", _buffer, message));
    _lock = lock;
}
/*
 * Writes a new buffer with WRITE lock
 */
int BingoPgBuffer::writeNewBuffer(PG_OBJECT rel_ptr, unsigned int block_num)
{
    /*
     * Clear if it is a new buffer
     */
    if (_buffer != InvalidBuffer)
    {
        if (_blockIdx != block_num)
            clear();
        else
            return _buffer;
    }
    Relation rel = (Relation)rel_ptr;
    BlockNumber nblocks = 0;

    BINGO_PG_TRY
    {
        nblocks = RelationGetNumberOfBlocks(rel);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get number of blocks: %s", message));

    /*
     * Bingo forbids noncontiguous access
     */
    if (block_num > nblocks)
    {
        throw Error("internal error: access to noncontiguous page in bingo index");
    }
    //   if(block_num < nblocks)
    //      throw Error("internal error: access to already pinned block in bingo index");

    /*
     * smgr insists we use P_NEW to extend the relation
     */
    if (block_num == nblocks)
    {
        int buffer_block_num = 0;
        BINGO_PG_TRY
        {
            _buffer = ReadBuffer(rel, P_NEW);
            buffer_block_num = BufferGetBlockNumber(_buffer);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not create a new buffer %s", message));

        if (buffer_block_num != block_num)
            throw Error("internal error: unexpected relation size: %u, should be %u", buffer_block_num, block_num);
    }
    else
    {
        BINGO_PG_TRY
        {
            _buffer = ReadBufferExtended(rel, MAIN_FORKNUM, block_num, RBM_NORMAL, NULL);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not extend the existing buffer: %s", message));
    }
    /*
     * Lock buffer on writing
     */
    BINGO_PG_TRY
    {
        LockBuffer(_buffer, BUFFER_LOCK_EXCLUSIVE);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not lock the buffer: %s", message));

    /*
     * initialize the page
     */
    //   PageInit(BufferGetPage(buf), BufferGetPageSize(buf), sizeof (HashPageOpaqueData));
    BINGO_PG_TRY
    {
        PageInit(BufferGetPage(_buffer), BufferGetPageSize(buf), 0);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not initialize the page %d: %s", _buffer, message));
    _lock = BINGO_PG_WRITE;
    /*
     * Store block index
     */
    _blockIdx = block_num;
    return _buffer;
}
/*
 * Reads a buffer
 */
int BingoPgBuffer::readBuffer(PG_OBJECT rel_ptr, unsigned int block_num, int lock)
{
    /*
     * Clear a buffer if it is different
     */
    if (_buffer != InvalidBuffer)
    {
        if (_blockIdx != block_num)
            clear();
        else
        {
            changeAccess(lock);
            return _buffer;
        }
    }

    Relation rel = (Relation)rel_ptr;
    Buffer buf = 0;
    BINGO_PG_TRY
    {
        buf = ReadBuffer(rel, block_num);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not read the buffer %d: %s", block_num, message));

    /*
     * Lock buffer
     */
    if (lock != BINGO_PG_NOLOCK)
    {
        BINGO_PG_TRY
        {
            LockBuffer(buf, _getAccess(lock));
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not lock the buffer %d: %s", buf, message));
    }

    _lock = lock;
    _buffer = buf;
    /*
     * Store block index
     */
    _blockIdx = block_num;
    return _buffer;
}

/*
 * Clears and releases the buffer
 */
void BingoPgBuffer::clear()
{
    if (_buffer == InvalidBuffer)
        return;
    BINGO_PG_TRY
    {
        switch (_lock)
        {
        case BINGO_PG_WRITE:
            MarkBufferDirty(_buffer);
            UnlockReleaseBuffer(_buffer);
            break;
        case BINGO_PG_READ:
            UnlockReleaseBuffer(_buffer);
            break;
        case BINGO_PG_NOLOCK:
            ReleaseBuffer(_buffer);
            break;
        default:
            break;
        }
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not release the buffer %d: %s", _buffer, message));

    _buffer = InvalidBuffer;
    _lock = BINGO_PG_NOLOCK;
    _blockIdx = 0;
}

int BingoPgBuffer::_getAccess(int lock)
{
    switch (lock)
    {
    case BINGO_PG_WRITE:
        return BUFFER_LOCK_EXCLUSIVE;
    case BINGO_PG_READ:
        return BUFFER_LOCK_SHARE;
    }
    return BUFFER_LOCK_UNLOCK;
}

void* BingoPgBuffer::getIndexData(int& data_len)
{
    char* data_ptr = 0;
    BINGO_PG_TRY
    {
        Page page = BufferGetPage(getBuffer());
        IndexTuple itup = (IndexTuple)PageGetItem(page, PageGetItemId(page, BINGO_TUPLE_OFFSET));

        int hoff = IndexInfoFindDataOffset(itup->t_info);
        data_ptr = (char*)itup + hoff;

        data_len = IndexTupleSize(itup) - hoff;
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get index data from the block %d: %s", _blockIdx, message));

    if (data_ptr == 0)
        throw Error("internal error: empty ptr data for the block %d", _blockIdx);

    if (data_len < 0)
        throw Error("internal error: corrupted block %d data len is %d", _blockIdx, data_len);

    return data_ptr;
}

void BingoPgBuffer::formIndexTuple(void* map_data, int size)
{
    BINGO_PG_TRY
    {
        Page page = BufferGetPage(getBuffer());
        Datum map_datum = PointerGetDatum(map_data);
#if PG_VERSION_NUM / 100 >= 1200
        TupleDesc index_desc = CreateTemplateTupleDesc(1);
#else
        TupleDesc index_desc = CreateTemplateTupleDesc(1, false);
#endif

#if PG_VERSION_NUM / 100 >= 1100
        index_desc->attrs[0].attlen = size;
        index_desc->attrs[0].attalign = 'c';
        index_desc->attrs[0].attbyval = false;
#else
        index_desc->attrs[0]->attlen = size;
        index_desc->attrs[0]->attalign = 'c';
        index_desc->attrs[0]->attbyval = false;
#endif
        bool isnull = false;

        IndexTuple itup = index_form_tuple(index_desc, &map_datum, &isnull);
#if PG_VERSION_NUM / 100 >= 1100
        int itemsz = IndexTupleSize(itup);
#else
        int itemsz = IndexTupleDSize(*itup);
#endif
        itemsz = MAXALIGN(itemsz);

        if (PageAddItem(page, (Item)itup, itemsz, 0, false, false) == InvalidOffsetNumber)
        {
            pfree(itup);
            FreeTupleDesc(index_desc);
            throw Error("internal error: failed to add index item");
        }

        pfree(itup);
        FreeTupleDesc(index_desc);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not form index tuple: %s", message));
}

using namespace indigo;

void BingoPgBuffer::formEmptyIndexTuple(int size)
{
    QS_DEF(Array<char>, buf);
    buf.resize(size);
    buf.zerofill();
    formIndexTuple(buf.ptr(), buf.sizeInBytes());
}

bool BingoPgBuffer::isReady() const
{
    return _buffer != InvalidBuffer;
}