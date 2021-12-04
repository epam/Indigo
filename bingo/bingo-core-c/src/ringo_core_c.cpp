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

#include "bingo_core_c_internal.h"

#include "base_cpp/profiling.h"
#include "molecule/cmf_saver.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_saver.h"
#include "reaction/crf_saver.h"
#include "reaction/icr_saver.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_cml_saver.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rsmiles_saver.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/rxnfile_saver.h"

using namespace indigo::bingo_core;

int BingoCore::ringoIndexProcessSingleRecord() {
    BufferScanner scanner(self.index_record_data.ref());

    NullOutput output;

    TRY_READ_TARGET_RXN
    {
        try
        {
            if (self.single_ringo_index.get() == NULL)
            {
                self.single_ringo_index.create();
                self.single_ringo_index->init(*self.bingo_context);
                self.single_ringo_index->skip_calculate_fp = self.skip_calculate_fp;
            }

            self.ringo_index = self.single_ringo_index.get();
            self.ringo_index->prepare(scanner, output, NULL);
        }
        catch (CmfSaver::Error& e)
        {
            if (self.bingo_context->reject_invalid_structures)
                throw;
            self.warning.readString(e.message(), true);
            return 0;
        }
        catch (CrfSaver::Error& e)
        {
            if (self.bingo_context->reject_invalid_structures)
                throw;
            self.warning.readString(e.message(), true);
            return 0;
        }
    }
    CATCH_READ_TARGET_RXN({
        if (self.bingo_context->reject_invalid_structures)
            throw;

        self.warning.readString(e.message(), true);
        return 0;
    });
    return 1;
}

CEXPORT int ringoIndexProcessSingleRecord(){
    BINGO_BEGIN {
        return self.ringoIndexProcessSingleRecord();
    }
    BINGO_END(1, -1)
}

int BingoCore::ringoIndexReadPreparedReaction(int* id, const char** crf_buf, int* crf_buf_len, const char** fingerprint_buf, int* fingerprint_buf_len)
{
    if (id)
        *id = self.index_record_data_id;

    const Array<char>& crf = self.ringo_index->getCrf();

    *crf_buf = crf.ptr();
    *crf_buf_len = crf.size();

    *fingerprint_buf = (const char*)self.ringo_index->getFingerprint();
    *fingerprint_buf_len = self.bingo_context->fp_parameters.fingerprintSizeExtOrd() * 2;

    return 1;
}

CEXPORT int ringoIndexReadPreparedReaction(int* id, const char** crf_buf, int* crf_buf_len, const char** fingerprint_buf, int* fingerprint_buf_len)
{
    profTimerStart(t0, "index.prepare_reaction");

    BINGO_BEGIN
    {
        return self.ringoIndexReadPreparedReaction(id, crf_buf, crf_buf_len, fingerprint_buf, fingerprint_buf_len);
    }
    BINGO_END(-2, -2)
}

void _ringoCheckPseudoAndCBDM(BingoCore& self)
{
    if (self.bingo_context == 0)
        throw BingoError("context not set");

    if (self.ringo_context == 0)
        throw BingoError("context not set");

    // TODO: pass this check inside RingoSubstructure
    if (!self.bingo_context->treat_x_as_pseudoatom.hasValue())
        throw BingoError("treat_x_as_pseudoatom option not set");
    if (!self.bingo_context->ignore_closing_bond_direction_mismatch.hasValue())
        throw BingoError("ignore_closing_bond_direction_mismatch option not set");
}

CEXPORT int ringoSetupMatch(const char* search_type, const char* query, const char* options)
{
    profTimerStart(t0, "match.setup_match");

    BINGO_BEGIN
    {
        _ringoCheckPseudoAndCBDM(self);

        TRY_READ_TARGET_RXN
        {
            if (strcasecmp(search_type, "RSUB") == 0 || strcasecmp(search_type, "RSMARTS") == 0)
            {
                RingoSubstructure& substructure = self.ringo_context->substructure;

                if (substructure.parse(options))
                {
                    if (strcasecmp(search_type, "RSUB") == 0)
                        substructure.loadQuery(query);
                    else
                        substructure.loadSMARTS(query);
                    self.ringo_search_type = BingoCore::_SUBSTRUCTRE;
                    return 1;
                }
            }
            else if (strcasecmp(search_type, "REXACT") == 0)
            {
                RingoExact& exact = self.ringo_context->exact;
                exact.setParameters(options);
                exact.loadQuery(query);
                self.ringo_search_type = BingoCore::_EXACT;
                return 1;
            }
            self.ringo_search_type = BingoCore::_UNDEF;
            throw BingoError("Unknown search type '%s' or options string '%s'", search_type, options);
        }
        CATCH_READ_TARGET_RXN(self.error.readString(e.message(), 1); return -1;);
    }
    BINGO_END(-2, -2)
}

// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int ringoMatchTarget(const char* target, int target_buf_len)
{
    profTimerStart(t0, "match.match_target");

    BINGO_BEGIN
    {
        if (self.ringo_search_type == BingoCore::_UNDEF)
            throw BingoError("Undefined search type");

        TRY_READ_TARGET_RXN
        {
            BufferScanner scanner(target, target_buf_len);
            if (self.ringo_search_type == BingoCore::_SUBSTRUCTRE)
            {
                RingoSubstructure& substructure = self.ringo_context->substructure;
                substructure.loadTarget(scanner);
                return substructure.matchLoadedTarget() ? 1 : 0;
            }
            else if (self.ringo_search_type == BingoCore::_EXACT)
            {
                RingoExact& exact = self.ringo_context->exact;
                exact.loadTarget(scanner);
                return exact.matchLoadedTarget() ? 1 : 0;
            }
            else
                throw BingoError("Invalid search type");
        }
        CATCH_READ_TARGET_RXN(self.warning.readString(e.message(), 1); return -1;);
    }
    BINGO_END(-2, -2)
}

// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int ringoMatchTargetBinary(const char* target_bin, int target_bin_len)
{
    profTimerStart(t0, "match.match_target_binary");

    BINGO_BEGIN
    {
        if (self.ringo_search_type == BingoCore::_UNDEF)
            throw BingoError("Undefined search type");

        TRY_READ_TARGET_RXN
        {
            BufferScanner scanner(target_bin, target_bin_len);

            if (self.ringo_search_type == BingoCore::_SUBSTRUCTRE)
            {
                RingoSubstructure& substructure = self.ringo_context->substructure;
                return substructure.matchBinary(scanner) ? 1 : 0;
            }
            else if (self.ringo_search_type == BingoCore::_EXACT)
            {
                RingoExact& exact = self.ringo_context->exact;
                return exact.matchBinary(scanner) ? 1 : 0;
            }
            else
                throw BingoError("Invalid search type");
        }
        CATCH_READ_TARGET_RXN(self.warning.readString(e.message(), 1); return -1;);
    }
    BINGO_END(-2, -2)
}

CEXPORT const char* ringoRSMILES(const char* target_buf, int target_buf_len)
{
    profTimerStart(t0, "rsmiles");

    BINGO_BEGIN
    {
        _ringoCheckPseudoAndCBDM(self);

        BufferScanner scanner(target_buf, target_buf_len);

        QS_DEF(Reaction, target);

        ReactionAutoLoader loader(scanner);
        self.bingo_context->setLoaderSettings(loader);
        loader.loadReaction(target);

        ArrayOutput out(self.buffer);

        RSmilesSaver saver(out);

        saver.saveReaction(target);
        out.writeByte(0);
        return self.buffer.ptr();
    }
    BINGO_END(0, 0)
}

CEXPORT const char* ringoRxnfile(const char* reaction, int reaction_len){BINGO_BEGIN{_ringoCheckPseudoAndCBDM(self);

BufferScanner scanner(reaction, reaction_len);

QS_DEF(Reaction, target);

ReactionAutoLoader loader(scanner);
self.bingo_context->setLoaderSettings(loader);
loader.loadReaction(target);

ArrayOutput out(self.buffer);

RxnfileSaver saver(out);

saver.saveReaction(target);
out.writeByte(0);
return self.buffer.ptr();
}
BINGO_END(0, 0)
}

CEXPORT const char* ringoRCML(const char* reaction, int reaction_len){BINGO_BEGIN{// TODO: remove copy/paste in ringoRCML, ringoRxnfile and etc.
                                                                                  _ringoCheckPseudoAndCBDM(self);

BufferScanner scanner(reaction, reaction_len);

QS_DEF(Reaction, target);

ReactionAutoLoader loader(scanner);
self.bingo_context->setLoaderSettings(loader);
loader.loadReaction(target);

ArrayOutput out(self.buffer);

ReactionCmlSaver saver(out);

saver.saveReaction(target);
out.writeByte(0);
return self.buffer.ptr();
}
BINGO_END(0, 0)
}

CEXPORT const char* ringoAAM(const char* reaction, int reaction_len, const char* mode){BINGO_BEGIN{_ringoCheckPseudoAndCBDM(self);

self.ringo_context->ringoAAM.parse(mode);

BufferScanner reaction_scanner(reaction, reaction_len);
self.ringo_context->ringoAAM.loadReaction(reaction_scanner);

self.ringo_context->ringoAAM.getResult(self.buffer);
self.buffer.push(0);
return self.buffer.ptr();
}
BINGO_END(0, 0)
}

