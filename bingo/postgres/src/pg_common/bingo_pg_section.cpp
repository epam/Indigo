#include "bingo_pg_section.h"
#include "bingo_pg_index.h"
#include "bingo_pg_search_engine.h"
#include "bingo_pg_common.h"

CEXPORT {
#include "postgres.h"
#include "storage/itemptr.h"
#include "storage/block.h"
}

BingoPgSection::BingoPgSection(BingoPgIndex& bingo_idx, int offset):
_index(bingo_idx.getIndexPtr()),
_offset(offset),
_writeStrategy(false){

   /*
    * Clear section metainfo
    */
   clear();

   /*
    * Prepare section strategy
    */
   _writeStrategy = false;
   if(bingo_idx.getIndexStrategy() == BingoPgIndex::BUILDING_STRATEGY)
      _writeStrategy = true;
   

   if(_writeStrategy) {
      /*
       * If write then create new buffers
       */
      _sectionInfoBuffer.writeNewBuffer(_index, offset);
      _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);
      _sectionInfo.n_blocks_for_map = bingo_idx.getMapSize();
      _sectionInfo.n_blocks_for_fp = bingo_idx.getFpSize();
      /*
       * Initialize existing structures fingerprint
       */
      _existStructures.reset(new BingoPgBufferCacheFp(offset + 1, _index, _writeStrategy));

      /*
       * Initialize bits number buffers
       */
      for (int idx = 0; idx < SECTION_BITSNUMBER_PAGES; ++idx) {
         BingoPgBuffer& bits_buffer = _bitsCountBuffers.push();
         bits_buffer.writeNewBuffer(_index, offset + idx + SECTION_META_PAGES);
         bits_buffer.formEmptyIndexTuple(SECTION_BITS_PER_BLOCK * sizeof(unsigned short));
         bits_buffer.changeAccess(BINGO_PG_NOLOCK);
      }

   } else {
      /*
       * Read section meta info
       */
      _sectionInfoBuffer.readBuffer(_index, _offset, BINGO_PG_READ);
      int data_len;
      BingoSectionInfoData* data = (BingoSectionInfoData*)_sectionInfoBuffer.getIndexData(data_len);
      _sectionInfo = *data;
      
      _existStructures.reset(new BingoPgBufferCacheFp(offset + 1, _index, _writeStrategy));
   }
   
   int fp_count = _sectionInfo.n_blocks_for_fp;
   int map_count = _sectionInfo.n_blocks_for_map;
   int bin_count = _sectionInfo.n_blocks_for_bin;

   /*
    * Prepare for reading or writing all the data buffers
    */
   int block_offset = offset + SECTION_META_PAGES + SECTION_BITSNUMBER_PAGES;
   for (int i = 0; i < map_count; ++i) {
      _buffersMap.add(new BingoPgBufferCacheMap(block_offset, _index, _writeStrategy));
      ++block_offset;
   }
   for (int i = 0; i < fp_count; ++i) {
      _buffersFp.add(new BingoPgBufferCacheFp(block_offset, _index, _writeStrategy));
      ++block_offset;
   }
   
   for (int i = 0; i < bin_count; ++i) {
      _buffersBin.add(new BingoPgBufferCacheBin(block_offset, _index, _writeStrategy));
      ++block_offset;
   }

}

BingoPgSection::~BingoPgSection() {
   /*
    * Write meta info
    */
   _sectionInfo.n_blocks_for_bin = _buffersBin.size();
   if(_writeStrategy) {
      _sectionInfo.section_size = getPagesCount();
      _sectionInfoBuffer.changeAccess(BINGO_PG_WRITE);
      _sectionInfoBuffer.formIndexTuple(&_sectionInfo, sizeof(_sectionInfo));
      _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);
   } else {
      _sectionInfoBuffer.changeAccess(BINGO_PG_WRITE);
      int data_len;
      BingoSectionInfoData* data = (BingoSectionInfoData*)_sectionInfoBuffer.getIndexData(data_len);
      *data = _sectionInfo;
      _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);
   }
}

void BingoPgSection::clear() {
   _sectionInfo.n_blocks_for_bin = 0;
   _sectionInfo.n_blocks_for_fp = 0;
   _sectionInfo.n_blocks_for_map = 0;
   _sectionInfo.n_structures = 0;
   _sectionInfo.section_size = 0;
   _sectionInfo.last_cmf = -1;
   _sectionInfo.last_xyz = -1;
   _sectionInfo.has_removed = 0;
   _sectionInfoBuffer.clear();
   _existStructures.reset(0);
   _buffersMap.clear();
   _buffersFp.clear();
   _buffersBin.clear();
}

bool BingoPgSection::isExtended() {
   return _sectionInfo.n_structures < BINGO_MOLS_PER_SECTION;
}

void BingoPgSection::addStructure(BingoPgFpData& item_data) {
   int current_str = _sectionInfo.n_structures;

   /*
    * Set fp bits
    */
   for (int idx = item_data.bitBegin(); idx != item_data.bitEnd(); idx = item_data.bitNext(idx)) {
      int bit_idx = item_data.getBit(idx);
      _buffersFp.at(bit_idx)->setBit(current_str, true);
   }

   int map_buf_idx = current_str / BINGO_MOLS_PER_MAPBLOCK;
   int map_idx = current_str % BINGO_MOLS_PER_MAPBLOCK;
   /*
    * Set tid map
    */
   _buffersMap[map_buf_idx]->setTidItem(map_idx, item_data.getTidItem());

   /*
    * Prepare and set cmf map
    */
   _setCmfData(item_data.getCmfBuf(), map_buf_idx, map_idx);
   /*
    * Prepare and set xyz map
    */
   _setXyzData(item_data.getXyzBuf(), map_buf_idx, map_idx);

   /*
    * Set bits number
    */
    _setBitsCountData(item_data.getBitsCount());
   /*
    * Set structure index
    */
   item_data.setStructureIdx(_sectionInfo.n_structures);

   /*
    * Set structure bit
    */
   _existStructures->setBit(current_str, true);

   /*
    * Increment structures number
    */
   ++_sectionInfo.n_structures;

}

