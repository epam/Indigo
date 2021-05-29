#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "fmgr.h"
#include "utils/rel.h"
#include "utils/relcache.h"
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_cursor.h"
#include "bingo_pg_text.h"
#include "pg_bingo_context.h"

using namespace indigo;

IMPL_ERROR(BingoPgConfig, "bingo postgres config");

BingoPgConfig::BingoPgConfig()
{
    _stringParams.insert("SIMILARITY_TYPE");
}

void BingoPgConfig::readDefaultConfig(const char* schema_name)
{
    _rawConfig.clear();
    _tauParameters.clear();
    /*
     * Seek for default config table
     */
    {
        BingoPgCursor config_table("SELECT cname, cvalue FROM %s.bingo_config", schema_name);
        while (config_table.next())
        {
            Datum name_datum = config_table.getDatum(1);
            Datum value_datum = config_table.getDatum(2);
            replaceInsertParameter(name_datum, value_datum);
        }
    }
    {
        BingoPgCursor config_table("SELECT rule_idx, tau_beg, tau_end FROM %s.bingo_tau_config", schema_name);
        while (config_table.next())
        {
            Datum rule_datum = config_table.getDatum(1);
            Datum beg_datum = config_table.getDatum(2);
            Datum end_datum = config_table.getDatum(3);
            _replaceInsertTauParameter(rule_datum, beg_datum, end_datum);
        }
    }
}

void BingoPgConfig::updateByIndexConfig(PG_OBJECT index_ptr)
{
    Relation relation = (Relation)index_ptr;

    if (relation->rd_options == 0)
        return;
    BingoStdRdOptions* opt = (BingoStdRdOptions*)relation->rd_options;

    BingoIndexOptions& options = opt->index_parameters;

    // TODO use isset instead of -1 not set for variables
    int name_key;
    if (options.treat_x_as_pseudoatom >= 0)
    {
        name_key = _rawConfig.findOrInsert("treat_x_as_pseudoatom");
        _toString(options.treat_x_as_pseudoatom, _rawConfig.value(name_key));
    }
    if (options.ignore_closing_bond_direction_mismatch >= 0)
    {
        name_key = _rawConfig.findOrInsert("ignore_closing_bond_direction_mismatch");
        _toString(options.ignore_closing_bond_direction_mismatch, _rawConfig.value(name_key));
    }
    if (options.ignore_stereocenter_errors >= 0)
    {
        name_key = _rawConfig.findOrInsert("ignore_stereocenter_errors");
        _toString(options.ignore_stereocenter_errors, _rawConfig.value(name_key));
    }
    if (options.stereochemistry_bidirectional_mode >= 0)
    {
        name_key = _rawConfig.findOrInsert("stereochemistry_bidirectional_mode");
        _toString(options.stereochemistry_bidirectional_mode, _rawConfig.value(name_key));
    }
    if (options.stereochemistry_detect_haworth_projection >= 0)
    {
        name_key = _rawConfig.findOrInsert("stereochemistry_detect_haworth_projection");
        _toString(options.stereochemistry_detect_haworth_projection, _rawConfig.value(name_key));
    }
    if (options.ignore_cistrans_errors >= 0)
    {
        name_key = _rawConfig.findOrInsert("ignore_cistrans_errors");
        _toString(options.ignore_cistrans_errors, _rawConfig.value(name_key));
    }
    if (options.allow_non_unique_dearomatization >= 0)
    {
        name_key = _rawConfig.findOrInsert("allow_non_unique_dearomatization");
        _toString(options.allow_non_unique_dearomatization, _rawConfig.value(name_key));
    }
    if (options.zero_unknown_aromatic_hydrogens >= 0)
    {
        name_key = _rawConfig.findOrInsert("zero_unknown_aromatic_hydrogens");
        _toString(options.zero_unknown_aromatic_hydrogens, _rawConfig.value(name_key));
    }
    if (options.reject_invalid_structures >= 0)
    {
        name_key = _rawConfig.findOrInsert("reject_invalid_structures");
        _toString(options.reject_invalid_structures, _rawConfig.value(name_key));
    }
    if (options.ignore_bad_valence >= 0)
    {
        name_key = _rawConfig.findOrInsert("ignore_bad_valence");
        _toString(options.ignore_bad_valence, _rawConfig.value(name_key));
    }
    if (options.fp_any_size >= 0)
    {
        name_key = _rawConfig.findOrInsert("fp_any_size");
        _toString(options.fp_any_size, _rawConfig.value(name_key));
    }
    if (options.fp_ord_size >= 0)
    {
        name_key = _rawConfig.findOrInsert("fp_ord_size");
        _toString(options.fp_ord_size, _rawConfig.value(name_key));
    }
    if (options.fp_sim_size >= 0)
    {
        name_key = _rawConfig.findOrInsert("fp_sim_size");
        _toString(options.fp_sim_size, _rawConfig.value(name_key));
    }
    if (options.fp_tau_size >= 0)
    {
        name_key = _rawConfig.findOrInsert("fp_tau_size");
        _toString(options.fp_tau_size, _rawConfig.value(name_key));
    }
    if (options.sim_screening_pass_mark >= 0)
    {
        name_key = _rawConfig.findOrInsert("sim_screening_pass_mark");
        _toString(options.sim_screening_pass_mark, _rawConfig.value(name_key));
    }
    if (options.sub_screening_max_bits >= 0)
    {
        name_key = _rawConfig.findOrInsert("sub_screening_max_bits");
        _toString(options.sub_screening_max_bits, _rawConfig.value(name_key));
    }
    if (options.nthreads >= 0)
    {
        name_key = _rawConfig.findOrInsert("nthreads");
        _toString(options.nthreads, _rawConfig.value(name_key));
    }
}

