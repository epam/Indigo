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

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "base_cpp/scanner.h"
#include "oracle/ora_wrap.h"
#include "oracle/bingo_oracle.h"

#include "layout/molecule_layout.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/cml_saver.h"
#include "molecule/icm_loader.h"
#include "molecule/icm_saver.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"

#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

#include "molecule/elements.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_oracle.h"

#include "base_cpp/cancellation_handler.h"
#include "molecule/inchi_wrapper.h"

static void _mangoUpdateMolecule(Molecule& target, const char* options, BingoOracleContext& context)
{
    if (strlen(options) > 0)
    {
        AromaticityOptions opt;
        opt.unique_dearomatization = false;

        if (strcasecmp(options, "aromatize") == 0)
            target.aromatize(opt);
        else if (strcasecmp(options, "dearomatize") == 0)
            target.dearomatize(opt);
        else
            throw BingoError("unsupport options: %s. Can be either 'aromatize' or 'dearomatize'", options);
    }
}

static OCIString* _mangoSMILES(OracleEnv& env, const std::string& target_buf, const char* options, BingoOracleContext& context, bool canonical)
{
    QS_DEF(Molecule, target);

    profTimerStart(tload, "smiles.load_molecule");
    MoleculeAutoLoader loader(target_buf);
    context.setLoaderSettings(loader);
    loader.loadMolecule(target);
    profTimerStop(tload);

    _mangoUpdateMolecule(target, options, context);
    AutoPtr<CancellationHandler> handler(nullptr);
    if (context.timeout > 0)
    {
        handler.reset(new TimeoutCancellationHandler(context.timeout));
    }
    AutoCancellationHandler auto_handler(handler.release());

    if (canonical)
        MoleculeAromatizer::aromatizeBonds(target, AromaticityOptions::BASIC);

    QS_DEF(std::string, smiles);

    StringOutput out(smiles);

    if (canonical)
    {
        profTimerStart(tload, "smiles.cano_saver");

        CanonicalSmilesSaver saver(out);

        saver.saveMolecule(target);
    }
    else
    {
        profTimerStart(tload, "smiles.saver");

        SmilesSaver saver(out);

        saver.saveMolecule(target);
    }

    if (smiles.size() == 0)
        // Oracle would treat empty string as NULL value.
        // To give it non-NULL, we give it a space (which is correct SMILES)
        smiles += ' ';

    OCIString* result = 0;
    env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)smiles.c_str(), smiles.size(), &result));

    return result;
}

ORAEXT OCIString* oraMangoSMILES(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, const char* options, short options_ind,
                                 short* return_indicator)
{
    OCIString* result = NULL;

    ORA_SAFEBLOCK_BEGIN("smiles")
    {
        OracleEnv env(ctx, logger);

        *return_indicator = OCI_IND_NULL;

        if (options_ind != OCI_IND_NOTNULL)
            options = "";

        if (target_indicator == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();

            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, buf);

            target_lob.readAll(buf, false);

            result = _mangoSMILES(env, buf, options, context, false);
        }

        if (result == 0)
        {
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
        }
        else
            *return_indicator = OCI_IND_NOTNULL;
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCIString* oraMangoCanonicalSMILES(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    profTimersReset();
    profTimerStart(tall, "smiles.all");

    OCIString* result = NULL;

    ORA_SAFEBLOCK_BEGIN("canonicalSmiles")
    {
        OracleEnv env(ctx, logger);

        *return_indicator = OCI_IND_NULL;

        if (target_indicator == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();

            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, buf);

            profTimerStart(treadlob, "smiles.read_lob");
            target_lob.readAll(buf, false);
            profTimerStop(treadlob);

            result = _mangoSMILES(env, buf, "", context, true);
        }

        if (result == 0)
        {
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
        }
        else
            *return_indicator = OCI_IND_NOTNULL;
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCIString* oraMangoCheckMolecule(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    OCIString* result = NULL;

    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        *return_indicator = OCI_IND_NULL;

        if (target_indicator != OCI_IND_NOTNULL)
        {
            static const char* msg = "null molecule given";

            env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)msg, strlen(msg), &result));
            *return_indicator = OCI_IND_NOTNULL;
        }
        else
        {
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, buf);
            QS_DEF(Molecule, mol);

            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

            target_lob.readAll(buf, false);

            TRY_READ_TARGET_MOL
            {
                MoleculeAutoLoader loader(buf);

                context.setLoaderSettings(loader);
                loader.loadMolecule(mol);
                Molecule::checkForConsistency(mol);
            }
            CATCH_READ_TARGET_MOL(OCIStringAssignText(env.envhp(), env.errhp(), (text*)e.message(), strlen(e.message()), &result);
                                  *return_indicator = OCI_IND_NOTNULL;)
            catch (Exception& e)
            {
                char buf[4096];
                snprintf(buf, NELEM(buf), "INTERNAL ERROR: %s", e.message());
                OCIStringAssignText(env.envhp(), env.errhp(), (text*)buf, strlen(buf), &result);
                *return_indicator = OCI_IND_NOTNULL;
            }

            if (*return_indicator == OCI_IND_NULL)
                // This is needed for Oracle 9. Returning NULL drops the extproc.
                OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
        }
    }
    ORABLOCK_END

    return result;
}

