/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "oracle/bingo_oracle.h"

#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "core/mango_matchers.h"
#include "molecule/elements.h"
#include "molecule/icm_loader.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_mass.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_tautomer_matcher.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_oracle.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

using namespace indigo;

static OCINumber* _mangoSub(OracleEnv& env, MangoOracleContext& context, const Array<char>& query_buf, const Array<char>& target_buf, const char* params)
{
    if (context.substructure.parse(params))
    {
        context.substructure.loadQuery(query_buf);

        TRY_READ_TARGET_MOL
        {
            context.substructure.loadTarget(target_buf);
        }
        CATCH_READ_TARGET_MOL(return 0)

        int result = context.substructure.matchLoadedTarget() ? 1 : 0;

        return OracleExtproc::createInt(env, result);
    }

    if (context.tautomer.parseSub(params))
    {
        context.tautomer.loadQuery(query_buf);

        TRY_READ_TARGET_MOL
        {
            context.tautomer.loadTarget(target_buf);
        }
        CATCH_READ_TARGET_MOL(return 0)

        int result = context.tautomer.matchLoadedTarget() ? 1 : 0;

        return OracleExtproc::createInt(env, result);
    }

    throw BingoError("cannot parse parameters");
}

static OCINumber* _mangoExact(OracleEnv& env, MangoOracleContext& context, const Array<char>& query_buf, const Array<char>& target_buf, const char* params)
{
    if (context.exact.parse(params))
    {
        context.exact.loadQuery(query_buf);

        TRY_READ_TARGET_MOL
        {
            context.exact.loadTarget(target_buf);
        }
        CATCH_READ_TARGET_MOL(return 0)

        int result = context.exact.matchLoadedTarget() ? 1 : 0;

        return OracleExtproc::createInt(env, result);
    }

    if (context.tautomer.parseExact(params))
    {
        context.tautomer.loadQuery(query_buf);

        TRY_READ_TARGET_MOL
        {
            context.tautomer.loadTarget(target_buf);
        }
        CATCH_READ_TARGET_MOL(return 0)

        int result = context.tautomer.matchLoadedTarget() ? 1 : 0;

        return OracleExtproc::createInt(env, result);
    }

    throw BingoError("cannot parse parameters");
}

static OCINumber* _mangoSim(OracleEnv& env, MangoOracleContext& context, const Array<char>& query_buf, const Array<char>& target_buf, const char* params)
{
    MangoSimilarity& instance = context.similarity;

    instance.setMetrics(params);
    instance.loadQuery(query_buf);

    TRY_READ_TARGET_MOL
    {
        double result = instance.calc(target_buf);

        return OracleExtproc::createDouble(env, (double)result);
    }
    CATCH_READ_TARGET_MOL(return 0)
}

// used for SUB(), EXACT() and SIM() but not for GROSS()
static OCINumber* _mangoCommon(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, OCILobLocator* query_loc, short query_ind,
                               const char* params, short params_ind, short* return_ind,
                               OCINumber* (*callback)(OracleEnv& env, MangoOracleContext& context, const Array<char>& query_buf, const Array<char>& target_buf,
                                                      const char* params))
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("Null query given");
        if (params_ind != OCI_IND_NOTNULL)
            params = 0;

        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

            QS_DEF(Array<char>, query_buf);
            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);
            OracleLOB query_lob(env, query_loc);

            target_lob.readAll(target_buf, false);
            query_lob.readAll(query_buf, false);

            result = callback(env, context, query_buf, target_buf, params);
        }

        if (result == 0)
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            result = OracleExtproc::createInt(env, 0);
        else
            *return_ind = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCINumber* oraMangoSub(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, OCILobLocator* query_loc, short query_ind,
                              const char* params, short params_ind, short* return_ind)
{
    return _mangoCommon(ctx, context_id, target_loc, target_ind, query_loc, query_ind, params, params_ind, return_ind, _mangoSub);
}

