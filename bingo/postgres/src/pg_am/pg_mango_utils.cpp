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
    BINGO_FUNCTION_EXPORT(smiles);

    BINGO_FUNCTION_EXPORT(cansmiles);

    BINGO_FUNCTION_EXPORT(molfile);

    BINGO_FUNCTION_EXPORT(cml);

    BINGO_FUNCTION_EXPORT(checkmolecule);

    BINGO_FUNCTION_EXPORT(gross);

    BINGO_FUNCTION_EXPORT(getweight);

    BINGO_FUNCTION_EXPORT(getmass);

    BINGO_FUNCTION_EXPORT(fingerprint);

    BINGO_FUNCTION_EXPORT(compactmolecule);

    BINGO_FUNCTION_EXPORT(inchi);

    BINGO_FUNCTION_EXPORT(inchikey);

    BINGO_FUNCTION_EXPORT(standardize);
}

Datum smiles(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("smiles");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoSMILES(mol_buf, buf_size, 0);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("smiles", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum cansmiles(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("cansmiles");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoSMILES(mol_buf, buf_size, 1);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("cansmiles", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum molfile(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("molfile");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoMolfile(mol_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("molfile", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum cml(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("cml");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoCML(mol_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("cml", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum checkmolecule(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("checkmolecule");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoCheckMolecule(mol_buf, buf_size);
            if (bingo_result == 0) {
                PG_RETURN_NULL();
            }

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("checkmolecule", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum gross(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("gross");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoGross(mol_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("gross", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum getweight(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    Datum options_datum = PG_GETARG_DATUM(1);
    double result = 0;

    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("getweight");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        BingoPgText mol_options(options_datum);

        int buf_len;
        const char* buf = mol_text.getText(buf_len);
        try {
            bingoCore.mangoMassD(buf, buf_len, mol_options.getString(), &result);
        } CORE_CATCH_REJECT_WARNING("getweight", PG_RETURN_NULL())
    }
    PG_BINGO_END

    PG_RETURN_FLOAT8(result);
}

Datum getmass(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);

    double result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("getmass");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        int buf_len;
        const char* buf = mol_text.getText(buf_len);
        try {
            bingoCore.mangoMassD(buf, buf_len, 0, &result);
        } CORE_CATCH_REJECT_WARNING("getmass", PG_RETURN_NULL())
    }
    PG_BINGO_END

    PG_RETURN_FLOAT8(result);
}

Datum fingerprint(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    Datum options_datum = PG_GETARG_DATUM(1);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("fingerprint");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        BingoPgText mol_options(options_datum);

        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);

        int res_buf;
        try {
            const char* bingo_result = bingoCore.mangoFingerprint(mol_buf, buf_size, mol_options.getString(), &res_buf);

            BingoPgText result_data;
            result_data.initFromBuffer(bingo_result, res_buf);

            result = result_data.release();
        } CORE_CATCH_REJECT_WARNING("fingerprint", PG_RETURN_NULL())
        
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(result);
}

Datum compactmolecule(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    bool options_xyz = PG_GETARG_BOOL(1);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("compactmolecule");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);

        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);

        int res_buf;
        try {
            const char* bingo_result = bingoCore.mangoICM(mol_buf, buf_size, options_xyz, &res_buf);

            BingoPgText result_data;
            result_data.initFromBuffer(bingo_result, res_buf);

            result = result_data.release();
        } CORE_CATCH_REJECT_WARNING("compactmolecule", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(result);
}

Datum inchi(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    Datum options_datum = PG_GETARG_DATUM(1);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("inchi");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        BingoPgText mol_options(options_datum);

        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);

        int res_buf;
        try {
            const char* bingo_result = bingoCore.mangoInChI(mol_buf, buf_size, mol_options.getString(), &res_buf);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("inchi", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum inchikey(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("inchikey");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        const char* mol_buf = mol_text.getString();
        try {
            const char* bingo_result = bingoCore.mangoInChIKey(mol_buf);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("inchikey", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum standardize(PG_FUNCTION_ARGS)
{
    Datum mol_datum = PG_GETARG_DATUM(0);
    Datum options_datum = PG_GETARG_DATUM(1);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("standardize");
        auto& bingoCore = bingo_handler.bingoCore;

        BingoPgText mol_text(mol_datum);
        BingoPgText st_options(options_datum);

        int buf_size;
        const char* mol_buf = mol_text.getText(buf_size);
        try {
            const char* bingo_result = bingoCore.mangoStandardize(mol_buf, buf_size, st_options.getString());

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_REJECT_WARNING("standardize", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}
