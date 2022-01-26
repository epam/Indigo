#ifndef _BINGO_PG_CONFIG_H__
#define _BINGO_PG_CONFIG_H__

#include "base_cpp/exception.h"
#include "bingo_postgres.h"

namespace indigo
{
    class Scanner;
    class Output;
    namespace bingo_core
    {
        class BingoCore;
    }
} // namespace indigo

class BingoPgConfig
{
public:
    BingoPgConfig(indigo::bingo_core::BingoCore&);
    ~BingoPgConfig()
    {
    }

    void readDefaultConfig(const char* schema_name);
    void updateByIndexConfig(PG_OBJECT index);
    void replaceInsertParameter(uintptr_t name_datum, uintptr_t value_datum);
    void setUpBingoConfiguration();

    void serialize(indigo::Array<char>& config_data);
    void deserialize(void* data, int data_len);
    indigo::bingo_core::BingoCore& bingoCore;

    DECL_ERROR;

private:
    BingoPgConfig(const BingoPgConfig&); // no implicit copy

    void _readTable(uintptr_t id, bool tau);
    int _getNumericValue(indigo::Array<char>& config_data);

    void _replaceInsertTauParameter(uintptr_t rule_datum, uintptr_t beg_datum, uintptr_t end_datum);
    void _toString(int value, indigo::Array<char>&);
    void _optionToString(int option_value, const std::string& option_name);

    std::unordered_map<std::string, indigo::Array<char>> _rawConfig;
    std::unordered_map<std::string, indigo::Array<char>> _stringParams;

    class TauParameter
    {
    public:
        TauParameter(){};
        ~TauParameter(){};
        indigo::Array<char> beg;
        indigo::Array<char> end;
        void serialize(indigo::Scanner*, indigo::Output*);
    };

    std::unordered_map<int, TauParameter> _tauParameters;
};

#endif /* BINGO_PG_CONFIG_H */
