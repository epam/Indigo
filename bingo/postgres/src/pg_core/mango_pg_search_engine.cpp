#include "mango_pg_search_engine.h"

#include "bingo_core_c.h"
#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include "bingo_pg_text.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"

extern "C" {
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

void MangoPgFpData::insertHash(dword hash, int c_cnt) {
   int *comp_count = _hashes.at2(hash);
   if(comp_count) {
      (*comp_count) += c_cnt;
   } else {
      _hashes.insert(hash, c_cnt);
   }
}

void MangoPgFpData::setGrossStr(const char* gross_str, const char* counter_str) {
   _gross.readString("'", true);
   _gross.appendString(gross_str, true);
   _gross.appendString("'", true);
   _gross.appendString(counter_str, true);
}

MangoPgSearchEngine::MangoPgSearchEngine(BingoPgConfig& bingo_config, const char* rel_name):
BingoPgSearchEngine(),
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

MangoPgSearchEngine::~MangoPgSearchEngine() {
   bingoIndexEnd();
}

bool MangoPgSearchEngine::matchTarget(int section_idx, int structure_idx) {
   bool result = false;
   QS_DEF(Array<char>, mol_buf);
   QS_DEF(Array<char>, xyz_buf);
   mol_buf.clear();
   xyz_buf.clear();

   if(_searchType == BingoPgCommon::MOL_SUB || _searchType == BingoPgCommon::MOL_EXACT || _searchType == BingoPgCommon::MOL_SMARTS) {
      _bufferIndexPtr->readCmfItem(section_idx, structure_idx, mol_buf);
      if(mangoNeedCoords()) {
         _bufferIndexPtr->readXyzItem(section_idx, structure_idx, xyz_buf);
      }
      result = (mangoMatchTargetBinary(mol_buf.ptr(), mol_buf.sizeInBytes(), xyz_buf.ptr(), xyz_buf.sizeInBytes()) == 1);
   } else if(_searchType == BingoPgCommon::MOL_GROSS) {
      BingoPgText gross_text;
      _searchCursor->getText(2, gross_text);

      int gross_len;
      gross_text.getText(gross_len);

      result = mangoMatchTarget(gross_text.getString(), gross_len) == 1;
   } else if(_searchType == BingoPgCommon::MOL_MASS || _searchType == BingoPgCommon::MOL_SIM) {
      return true;
   }
   return result;
}

void MangoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT scan_desc_ptr) {

   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;

   if(scan_desc->numberOfKeys >= 1 && scan_desc->numberOfKeys <= 2) {
      _searchType = scan_desc->keyData[0].sk_strategy;
   } else {
      throw Error("unsupported number of scan keys %d", scan_desc->numberOfKeys);
   }
   /*
    * Set pseudo types
    */
   if(_searchType == BingoPgCommon::MOL_MASS_GREAT || _searchType == BingoPgCommon::MOL_MASS_LESS)
      _searchType = BingoPgCommon::MOL_MASS;
   
   _queryFpData.reset(new MangoPgFpData());

   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   BingoPgSearchEngine::prepareQuerySearch(bingo_idx, scan_desc);

   loadDictionary(bingo_idx);

   switch(_searchType) {
      case BingoPgCommon::MOL_SUB:
         _prepareSubSearch(scan_desc);
         break;
      case BingoPgCommon::MOL_EXACT:
         _prepareExactSearch(scan_desc);
         break;
      case BingoPgCommon::MOL_GROSS:
         _prepareGrossSearch(scan_desc);
         break;
      case BingoPgCommon::MOL_SMARTS:
         _prepareSmartsSearch(scan_desc);
         break;
      case BingoPgCommon::MOL_MASS:
         _prepareMassSearch(scan_desc);
         break;
      case BingoPgCommon::MOL_SIM:
         _prepareSimSearch(scan_desc);
         break;
      default:
         throw Error("unsupported search type %d", _searchType);
         break;
   }

}

