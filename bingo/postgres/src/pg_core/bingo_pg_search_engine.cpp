#include "bingo_pg_search_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"

#include "bingo_pg_text.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "access/itup.h"
}

void BingoPgFpData::setTidItem(PG_OBJECT item_ptr) {

   ItemPointerData& item_p = *(ItemPointer) item_ptr;
   _mapData.tid_map = (BingoItemData&)item_p;
}

void BingoPgFpData::setFingerPrints(const char* fp_buf, int size_bits) {
   _fingerprintBits.clear();

   for (int bit_idx = 0; bit_idx < size_bits; ++bit_idx) {
      if (bitGetBit(fp_buf, bit_idx)) {
         _fingerprintBits.push(bit_idx);
      }
   }
}

void BingoPgFpData::setCmf(const char* cmf_buf, int cmf_len) {
   _cmfBuf.copy(cmf_buf, cmf_len);
}

void BingoPgFpData::setXyz(const char* xyz_buf, int xyz_len) {
   _xyzBuf.copy(xyz_buf, xyz_len);
}

BingoPgSearchEngine::BingoPgSearchEngine():
_fetchFound(false),
_currentSection(-1),
_currentIdx(-1),
_bufferIndexPtr(0),
_sectionBitset(BINGO_MOLS_PER_SECTION){
   _bingoSession = bingoAllocateSessionID();
}

BingoPgSearchEngine::~BingoPgSearchEngine(){
   bingoReleaseSessionID(_bingoSession);
}

void BingoPgSearchEngine::setItemPointer(PG_OBJECT result_ptr) {
   _bufferIndexPtr->readTidItem(_currentSection, _currentIdx, result_ptr);
}

bool BingoPgSearchEngine::matchTarget(BingoItemData& item_data) {
   return matchTarget(ItemPointerGetBlockNumber(&item_data), ItemPointerGetOffsetNumber(&item_data));
}

void BingoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT) {
   _bufferIndexPtr = &bingo_idx;
   _currentSection = _bufferIndexPtr->readBegin();
   _currentIdx = -1;
   _fetchFound = false;
}

bool BingoPgSearchEngine::_searchNextCursor(PG_OBJECT result_ptr) {
   BingoItemData cmf_item;
   /*
    * Iterate through the cursor
    */
   while (_searchCursor->next()) {
      _searchCursor->getId(1, cmf_item);

      /*
       * If structure is removed from index then seek to the next
       */
      if(_bufferIndexPtr->isStructureRemoved(cmf_item))
         continue;
      /*
       * If structure is not match then seek to the next
       */
      if (!matchTarget(cmf_item))
         continue;
      /*
       * Return tid map
       */
      _bufferIndexPtr->readTidItem(cmf_item, result_ptr);
      return true;
   }

   return false;
}

bool BingoPgSearchEngine::_searchNextSub(PG_OBJECT result_ptr) {
   BingoPgFpData& query_data = _queryFpData.ref();
   BingoPgIndex& bingo_index = *_bufferIndexPtr;
   /*
    * If there are mathces found on the previous steps
    */
   if(_fetchFound) {
       if(_fetchForNext()) {
          setItemPointer(result_ptr);
          return true;
       } else {
          _fetchFound = false;
          _currentSection = bingo_index.readNext(_currentSection);
       }
   }

   /*
    * Iterate through the sections
    */
   for (; _currentSection < bingo_index.readEnd(); _currentSection = bingo_index.readNext(_currentSection)) {
      /*
       * Get section existing structures
       */
      bingo_index.getSectionBitset(_currentSection, _sectionBitset);
      _currentIdx = -1;
      /*
       * If there is no fingerprints then check every molecule
       */
      if (query_data.bitEnd() != 0) {
         /*
          * Iterate through the query bits
          */
         for (int fp_idx = query_data.bitBegin(); fp_idx != query_data.bitEnd() && _sectionBitset.bitsNumber() > 0; fp_idx = query_data.bitNext(fp_idx)) {
            int fp_block = query_data.getBit(fp_idx);
            /*
             * Get fingerprint buffer in the current section
             */
            bingo_index.andWithBitset(_currentSection, fp_block, _sectionBitset);
         }
      }
      /*
       * If bitset is not null then matches are found
       */
      if (_sectionBitset.bitsNumber() > 0) {
         /*
          * Set first match as an answer
          */
         if(_fetchForNext()) {
            setItemPointer(result_ptr);
            /*
             * Set fetch found to return on the next steps
             */
            _fetchFound = true;
            return true;
         }

      }
   }

   /*
    * No matches or section ends
    */
   return false;
}

using namespace indigo;

