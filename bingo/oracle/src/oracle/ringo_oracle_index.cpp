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

#include "base_cpp/output.h"
#include "graph/embedding_enumerator.h"
#include "molecule/cmf_saver.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_pi_systems_matcher.h"
#include "molecule/molfile_loader.h"
#include "oracle/bingo_fingerprints.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"
#include "oracle/ringo_fetch_context.h"
#include "oracle/ringo_oracle.h"
#include "oracle/rowid_saver.h"
#include "reaction/crf_saver.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rxnfile_loader.h"
#include <memory>

bool _ringoRegisterReaction(OracleEnv& env, const char* rowid, const Array<char>& reaction_buf, RingoOracleContext& context, RingoIndex& index,
                            BingoFingerprints& fingerprints)
{
    QS_DEF(Array<char>, data);
    QS_DEF(Array<char>, compressed_rowid);
    ArrayOutput output(data);

    output.writeChar(0); // 0 -- present, 1 -- removed from index

    ArrayOutput rid_output(compressed_rowid);
    RowIDSaver rid_saver(context.context().rid_dict, rid_output);

    rid_saver.saveRowID(rowid);

    output.writeByte((byte)compressed_rowid.size());
    output.writeArray(compressed_rowid);

    TRY_READ_TARGET_RXN
    {
        BufferScanner scanner(reaction_buf);

        try
        {
            index.prepare(scanner, output, NULL);
        }
        catch (CmfSaver::Error& e)
        {
            if (context.context().reject_invalid_structures)
                throw; // Rethrow this exception further
            env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message());
            return false;
        }
        catch (CrfSaver::Error& e)
        {
            if (context.context().reject_invalid_structures)
                throw; // Rethrow this exception further
            env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message());
            return false;
        }
    }
    CATCH_READ_TARGET_RXN_ROWID(rowid, {
        if (context.context().reject_invalid_structures)
            throw; // Rethrow this exception further
        return false;
    });

    // some magic: round it up to avoid ora-22282
    if (data.size() % 2 == 1)
        output.writeChar(0);

    int blockno, offset;

    context.context().storage.add(env, data, blockno, offset);

    fingerprints.addFingerprint(env, index.getFingerprint());

    context.shadow_table.addReaction(env, index, rowid, blockno + 1, offset);
    return true;
}

void ringoRegisterTable(OracleEnv& env, RingoOracleContext& context, const char* source_table, const char* source_column, const char* target_datatype)
{
    QS_DEF(Array<char>, reaction_buf);
    OracleStatement statement(env);
    std::unique_ptr<OracleLOB> reaction_lob;
    OraRowidText rowid;
    char varchar2_text[4001];

    // Oracle's BLOB and CLOB types always come uppercase
    bool blob = (strcmp(target_datatype, "BLOB") == 0);
    bool clob = (strcmp(target_datatype, "CLOB") == 0);

    int total_count = 0;

    OracleStatement::executeSingleInt(total_count, env, "SELECT COUNT(*) FROM %s WHERE %s IS NOT NULL AND LENGTH(%s) > 0", source_table, source_column,
                                      source_column);

    context.context().longOpInit(env, total_count, "Building reaction index", source_table, "reactions");

    statement.append("SELECT %s, RowidToChar(rowid) FROM %s WHERE %s IS NOT NULL AND LENGTH(%s) > 0", source_column, source_table, source_column,
                     source_column);

    statement.prepare();

    if (blob)
    {
        reaction_lob = std::make_unique<OracleLOB>(env);
        statement.defineBlobByPos(1, *reaction_lob);
    }
    else if (clob)
    {
        reaction_lob = std::make_unique<OracleLOB>(env);
        statement.defineClobByPos(1, *reaction_lob);
    }
    else
        statement.defineStringByPos(1, varchar2_text, sizeof(varchar2_text));

    statement.defineStringByPos(2, rowid.ptr(), sizeof(rowid));

    BingoFingerprints& fingerprints = context.fingerprints;
    int nthreads = 0;

    fingerprints.validateForUpdate(env);
    context.context().configGetInt(env, "NTHREADS", nthreads);

    nthreads = 1;

    // if (nthreads == 1)
    {
        int n = 0;

        QS_DEF(RingoIndex, index);
        index.init(context.context());

        if (statement.executeAllowNoData())
            do
            {
                env.dbgPrintf("inserting reaction #%d with rowid %s\n", n, rowid.ptr());

                if (blob || clob)
                    reaction_lob->readAll(reaction_buf, false);
                else
                    reaction_buf.readString(varchar2_text, false);

                try
                {
                    if (_ringoRegisterReaction(env, rowid.ptr(), reaction_buf, context, index, fingerprints))
                        n++;
                }
                catch (Exception& ex)
                {
                    char buf[4096];
                    snprintf(buf, NELEM(buf), "Failed on record with rowid=%s. Error message is '%s'", rowid.ptr(), ex.message());

                    throw Exception(buf);
                }

                if ((n % 50) == 0)
                    context.context().longOpUpdate(env, n);

                if ((n % 1000) == 0)
                {
                    env.dbgPrintfTS("done %d reactions ; flushing\n", n);
                    context.context().storage.flush(env);
                }
            } while (statement.fetch());

        fingerprints.flush(env);
    }
}