int BingoPgSection::getPagesCount() const {
   return _buffersMap.size() + _buffersFp.size() + _buffersBin.size() + SECTION_META_PAGES + SECTION_BITSNUMBER_PAGES;
}

void BingoPgSection::getSectionStructures(BingoPgExternalBitset& section_bitset) {
   _existStructures->getCopy(section_bitset);
}

void BingoPgSection::removeStructure(int mol_idx) {
   _existStructures->setBit(mol_idx, false);
   _sectionInfo.has_removed = 1;
}

bool BingoPgSection::isStructureRemoved(int mol_idx) {
   if(_sectionInfo.has_removed == 0)
      return false;
   return (!_existStructures->getBit(mol_idx));
}

void BingoPgSection::readSectionBitsCount(indigo::Array<int>& bits_number) {
   bits_number.resize(_sectionInfo.n_structures);
   bits_number.zerofill();

   if(_bitsCountBuffers.size() == 0)
      _bitsCountBuffers.resize(SECTION_BITSNUMBER_PAGES);
   
   int data_len, str_idx;
   unsigned short* buffer_data;
   for (int buf_idx = 0; buf_idx < SECTION_BITSNUMBER_PAGES; ++buf_idx) {
      if(buf_idx * SECTION_BITS_PER_BLOCK >= _sectionInfo.n_structures)
         break;

      BingoPgBuffer& bits_buffer = _bitsCountBuffers[buf_idx];
      bits_buffer.readBuffer(_index, _offset + buf_idx + SECTION_META_PAGES, BINGO_PG_READ);
      buffer_data = (unsigned short*) bits_buffer.getIndexData(data_len);
      for (int page_str_idx = 0; page_str_idx < SECTION_BITS_PER_BLOCK; ++page_str_idx) {
         str_idx = buf_idx * SECTION_BITS_PER_BLOCK + page_str_idx;
         if (str_idx >= _sectionInfo.n_structures)
            break;
         bits_number[str_idx] = buffer_data[page_str_idx];
      }
      bits_buffer.changeAccess(BINGO_PG_NOLOCK);
   }

   
}

void BingoPgSection::_setCmfData(indigo::Array<char>& cmf_buf, int map_buf_idx, int map_idx) {
   /*
    * Set binary info
    */
   ItemPointerData cmf_item;
   _setBinData(cmf_buf, _sectionInfo.last_cmf, cmf_item);
   /*
    * Set mappings
    */
   _buffersMap[map_buf_idx]->setCmfItem(map_idx, cmf_item);

}

void BingoPgSection::_setXyzData(indigo::Array<char>& xyz_buf, int map_buf_idx, int map_idx) {
   /*
    * Set binary info
    */
   ItemPointerData xyz_item;
   _setBinData(xyz_buf, _sectionInfo.last_xyz, xyz_item);
   /*
    * Set mappings
    */
   _buffersMap[map_buf_idx]->setXyzItem(map_idx, xyz_item);
}

void BingoPgSection::_setBinData(indigo::Array<char>& buf, int& last_buf, ItemPointerData& item_data) {
   if(buf.size() == 0) {
      BINGO_PG_TRY {
         ItemPointerSet(&item_data, InvalidBlockNumber, 0);
      } BINGO_PG_HANDLE(throw Error("internal error: can not set block data: %s", err->message));
      return;
   }
   /*
    * Dynamic binary buffers
    */

   if(last_buf == -1) {
      int block_off = _offset + getPagesCount();
      _buffersBin.add(new BingoPgBufferCacheBin(block_off, _index, true));
      last_buf = _buffersBin.size() - 1;
   }
   
   BingoPgBufferCacheBin* cache_bin = _buffersBin[last_buf];
   /*
    * If not enough space for inserting a new structure - then create and new buffer
    */
   if(!cache_bin->isEnoughSpace(buf.sizeInBytes())) {
      int block_off = _offset + getPagesCount();
      _buffersBin.add(new BingoPgBufferCacheBin(block_off, _index, true));
      last_buf = _buffersBin.size() - 1;
   }
   cache_bin = _buffersBin[last_buf];

   /*
    * Get cmf offset for storing cmf mapping
    */
   unsigned short bin_offset = cache_bin->addBin(buf);

   /*
    * Set mappings
    */
   BINGO_PG_TRY {
      ItemPointerSet(&item_data, last_buf, bin_offset);
   } BINGO_PG_HANDLE(throw Error("internal error: can not set block data: %s", err->message));
}

void BingoPgSection::_setBitsCountData(unsigned short bits_count) {
   
   if(_bitsCountBuffers.size() == 0)
      _bitsCountBuffers.resize(SECTION_BITSNUMBER_PAGES);
   
   int data_len;
   int buf_idx = _sectionInfo.n_structures / SECTION_BITS_PER_BLOCK;
   int page_str_idx = _sectionInfo.n_structures % SECTION_BITS_PER_BLOCK;

   BingoPgBuffer& bits_buffer = _bitsCountBuffers[buf_idx];
   bits_buffer.readBuffer(_index, _offset + buf_idx + SECTION_META_PAGES, BINGO_PG_WRITE);
   unsigned short* buffer_data = (unsigned short*) bits_buffer.getIndexData(data_len);
   buffer_data[page_str_idx] = bits_count;
   bits_buffer.changeAccess(BINGO_PG_NOLOCK);
}
