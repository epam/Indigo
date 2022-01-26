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

#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_cursor.h"
#include "bingo_pg_text.h"
#include "pg_bingo_context.h"

using namespace indigo;

IMPL_ERROR(BingoPgConfig, "bingo postgres config");

BingoPgConfig::BingoPgConfig(bingo_core::BingoCore& bingoCore) : bingoCore(bingoCore)
{
    _stringParams["SIMILARITY_TYPE"];
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
    _optionToString(options.treat_x_as_pseudoatom, "treat_x_as_pseudoatom");
    _optionToString(options.ignore_closing_bond_direction_mismatch, "ignore_closing_bond_direction_mismatch");
    _optionToString(options.ignore_stereocenter_errors, "ignore_stereocenter_errors");
    _optionToString(options.stereochemistry_bidirectional_mode, "stereochemistry_bidirectional_mode");
    _optionToString(options.stereochemistry_detect_haworth_projection, "stereochemistry_detect_haworth_projection");
    _optionToString(options.ignore_cistrans_errors, "ignore_cistrans_errors");
    _optionToString(options.allow_non_unique_dearomatization, "allow_non_unique_dearomatization");
    _optionToString(options.zero_unknown_aromatic_hydrogens, "zero_unknown_aromatic_hydrogens");
    _optionToString(options.reject_invalid_structures, "reject_invalid_structures");
    _optionToString(options.ignore_bad_valence, "ignore_bad_valence");
    _optionToString(options.fp_any_size, "fp_any_size");
    _optionToString(options.fp_ord_size, "fp_ord_size");
    _optionToString(options.fp_sim_size, "fp_sim_size");
    _optionToString(options.fp_tau_size, "fp_tau_size");
    _optionToString(options.sim_screening_pass_mark, "sim_screening_pass_mark");
    _optionToString(options.sub_screening_max_bits, "sub_screening_max_bits");
    _optionToString(options.nthreads, "nthreads");
}

void BingoPgConfig::replaceInsertParameter(uintptr_t name_datum, uintptr_t value_datum)
{
    /*
     * Name and value are strings
     */
    BingoPgText pname_text(name_datum);
    BingoPgText value_text(value_datum);

    if (_rawConfig.find(pname_text.getString()) == _rawConfig.end())
    {
        _rawConfig[pname_text.getString()];
    }

    _rawConfig.at(pname_text.getString()).readString(value_text.getString(), true);
}

void BingoPgConfig::setUpBingoConfiguration()
{
    if (_rawConfig.size() == 0)
        throw Error("configuration not set yet");

    /*
     * Iterate through all the configs
     */
    for (auto& kv : _rawConfig)
    {
        if (_stringParams.find(kv.first) != _stringParams.end())
        {
            bingoCore.bingoSetConfigBin(kv.first.c_str(), kv.second.ptr(), 0);
        }
        else
        {
            bingoCore.bingoSetConfigInt(kv.first.c_str(), _getNumericValue(kv.second));
        }
    }

    for (auto& kv : _tauParameters)
    {
        TauParameter& param = kv.second;
        bingoCore.bingoAddTautomerRule(kv.first, param.beg.ptr(), param.end.ptr());
    }
}

void BingoPgConfig::serialize(indigo::Array<char>& config_data)
{
    ArrayOutput data_out(config_data);
    BingoPgCommon::DataProcessing::handleStringArrayMap(_rawConfig, 0, &data_out);
    BingoPgCommon::DataProcessing::handleTypeObjMap(_tauParameters, 0, &data_out);
}

void BingoPgConfig::deserialize(void* data, int data_len)
{
    BufferScanner data_in((char*)data, data_len);
    BingoPgCommon::DataProcessing::handleStringArrayMap(_rawConfig, &data_in, 0);
    BingoPgCommon::DataProcessing::handleTypeObjMap(_tauParameters, &data_in, 0);
}

int BingoPgConfig::_getNumericValue(indigo::Array<char>& config_data)
{
    BufferScanner scanner(config_data);
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

    if (_tauParameters.find(rule_idx) == _tauParameters.end())
    {
        _tauParameters[rule_idx];
    }
    TauParameter& param = _tauParameters.at(rule_idx);

    param.beg.readString(beg_text.getString(), true);
    param.end.readString(end_text.getString(), true);
}

void BingoPgConfig::_toString(int value, Array<char>& a)
{
    ArrayOutput ao(a);
    ao.printf("%d", value);
}

void BingoPgConfig::_optionToString(int option_value, const std::string& option_name)
{
    if (option_value >= 0)
    {
        if (_rawConfig.find(option_name) == _rawConfig.end())
        {
            _rawConfig[option_name];
        }
        _toString(option_value, _rawConfig.at(option_name));
    }
}

void BingoPgConfig::TauParameter::serialize(Scanner* scanner, Output* output)
{
    BingoPgCommon::DataProcessing::handleArray(this->beg, scanner, output);
    BingoPgCommon::DataProcessing::handleArray(this->end, scanner, output);
}
