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
    BINGO_FUNCTION_EXPORT(_rsub_internal);

    BINGO_FUNCTION_EXPORT(_rsmarts_internal);

    BINGO_FUNCTION_EXPORT(_rexact_internal);
}

using namespace indigo;

/*
 * Helper class for searching setup and perfoming
 */
class _RingoContextHandler : public BingoPgCommon::BingoSessionHandler
{
public:
    _RingoContextHandler(int type, unsigned int func_oid) : BingoSessionHandler(func_oid), _type(type)
    {
        BingoPgCommon::getSearchTypeString(_type, _typeStr, false);
        setFunctionName(_typeStr.ptr());
    }

    ~_RingoContextHandler() override
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
        int res = ringoSetupMatch(_typeStr.ptr(), query_text.getString(), options_text.getString());
        if (res < 0)
            throw BingoPgError("Error while bingo%s loading a reaction: %s", _typeStr.ptr(), bingoGetError());

        int target_size;
        const char* target_data = target_text.getText(target_size);

        res = ringoMatchTarget(target_data, target_size);

        if (res < 0)
        {
            QS_DEF(Array<char>, buffer_warn);
            buffer_warn.readString(bingoGetWarning(), true);
            const char* react_name = bingoGetNameCore(target_data, target_size);
            if (react_name != 0 && strlen(react_name) > 0)
                elog(WARNING, "warning while bingo%s loading a reaction with name='%s': %s", _typeStr.ptr(), react_name, buffer_warn.ptr());
            else
                elog(WARNING, "warning while bingo%s loading a reaction: %s", _typeStr.ptr(), buffer_warn.ptr());
        }

        return res;
    }

private:
    _RingoContextHandler(const _RingoContextHandler&); // no implicit copy
    int _type;
    indigo::Array<char> _typeStr;
};

Datum _rsub_internal(PG_FUNCTION_ARGS)
{
    Datum query_datum = PG_GETARG_DATUM(0);
    Datum target_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);

    int result = 0;
    PG_BINGO_BEGIN
    {
        _RingoContextHandler bingo_context(BingoPgCommon::REACT_SUB, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result > 0);
}

Datum _rsmarts_internal(PG_FUNCTION_ARGS)
{
    Datum query_datum = PG_GETARG_DATUM(0);
    Datum target_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);

    int result = 0;
    PG_BINGO_BEGIN
    {
        _RingoContextHandler bingo_context(BingoPgCommon::REACT_SMARTS, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result > 0);
}

Datum _rexact_internal(PG_FUNCTION_ARGS)
{
    Datum query_datum = PG_GETARG_DATUM(0);
    Datum target_datum = PG_GETARG_DATUM(1);
    Datum options_datum = PG_GETARG_DATUM(2);

    int result = 0;
    PG_BINGO_BEGIN
    {
        _RingoContextHandler bingo_context(BingoPgCommon::REACT_EXACT, fcinfo->flinfo->fn_oid);
        result = bingo_context.matchInternal(query_datum, target_datum, options_datum);
        if (result < 0)
            PG_RETURN_NULL();
    }
    PG_BINGO_END

    PG_RETURN_BOOL(result > 0);
}