ORAEXT void oraRingoCreateIndex(OCIExtProcContext* ctx, int context_id, const char* params, short params_ind, const char* full_table_name,
                                short full_table_name_ind, const char* column_name, short column_name_ind, const char* column_data_type,
                                short column_data_type_ind){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintfTS("Creating index\n");

BingoOracleContext& bcontext = BingoOracleContext::get(env, context_id, false, 0);

// parse parameters before creating RingoOracleContext because
// it creates the BingoSreening according to fingerprintSizeNoTau()
if (params_ind == OCI_IND_NOTNULL)
    bcontext.parseParameters(env, params);

RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);

BingoStorage& storage = context.context().storage;
BingoFingerprints& fingerprints = context.fingerprints;

context.shadow_table.drop(env);
context.shadow_table.create(env);

fingerprints.drop(env);
fingerprints.create(env);
storage.drop(env);
storage.create(env);
storage.validateForInsert(env);

ringoRegisterTable(env, context, full_table_name, column_name, column_data_type);

storage.finish(env);
context.context().saveCmfDict(env);
context.context().saveRidDict(env);
OracleStatement::executeSingle(env, "COMMIT");
RingoFetchContext::removeByContextID(context_id);
RingoContext::remove(context_id);
BingoContext::remove(context_id);
}
ORABLOCK_END
}

ORAEXT void oraRingoDropIndex(OCIExtProcContext* ctx, int context_id){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintfTS("Dropping index #%d\n", context_id);
RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);

context.shadow_table.drop(env);
context.context().storage.drop(env);
context.fingerprints.drop(env);

RingoFetchContext::removeByContextID(context_id);
RingoContext::remove(context_id);
BingoContext::remove(context_id);

// TEMP: remove CMF dictionary
OracleStatement::executeSingle(env, "DELETE FROM CONFIG_BLOB WHERE n=%d", context_id);
}
ORABLOCK_END
}

ORAEXT void oraRingoTruncateIndex(OCIExtProcContext* ctx, int context_id){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintfTS("Truncating index #%d\n", context_id);
RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);

context.shadow_table.truncate(env);
context.context().storage.truncate(env);
context.fingerprints.truncate(env);
RingoFetchContext::removeByContextID(context_id);

RingoContext::remove(context_id);
BingoContext::remove(context_id);
}
ORABLOCK_END
}

ORAEXT void oraRingoIndexInsert(OCIExtProcContext* ctx, int context_id, const char* rowid, short rowid_ind, OCILobLocator* target_loc,
                                short target_ind){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (rowid_ind != OCI_IND_NOTNULL)
    throw BingoError("null rowid given");

if (target_ind != OCI_IND_NOTNULL)
    // somebody added a NULL value into the table; ignore it
    return;

RingoOracleContext& context = RingoOracleContext::get(env, context_id, true);

env.dbgPrintf("inserting reaction with rowid %s\n", rowid);

QS_DEF(RingoIndex, index);
index.init(context.context());
BingoFingerprints& fingerprints = context.fingerprints;
BingoStorage& storage = context.context().storage;

storage.lock(env);
storage.validateForInsert(env);

fingerprints.validateForUpdate(env);

QS_DEF(Array<char>, target_buf);

OracleLOB target_lob(env, target_loc);

target_lob.readAll(target_buf, false);

_ringoRegisterReaction(env, rowid, target_buf, context, index, fingerprints);

storage.finish(env);
// fingerprints.flush(env);

if (context.context().cmf_dict.isModified())
    context.context().saveCmfDict(env);

if (context.context().rid_dict.isModified())
    context.context().saveRidDict(env);
}
ORABLOCK_END
}

ORAEXT void oraRingoIndexDelete(OCIExtProcContext* ctx, int context_id, const char* rowid, short rowid_ind){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (rowid_ind != OCI_IND_NOTNULL)
    throw BingoError("null rowid given");

RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);

int blockno, offset;

if (context.shadow_table.getReactionLocation(env, rowid, blockno, offset))
{
    env.dbgPrintf("deleting reaction that has rowid %s\n", rowid);

    BingoStorage& storage = context.context().storage;

    storage.lock(env);
    storage.markRemoved(env, blockno, offset);

    context.shadow_table.deleteReaction(env, rowid);
}
else
    env.dbgPrintf("reaction with rowid %s not found\n", rowid);
}
ORABLOCK_END
}

ORAEXT void oraRingoFlushInserts(OCIExtProcContext* ctx, int commit)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);
        int i;

        for (i = RingoContext::begin(); i != RingoContext::end(); i = RingoContext::next(i))
        {
            env.dbgPrintfTS("flushing inserts of context #%d\n", i);

            RingoOracleContext& context = RingoOracleContext::get(env, i, false);

            context.fingerprints.flush(env);
            if (commit)
                OracleStatement::executeSingle(env, "COMMIT");
            context.context().unlock(env);
        }
    }
    ORABLOCK_END
}
