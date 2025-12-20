/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

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

    bool isReady() const;

    DECL_ERROR;

private:
    BingoPgBuffer(const BingoPgBuffer&); // no implicit copy

    int _getAccess(int lock);

    int _buffer;
    int _lock;
    unsigned int _blockIdx;
};

#endif /* BINGO_PG_BUFFER_H */
