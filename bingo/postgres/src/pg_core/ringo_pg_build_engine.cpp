#include "ringo_pg_build_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "bingo_core_c.h"

#include "bingo_pg_search_engine.h"
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

//   _relName.readString(rel_name, true);
//   _shadowRelName.readString(rel_name, true);
//   _shadowRelName.appendString("_shadow", true);
//   _shadowHashRelName.readString(rel_name, true);
//   _shadowHashRelName.appendString("_shadow_hash", true);
}

RingoPgBuildEngine::~RingoPgBuildEngine() {
   bingoIndexEnd();
}

bool RingoPgBuildEngine::processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>& data_ptr) {

   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   data_ptr.reset(new BingoPgFpData());
   BingoPgFpData& data = (BingoPgFpData&)data_ptr.ref();

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
//   dword ex_hash;
//   int ex_hash_count;
//   ringoGetHash(1, -1, &ex_hash_count, &ex_hash);
//   int target_fragments = 0;
//   for (int comp_idx = 0; comp_idx < ex_hash_count; ++comp_idx) {
//      int comp_count;
//      RingoGetHash(1, comp_idx, &comp_count, &ex_hash);
//      data.insertHash(ex_hash, comp_count);
//      target_fragments += comp_count;
//   }
//   data.setFragmentsCount(target_fragments);

//   int icm_len;
//   const char* icm_data = RingoICM(struct_ptr, struct_size, false, &icm_len);
//
//   elog(INFO, "icm = %d cmf = %d", icm_len, cmf_len);
   /*
    * Set common info
    */
   data.setCmf(crf_buf, crf_len);
   data.setFingerPrints(fp_buf, getFpSize());

   return true;
}

void RingoPgBuildEngine::insertShadowInfo(BingoPgFpData& item_data) {
//   RingoPgFpData& data = (RingoPgFpData&)item_data;
//
//   const char* shadow_rel_name = _shadowRelName.ptr();
//   const char* shadow_hash_name = _shadowHashRelName.ptr();
//   BingoItemData* tid_ptr = &data.getTidItem();
//
//   BingoPgCommon::executeQuery("INSERT INTO %s(b_id,tid_map,mass,fragments,gross,cnt_C,cnt_N,cnt_O,cnt_P,cnt_S,cnt_H) VALUES ("
//   "'(%d, %d)'::tid, '(%d, %d)'::tid, %f, %d, %s)",
//           shadow_rel_name,
//           data.getSectionIdx(), data.getStructureIdx(),
//           ItemPointerGetBlockNumber(tid_ptr), ItemPointerGetOffsetNumber(tid_ptr),
//           data.getMass(),
//           data.getFragmentsCount(),
//           data.getGrossStr());
//
//   const RedBlackMap<dword, int>& hashes = data.getHashes();
//   for (int h_idx = hashes.begin(); h_idx != hashes.end(); h_idx = hashes.next(h_idx)) {
//      BingoPgCommon::executeQuery("INSERT INTO %s(b_id, ex_hash, f_count) VALUES ('(%d, %d)'::tid, %d, %d)",
//              shadow_hash_name,
//              data.getSectionIdx(), data.getStructureIdx(),
//              hashes.key(h_idx), hashes.value(h_idx));
//   }


}

void RingoPgBuildEngine::prepareShadowInfo() {
//   /*
//    * Create auxialiry tables
//    */
//   const char* rel_name = _relName.ptr();
//   const char* shadow_rel_name = _shadowRelName.ptr();
//   const char* shadow_hash_name = _shadowHashRelName.ptr();
//
//   /*
//    * Drop table if exists (in case of truncate index)
//    */
//   if(BingoPgCommon::tableExists(shadow_rel_name)) {
//      BingoPgCommon::dropDependency(shadow_rel_name);
//      BingoPgCommon::executeQuery("DROP TABLE %s", shadow_rel_name);
//   }
//
//   BingoPgCommon::executeQuery("CREATE TABLE %s ("
//   "b_id tid,"
//   "tid_map tid,"
//   "mass real,"
//   "fragments integer,"
//   "gross text,"
//   "cnt_C integer,"
//   "cnt_N integer,"
//   "cnt_O integer,"
//   "cnt_P integer,"
//   "cnt_S integer,"
//   "cnt_H integer,"
//   "xyz bytea)", shadow_rel_name);
//
//   if(BingoPgCommon::tableExists(shadow_hash_name)) {
//      BingoPgCommon::dropDependency(shadow_hash_name);
//      BingoPgCommon::executeQuery("DROP TABLE %s", shadow_hash_name);
//   }
//   BingoPgCommon::executeQuery("CREATE TABLE %s (b_id tid, ex_hash integer, f_count integer)", shadow_hash_name);
//   /*
//    * Create dependency for new tables
//    */
//   BingoPgCommon::createDependency(shadow_rel_name, rel_name);
//   BingoPgCommon::createDependency(shadow_hash_name, rel_name);
}

void RingoPgBuildEngine::finishShadowProcessing() {
//   /*
//    * Create shadow indexes
//    */
//   const char* shadow_rel_name = _shadowRelName.ptr();
//   const char* shadow_hash_rel_name = _shadowHashRelName.ptr();
//
//   BingoPgCommon::executeQuery("CREATE INDEX %s_mass_idx ON %s using hash(mass)", shadow_rel_name, shadow_rel_name);
//   BingoPgCommon::executeQuery("CREATE INDEX %s_cmf_idx ON %s(b_id)", shadow_rel_name, shadow_rel_name);
//   BingoPgCommon::executeQuery("CREATE INDEX %s_hash_idx ON %s using hash(ex_hash)", shadow_hash_rel_name, shadow_hash_rel_name);

}


void RingoPgBuildEngine::_errorHandler(const char* message, void*) {
   elog(ERROR, "Error while building reaction index: %s", message);
}

