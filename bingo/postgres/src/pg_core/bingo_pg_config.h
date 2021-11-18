#ifndef _BINGO_PG_CONFIG_H__
#define _BINGO_PG_CONFIG_H__

#include <base_cpp/exception.h>
#include <base_cpp/red_black.h>

#include <bingo_postgres.h>

namespace indigo
{
    class Scanner;
    class Output;
} // namespace indigo

class BingoPgConfig
{
public:
    BingoPgConfig();
    ~BingoPgConfig()
    {
    }

    void readDefaultConfig(const char* schema_name);
    void updateByIndexConfig(PG_OBJECT index);
    void replaceInsertParameter(uintptr_t name_datum, uintptr_t value_datum);
    void setUpBingoConfiguration();

    void serialize(indigo::Array<char>& config_data);
    void deserialize(void* data, int data_len);

    DECL_ERROR;

private:
    BingoPgConfig(const BingoPgConfig&); // no implicit copy

    void _readTable(uintptr_t id, bool tau);
    int _getNumericValue(int c_idx);

    void _replaceInsertTauParameter(uintptr_t rule_datum, uintptr_t beg_datum, uintptr_t end_datum);
    void _toString(int value, indigo::Array<char>&);

    indigo::RedBlackStringObjMap<indigo::Array<char>> _rawConfig;
    indigo::RedBlackStringObjMap<indigo::Array<char>> _stringParams;

    class TauParameter
    {
    public:
        TauParameter(){};
        ~TauParameter(){};
        indigo::Array<char> beg;
        indigo::Array<char> end;
        void serialize(indigo::Scanner*, indigo::Output*);
    };

    indigo::RedBlackObjMap<int, TauParameter> _tauParameters;
};

#endif /* BINGO_PG_CONFIG_H */
