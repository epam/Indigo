#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "fmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "bingo_pg_common.h"
#include "bingo_postgres.h"

#include "bingo_core_c.h"
#include "bingo_pg_text.h"
#include "pg_bingo_context.h"

extern "C"
{
    BINGO_FUNCTION_EXPORT(_sub_internal);

    BINGO_FUNCTION_EXPORT(_smarts_internal);

    BINGO_FUNCTION_EXPORT(_exact_internal);

    BINGO_FUNCTION_EXPORT(getsimilarity);

    BINGO_FUNCTION_EXPORT(_gross_internal);

    BINGO_FUNCTION_EXPORT(_sim_internal);

    BINGO_FUNCTION_EXPORT(_match_mass_less);

    BINGO_FUNCTION_EXPORT(_match_mass_great);

    BINGO_FUNCTION_EXPORT(_mass_in);

    BINGO_FUNCTION_EXPORT(_mass_out);
}

using namespace indigo;

/*
 * Helper class for searching setup and perfoming
 */
class _MangoContextHandler : public BingoPgCommon::BingoSessionHandler
{
public:
    _MangoContextHandler(int type, unsigned int func_oid) : BingoSessionHandler(func_oid), _type(type)
    {
        BingoPgCommon::getSearchTypeString(_type, _typeStr, true);
        if (_typeStr.size() == 0) {
            _typeStr.readString("", true);
        }
        setFunctionName(_typeStr.ptr());
        _errorStr.readString("Error while bingo", true);
        _errorStr.appendString(_typeStr.ptr(), true);
        _errorStr.appendString(" loading molecule", true);
    }

    ~_MangoContextHandler() override
    {
    }

    /*
     * Match method
     * Returns true if matching is successfull
     * Throws an error if query can not be loaded
     */
    int matchInternal(Datum query_datum, Datum target_datum, Datum options_datum)
    {
        BingoPgText query_text(query_datum);
        BingoPgText target_text(target_datum);
        BingoPgText options_text(options_datum);

        /*
         * Set up match parameters
         */
        try {
            bingoCore.mangoSetupMatch(_typeStr.ptr(), query_text.getString(), options_text.getString());
        } CORE_CATCH_ERROR(_errorStr.ptr())

        int target_size;
        const char* target_data = target_text.getText(target_size);

        QS_DEF(Array<char>, buffer_warn);
        if (_type == BingoPgCommon::MOL_GROSS)
        {
            buffer_warn.readString(_typeStr.ptr(), true);
            const char* mol_name = bingoCore.bingoGetNameCore(target_data, target_size);
            if (mol_name != 0 && strlen(mol_name) > 0)
            {
                buffer_warn.appendString(" molecule with name='", true);
                buffer_warn.appendString(mol_name, true);
                buffer_warn.appendString("'", true);
            }

            setFunctionName(buffer_warn.ptr());
            try {
                target_data = bingoCore.mangoGross(target_data, target_size);
            } CORE_CATCH_WARNING_RETURN("bingo.gross", return -1)
        }
        int res;

        try {
            res = bingoCore.mangoMatchTarget(target_data, target_size);
        } CORE_CATCH_ERROR("Unexpected error during match")
        if (res < 0)
        {
            buffer_warn.readString(bingoCore.warning.ptr(), true);
            const char* mol_name = bingoCore.bingoGetNameCore(target_data, target_size);
            if (mol_name != 0 && strlen(mol_name) > 0)
                elog(WARNING, "warning while bingo%s loading molecule with name ='%s': %s", _typeStr.ptr(), mol_name, buffer_warn.ptr());
            else
                elog(WARNING, "warning while bingo%s loading molecule: %s", _typeStr.ptr(), buffer_warn.ptr());
        }

        return res;
    }

private:
    _MangoContextHandler(const _MangoContextHandler&); // no implicit copy
    int _type;
    indigo::Array<char> _typeStr;
    indigo::Array<char> _errorStr;
};

Datum _sub_internal(PG_FUNCTION_ARGS)
{
    Datum query_datum = PG_GETARG_DATUM(0);
    Datum target_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);
    int result = 0;
    PG_BINGO_BEGIN
    {
        _MangoContextHandler bingo_context(BingoPgCommon::MOL_SUB, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result > 0);
}

