#include "ringo_pg_build_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "bingo_core_c.h"

#include "ringo_pg_search_engine.h"
#include "bingo_pg_text.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include <float.h>

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "access/itup.h"
#include "access/htup.h"
#include "access/genam.h"
#include "access/relscan.h"
#include "access/tupdesc.h"
#include "utils/typcache.h"
}

using namespace indigo;

RingoPgBuildEngine::RingoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name):
BingoPgBuildEngine(),
_searchType(-1) {
   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);
   /*
    * Set up bingo configuration
    */
   bingo_config.setUpBingoConfiguration();
   bingoTautomerRulesReady(0,0,0);
   bingoIndexBegin();

   _relName.readString(rel_name, true);
   _shadowRelName.readString(rel_name, true);
   _shadowRelName.appendString("_shadow", true);
}

RingoPgBuildEngine::~RingoPgBuildEngine() {
   bingoIndexEnd();
}

bool RingoPgBuildEngine::processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>& data_ptr) {

   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   data_ptr.reset(new RingoPgFpData());
   RingoPgFpData& data = (RingoPgFpData&)data_ptr.ref();

   int struct_size;
   const char* struct_ptr = struct_text.getText(struct_size);
   /*
    * Set target data
    */
   bingoSetIndexRecordData(0, struct_ptr, struct_size);
   /*
    * Process target
    */
   if(ringoIndexProcessSingleRecord() == 0)
      return false;

   const char* crf_buf;
   int crf_len;
   const char*fp_buf;
   int fp_len;
   /*
    * Get prepared data
    */
   ringoIndexReadPreparedReaction(0, &crf_buf, &crf_len, &fp_buf, &fp_len);

   /*
    * Set hash information
    */
   dword ex_hash;
   ringoGetHash(1, &ex_hash);
   data.setHash(ex_hash);
   
   /*
    * Set common info
    */
   data.setCmf(crf_buf, crf_len);
   data.setFingerPrints(fp_buf, getFpSize());

   return true;
}
void RingoPgBuildEngine::insertShadowInfo(BingoPgFpData& item_data) {
   RingoPgFpData& data = (RingoPgFpData&) item_data;

   const char* shadow_rel_name = _shadowRelName.ptr();
   BingoItemData* tid_ptr = &data.getTidItem();

   BingoPgCommon::executeQuery("INSERT INTO %s(b_id,tid_map,ex_hash) VALUES ("
           "'(%d, %d)'::tid, '(%d, %d)'::tid, %d)",
           shadow_rel_name,
           data.getSectionIdx(), data.getStructureIdx(),
           ItemPointerGetBlockNumber(tid_ptr), ItemPointerGetOffsetNumber(tid_ptr),
           data.getHash());

}

int RingoPgBuildEngine::getFpSize() {
   int result;
   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   bingoGetConfigInt("reaction-fp-size-bytes", &result);

   return result * 8;
}

void RingoPgBuildEngine::prepareShadowInfo() {
   /*
    * Create auxialiry tables
    */
   const char* rel_name = _relName.ptr();
   const char* shadow_rel_name = _shadowRelName.ptr();

   /*
    * Drop table if exists (in case of truncate index)
    */
   if(BingoPgCommon::tableExists(shadow_rel_name)) {
      BingoPgCommon::dropDependency(shadow_rel_name);
      BingoPgCommon::executeQuery("DROP TABLE %s", shadow_rel_name);
   }

   BingoPgCommon::executeQuery("CREATE TABLE %s ("
   "b_id tid,"
   "tid_map tid,"
   "ex_hash integer)", shadow_rel_name);

   /*
    * Create dependency for new tables
    */
   BingoPgCommon::createDependency(shadow_rel_name, rel_name);
}

void RingoPgBuildEngine::finishShadowProcessing() {
   /*
    * Create shadow indexes
    */
   const char* shadow_rel_name = _shadowRelName.ptr();

   BingoPgCommon::executeQuery("CREATE INDEX %s_hash_idx ON %s using hash(ex_hash)", shadow_rel_name, shadow_rel_name);

}


void RingoPgBuildEngine::_errorHandler(const char* message, void*) {
   elog(ERROR, "Error while building reaction index: %s", message);
}

