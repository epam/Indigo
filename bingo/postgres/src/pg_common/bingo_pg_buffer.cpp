#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "access/generic_xlog.h"
#include "access/itup.h"
#include "access/xloginsert.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "storage/bufmgr.h"
#include "storage/bufpage.h"
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
BingoPgBuffer::BingoPgBuffer()
    : _buffer(InvalidBuffer), _lock(BINGO_PG_NOLOCK), _blockIdx(0), _relation(0), _walState(nullptr), _writePage(nullptr), _walEnabled(true),
      _rawPageWal(false), _writeAborted(false), _reusableTailStart(UINT_MAX)
{
}
/*
 * New buffer constructor
 */
BingoPgBuffer::BingoPgBuffer(PG_OBJECT rel_ptr, unsigned int block_num)
    : _buffer(InvalidBuffer), _lock(BINGO_PG_NOLOCK), _blockIdx(0), _relation(0), _walState(nullptr), _writePage(nullptr), _walEnabled(true),
      _rawPageWal(false), _writeAborted(false), _reusableTailStart(UINT_MAX)
{
    writeNewBuffer(rel_ptr, block_num);
}
/*
 * Existing buffer constructor
 */
BingoPgBuffer::BingoPgBuffer(PG_OBJECT rel_ptr, unsigned int block_num, int lock)
    : _buffer(InvalidBuffer), _lock(BINGO_PG_NOLOCK), _blockIdx(0), _relation(0), _walState(nullptr), _writePage(nullptr), _walEnabled(true),
      _rawPageWal(false), _writeAborted(false), _reusableTailStart(UINT_MAX)
{
    readBuffer(rel_ptr, block_num, lock);
}
/*
 * Destructor
 */
BingoPgBuffer::~BingoPgBuffer()
{
    try
    {
        if (_lock == BINGO_PG_WRITE)
            _abortWrite();
        clear();
    }
    catch (Exception& e)
    {
        elog(WARNING, "internal error while cleaning up bingo block %u: %s", _blockIdx, e.message());
    }
    catch (...)
    {
        elog(WARNING, "internal error while cleaning up bingo block %u", _blockIdx);
    }
}

void BingoPgBuffer::setWalEnabled(bool enabled)
{
    if (_lock == BINGO_PG_WRITE || _walState != nullptr)
        throw Error("internal error: can not change WAL mode while a bingo buffer is being written");
    _walEnabled = enabled;
}

void BingoPgBuffer::setRawPageWal(bool enabled)
{
    if (_lock == BINGO_PG_WRITE || _walState != nullptr)
        throw Error("internal error: can not change raw-page WAL mode while a bingo buffer is being written");
    _rawPageWal = enabled;
}

void BingoPgBuffer::setReusableTailStart(unsigned int block_num)
{
    if (_buffer != InvalidBuffer || _walState != nullptr)
        throw Error("internal error: can not change reusable tail boundary while a bingo buffer is active");
    _reusableTailStart = block_num;
}