bool MangoPgSearchEngine::searchNext(PG_OBJECT result_ptr) {

   bool result = false;
   if (_searchType == BingoPgCommon::MOL_EXACT || _searchType == BingoPgCommon::MOL_GROSS || _searchType == BingoPgCommon::MOL_MASS) {
      result = _searchNextCursor(result_ptr);
   } else if(_searchType == BingoPgCommon::MOL_SUB || _searchType == BingoPgCommon::MOL_SMARTS) {
      result = _searchNextSub(result_ptr);
   } else if(_searchType == BingoPgCommon::MOL_SIM) {
      result = _searchNextSim(result_ptr);
   }

   return result;
}

void MangoPgSearchEngine::_errorHandler(const char* message, void*) {
   throw Error("Error while searching a molecule: %s", message);
}

void MangoPgSearchEngine::_prepareExactQueryStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str, indigo::Array<char>& where_clause_str) {
   int hash_elements_count, count;
   dword hash;

   ArrayOutput what_clause(what_clause_str);
   ArrayOutput from_clause(from_clause_str);
   ArrayOutput where_clause(where_clause_str);

   what_clause.printf("sh.b_id");

   mangoGetHash(false, -1, &hash_elements_count, &hash);

   from_clause.printf("%s sh", _shadowRelName.ptr());

   for (int i = 0; i < hash_elements_count; i++)
      from_clause.printf(", %s t%d", _shadowHashRelName.ptr(), i);

   /*
    * Create complex WHERE clause
    */

   bool where_was_added = false;
   if (hash_elements_count > 0) {
      where_was_added = true;
      /*
       * molecule ids must be same
       */
      where_clause.printf("sh.b_id = t0.b_id AND ");
      for (int i = 1; i < hash_elements_count; i++)
         where_clause.printf("t%d.b_id = t%d.b_id AND ", i - 1, i);
      /*
       * query components must match target components
       */
      for (int i = 0; i < hash_elements_count; i++) {
         mangoGetHash(false, i, &count, &hash);
         where_clause.printf("t%d.ex_hash = %d AND ", i, hash);
      }

      /*
       * components count mast must target components count
       */
      Array<char> rel;
      if (mangoExactNeedComponentMatching())
         rel.readString(">=", true);
      else
         rel.readString("=", true);

      for (int i = 0; i < hash_elements_count; i++) {
         if (i != 0)
            where_clause.printf("AND ");
         mangoGetHash(false, i, &count, &hash);
         where_clause.printf("t%d.f_count %s %d ", i, rel.ptr(), count);
      }
   }
   if (!mangoExactNeedComponentMatching()) {
      if (where_was_added)
         where_clause.printf("AND ");

      /*
       * There must be no other components in target
       */
      int query_fragments_count = 0;
      for (int i = 0; i < hash_elements_count; i++) {
         mangoGetHash(false, i, &count, &hash);
         query_fragments_count += count;
      }
      where_clause.printf("sh.fragments = %d", query_fragments_count);
   }
   what_clause_str.push(0);
   from_clause_str.push(0);
   where_clause_str.push(0);

}
void MangoPgSearchEngine::_prepareExactTauStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str, indigo::Array<char>& where_clause_str) {
   ArrayOutput what_clause(what_clause_str);
   ArrayOutput from_clause(from_clause_str);
   ArrayOutput where_clause(where_clause_str);

   what_clause.printf("b_id");
   from_clause.printf("%s", _shadowRelName.ptr());

   const char* query_gross = mangoTauGetQueryGross();
   where_clause.printf("gross='%s' OR gross LIKE '%s H%%'", query_gross, query_gross);

   what_clause_str.push(0);
   from_clause_str.push(0);
   where_clause_str.push(0);

}

void MangoPgSearchEngine::_prepareSubSearch(PG_OBJECT scan_desc_ptr) {
   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;
   QS_DEF(Array<char>, search_type);
   BingoPgText search_query;
   BingoPgText search_options;
   BingoPgFpData& data = _queryFpData.ref();

   BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

   _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);

   /*
    * Set up matching parameters
    */
   if(mangoSetupMatch(search_type.ptr(), search_query.getString(), search_options.getString()) < 0)
      throw Error("Can not set search context: %s", bingoGetError());


   const char* fingerprint_buf;
   int fp_len;

   mangoGetQueryFingerprint(&fingerprint_buf, &fp_len);

   int size_bits = fp_len * 8;
   data.setFingerPrints(fingerprint_buf, size_bits);

}

