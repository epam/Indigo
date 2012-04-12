extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "access/itup.h"
}
#ifdef qsort
#undef qsort
#endif

#ifdef printf
#undef printf
#endif

#include "bingo_pg_search_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/array.h"
#include "bingo_core_c.h"

#include "bingo_pg_text.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"


using namespace indigo;

void BingoPgFpData::setTidItem(PG_OBJECT item_ptr) {

   ItemPointerData& item_p = *(ItemPointer) item_ptr;
   _mapData.tid_map = (ItemPointerData&)item_p;
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

void BingoPgSearchEngine::loadDictionary(BingoPgIndex& bingo_index) {
   _setBingoContext();

   QS_DEF(Array<char>, dict);
   bingo_index.readDictionary(dict);
   bingoSetConfigBin("cmf_dict", dict.ptr(), dict.sizeInBytes());
}

const char* BingoPgSearchEngine::getDictionary(int& size) {
   _setBingoContext();

   const char* dict_buf;

   bingoGetConfigBin("cmf-dict", &dict_buf, &size);

   return dict_buf;
}

bool BingoPgSearchEngine::matchTarget(ItemPointerData& item_data) {
   return matchTarget(ItemPointerGetBlockNumber(&item_data), ItemPointerGetOffsetNumber(&item_data));
}

void BingoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT) {
   _bufferIndexPtr = &bingo_idx;
   _currentSection = _bufferIndexPtr->readBegin();
   _currentIdx = -1;
   _fetchFound = false;
}

bool BingoPgSearchEngine::_searchNextCursor(PG_OBJECT result_ptr) {
   ItemPointerData cmf_item;
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
    * If there are matches found on the previous steps
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
         for (int fp_idx = query_data.bitBegin(); fp_idx != query_data.bitEnd() && _sectionBitset.hasBits(); fp_idx = query_data.bitNext(fp_idx)) {
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
      if (_sectionBitset.hasBits()) {
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
      if(matchTarget(_currentSection, _currentIdx))
         return true;
   }

   return false;
}