void BingoPgConfig::replaceInsertParameter(uintptr_t name_datum, uintptr_t value_datum)
{
    /*
     * Name and value are strings
     */
    BingoPgText pname_text(name_datum);
    BingoPgText value_text(value_datum);

    int name_key = _rawConfig.findOrInsert(pname_text.getString());

    _rawConfig.value(name_key).readString(value_text.getString(), true);
}

void BingoPgConfig::setUpBingoConfiguration()
{
    if (_rawConfig.size() == 0)
        throw Error("configuration not set yet");

    /*
     * Iterate through all the configs
     */
    for (int c_idx = _rawConfig.begin(); c_idx != _rawConfig.end(); c_idx = _rawConfig.next(c_idx))
    {
        const char* key = _rawConfig.key(c_idx);
        if (_stringParams.find(_rawConfig.key(c_idx)))
        {
            bingoSetConfigBin(_rawConfig.key(c_idx), _rawConfig.value(c_idx).ptr(), 0);
        }
        else
        {
            bingoSetConfigInt(_rawConfig.key(c_idx), _getNumericValue(c_idx));
        }
    }

    for (int c_idx = _tauParameters.begin(); c_idx != _tauParameters.end(); c_idx = _tauParameters.next(c_idx))
    {
        TauParameter& param = _tauParameters.value(c_idx);
        bingoAddTautomerRule(_tauParameters.key(c_idx), param.beg.ptr(), param.end.ptr());
    }
}

void BingoPgConfig::serialize(indigo::std::string& config_data)
{
    StringOutput data_out(config_data);
    BingoPgCommon::DataProcessing::handleRedBlackStringArr(_rawConfig, 0, &data_out);
    BingoPgCommon::DataProcessing::handleRedBlackObject(_tauParameters, 0, &data_out);
}

void BingoPgConfig::deserialize(void* data, int data_len)
{
    BufferScanner data_in((char*)data, data_len);
    BingoPgCommon::DataProcessing::handleRedBlackStringArr(_rawConfig, &data_in, 0);
    BingoPgCommon::DataProcessing::handleRedBlackObject(_tauParameters, &data_in, 0);
}

int BingoPgConfig::_getNumericValue(int c_idx)
{
    BufferScanner scanner(_rawConfig.value(c_idx));
    return scanner.readInt();
}

void BingoPgConfig::_replaceInsertTauParameter(uintptr_t rule_datum, uintptr_t beg_datum, uintptr_t end_datum)
{
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

void BingoPgConfig::_toString(int value, std::string& a)
{
    StringOutput ao(a);
    ao.printf("%d", value);
}

void BingoPgConfig::TauParameter::serialize(Scanner* scanner, Output* output)
{
    BingoPgCommon::DataProcessing::handleArray(this->beg, scanner, output);
    BingoPgCommon::DataProcessing::handleArray(this->end, scanner, output);
}