Datum _smarts_internal(PG_FUNCTION_ARGS)
{
    Datum query_datum = PG_GETARG_DATUM(0);
    Datum target_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);
    int result = 0;
    PG_BINGO_BEGIN
    {
        _MangoContextHandler bingo_context(BingoPgCommon::MOL_SMARTS, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END
    PG_RETURN_BOOL(result > 0);
}

Datum _exact_internal(PG_FUNCTION_ARGS)
{
    Datum query_datum = PG_GETARG_DATUM(0);
    Datum target_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);
    int result = 0;
    PG_BINGO_BEGIN
    {
        _MangoContextHandler bingo_context(BingoPgCommon::MOL_EXACT, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END
    PG_RETURN_BOOL(result > 0);
}

Datum getsimilarity(PG_FUNCTION_ARGS)
{
    Datum target_datum = PG_GETARG_DATUM(0);
    Datum query_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);

    float res = 0;
    PG_BINGO_BEGIN
    {
        int result = 0;
        _MangoContextHandler bingo_context(BingoPgCommon::MOL_SIM, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();

        if (result > 0)
            bingo_context.bingoCore.mangoSimilarityGetScore(&res);
    }
    PG_BINGO_END
    PG_RETURN_FLOAT4(res);
}

Datum _gross_internal(PG_FUNCTION_ARGS)
{
    Datum query_sign = PG_GETARG_DATUM(0);
    Datum query_datum = PG_GETARG_DATUM(1);
    Datum target_datum = PG_GETARG_DATUM(2);

    int result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgText query_text(query_datum);
        BingoPgText sign_text(query_sign);
        QS_DEF(indigo::Array<char>, bingo_query);
        bingo_query.readString(sign_text.getString(), false);
        bingo_query.appendString(" ", false);
        bingo_query.appendString(query_text.getString(), false);

        query_text.initFromArray(bingo_query);

        _MangoContextHandler bingo_context(BingoPgCommon::MOL_GROSS, fcinfo->flinfo->fn_oid);

        result = bingo_context.matchInternal(query_text.getDatum(), target_datum, 0);

        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result > 0);
}

Datum _sim_internal(PG_FUNCTION_ARGS)
{
    float min_bound = PG_GETARG_FLOAT4(0);
    float max_bound = PG_GETARG_FLOAT4(1);
    Datum query_datum = PG_GETARG_DATUM(2);
    Datum target_datum = PG_GETARG_DATUM(3);
    Datum options_datum = PG_GETARG_DATUM(4);
    int result = 0;
    bool res_bool = false;
    PG_BINGO_BEGIN
    {
        _MangoContextHandler bingo_context(BingoPgCommon::MOL_SIM, fcinfo->flinfo->fn_oid);
        float mol_sim = 0;
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);

        if (result < 0)
            PG_RETURN_NULL();

        if (result > 0)
            bingo_context.bingoCore.mangoSimilarityGetScore(&mol_sim);

        res_bool = (mol_sim <= max_bound) && (mol_sim >= min_bound);
    }
    PG_BINGO_END
    PG_RETURN_BOOL(res_bool);
}

Datum _match_mass_less(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    char* mass_datum = PG_GETARG_CSTRING(1);

    bool result = false;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("mass less");

        BufferScanner scanner(mass_datum);
        float usr_mass = scanner.readFloat();

        BingoPgText mol_text(mol_datum);

        float mol_mass = 0;

        int buf_len;
        const char* buf = mol_text.getText(buf_len);

        try {
            bingo_handler.bingoCore.mangoMass(buf, buf_len, 0, &mol_mass);
        } CORE_CATCH_ERROR("mass matcher: error while calculating mass")

        result = mol_mass < usr_mass;
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result);
}

Datum _match_mass_great(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    char* mass_datum = PG_GETARG_CSTRING(1);

    bool result = false;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("mass great");

        BufferScanner scanner(mass_datum);
        float usr_mass = scanner.readFloat();

        BingoPgText mol_text(mol_datum);

        float mol_mass = 0;

        int buf_len;
        const char* buf = mol_text.getText(buf_len);

        try {
            bingo_handler.bingoCore.mangoMass(buf, buf_len, 0, &mol_mass);
        } CORE_CATCH_ERROR("mass matcher: error while calculating mass")

        result = mol_mass > usr_mass;
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result);
}

Datum _mass_in(PG_FUNCTION_ARGS)
{
    char* str = PG_GETARG_CSTRING(0);
    int size = strlen(str) + 1;
    char* result = (char*)palloc(size);
    memcpy(result, str, size);
    PG_RETURN_POINTER(result);
}

Datum _mass_out(PG_FUNCTION_ARGS)
{
    char* str = PG_GETARG_CSTRING(0);
    int size = strlen(str) + 1;
    char* result = (char*)palloc(size);
    memcpy(result, str, size);
    PG_RETURN_CSTRING(result);
}