ORAEXT OCINumber* oraMangoSmarts(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, const char* query, short query_ind,
                                 short* return_ind)
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("Null query given");

        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

            QS_DEF(Array<char>, query_buf);
            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);
            query_buf.readString(query, false);

            context.substructure.loadSMARTS(query_buf);

            TRY_READ_TARGET_MOL
            {
                context.substructure.loadTarget(target_buf);
            }
            CATCH_READ_TARGET_MOL(return OracleExtproc::createInt(env, 0))

            int match = context.substructure.matchLoadedTarget() ? 1 : 0;

            result = OracleExtproc::createInt(env, match);
            *return_ind = OCI_IND_NOTNULL;
        }
        else
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            result = OracleExtproc::createInt(env, 0);
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCINumber* oraMangoExact(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, OCILobLocator* query_loc, short query_ind,
                                const char* params, short params_ind, short* return_ind)
{
    return _mangoCommon(ctx, context_id, target_loc, target_ind, query_loc, query_ind, params, params_ind, return_ind, _mangoExact);
}

ORAEXT OCINumber* oraMangoSim(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, OCILobLocator* query_loc, short query_ind,
                              const char* params, short params_ind, short* return_ind)
{
    return _mangoCommon(ctx, context_id, target_loc, target_ind, query_loc, query_ind, params, params_ind, return_ind, _mangoSim);
}

ORAEXT OCILobLocator* oraMangoSubHi(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, OCILobLocator* query_loc,
                                    short query_ind, const char* params, short params_ind, short* return_ind)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        *return_ind = OCI_IND_NULL;

        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("Null query given");
        if (params_ind != OCI_IND_NOTNULL)
            params = 0;

        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

            QS_DEF(Array<char>, query_buf);
            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);
            OracleLOB query_lob(env, query_loc);

            target_lob.readAll(target_buf, false);
            query_lob.readAll(query_buf, false);

            if (context.substructure.parse(params))
            {
                context.substructure.preserve_bonds_on_highlighting = true;
                context.substructure.loadQuery(query_buf);
                context.substructure.loadTarget(target_buf);
                if (!context.substructure.matchLoadedTarget())
                    throw BingoError("SubHi: match not found");

                context.substructure.getHighlightedTarget(target_buf);
            }
            else if (context.tautomer.parseSub(params))
            {
                context.tautomer.preserve_bonds_on_highlighting = true;
                context.tautomer.loadQuery(query_buf);
                context.tautomer.loadTarget(target_buf);
                if (!context.tautomer.matchLoadedTarget())
                    throw BingoError("SubHi: match not found");

                context.tautomer.getHighlightedTarget(target_buf);
            }
            else
                throw BingoError("SubHi: can't parse params '%s'", params);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, target_buf);
            lob.doNotDelete();
            *return_ind = OCI_IND_NOTNULL;
            return lob.get();
        }
    }
    ORABLOCK_END

    return 0;
}

ORAEXT OCILobLocator* oraMangoSmartsHi(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, const char* query, short query_ind,
                                       short* return_ind)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        *return_ind = OCI_IND_NULL;

        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("Null query given");
        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

            QS_DEF(Array<char>, query_buf);
            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);
            query_buf.readString(query, false);

            context.substructure.preserve_bonds_on_highlighting = true;
            context.substructure.loadSMARTS(query_buf);
            context.substructure.loadTarget(target_buf);
            if (!context.substructure.matchLoadedTarget())
                throw BingoError("SmartsHi: match not found");

            context.substructure.getHighlightedTarget(target_buf);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, target_buf);
            lob.doNotDelete();
            *return_ind = OCI_IND_NOTNULL;
            return lob.get();
        }
    }
    ORABLOCK_END

    return 0;
}

ORAEXT OCILobLocator* oraMangoExactHi(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, OCILobLocator* query_loc,
                                      short query_ind, const char* params, short params_ind, short* return_ind)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        *return_ind = OCI_IND_NULL;

        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("Null query given");
        if (params_ind != OCI_IND_NOTNULL)
            params = 0;
        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

            QS_DEF(Array<char>, query_buf);
            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);
            OracleLOB query_lob(env, query_loc);

            target_lob.readAll(target_buf, false);
            query_lob.readAll(query_buf, false);

            if (context.tautomer.parseExact(params))
            {
                context.tautomer.preserve_bonds_on_highlighting = true;
                context.tautomer.loadQuery(query_buf);
                context.tautomer.loadTarget(target_buf);
                if (!context.tautomer.matchLoadedTarget())
                    throw BingoError("ExactHi: match not found");

                context.tautomer.getHighlightedTarget(target_buf);
            }
            else
                throw BingoError("ExactHi: can't parse params '%s'", params);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, target_buf);
            lob.doNotDelete();
            *return_ind = OCI_IND_NOTNULL;
            return lob.get();
        }
    }
    ORABLOCK_END

    return 0;
}