bool BingoPgSearchEngine::_searchNextSim(PG_OBJECT result_ptr) {
   _setBingoContext();

   BingoPgFpData& query_data = _queryFpData.ref();
   BingoPgIndex& bingo_index = *_bufferIndexPtr;
   QS_DEF(Array<int>, bits_count);
   QS_DEF(Array<int>, common_ones);
   BingoPgExternalBitset screening_bitset(BINGO_MOLS_PER_SECTION);
   
   int* min_bounds, * max_bounds;
   /*
    * If there are mathces found on the previous steps
    */
   if(_fetchFound) {
       if(_fetchForNext()) {
          setItemPointer(result_ptr);
          return true;
       } else {
          _fetchFound = false;
          _currentSection = bingo_index.readNext(_currentSection);
       }
   }

   /*
    * Iterate through the sections
    */
   for (; _currentSection < bingo_index.readEnd(); _currentSection = bingo_index.readNext(_currentSection)) {
      _currentIdx = -1;
      /*
       * Get section existing structures
       */
      bingo_index.getSectionBitset(_currentSection, _sectionBitset);
      int possible_str_count = _sectionBitset.bitsNumber();
      /*
       * If there is no bits then screen whole the structures
       */
      if (query_data.bitEnd() != 0 && possible_str_count > 0) {
         /*
          * Read structures bits count
          */
         bingo_index.getSectionBitsCount(_currentSection, bits_count);
         /*
          * Prepare min max bounds
          */
         mangoSimilarityGetBitMinMaxBoundsArray(bits_count.size(), bits_count.ptr(), &min_bounds, &max_bounds);

         /*
          * Prepare common bits array
          */
         common_ones.resize(bits_count.size());
         common_ones.zerofill();
         /*
          * Iterate through the query bits
          */
         int iteration_idx = 0;
         int fp_count = query_data.bitEnd();
         for (int fp_idx = query_data.bitBegin(); fp_idx != query_data.bitEnd() && possible_str_count > 0; fp_idx = query_data.bitNext(fp_idx)) {
            int fp_block = query_data.getBit(fp_idx);
            /*
             * Copy passed structures on each iteration step
             */
            screening_bitset.copy(_sectionBitset);

            /*
             * Get commons in fingerprint buffer
             */
            bingo_index.andWithBitset(_currentSection, fp_block, screening_bitset);
            int screen_idx = screening_bitset.begin();
            for (; screen_idx != screening_bitset.end() && possible_str_count > 0; screen_idx = screening_bitset.next(screen_idx)) {
               /*
                * Calculate new common ones
                */
               int& one_counter = common_ones[screen_idx];
               ++one_counter;
               /*
                * If common ones is out of bounds then it is not passed the screening
                */
               if ((one_counter > max_bounds[screen_idx]) || ((one_counter + fp_count - iteration_idx) < min_bounds[screen_idx])) {
                  _sectionBitset.set(screen_idx, false);
                  --possible_str_count;
               }
            }
            ++iteration_idx;
         }

         /*
          * Screen the last time for all the possible structures
          */
         int screen_idx = _sectionBitset.begin();
         for (; screen_idx != _sectionBitset.end() && possible_str_count > 0; screen_idx = _sectionBitset.next(screen_idx)) {
            int& one_counter = common_ones[screen_idx];
            if ((one_counter > max_bounds[screen_idx]) || (one_counter < min_bounds[screen_idx])) {
               /*
                * Not passed screening
                */
               _sectionBitset.set(screen_idx, false);
               --possible_str_count;
            }
         }

      }

      /*
       * If bitset is not null then matches are found
       */
      if (_sectionBitset.bitsNumber() > 0) {
         /*
          * Set first match as an answer
          */
         if(_fetchForNext()) {
            setItemPointer(result_ptr);
            /*
             * Set fetch found to return on the next steps
             */
            _fetchFound = true;
            return true;
         }

      }
   }

   /*
    * No matches or section ends
    */
   return false;
}

void BingoPgSearchEngine::_setBingoContext() {
   bingoSetSessionID(_bingoSession);
   bingoSetContext(0);
}

bool BingoPgSearchEngine::_fetchForNext() {
   /*
    * Seek for next target matched by fp engine
    */
   if(_currentIdx == -1)
      _currentIdx = _sectionBitset.begin();
   else
      _currentIdx = _sectionBitset.next(_currentIdx);

   for (; _currentIdx != _sectionBitset.end(); _currentIdx = _sectionBitset.next(_currentIdx)) {
      /*
       * Match the next target
       */
      if(matchTarget(_currentSection, _currentIdx));
         return true;
   }

   return false;
}