#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "fmgr.h"
#include "postgres.h"
}

#include "bingo_pg_fix_post.h"

#include "bingo_core_c.h"
#include "bingo_pg_common.h"
#include "bingo_pg_text.h"
#include "bingo_postgres.h"

extern "C"
{
    BINGO_FUNCTION_EXPORT(aam);

    BINGO_FUNCTION_EXPORT(rxnfile);

    BINGO_FUNCTION_EXPORT(rcml);

    BINGO_FUNCTION_EXPORT(checkreaction);

    BINGO_FUNCTION_EXPORT(rsmiles);

    BINGO_FUNCTION_EXPORT(rfingerprint);

    BINGO_FUNCTION_EXPORT(compactreaction);
}

Datum aam(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);
    Datum mode_datum = PG_GETARG_DATUM(1);

    void* result = 0;

    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("aam");

        BingoPgText react_text(react_datum);
        BingoPgText aam_mode(mode_datum);

        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        const char* bingo_result = ringoAAM(react_buf, buf_size, aam_mode.getString());
        CORE_HANDLE_REJECT_WARNING(bingo_result == 0, "aam", PG_RETURN_NULL());

        BingoPgText result_text;
        result_text.initFromString(bingo_result);
        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum rxnfile(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {

        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("rxnfile");

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        const char* bingo_result = ringoRxnfile(react_buf, buf_size);
        CORE_HANDLE_REJECT_WARNING(bingo_result == 0, "rxnfile", PG_RETURN_NULL());

        BingoPgText result_text;
        result_text.initFromString(bingo_result);
        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum rcml(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("rcml");

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        const char* bingo_result = ringoRCML(react_buf, buf_size);
        CORE_HANDLE_REJECT_WARNING(bingo_result == 0, "rcml", PG_RETURN_NULL());

        BingoPgText result_text;
        result_text.initFromString(bingo_result);
        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum checkreaction(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("checkreaction");

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        const char* bingo_result = ringoCheckReaction(react_buf, buf_size);
        if (bingo_result == 0)
            PG_RETURN_NULL();

        BingoPgText result_text;
        result_text.initFromString(bingo_result);
        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum rsmiles(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("rsmiles");

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        const char* bingo_result = ringoRSMILES(react_buf, buf_size);
        CORE_HANDLE_REJECT_WARNING(bingo_result == 0, "rsmiles", PG_RETURN_NULL());

        BingoPgText result_text;
        result_text.initFromString(bingo_result);
        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum rfingerprint(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);
    Datum options_datum = PG_GETARG_DATUM(1);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("rfingerprint");

        BingoPgText r_text(react_datum);
        BingoPgText react_options(options_datum);

        int buf_size;
        const char* r_buf = r_text.getText(buf_size);

        int res_buf;
        const char* bingo_result = ringoFingerprint(r_buf, buf_size, react_options.getString(), &res_buf);
        CORE_HANDLE_REJECT_WARNING(bingo_result == 0, "rfingerprint", PG_RETURN_NULL());

        BingoPgText result_data;
        result_data.initFromBuffer(bingo_result, res_buf);

        result = result_data.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(result);
}

Datum compactreaction(PG_FUNCTION_ARGS)
{
    Datum react_datum = PG_GETARG_DATUM(0);
    Datum options_xyz = PG_GETARG_BOOL(1);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("compactreaction");

        BingoPgText r_text(react_datum);

        int buf_size;
        const char* r_buf = r_text.getText(buf_size);

        int res_buf;
        const char* bingo_result = ringoICR(r_buf, buf_size, options_xyz, &res_buf);
        CORE_HANDLE_REJECT_WARNING(bingo_result == 0, "compactreaction", PG_RETURN_NULL());

        BingoPgText result_data;
        result_data.initFromBuffer(bingo_result, res_buf);

        result = result_data.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(result);
}