static OCIString* _mangoGrossCalc(OracleEnv& env, MangoOracleContext& context, const Array<char>& target_buf)
{
    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(target_buf);
    context.context().setLoaderSettings(loader);
    loader.loadMolecule(target);

    OCIString* result = 0;

    QS_DEF(Array<int>, gross);
    QS_DEF(Array<char>, gross_str);

    MoleculeGrossFormula::collect(target, gross);
    MoleculeGrossFormula::toString(gross, gross_str);

    if (gross_str.size() == 1)
        // We can not return empty string to Oracle, as empty string is NULL to Oracle.
        // So we return a string containing one space
        env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)" ", 1, &result));
    else
        env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)gross_str.ptr(), gross_str.size() - 1, &result));

    return result;
}

static OCINumber* _mangoGross(OracleEnv& env, MangoOracleContext& context, const Array<char>& target_buf, const char* query)
{
    MangoGross& instance = context.gross;

    instance.parseQuery(query);

    int res = 0;

    TRY_READ_TARGET_MOL
    {
        res = instance.checkMolecule(target_buf);
    }
    CATCH_READ_TARGET_MOL(return 0)

    return OracleExtproc::createInt(env, res ? 1 : 0);
}

ORAEXT OCIString* oraMangoGrossCalc(OCIExtProcContext* ctx, OCILobLocator* target_loc, short target_ind, short* return_ind)
{
    OCIString* result = NULL;

    ORA_SAFEBLOCK_BEGIN("mangoGrossCalc")
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, 0, false);
            block_throw_error = context.context().reject_invalid_structures.get();

            QS_DEF(Array<char>, target_buf);
            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);

            result = _mangoGrossCalc(env, context, target_buf);
            *return_ind = OCI_IND_NOTNULL;
        }
        else
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCINumber* oraMangoGross(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, const char* query, short query_ind,
                                short* return_ind)
{
    OCINumber* result = NULL;

    ORA_SAFEBLOCK_BEGIN("mangoGross")
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("Null query given");

        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);
            block_throw_error = context.context().reject_invalid_structures.get();

            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);

            result = _mangoGross(env, context, target_buf, query);
        }

        if (result == 0)
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            result = OracleExtproc::createInt(env, 0);
        else
            *return_ind = OCI_IND_NOTNULL;
    }
    ORA_SAFEBLOCK_END

    return result;
}

static OCINumber* _mangoMass(OracleEnv& env, MangoOracleContext& context, const Array<char>& target_buf, const char* type)
{
    double molmass = 0;
    TRY_READ_TARGET_MOL
    {
        QS_DEF(Molecule, target);
        BufferScanner scanner(target_buf);
        MoleculeAutoLoader loader(scanner);
        BingoOracleContext& bingo_context = context.context();

        bingo_context.setLoaderSettings(loader);

        loader.skip_3d_chirality = true;
        loader.loadMolecule(target);

        MoleculeMass mass_calulator;
        mass_calulator.relative_atomic_mass_map = &bingo_context.relative_atomic_mass_map;

        if (type == 0 || strcasecmp(type, "molecular-weight") == 0)
            molmass = mass_calulator.molecularWeight(target);
        else if (strcasecmp(type, "most-abundant-mass") == 0)
            molmass = mass_calulator.mostAbundantMass(target);
        else if (strcasecmp(type, "monoisotopic-mass") == 0)
            molmass = mass_calulator.monoisotopicMass(target);
        else
            throw BingoError("unknown mass specifier: %s", type);
    }
    CATCH_READ_TARGET_MOL(return 0)

    return OracleExtproc::createDouble(env, molmass);
}

ORAEXT OCINumber* oraMangoMolecularMass(OCIExtProcContext* ctx, int context_id, OCILobLocator* target_loc, short target_ind, const char* type, short type_ind,
                                        short* return_ind)
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (type_ind != OCI_IND_NOTNULL)
            type = 0;

        if (target_ind == OCI_IND_NOTNULL)
        {
            MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

            QS_DEF(Array<char>, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);

            result = _mangoMass(env, context, target_buf, type);
        }

        if (result == 0)
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            result = OracleExtproc::createDouble(env, 0);
        else
            *return_ind = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}