void _ICM(BingoOracleContext& context, OracleLOB& target_lob, int save_xyz, std::string& icm)
{
    QS_DEF(std::string, target);
    QS_DEF(Molecule, mol);

    target_lob.readAll(target, false);

    MoleculeAutoLoader loader(target);
    context.setLoaderSettings(loader);
    loader.loadMolecule(mol);

    if ((save_xyz != 0) && !mol.have_xyz)
        throw BingoError("molecule has no XYZ");

    StringOutput output(icm);
    IcmSaver saver(output);

    saver.save_xyz = (save_xyz != 0);
    saver.saveMolecule(mol);
}

ORAEXT OCILobLocator* oraMangoICM(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, int save_xyz, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORA_SAFEBLOCK_BEGIN("ICM")
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (target_indicator == OCI_IND_NOTNULL)
        {
            OracleLOB target_lob(env, target_locator);
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();

            QS_DEF(std::string, icm);

            _ICM(context, target_lob, save_xyz, icm);

            OracleLOB lob(env);

            lob.createTemporaryBLOB();
            lob.write(0, icm);
            lob.doNotDelete();
            result = lob.get();
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT void oraMangoICM2(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, OCILobLocator* result_locator, short result_indicator,
                         int save_xyz){ORA_SAFEBLOCK_BEGIN("ICM2"){OracleEnv env(ctx, logger);

if (target_indicator == OCI_IND_NULL)
    throw BingoError("null molecule given");
if (result_indicator == OCI_IND_NULL)
    throw BingoError("null LOB given");

OracleLOB target_lob(env, target_locator);
BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
block_throw_error = context.reject_invalid_structures.get();

QS_DEF(std::string, icm);

_ICM(context, target_lob, save_xyz, icm);

OracleLOB result_lob(env, result_locator);

result_lob.write(0, icm);
result_lob.trim(icm.size());
}
ORA_SAFEBLOCK_END
}

ORAEXT OCILobLocator* oraMangoMolfile(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, const char* options, short options_ind,
                                      short* return_indicator)
{
    OCILobLocator* result = 0;

    ORA_SAFEBLOCK_BEGIN("molfile")
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (options_ind != OCI_IND_NOTNULL)
            options = "";

        if (target_indicator == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, target);
            QS_DEF(std::string, icm);
            QS_DEF(Molecule, mol);

            target_lob.readAll(target, false);

            MoleculeAutoLoader loader(target);
            context.setLoaderSettings(loader);
            loader.loadMolecule(mol);

            _mangoUpdateMolecule(mol, options, context);

            if (!mol.have_xyz)
            {
                MoleculeLayout layout(mol);

                layout.make();
                mol.clearBondDirections();
                mol.stereocenters.markBonds();
                mol.allene_stereo.markBonds();
            }

            StringOutput output(icm);
            MolfileSaver saver(output);

            saver.saveMolecule(mol);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, icm);
            lob.doNotDelete();
            result = lob.get();
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCILobLocator* oraMangoCML(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORA_SAFEBLOCK_BEGIN("cml")
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (target_indicator == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, target);
            QS_DEF(std::string, icm);
            QS_DEF(Molecule, mol);

            target_lob.readAll(target, false);

            MoleculeAutoLoader loader(target);
            context.setLoaderSettings(loader);
            loader.loadMolecule(mol);

            if (!mol.have_xyz)
            {
                MoleculeLayout layout(mol);

                layout.make();
                mol.clearBondDirections();
                mol.stereocenters.markBonds();
                mol.allene_stereo.markBonds();
            }

            StringOutput output(icm);
            CmlSaver saver(output);

            saver.saveMolecule(mol);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, icm);
            lob.doNotDelete();
            result = lob.get();
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCILobLocator* oraMangoInchi(OCIExtProcContext* ctx, OCILobLocator* target_loc, short target_ind, const char* options, short options_ind,
                                    short* return_ind)
{
    OCILobLocator* result = NULL;

    ORA_SAFEBLOCK_BEGIN("inchi")
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (options_ind != OCI_IND_NOTNULL)
            options = "";

        if (target_ind == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();

            QS_DEF(std::string, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);

            QS_DEF(Molecule, target);

            MoleculeAutoLoader loader(target_buf);
            context.setLoaderSettings(loader);
            loader.loadMolecule(target);

            QS_DEF(std::string, inchi);

            InchiWrapper inchi_calc;
            inchi_calc.setOptions(options);
            inchi_calc.saveMoleculeIntoInchi(target, inchi);

            OracleLOB lob(env);
            lob.createTemporaryCLOB();
            lob.write(0, inchi);
            lob.doNotDelete();
            result = lob.get();
            *return_ind = OCI_IND_NOTNULL;
        }
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCIString* oraMangoInchiKey(OCIExtProcContext* ctx, OCILobLocator* inchi_loc, short inchi_ind, short* return_ind)
{
    OCIString* result = NULL;

    ORA_SAFEBLOCK_BEGIN("inchikey")
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (inchi_ind == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();

            QS_DEF(std::string, inchi);
            OracleLOB inchi_lob(env, inchi_loc);
            inchi_lob.readAll(inchi, true);

            QS_DEF(std::string, inchikey_buf);

            InchiWrapper::InChIKey(inchi.c_str(), inchikey_buf);

            env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)inchikey_buf.data(), inchikey_buf.size(), &result));
        }

        if (result != 0)
            *return_ind = OCI_IND_NOTNULL;
    }
    ORA_SAFEBLOCK_END

    return result;
}

ORAEXT OCILobLocator* oraMangoFingerprint(OCIExtProcContext* ctx, OCILobLocator* target_loc, short target_ind, const char* options, short options_ind,
                                          short* return_ind)
{
    OCILobLocator* result = NULL;

    ORA_SAFEBLOCK_BEGIN("fingerprint")
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (options_ind != OCI_IND_NOTNULL)
            options = "";

        if (target_ind == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);
            block_throw_error = context.reject_invalid_structures.get();

            QS_DEF(std::string, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);

            QS_DEF(Molecule, target);

            MoleculeAutoLoader loader(target_buf);
            context.setLoaderSettings(loader);
            loader.loadMolecule(target);

            MoleculeFingerprintBuilder builder(target, context.fp_parameters);
            builder.parseFingerprintType(options, false);

            builder.process();

            const char* buf = (const char*)builder.get();
            int buf_len = context.fp_parameters.fingerprintSize();

            OracleLOB lob(env);

            lob.createTemporaryBLOB();
            lob.write(0, buf, buf_len);
            lob.doNotDelete();
            result = lob.get();

            *return_ind = OCI_IND_NOTNULL;
        }
    }
    ORA_SAFEBLOCK_END

    return result;
}