CEXPORT const char* ringoCheckReaction(const char* reaction, int reaction_len){BINGO_BEGIN{TRY_READ_TARGET_RXN{_ringoCheckPseudoAndCBDM(self);

QS_DEF(Reaction, rxn);

BufferScanner reaction_scanner(reaction, reaction_len);
ReactionAutoLoader loader(reaction_scanner);
self.bingo_context->setLoaderSettings(loader);
loader.loadReaction(rxn);
Reaction::checkForConsistency(rxn);
}
CATCH_READ_TARGET_RXN(self.buffer.readString(e.message(), true); return self.buffer.ptr())
catch (Exception& e)
{
    e.appendMessage(" INTERNAL ERROR");
    self.buffer.readString(e.message(), true);
    return self.buffer.ptr();
}
catch (...)
{
    return "INTERNAL UNKNOWN ERROR";
}
}
BINGO_END(0, 0)
}

CEXPORT int ringoGetQueryFingerprint(const char** query_fp, int* query_fp_len)
{
    profTimerStart(t0, "match.query_fingerprint");

    BINGO_BEGIN
    {
        if (self.ringo_search_type == BingoCore::_UNDEF)
            throw BingoError("Undefined search type");

        if (self.ringo_search_type == BingoCore::_SUBSTRUCTRE)
        {
            RingoSubstructure& substructure = self.ringo_context->substructure;

            self.buffer.copy((const char*)substructure.getQueryFingerprint(), self.bingo_context->fp_parameters.fingerprintSizeExtOrd() * 2);
        }
        else
            throw BingoError("Invalid search type");

        *query_fp = self.buffer.ptr();
        *query_fp_len = self.buffer.size();
    }
    BINGO_END(1, -2)
}

CEXPORT int ringoSetHightlightingMode(int enable)
{
    BINGO_BEGIN
    {
        if (self.ringo_search_type == BingoCore::_SUBSTRUCTRE)
        {
            RingoSubstructure& substructure = self.ringo_context->substructure;
            return substructure.preserve_bonds_on_highlighting = (enable != 0);
        }
        else
            throw BingoError("Invalid search type");
    }
    BINGO_END(1, -2);
}

CEXPORT const char* ringoGetHightlightedReaction()
{
    BINGO_BEGIN
    {
        if (self.ringo_search_type == BingoCore::_SUBSTRUCTRE)
        {
            RingoSubstructure& substructure = self.ringo_context->substructure;
            substructure.getHighlightedTarget(self.buffer);
        }
        else
            throw BingoError("Invalid search type");

        self.buffer.push(0);
        return self.buffer.ptr();
    }
    BINGO_END(0, 0);
}

CEXPORT const char* ringoICR(const char* reaction, int reaction_len, bool save_xyz, int* out_len){BINGO_BEGIN{_ringoCheckPseudoAndCBDM(self);

BufferScanner scanner(reaction, reaction_len);

QS_DEF(Reaction, target);

ReactionAutoLoader loader(scanner);
self.bingo_context->setLoaderSettings(loader);
loader.loadReaction(target);

ArrayOutput out(self.buffer);

if ((save_xyz != 0) && !Reaction::haveCoord(target))
    throw BingoError("reaction has no XYZ");

IcrSaver saver(out);
saver.save_xyz = (save_xyz != 0);
saver.saveReaction(target);

*out_len = self.buffer.size();
return self.buffer.ptr();
}
BINGO_END(0, 0)
}

int BingoCore::ringoGetHash(bool for_index, dword* hash){
    if (for_index){
        * hash = self.ringo_index->getHash();
        return 1;
    }
    else
    {
        if (self.ringo_search_type != BingoCore::_EXACT)
            throw BingoError("Hash is valid only for exact search type");

        RingoExact& exact = self.ringo_context->exact;
        *hash = exact.getQueryHash();
        return 1;
    }
    return 1;
}

CEXPORT int ringoGetHash(bool for_index, dword* hash){
    BINGO_BEGIN{
        return self.ringoGetHash(for_index, hash);
    }
    BINGO_END(-2, -2)
}

CEXPORT const char* ringoFingerprint(const char* reaction, int reaction_len, const char* options, int* out_len)
{
    BINGO_BEGIN
    {
        _ringoCheckPseudoAndCBDM(self);

        if (!self.bingo_context->fp_parameters_ready)
            throw BingoError("Fingerprint settings not ready");

        BufferScanner scanner(reaction, reaction_len);

        QS_DEF(Reaction, target);

        ReactionAutoLoader loader(scanner);
        self.bingo_context->setLoaderSettings(loader);
        loader.loadReaction(target);

        ReactionFingerprintBuilder builder(target, self.bingo_context->fp_parameters);
        builder.parseFingerprintType(options, false);

        builder.process();

        const char* buf = (const char*)builder.get();
        int buf_len = self.bingo_context->fp_parameters.fingerprintSizeExtOrdSim() * 2;

        self.buffer.copy(buf, buf_len);

        *out_len = self.buffer.size();
        return self.buffer.ptr();
    }
    BINGO_END(0, 0)
}