void BingoPgBuffer::_beginWrite(bool full_image)
{
    if (_buffer == InvalidBuffer)
        throw Error("internal error: can not begin writing an invalid bingo buffer");

    _writeAborted = false;
    if (!_walEnabled || _rawPageWal)
    {
        _writePage = BufferGetPage(_buffer);
        return;
    }

    Relation rel = (Relation)_relation;
    GenericXLogState* state = nullptr;
    Page page = nullptr;
    BINGO_PG_TRY
    {
        state = GenericXLogStart(rel);
        page = GenericXLogRegisterBuffer(state, _buffer, full_image ? GENERIC_XLOG_FULL_IMAGE : 0);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not start WAL for bingo block %u: %s", _blockIdx, message));

    _walState = state;
    _writePage = page;
}

void BingoPgBuffer::_finishWrite()
{
    if (_buffer == InvalidBuffer || _writePage == nullptr)
        return;

    if (_writeAborted)
    {
        _writePage = nullptr;
        _writeAborted = false;
        return;
    }

    if (_walState != nullptr)
    {
        GenericXLogState* state = (GenericXLogState*)_walState;
        BINGO_PG_TRY
        {
            GenericXLogFinish(state);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not finish WAL for bingo block %u: %s", _blockIdx, message));
        _walState = nullptr;
    }
    else if (_rawPageWal && _walEnabled)
    {
        Relation rel = (Relation)_relation;
        if (RelationNeedsWAL(rel))
        {
            START_CRIT_SECTION();
            MarkBufferDirty(_buffer);
            log_newpage_buffer(_buffer, false);
            END_CRIT_SECTION();
        }
        else
        {
            MarkBufferDirty(_buffer);
        }
    }
    else
    {
        BINGO_PG_TRY
        {
            MarkBufferDirty(_buffer);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not set buffer dirty %d: %s", _buffer, message));
    }
    _writePage = nullptr;
}

void BingoPgBuffer::_abortWrite()
{
    if (_walState != nullptr)
    {
        GenericXLogState* state = (GenericXLogState*)_walState;
        BINGO_PG_TRY
        {
            GenericXLogAbort(state);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not abort WAL for bingo block %u: %s", _blockIdx, message));
        _walState = nullptr;
    }
    _writePage = nullptr;
    _writeAborted = true;
}

void* BingoPgBuffer::_getPage() const
{
    if (_writePage != nullptr)
        return _writePage;
    return BufferGetPage(_buffer);
}

void* BingoPgBuffer::getPage() const
{
    return _getPage();
}

/*
 * Changes an access for the buffer
 */
void BingoPgBuffer::changeAccess(int lock)
{
    if (_buffer == InvalidBuffer)
        return;
    if (_lock == lock)
        return;

    if (_lock == BINGO_PG_WRITE)
        _finishWrite();

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

    if (_lock == BINGO_PG_WRITE)
        _beginWrite(false);
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
        {
            ReleaseBuffer(_buffer);
            _buffer = InvalidBuffer;
            throw Error("internal error: unexpected relation size: %u, should be %u", buffer_block_num, block_num);
        }
    }
    else
    {
        BINGO_PG_TRY
        {
            _buffer = ReadBufferExtended(rel, MAIN_FORKNUM, block_num, RBM_NORMAL, NULL);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not read the existing buffer: %s", message));
    }
    /*
     * Lock buffer on writing
     */
    BINGO_PG_TRY
    {
        LockBuffer(_buffer, BUFFER_LOCK_EXCLUSIVE);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not lock the buffer: %s", message));
    _lock = BINGO_PG_WRITE;
    /*
     * Store block index
     */
    _blockIdx = block_num;
    _relation = rel_ptr;

    Page disk_page = BufferGetPage(_buffer);
    const bool existing_page = block_num < nblocks;
    const bool proven_orphan_tail = existing_page && block_num >= _reusableTailStart;
    if (existing_page && !PageIsNew(disk_page) && !proven_orphan_tail)
    {
        UnlockReleaseBuffer(_buffer);
        _buffer = InvalidBuffer;
        _lock = BINGO_PG_NOLOCK;
        _blockIdx = 0;
        _relation = 0;
        throw Error("internal error: refusing to initialize published bingo index block %u", block_num);
    }

    /*
     * initialize the page
     */
    //   PageInit(BufferGetPage(buf), BufferGetPageSize(buf), sizeof (HashPageOpaqueData));
    _beginWrite(true);
    BINGO_PG_TRY
    {
        PageInit((Page)_getPage(), BufferGetPageSize(_buffer), 0);
    }
    BINGO_PG_HANDLE({
        _abortWrite();
        UnlockReleaseBuffer(_buffer);
        _buffer = InvalidBuffer;
        _lock = BINGO_PG_NOLOCK;
        _blockIdx = 0;
        _relation = 0;
        throw Error("internal error: can not initialize the page: %s", message);
    });

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
    _relation = rel_ptr;
    if (_lock == BINGO_PG_WRITE)
        _beginWrite(false);
    return _buffer;
}

/*
 * Clears and releases the buffer
 */
void BingoPgBuffer::clear()
{
    if (_buffer == InvalidBuffer)
        return;

    if (_lock == BINGO_PG_WRITE)
        _finishWrite();

    BINGO_PG_TRY
    {
        switch (_lock)
        {
        case BINGO_PG_WRITE:
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
    _relation = 0;
    _walState = nullptr;
    _writePage = nullptr;
    _writeAborted = false;
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
    Page page = (Page)_getPage();

    if (PageIsNew(page))
        throw Error("internal error: uninitialized bingo index block %d", _blockIdx);

    OffsetNumber max_offset = PageGetMaxOffsetNumber(page);
    if (max_offset < BINGO_TUPLE_OFFSET)
        throw Error("internal error: bingo index block %d has no index tuple", _blockIdx);

    ItemId item_id = PageGetItemId(page, BINGO_TUPLE_OFFSET);
    if (!ItemIdIsNormal(item_id))
        throw Error("internal error: bingo index block %d has an invalid index tuple", _blockIdx);

    IndexTuple itup = (IndexTuple)PageGetItem(page, item_id);
    int hoff = IndexInfoFindDataOffset(itup->t_info);
    int tuple_size = IndexTupleSize(itup);

    if (tuple_size < hoff)
        throw Error("internal error: corrupted block %d data len is %d", _blockIdx, tuple_size - hoff);

    char* data_ptr = (char*)itup + hoff;
    if (data_ptr == 0)
        throw Error("internal error: empty ptr data for the block %d", _blockIdx);

    data_len = tuple_size - hoff;
    return data_ptr;
}

void BingoPgBuffer::formIndexTuple(void* map_data, int size)
{
    bool add_failed = false;
    BINGO_PG_TRY
    {
        Page page = (Page)_getPage();
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
            add_failed = true;

        pfree(itup);
        FreeTupleDesc(index_desc);
    }
    BINGO_PG_HANDLE({
        _abortWrite();
        throw Error("internal error: can not form index tuple: %s", message);
    });

    if (add_failed)
    {
        _abortWrite();
        throw Error("internal error: failed to add index item");
    }
}

using namespace indigo;

void BingoPgBuffer::formEmptyIndexTuple(int size)
{
    QS_DEF(Array<char>, buf);
    buf.resize(size);
    buf.zerofill();
    formIndexTuple(buf.ptr(), buf.sizeInBytes());
}

void BingoPgBuffer::replaceIndexData(const void* data, int size)
{
    int data_len = 0;
    void* target = getIndexData(data_len);
    if (size > data_len)
        throw Error("internal error: replacement data size %d exceeds bingo block %u capacity %d", size, _blockIdx, data_len);
    memcpy(target, data, size);
    if (size < data_len)
        memset((char*)target + size, 0, data_len - size);
}

bool BingoPgBuffer::isReady() const
{
    return _buffer != InvalidBuffer;
}
