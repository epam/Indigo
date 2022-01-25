#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "fmgr.h"
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText react_text(react_datum);
        BingoPgText aam_mode(mode_datum);

        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        try
        {
            const char* bingo_result = bingoCore.ringoAAM(react_buf, buf_size, aam_mode.getString());

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        }
        CORE_CATCH_REJECT_WARNING("aam", PG_RETURN_NULL())
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        try
        {
            const char* bingo_result = bingoCore.ringoRxnfile(react_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        }
        CORE_CATCH_REJECT_WARNING("rxnfile", PG_RETURN_NULL())
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        try
        {
            const char* bingo_result = bingoCore.ringoRCML(react_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        }
        CORE_CATCH_REJECT_WARNING("rcml", PG_RETURN_NULL())
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        try
        {
            const char* bingo_result = bingoCore.ringoCheckReaction(react_buf, buf_size);
            if (bingo_result == 0)
            {
                PG_RETURN_NULL();
            }
            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        }
        CORE_CATCH_REJECT_WARNING("checkreaction", PG_RETURN_NULL())
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText react_text(react_datum);
        int buf_size;
        const char* react_buf = react_text.getText(buf_size);
        try
        {
            const char* bingo_result = bingoCore.ringoRSMILES(react_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        }
        CORE_CATCH_REJECT_WARNING("rsmiles", PG_RETURN_NULL())
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText r_text(react_datum);
        BingoPgText react_options(options_datum);

        int buf_size;
        const char* r_buf = r_text.getText(buf_size);

        int res_buf;
        try
        {
            const char* bingo_result = bingoCore.ringoFingerprint(r_buf, buf_size, react_options.getString(), &res_buf);

            BingoPgText result_data;
            result_data.initFromBuffer(bingo_result, res_buf);

            result = result_data.release();
        }
        CORE_CATCH_REJECT_WARNING("rfingerprint", PG_RETURN_NULL())
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
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText r_text(react_datum);

        int buf_size;
        const char* r_buf = r_text.getText(buf_size);

        int res_buf;
        try
        {
            const char* bingo_result = bingoCore.ringoICR(r_buf, buf_size, options_xyz, &res_buf);

            BingoPgText result_data;
            result_data.initFromBuffer(bingo_result, res_buf);

            result = result_data.release();
        }
        CORE_CATCH_REJECT_WARNING("compactreaction", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(result);
}