void MangoPgSearchEngine::_prepareExactSearch(PG_OBJECT scan_desc_ptr) {
   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;
   QS_DEF(Array<char>, what_clause);
   QS_DEF(Array<char>, from_clause);
   QS_DEF(Array<char>, where_clause);
   QS_DEF(Array<char>, search_type);
   BingoPgText search_query;
   BingoPgText search_options;

   BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

   _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);

   /*
    * Set up matching parameters
    */
   if(mangoSetupMatch(search_type.ptr(), search_query.getString(), search_options.getString()) < 0)
      throw Error("Can not set search context: %s", bingoGetError());

   if (strcasestr(search_options.getString(), "TAU") != 0) {
      _prepareExactTauStrings(what_clause, from_clause, where_clause);
   } else {
      _prepareExactQueryStrings(what_clause, from_clause, where_clause);
   }

   _searchCursor.reset(new BingoPgCursor("SELECT %s FROM %s WHERE %s",what_clause.ptr(), from_clause.ptr(), where_clause.ptr()));
}

void MangoPgSearchEngine::_prepareGrossSearch(PG_OBJECT scan_desc_ptr) {
   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;

   QS_DEF(Array<char>, gross_query);
   QS_DEF(Array<char>, search_type);
   BingoPgText search_sigh;
   BingoPgText search_mol;

   BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

   _getScanQueries(scan_desc->keyData[0].sk_argument, search_sigh, search_mol);

   gross_query.readString(search_sigh.getString(), true);
   gross_query.appendString(" ", true);
   gross_query.appendString(search_mol.getString(), true);

   /*
    * Set up matching parameters
    */
   if(mangoSetupMatch(search_type.ptr(), gross_query.ptr(), 0) < 0)
      throw Error("Can not set search context: %s", bingoGetError());

   _searchCursor.reset(new BingoPgCursor("SELECT b_id, gross FROM %s WHERE %s", _shadowRelName.ptr(), mangoGrossGetConditions()));
}

void MangoPgSearchEngine::_prepareSmartsSearch(PG_OBJECT scan_desc_ptr) {
   _prepareSubSearch(scan_desc_ptr);
}

void MangoPgSearchEngine::_prepareMassSearch(PG_OBJECT scan_desc_ptr) {
   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;
   bool min_mass_flag = false;
   bool max_mass_flag = false;
   float min_mass = 0;
   float max_mass = FLT_MAX;
   QS_DEF(Array<char>, where_clause_str);
   ArrayOutput where_clause(where_clause_str);

   for (int arg_idx = 0; arg_idx < scan_desc->numberOfKeys; ++arg_idx) {
      ScanKeyData& key_data = scan_desc->keyData[arg_idx];
      if(key_data.sk_strategy == BingoPgCommon::MOL_MASS_LESS) {
         char* mass_string = DatumGetCString(key_data.sk_argument);
         BufferScanner mass_scanner(mass_string);
         max_mass = mass_scanner.readFloat();
         max_mass_flag = true;
      } else if(key_data.sk_strategy == BingoPgCommon::MOL_MASS_GREAT) {
         char* mass_string = DatumGetCString(key_data.sk_argument);
         BufferScanner mass_scanner(mass_string);
         min_mass = mass_scanner.readFloat();
         min_mass_flag = true;
      } else {
         throw Error("unsupported search mass and other types");
      }
   }

   if(min_mass_flag)
      where_clause.printf("mass > %f", min_mass);
   if(max_mass_flag)
      where_clause.printf("AND mass < %f", max_mass);
   where_clause.writeChar(0);

   _searchCursor.reset(new BingoPgCursor("SELECT b_id FROM %s WHERE %s",_shadowRelName.ptr(), where_clause_str.ptr()));

}

