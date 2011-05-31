#include "bingo_pg_config.h"
#include "bingo_pg_common.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "bingo_core_c.h"
#include "pg_bingo_context.h"
#include "bingo_pg_text.h"
#include "bingo_pg_cursor.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "utils/rel.h"
//#include "catalog/namespace.h"
//#include "access/htup.h"
//#include "access/heapam.h"
//#include "storage/lock.h"
//#include "utils/tqual.h"
}

using namespace indigo;

void BingoPgConfig::readDefaultConfig(const char* schema_name) {
   _rawConfig.clear();
   _tauParameters.clear();
   /*
    * Seek for default config table
    */
   {
      BingoPgCursor config_table("SELECT cname, cvalue FROM %s.bingo_config", schema_name);
      while (config_table.next()) {
         Datum name_datum = config_table.getDatum(1);
         Datum value_datum = config_table.getDatum(2);
         replaceInsertParameter(name_datum, value_datum);
      }
   }
   {
      BingoPgCursor config_table("SELECT rule_idx, tau_beg, tau_end FROM %s.bingo_tau_config", schema_name);
      while (config_table.next()) {
         Datum rule_datum = config_table.getDatum(1);
         Datum beg_datum = config_table.getDatum(2);
         Datum end_datum = config_table.getDatum(3);
         _replaceInsertTauParameter(rule_datum, beg_datum, end_datum);
      }
   }


}
//void BingoPgConfig::readDefaultConfig() {
//   _rawConfig.clear();
//   /*
//    * Seek for default config table
//    */
//   Oid config_oid = RelnameGetRelid("bingo.bingo_config");
//   if(config_oid == InvalidOid)
//      elog(ERROR, "could not find config table: 'bingo.bingo_config'");
//   _readTable(config_oid, false);
//
//   Oid tau_oid = RelnameGetRelid("bingo_tau_config");
//   if(tau_oid == InvalidOid)
//      elog(ERROR, "could not find tau config table: 'bingo_tau_config'");
//
//   _readTable(tau_oid, true);
//
//}

void BingoPgConfig::updateByIndexConfig(PG_OBJECT index_ptr) {
   Relation relation = (Relation) index_ptr;

   if(relation->rd_options == 0)
      return;
   BingoStdRdOptions* opt = (BingoStdRdOptions*)relation->rd_options;

   BingoIndexOptions& options = opt->index_parameters;

   //TODO use isset instead of -1 not set for variables
   int name_key;
   if(options.treat_x_as_pseudoatom >= 0) {
      name_key = _rawConfig.findOrInsert("treat_x_as_pseudoatom");
      _toString(options.treat_x_as_pseudoatom, _rawConfig.value(name_key));
   }
   if(options.ignore_closing_bond_direction_mismatch >= 0) {
      name_key = _rawConfig.findOrInsert("ignore_closing_bond_direction_mismatch");
      _toString(options.ignore_closing_bond_direction_mismatch, _rawConfig.value(name_key));
   }
   if(options.fp_any_size >= 0) {
      name_key = _rawConfig.findOrInsert("fp_any_size");
      _toString(options.fp_any_size, _rawConfig.value(name_key));
   }
   if(options.fp_ord_size >= 0) {
      name_key = _rawConfig.findOrInsert("fp_ord_size");
      _toString(options.fp_ord_size, _rawConfig.value(name_key));
   }
   if(options.fp_sim_size >= 0) {
      name_key = _rawConfig.findOrInsert("fp_sim_size");
      _toString(options.fp_sim_size, _rawConfig.value(name_key));
   }
   if(options.fp_tau_size >= 0) {
      name_key = _rawConfig.findOrInsert("fp_tau_size");
      _toString(options.fp_tau_size, _rawConfig.value(name_key));
   }
   if(options.sim_screening_pass_mark >= 0) {
      name_key = _rawConfig.findOrInsert("sim_screening_pass_mark");
      _toString(options.sim_screening_pass_mark, _rawConfig.value(name_key));
   }
   if(options.sub_screening_max_bits >= 0) {
      name_key = _rawConfig.findOrInsert("sub_screening_max_bits");
      _toString(options.sub_screening_max_bits, _rawConfig.value(name_key));
   }

}

