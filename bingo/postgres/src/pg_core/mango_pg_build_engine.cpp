extern "C" {
#include "postgres.h"
#include "fmgr.h"
}
#ifdef qsort
#undef qsort
#endif

#include "mango_pg_build_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "bingo_core_c.h"

#include "mango_pg_search_engine.h"
#include "bingo_pg_text.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include <float.h>


using namespace indigo;

MangoPgBuildEngine::MangoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name):
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
   _shadowHashRelName.readString(rel_name, true);
   _shadowHashRelName.appendString("_shadow_hash", true);
}

MangoPgBuildEngine::~MangoPgBuildEngine() {
   bingoIndexEnd();
}

bool MangoPgBuildEngine::processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>& data_ptr) {

   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   data_ptr.reset(new MangoPgFpData());
   MangoPgFpData& data = (MangoPgFpData&)data_ptr.ref();

   int struct_size;
   const char* struct_ptr = struct_text.getText(struct_size);
   /*
    * Set target data
    */
   bingoSetIndexRecordData(0, struct_ptr, struct_size);
   /*
    * Process target
    */
   if(mangoIndexProcessSingleRecord() <= 0)
      return false;

   const char* cmf_buf;
   int cmf_len;
   const char*xyz_buf;
   int xyz_len;
   const char*gross_str;
   const char*counter_elements_str;
   const char*fp_buf;
   int fp_len;
   const char *fp_sim_str;
   float mass;
   int sim_fp_bits_count;
   /*
    * Get prepared data
    */
   mangoIndexReadPreparedMolecule(0, &cmf_buf, &cmf_len, &xyz_buf, &xyz_len,
                 &gross_str, &counter_elements_str, &fp_buf, &fp_len,
                 &fp_sim_str, &mass, &sim_fp_bits_count);


   /*
    * Set gross formula
    */
   data.setGrossStr(gross_str, counter_elements_str);

   /*
    * Set hash information
    */
   dword ex_hash;
   int ex_hash_count;
   mangoGetHash(1, -1, &ex_hash_count, &ex_hash);
   int target_fragments = 0;
   for (int comp_idx = 0; comp_idx < ex_hash_count; ++comp_idx) {
      int comp_count;
      mangoGetHash(1, comp_idx, &comp_count, &ex_hash);
      data.insertHash(ex_hash, comp_count);
      target_fragments += comp_count;
   }
   data.setFragmentsCount(target_fragments);

   /*
    * Set common info
    */
   data.setCmf(cmf_buf, cmf_len);
   data.setXyz(xyz_buf, xyz_len);
   data.setFingerPrints(fp_buf, getFpSize());
   data.setMass(mass);
   data.setBitsCount(sim_fp_bits_count);

   return true;
}

void MangoPgBuildEngine::insertShadowInfo(BingoPgFpData& item_data) {
   MangoPgFpData& data = (MangoPgFpData&)item_data;

   const char* shadow_rel_name = _shadowRelName.ptr();
   const char* shadow_hash_name = _shadowHashRelName.ptr();
   ItemPointerData* tid_ptr = &data.getTidItem();

   BingoPgCommon::executeQuery("INSERT INTO %s(b_id,tid_map,mass,fragments,gross,cnt_C,cnt_N,cnt_O,cnt_P,cnt_S,cnt_H) VALUES ("
   "'(%d, %d)'::tid, '(%d, %d)'::tid, %f, %d, %s)",
           shadow_rel_name,
           data.getSectionIdx(), data.getStructureIdx(),
           ItemPointerGetBlockNumber(tid_ptr), ItemPointerGetOffsetNumber(tid_ptr),
           data.getMass(),
           data.getFragmentsCount(),
           data.getGrossStr());

   const RedBlackMap<dword, int>& hashes = data.getHashes();
   for (int h_idx = hashes.begin(); h_idx != hashes.end(); h_idx = hashes.next(h_idx)) {
      BingoPgCommon::executeQuery("INSERT INTO %s(b_id, ex_hash, f_count) VALUES ('(%d, %d)'::tid, %d, %d)",
              shadow_hash_name,
              data.getSectionIdx(), data.getStructureIdx(),
              hashes.key(h_idx), hashes.value(h_idx));
   }


}

int MangoPgBuildEngine::getFpSize() {
   int result;
   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   bingoGetConfigInt("fp-size-bytes", &result);

   return result * 8;
}

void MangoPgBuildEngine::prepareShadowInfo(const char* schema_name, const char* index_schema) {
   /*
    * Create auxialiry tables
    */
   const char* rel_name = _relName.ptr();
   const char* shadow_rel_name = _shadowRelName.ptr();
   const char* shadow_hash_name = _shadowHashRelName.ptr();

   /*
    * Drop table if exists (in case of truncate index)
    */
   if(BingoPgCommon::tableExists(index_schema, shadow_rel_name)) {
      BingoPgCommon::dropDependency(schema_name, index_schema, shadow_rel_name);
      BingoPgCommon::executeQuery("DROP TABLE %s.%s", index_schema, shadow_rel_name);
   }

   BingoPgCommon::executeQuery("CREATE TABLE %s.%s ("
   "b_id tid,"
   "tid_map tid,"
   "mass real,"
   "fragments integer,"
   "gross text,"
   "cnt_C integer,"
   "cnt_N integer,"
   "cnt_O integer,"
   "cnt_P integer,"
   "cnt_S integer,"
   "cnt_H integer,"
   "xyz bytea)", index_schema, shadow_rel_name);

   if(BingoPgCommon::tableExists(index_schema, shadow_hash_name)) {
      BingoPgCommon::dropDependency(schema_name, index_schema, shadow_hash_name);
      BingoPgCommon::executeQuery("DROP TABLE %s.%s", index_schema, shadow_hash_name);
   }
   BingoPgCommon::executeQuery("CREATE TABLE %s.%s (b_id tid, ex_hash integer, f_count integer)", index_schema, shadow_hash_name);
   /*
    * Create dependency for new tables
    */
   BingoPgCommon::createDependency(schema_name, index_schema, shadow_rel_name, rel_name);
   BingoPgCommon::createDependency(schema_name, index_schema, shadow_hash_name, rel_name);
}

void MangoPgBuildEngine::finishShadowProcessing() {
   /*
    * Create shadow indexes
    */
   const char* shadow_rel_name = _shadowRelName.ptr();
   const char* shadow_hash_rel_name = _shadowHashRelName.ptr();

   BingoPgCommon::executeQuery("CREATE INDEX %s_mass_idx ON %s using hash(mass)", shadow_rel_name,  shadow_rel_name);
   BingoPgCommon::executeQuery("CREATE INDEX %s_cmf_idx ON %s(b_id)", shadow_rel_name,  shadow_rel_name);
   BingoPgCommon::executeQuery("CREATE INDEX %s_hash_idx ON %s using hash(ex_hash)", shadow_hash_rel_name,  shadow_hash_rel_name);

}


void MangoPgBuildEngine::_errorHandler(const char* message, void*) {
   throw BingoPgError("Error while building molecule index: %s", message);
}

