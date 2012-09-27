#ifndef _BINGO_PG_BUFFER_H__
#define	_BINGO_PG_BUFFER_H__

#include "bingo_postgres.h"
#include "base_cpp/exception.h"
/*
 * Class for postgres buffers handling
 */
class BingoPgBuffer {
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
   int getBuffer() const {return _buffer;}

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
   BingoPgBuffer(const BingoPgBuffer&); //no implicit copy

   int _getAccess(int lock);

   int _buffer;
   int _lock;
   unsigned int _blockIdx;
};
   
#endif	/* BINGO_PG_BUFFER_H */

