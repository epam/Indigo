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
#include "oracle/mango_oracle.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

#include "layout/reaction_layout.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_loader.h"
#include "oracle/bingo_oracle_context.h"
#include "reaction/icr_saver.h"
#include "reaction/reaction.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_cml_saver.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/rsmiles_saver.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/rxnfile_saver.h"
#include "ringo_oracle.h"

static OCIString* _ringoRSMILES(OracleEnv& env, const std::string& target_buf, BingoOracleContext& context)
{
    QS_DEF(Reaction, target);

    ReactionAutoLoader loader(target_buf);
    context.setLoaderSettings(loader);
    loader.loadReaction(target);

    QS_DEF(std::string, rsmiles);

    StringOutput out(rsmiles);

    RSmilesSaver saver(out);

    saver.saveReaction(target);

    OCIString* result = 0;
    env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)rsmiles.ptr(), rsmiles.size(), &result));

    return result;
}

ORAEXT OCIString* oraRingoRSMILES(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    OCIString* result = NULL;

    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);
        BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

        *return_indicator = OCI_IND_NULL;

        if (target_indicator == OCI_IND_NOTNULL)
        {
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, buf);

            target_lob.readAll(buf, false);

            result = _ringoRSMILES(env, buf, context);
        }

        if (result == 0)
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
        else
            *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCIString* oraRingoCheckReaction(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    OCIString* result = NULL;

    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);
        BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

        *return_indicator = OCI_IND_NULL;

        if (target_indicator != OCI_IND_NOTNULL)
        {
            static const char* msg = "null reaction given";

            env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)msg, strlen(msg), &result));
            *return_indicator = OCI_IND_NOTNULL;
        }
        else
        {
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, buf);
            QS_DEF(Reaction, reaction);

            target_lob.readAll(buf, false);

            TRY_READ_TARGET_RXN
            {
                ReactionAutoLoader loader(buf);
                context.setLoaderSettings(loader);
                loader.loadReaction(reaction);
                Reaction::checkForConsistency(reaction);
            }
            CATCH_READ_TARGET_RXN(OCIStringAssignText(env.envhp(), env.errhp(), (text*)e.message(), strlen(e.message()), &result);
                                  *return_indicator = OCI_IND_NOTNULL;);

            if (*return_indicator == OCI_IND_NULL)
                // This is needed for Oracle 9. Returning NULL drops the extproc.
                OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
        }
    }
    ORABLOCK_END

    return result;
}

void _ICR(OracleLOB& target_lob, int save_xyz, std::string& icr, BingoOracleContext& context)
{
    QS_DEF(std::string, target);
    QS_DEF(Reaction, reaction);

    target_lob.readAll(target, false);

    ReactionAutoLoader loader(target);
    context.setLoaderSettings(loader);
    loader.loadReaction(reaction);

    if ((save_xyz != 0) && !Reaction::haveCoord(reaction))
        throw BingoError("reaction has no XYZ");

    StringOutput output(icr);
    IcrSaver saver(output);

    saver.save_xyz = (save_xyz != 0);
    saver.saveReaction(reaction);
}

ORAEXT OCILobLocator* oraRingoICR(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, int save_xyz, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);
        BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

        if (target_indicator == OCI_IND_NOTNULL)
        {
            OracleLOB target_lob(env, target_locator);
            QS_DEF(std::string, icr);

            _ICR(target_lob, save_xyz, icr, context);

            OracleLOB lob(env);

            lob.createTemporaryBLOB();
            lob.write(0, icr);
            lob.doNotDelete();
            result = lob.get();
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORABLOCK_END

    return result;
}

ORAEXT void oraRingoICR2(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, OCILobLocator* result_locator, short result_indicator,
                         int save_xyz){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);
BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

if (target_indicator == OCI_IND_NULL)
    throw BingoError("null reaction given");
if (result_indicator == OCI_IND_NULL)
    throw BingoError("null LOB given");

OracleLOB target_lob(env, target_locator);
QS_DEF(std::string, icr);

_ICR(target_lob, save_xyz, icr, context);

OracleLOB result_lob(env, result_locator);

result_lob.write(0, icr);
result_lob.trim(icr.size());
}
ORABLOCK_END
}

ORAEXT OCILobLocator* oraRingoRxnfile(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);
        BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

        if (target_indicator == OCI_IND_NOTNULL)
        {
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, target);
            QS_DEF(std::string, icm);
            QS_DEF(Reaction, reaction);

            target_lob.readAll(target, false);

            ReactionAutoLoader loader(target);
            context.setLoaderSettings(loader);
            loader.loadReaction(reaction);

            if (!Reaction::haveCoord(reaction))
            {
                ReactionLayout layout(reaction);

                layout.make();
                reaction.markStereocenterBonds();
            }

            StringOutput output(icm);
            RxnfileSaver saver(output);

            saver.saveReaction(reaction);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, icm);
            lob.doNotDelete();
            result = lob.get();
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCILobLocator* oraRingoCML(OCIExtProcContext* ctx, OCILobLocator* target_locator, short target_indicator, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);
        BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

        if (target_indicator == OCI_IND_NOTNULL)
        {
            OracleLOB target_lob(env, target_locator);

            QS_DEF(std::string, target);
            QS_DEF(std::string, icm);
            QS_DEF(Reaction, reaction);

            target_lob.readAll(target, false);

            ReactionAutoLoader loader(target);
            context.setLoaderSettings(loader);
            loader.loadReaction(reaction);

            if (!Reaction::haveCoord(reaction))
            {
                ReactionLayout layout(reaction);

                layout.make();
                reaction.markStereocenterBonds();
            }

            StringOutput output(icm);
            ReactionCmlSaver saver(output);

            saver.saveReaction(reaction);

            OracleLOB lob(env);

            lob.createTemporaryCLOB();
            lob.write(0, icm);
            lob.doNotDelete();
            result = lob.get();
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCILobLocator* oraRingoFingerprint(OCIExtProcContext* ctx, OCILobLocator* target_loc, short target_ind, const char* options, short options_ind,
                                          short* return_ind)
{
    OCILobLocator* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_ind = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (options_ind != OCI_IND_NOTNULL)
            options = "";

        if (target_ind == OCI_IND_NOTNULL)
        {
            BingoOracleContext& context = BingoOracleContext::get(env, 0, false, 0);

            QS_DEF(std::string, target_buf);

            OracleLOB target_lob(env, target_loc);

            target_lob.readAll(target_buf, false);

            QS_DEF(Reaction, target);

            ReactionAutoLoader loader(target_buf);
            context.setLoaderSettings(loader);
            loader.loadReaction(target);

            ReactionFingerprintBuilder builder(target, context.fp_parameters);
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
    ORABLOCK_END

    return result;
}