void MangoPgSearchEngine::_prepareSimSearch(PG_OBJECT scan_desc_ptr) {
   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;
   QS_DEF(Array<char>, search_type);
   BingoPgText search_query;
   BingoPgText search_options;
   float min_bound = 0, max_bound = 1;
   BingoPgFpData& data = _queryFpData.ref();

   BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

   _getScanQueries(scan_desc->keyData[0].sk_argument, min_bound, max_bound, search_query, search_options);
            /*
    * Set up matching parameters
    */
   if (mangoSetupMatch(search_type.ptr(), search_query.getString(), search_options.getString()) < 0)
      throw Error("Can not set search context: %s", bingoGetError());

   if(min_bound > max_bound)
      throw Error("min bound %f can not be greater then max bound %f", min_bound, max_bound);
   
   mangoSimilaritySetMinMaxBounds(min_bound, max_bound);

   const char* fingerprint_buf;
   int fp_len;

   mangoGetQueryFingerprint(&fingerprint_buf, &fp_len);

   int size_bits = fp_len * 8;
   data.setFingerPrints(fingerprint_buf, size_bits);

}

void MangoPgSearchEngine::_getScanQueries(uintptr_t arg_datum, BingoPgText& str1, BingoPgText& str2) {
   /*
    * Get query info
    */
   BINGO_PG_TRY
   {
      HeapTupleHeader query_data = DatumGetHeapTupleHeader(arg_datum);
      Oid tupType = HeapTupleHeaderGetTypeId(query_data);
      int32 tupTypmod = HeapTupleHeaderGetTypMod(query_data);
      TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
      int ncolumns = tupdesc->natts;

      if (ncolumns != 2)
         throw Error("internal error: expecting two columns in query but was %d", ncolumns);

      HeapTupleData tuple;
      /*
       * Build a temporary HeapTuple control structure
       */
      tuple.t_len = HeapTupleHeaderGetDatumLength(query_data);
      ItemPointerSetInvalid(&(tuple.t_self));
      tuple.t_tableOid = InvalidOid;
      tuple.t_data = query_data;

      Datum *values = (Datum *) palloc(ncolumns * sizeof (Datum));
      bool *nulls = (bool *) palloc(ncolumns * sizeof (bool));

      /*
       *  Break down the tuple into fields
       */
      heap_deform_tuple(&tuple, tupdesc, values, nulls);

      /*
       * Query tuple consist of query and options
       */
      str1.init(values[0]);
      str2.init(values[1]);

      pfree(values);
      pfree(nulls);
      ReleaseTupleDesc(tupdesc);
   }
   BINGO_PG_HANDLE(throw Error("internal error: can not get scan query: %s", message));
}

void MangoPgSearchEngine::_getScanQueries(uintptr_t  arg_datum, float& min_bound, float& max_bound, BingoPgText& str1, BingoPgText& str2) {
   /*
    * Get query info
    */
   BINGO_PG_TRY
   {
      HeapTupleHeader query_data = DatumGetHeapTupleHeader(arg_datum);
      Oid tupType = HeapTupleHeaderGetTypeId(query_data);
      int32 tupTypmod = HeapTupleHeaderGetTypMod(query_data);
      TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
      int ncolumns = tupdesc->natts;

      if (ncolumns != 4)
         throw Error("internal error: expecting four columns in query but was %d", ncolumns);

      HeapTupleData tuple;
      /*
       * Build a temporary HeapTuple control structure
       */
      tuple.t_len = HeapTupleHeaderGetDatumLength(query_data);
      ItemPointerSetInvalid(&(tuple.t_self));
      tuple.t_tableOid = InvalidOid;
      tuple.t_data = query_data;

      Datum *values = (Datum *) palloc(ncolumns * sizeof (Datum));
      bool *nulls = (bool *) palloc(ncolumns * sizeof (bool));

      /*
       *  Break down the tuple into fields
       */
      heap_deform_tuple(&tuple, tupdesc, values, nulls);

      /*
       * Query tuple consist of query and options
       */
      min_bound = DatumGetFloat4(values[0]);
      max_bound = DatumGetFloat4(values[1]);
      str1.init(values[2]);
      str2.init(values[3]);

      pfree(values);
      pfree(nulls);
      ReleaseTupleDesc(tupdesc);
   }
   BINGO_PG_HANDLE(throw Error("internal error: can not get scan query: %s", message));
}

bool MangoPgSearchEngine::_searchNextSim(PG_OBJECT result_ptr) {
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