void BingoPgConfig::replaceInsertParameter(uintptr_t  name_datum, uintptr_t  value_datum) {
   /*
    * Name and value are strings
    */
   BingoPgText pname_text(name_datum);
   BingoPgText value_text(value_datum);

   int name_key = _rawConfig.findOrInsert(pname_text.getString());
   
   _rawConfig.value(name_key).readString(value_text.getString(), false);
}


void BingoPgConfig::setUpBingoConfiguration() {
   if(_rawConfig.size() == 0)
      throw Error("configuration not set yet");
   
   /*
    * Iterate through all the configs
    */
   for (int c_idx = _rawConfig.begin(); c_idx != _rawConfig.end(); c_idx = _rawConfig.next(c_idx)) {
      bingoSetConfigInt(_rawConfig.key(c_idx), _getNumericValue(c_idx));
   }

   for (int c_idx = _tauParameters.begin(); c_idx != _tauParameters.end(); c_idx = _tauParameters.next(c_idx)) {
      TauParameter& param =  _tauParameters.value(c_idx);
      bingoAddTautomerRule(_tauParameters.key(c_idx), param.beg.ptr(), param.end.ptr());
   }
}

void BingoPgConfig::serialize(indigo::Array<char>& config_data) {
   ArrayOutput data_out(config_data);
   BingoPgCommon::DataProcessing::handleRedBlackStringArr(_rawConfig, 0, &data_out);
   BingoPgCommon::DataProcessing::handleRedBlackObject(_tauParameters, 0, &data_out);
}

void BingoPgConfig::deserialize(void* data, int data_len) {
   BufferScanner data_in((char*)data, data_len);
   BingoPgCommon::DataProcessing::handleRedBlackStringArr(_rawConfig, &data_in, 0);
   BingoPgCommon::DataProcessing::handleRedBlackObject(_tauParameters, &data_in, 0);
}
//void BingoPgConfig::_readTable(unsigned int id, bool tau) {
//   HeapTuple config_tuple;
//   Relation config_rel = heap_open(id, AccessShareLock);
//   TupleDesc tupdesc = RelationGetDescr(config_rel);
//   int ncolumns = tupdesc->natts;
//
//   if (tau) {
//      if (ncolumns != 3)
//         elog(ERROR, "tau config table should contain 3 columns");
//   } else {
//      if (ncolumns != 2)
//         elog(ERROR, "config table should contain 2 columns");
//   }
//
//   /*
//    * Start iterate through the table
//    */
//   HeapScanDesc scan = heap_beginscan(config_rel, SnapshotNow, 0, NULL);
//
//   while ((config_tuple = heap_getnext(scan, ForwardScanDirection)) != NULL) {
//
//      Datum *values = (Datum *) palloc(ncolumns * sizeof (Datum));
//      bool *nulls = (bool *) palloc(ncolumns * sizeof (bool));
//
//      /*
//       *  Break down the tuple into fields
//       */
//      heap_deform_tuple(config_tuple, tupdesc, values, nulls);
//
//      /*
//       * Read variable name and value
//       */
//      if(tau)
//         _replaceInsertTauParameter(values[0], values[1], values[2]);
//      else
//         replaceInsertParameter(values[0], values[1]);
//
//      pfree(values);
//      pfree(nulls);
//
//   }
//   /*
//    * Close table
//    */
//   heap_endscan(scan);
//   heap_close(config_rel, AccessShareLock);
//}

int BingoPgConfig::_getNumericValue(int c_idx) {
   BufferScanner scanner(_rawConfig.value(c_idx));
   return scanner.readInt();
}

void BingoPgConfig::_replaceInsertTauParameter(uintptr_t  rule_datum, uintptr_t  beg_datum, uintptr_t  end_datum) {
   _tauParameters.clear();
   /*
    * tau parameter rule integer = begin string : end string
    */
   int rule_idx = DatumGetInt32(rule_datum);
   BingoPgText beg_text(beg_datum);
   BingoPgText end_text(end_datum);

   TauParameter& param = _tauParameters.findOrInsert(rule_idx);
   
   param.beg.readString(beg_text.getString(), true);
   param.end.readString(end_text.getString(), true);
}

void BingoPgConfig::_toString(int value, Array<char>& a) {
   ArrayOutput ao(a);
   ao.printf("%d", value);
}

void BingoPgConfig::TauParameter::serialize(Scanner* scanner, Output* output) {
   BingoPgCommon::DataProcessing::handleArray(this->beg, scanner, output);
   BingoPgCommon::DataProcessing::handleArray(this->end, scanner, output);
}
