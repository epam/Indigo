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
#include "molecule/canonical_smiles_saver.h"
#include "molecule/cmf_saver.h"
#include "molecule/cml_saver.h"
#include "molecule/icm_saver.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_mass.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_saver.h"

#include "molecule/inchi_wrapper.h"
#include "molecule/molecule_standardize.h"
#include "molecule/molecule_standardize_options.h"

using namespace indigo::bingo_core;

int BingoCore::mangoIndexProcessSingleRecord(){
    BufferScanner scanner(self.index_record_data.ref());

    NullOutput output;

    TRY_READ_TARGET_MOL
    {
        try
        {
            if (self.single_mango_index.get() == NULL)
            {
                self.single_mango_index.create();
                self.single_mango_index->init(*self.bingo_context);
                self.single_mango_index->skip_calculate_fp = self.skip_calculate_fp;
            }

            self.mango_index = self.single_mango_index.get();
            self.mango_index->prepare(scanner, output, NULL);
        }
        catch (CmfSaver::Error& e)
        {
            if (self.bingo_context->reject_invalid_structures)
                throw;
            self.warning.readString(e.message(), true);
            return 0;
        }
    }
    CATCH_READ_TARGET_MOL({
        if (self.bingo_context->reject_invalid_structures)
            throw;

        self.warning.readString(e.message(), true);
        return 0;
    });
    return 1;
}

CEXPORT int mangoIndexProcessSingleRecord(){
    BINGO_BEGIN {
        return self.mangoIndexProcessSingleRecord();
    }
    BINGO_END(1, -1)
}

int BingoCore::mangoIndexReadPreparedMolecule(int* id, const char** cmf_buf, int* cmf_buf_len, const char** xyz_buf, int* xyz_buf_len, const char** gross_str,
                                           const char** counter_elements_str, const char** fingerprint_buf, int* fingerprint_buf_len,
                                           const char** fingerprint_sim_str, float* mass,
                                           int* sim_fp_bits_count){
    if (id) * id = self.index_record_data_id;

    const Array<char>& cmf = self.mango_index->getCmf();
    const Array<char>& xyz = self.mango_index->getXyz();

    *cmf_buf = cmf.ptr();
    *cmf_buf_len = cmf.size();

    *xyz_buf = xyz.ptr();
    *xyz_buf_len = xyz.size();

    *fingerprint_buf = (const char*)self.mango_index->getFingerprint();
    *fingerprint_buf_len = self.bingo_context->fp_parameters.fingerprintSize();

    *fingerprint_sim_str = self.mango_index->getFingerprint_Sim_Str();
    *mass = self.mango_index->getMolecularMass();
    *gross_str = self.mango_index->getGrossString();

    *counter_elements_str = self.mango_index->getCountedElementsString();

    *sim_fp_bits_count = self.mango_index->getFpSimilarityBitsCount();
    return 1;
}

CEXPORT int mangoIndexReadPreparedMolecule(int* id, const char** cmf_buf, int* cmf_buf_len, const char** xyz_buf, int* xyz_buf_len, const char** gross_str,
                                           const char** counter_elements_str, const char** fingerprint_buf, int* fingerprint_buf_len,
                                           const char** fingerprint_sim_str, float* mass,
                                           int* sim_fp_bits_count){
    BINGO_BEGIN {
        return self.mangoIndexReadPreparedMolecule(id, cmf_buf, cmf_buf_len, xyz_buf, xyz_buf_len, gross_str,
                                           counter_elements_str, fingerprint_buf, fingerprint_buf_len,
                                           fingerprint_sim_str, mass, sim_fp_bits_count);
    }
    BINGO_END(-2, -2)
}

int BingoCore::mangoGetHash(bool for_index, int index, int* count, dword* hash)
{
    if (for_index)
    {
        // For index
        if (index == -1)
        {
            *count = self.mango_index->getHash().size();
        }
        else
        {
            const MangoExact::HashElement& elem = self.mango_index->getHash()[index];

            *count = elem.count;
            *hash = elem.hash;
        }
        return 1;
    }
    else
    {
        if (self.mango_search_type != BingoCore::_EXACT)
            throw BingoError("Hash is valid only for exact search type");

        MangoExact& exact = self.mango_context->exact;
        const MangoExact::Hash& hash_components = exact.getQueryHash();
        if (index == -1)
            *count = hash_components.size();
        else
        {
            *count = hash_components[index].count;
            *hash = hash_components[index].hash;
        }
        return 1;
    }
    return 1;
}

CEXPORT int mangoGetHash(bool for_index, int index, int* count, dword* hash)
{
    BINGO_BEGIN
    {
        return self.mangoGetHash(for_index, index, count, hash);
    }
    BINGO_END(-2, -2)
}

void _mangoCheckPseudoAndCBDM(BingoCore& self)
{
    if (self.bingo_context == 0)
        throw BingoError("context not set");

    if (self.mango_context == 0)
        throw BingoError("context not set");

    // TODO: pass this check inside MangoSubstructure
    if (!self.bingo_context->treat_x_as_pseudoatom.hasValue())
        throw BingoError("treat_x_as_pseudoatom option not set");
    if (!self.bingo_context->ignore_closing_bond_direction_mismatch.hasValue())
        throw BingoError("ignore_closing_bond_direction_mismatch option not set");
}

int BingoCore::mangoGetAtomCount(const char* target_buf, int target_buf_len){
    BufferScanner scanner(target_buf, target_buf_len);
    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(target);

    return target.vertexCount();
}

CEXPORT int mangoGetAtomCount(const char* target_buf, int target_buf_len){
    BINGO_BEGIN {
        return self.mangoGetAtomCount(target_buf, target_buf_len);
    }
    BINGO_END(-1, -1)
}

int BingoCore::mangoGetBondCount(const char* target_buf, int target_buf_len){
    BufferScanner scanner(target_buf, target_buf_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(target);

    return target.edgeCount();
}

CEXPORT int mangoGetBondCount(const char* target_buf, int target_buf_len){
    BINGO_BEGIN {
        return self.mangoGetBondCount(target_buf, target_buf_len);
    }
    BINGO_END(-1, -1)
}

int BingoCore::mangoSetupMatch(const char* search_type, const char* query, const char* options) {
    _mangoCheckPseudoAndCBDM(self);
    
    if (strcasecmp(search_type, "SUB") == 0)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;
        MangoTautomer& tautomer = self.mango_context->tautomer;

        if (substructure.parse(options))
        {
            substructure.loadQuery(query);
            self.mango_search_type = BingoCore::_SUBSTRUCTRE;
            return 1;
        }
        if (tautomer.parseSub(options))
        {
            if (!self.bingo_context->tautomer_rules_ready)
                throw BingoError("tautomer rules not set");

            tautomer.loadQuery(query);
            self.mango_search_type = BingoCore::_TAUTOMER;
            return 1;
        }
    }
    else if (strcasecmp(search_type, "SMARTS") == 0)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;
        if (substructure.parse(options))
        {
            substructure.loadSMARTS(query);
            self.mango_search_type = BingoCore::_SUBSTRUCTRE;
            return 1;
        }
    }
    else if (strcasecmp(search_type, "EXACT") == 0)
    {
        MangoExact& exact = self.mango_context->exact;
        MangoTautomer& tautomer = self.mango_context->tautomer;

        if (exact.parse(options))
        {
            exact.loadQuery(query);
            self.mango_search_type = BingoCore::_EXACT;
            return 1;
        }
        if (tautomer.parseExact(options))
        {
            // TODO: pass this check inside MangoSubstructure
            if (!self.bingo_context->tautomer_rules_ready)
                throw BingoError("tautomer rules not set");

            tautomer.loadQuery(query);
            self.mango_search_type = BingoCore::_TAUTOMER;
            return 1;
        }
    }
    else if (strcasecmp(search_type, "SIM") == 0)
    {
        MangoSimilarity& similarity = self.mango_context->similarity;
        similarity.loadQuery(query);
        similarity.setMetrics(options);
        self.mango_search_type = BingoCore::_SIMILARITY;
        return 1;
    }
    else if (strcasecmp(search_type, "GROSS") == 0)
    {
        MangoGross& gross = self.mango_context->gross;
        gross.parseQuery(query);
        self.mango_search_type = BingoCore::_GROSS;
        return 1;
    }
    self.mango_search_type = BingoCore::_UNDEF;

    throw BingoError("Unknown search type '%s' or options string '%s'", search_type, options);
}

CEXPORT int mangoSetupMatch(const char* search_type, const char* query, const char* options)
{
    profTimerStart(t0, "match.setup_match");

    BINGO_BEGIN
    {
        TRY_READ_TARGET_MOL
        {
            return self.mangoSetupMatch(search_type, query, options);
        }
        CATCH_READ_TARGET_MOL(self.error.readString(e.message(), 1); return -1;);
    }
    BINGO_END(-2, -2)
}

void BingoCore::mangoSimilarityGetBitMinMaxBoundsArray(int count, int* target_ones, int** min_bound_ptr, int** max_bound_ptr) {
    if (self.mango_search_type != BingoCore::_SIMILARITY) {
        throw BingoError("Undefined search type");
    }
    MangoSimilarity& similarity = self.mango_context->similarity;

    self.buffer.resize(sizeof(int) * 2 * count);

    int* min_bounds = (int*)self.buffer.ptr();
    int* max_bounds = min_bounds + count;
    for (int i = 0; i < count; i++)
    {
        max_bounds[i] = similarity.getUpperBound(target_ones[i]);
        min_bounds[i] = similarity.getLowerBound(target_ones[i]);
    }

    *min_bound_ptr = min_bounds;
    *max_bound_ptr = max_bounds;
}

CEXPORT int mangoSimilarityGetBitMinMaxBoundsArray(int count, int* target_ones, int** min_bound_ptr, int** max_bound_ptr) {
    BINGO_BEGIN{
        self.mangoSimilarityGetBitMinMaxBoundsArray(count, target_ones, min_bound_ptr, max_bound_ptr);
    }
    BINGO_END(1, -2)
}

void BingoCore::mangoSimilarityGetScore(float* score) {
    if (self.mango_search_type != BingoCore::_SIMILARITY) {
        throw BingoError("Undefined search type");
    }
    MangoSimilarity& similarity = self.mango_context->similarity;
    *score = similarity.getSimilarityScore();
}

CEXPORT int mangoSimilarityGetScore(float* score){
    BINGO_BEGIN{
        self.mangoSimilarityGetScore(score);
    }
    BINGO_END(1, -2)
}

void BingoCore::mangoSimilaritySetMinMaxBounds(float min_bound, float max_bound) {
    if (self.mango_search_type != BingoCore::_SIMILARITY) {
        throw BingoError("Undefined search type");
    }
    MangoSimilarity& similarity = self.mango_context->similarity;
    similarity.bottom = min_bound;
    similarity.top = max_bound;
    similarity.include_bottom = true;
    similarity.include_top = true;
}

CEXPORT int mangoSimilaritySetMinMaxBounds(float min_bound, float max_bound) {
    BINGO_BEGIN{
        self.mangoSimilaritySetMinMaxBounds(min_bound, max_bound);
    }
    BINGO_END(1, -2)
}

int BingoCore::mangoMatchTarget(const char* target, int target_buf_len) {
    if (self.mango_search_type == BingoCore::_UNDEF)
            throw BingoError("Undefined search type");
    int timeout = self.getTimeout();
    AutoCancellationHandler handler(new TimeoutCancellationHandler(timeout));

    TRY_READ_TARGET_MOL
    {
        BufferScanner scanner(target, target_buf_len);
        if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
        {
            MangoSubstructure& substructure = self.mango_context->substructure;
            substructure.loadTarget(scanner);
            return substructure.matchLoadedTarget() ? 1 : 0;
        }
        else if (self.mango_search_type == BingoCore::_TAUTOMER)
        {
            MangoTautomer& tautomer = self.mango_context->tautomer;
            tautomer.loadTarget(scanner);
            return tautomer.matchLoadedTarget() ? 1 : 0;
        }
        else if (self.mango_search_type == BingoCore::_EXACT)
        {
            MangoExact& exact = self.mango_context->exact;
            exact.loadTarget(scanner);
            return exact.matchLoadedTarget() ? 1 : 0;
        }
        else if (self.mango_search_type == BingoCore::_SIMILARITY)
        {
            MangoSimilarity& simlarity = self.mango_context->similarity;
            simlarity.calc(scanner);
            // Score should be obtained by calling mangoSimilarityGetScore
            return 1;
        }
        else if (self.mango_search_type == BingoCore::_GROSS)
        {
            MangoGross& gross = self.mango_context->gross;
            return gross.checkGross(target) ? 1 : 0;
        }
        else
            throw BingoError("Invalid search type");
    }
    CATCH_READ_TARGET_MOL(self.warning.readString(e.message(), 1); return -1;);
    return 0;
}

// Return value:
//   1 if the query is a substructure of the target
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int mangoMatchTarget(const char* target, int target_buf_len)
{
    profTimerStart(t0, "match.match_target");

    BINGO_BEGIN
    {
        return self.mangoMatchTarget(target, target_buf_len);
    }
    BINGO_END(-2, -2)
}

int BingoCore::mangoMatchTargetBinary(const char* target_bin, int target_bin_len, const char* target_xyz, int target_xyz_len) 
{
    if (self.mango_search_type == BingoCore::_UNDEF)
        throw BingoError("Undefined search type");
    int timeout = self.getTimeout();
    AutoCancellationHandler handler(new TimeoutCancellationHandler(timeout));

    TRY_READ_TARGET_MOL
    {
        BufferScanner scanner(target_bin, target_bin_len);
        BufferScanner* xyz_scanner = 0;
        Obj<BufferScanner> xyz_scanner_obj;
        if (target_xyz_len != 0)
        {
            xyz_scanner_obj.create(target_xyz, target_xyz_len);
            xyz_scanner = xyz_scanner_obj.get();
        }

        if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
        {
            MangoSubstructure& substructure = self.mango_context->substructure;
            return substructure.matchBinary(scanner, xyz_scanner) ? 1 : 0;
        }
        else if (self.mango_search_type == BingoCore::_TAUTOMER)
        {
            MangoTautomer& tautomer = self.mango_context->tautomer;
            return tautomer.matchBinary(scanner) ? 1 : 0;
        }
        else if (self.mango_search_type == BingoCore::_EXACT)
        {
            MangoExact& exact = self.mango_context->exact;
            return exact.matchBinary(scanner, xyz_scanner) ? 1 : 0;
        }
        else if (self.mango_search_type == BingoCore::_SIMILARITY)
        {
            MangoSimilarity& similarity = self.mango_context->similarity;
            return similarity.matchBinary(scanner) ? 1 : 0;
        }
        else
            throw BingoError("Invalid search type");
    }
    CATCH_READ_TARGET_MOL(self.warning.readString(e.message(), 1); return -1;);
    return 0;
}

// Return value:
//   1 if the query is a substructure of the target
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int mangoMatchTargetBinary(const char* target_bin, int target_bin_len, const char* target_xyz, int target_xyz_len)
{
    profTimerStart(t0, "match.match_target_binary");

    BINGO_BEGIN
    {
        return self.mangoMatchTargetBinary(target_bin, target_bin_len, target_xyz, target_xyz_len);
    }
    BINGO_END(-2, -2)
}

void BingoCore::mangoLoadTargetBinaryXyz(const char* target_xyz, int target_xyz_len)
{
    if (self.mango_search_type == BingoCore::_UNDEF)
        throw BingoError("Undefined search type");

    BufferScanner xyz_scanner(target_xyz, target_xyz_len);

    if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;
        substructure.loadBinaryTargetXyz(xyz_scanner);
    }
    else
        throw BingoError("Invalid search type");
}

CEXPORT int mangoLoadTargetBinaryXyz(const char* target_xyz, int target_xyz_len)
{
    profTimerStart(t0, "match.match_target_binary");

    BINGO_BEGIN
    {
        self.mangoLoadTargetBinaryXyz(target_xyz, target_xyz_len);
    }
    BINGO_END(1, -2)
}

void BingoCore::mangoSetHightlightingMode(int enable) {
    if (self.mango_context == 0) {
        throw BingoError("mango_context not set");
    }

    if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;
        substructure.preserve_bonds_on_highlighting = (enable != 0);
    }
    else if (self.mango_search_type == BingoCore::_TAUTOMER)
    {
        MangoTautomer& tautomer = self.mango_context->tautomer;
        tautomer.preserve_bonds_on_highlighting = (enable != 0);
    }
    else
        throw BingoError("Unsupported search type in mangoSetHightlightingMode");
}

CEXPORT int mangoSetHightlightingMode(int enable) {
    BINGO_BEGIN{
        self.mangoSetHightlightingMode(enable);
    }
    BINGO_END(1, -2)
}

const char* BingoCore::mangoGetHightlightedMolecule() {
    if (self.mango_context == 0) {
        throw BingoError("mango_context not set");
    }

    if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;
        substructure.getHighlightedTarget(self.buffer);
    }
    else if (self.mango_search_type == BingoCore::_TAUTOMER)
    {
        MangoTautomer& tautomer = self.mango_context->tautomer;
        tautomer.getHighlightedTarget(self.buffer);
    }
    else
        throw BingoError("Unsupported search type in mangoGetHightlightedMolecule");

    self.buffer.push(0);
    return self.buffer.ptr();
}

CEXPORT const char* mangoGetHightlightedMolecule() {
    BINGO_BEGIN{
        return self.mangoGetHightlightedMolecule();
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoSMILES(const char* target_buf, int target_buf_len, int canonical)
{
    int timeout = self.getTimeout();
    AutoCancellationHandler handler(new TimeoutCancellationHandler(timeout));

    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(target_buf, target_buf_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    if (canonical)
        MoleculeAromatizer::aromatizeBonds(target, AromaticityOptions::BASIC);

    ArrayOutput out(self.buffer);

    if (canonical)
    {
        CanonicalSmilesSaver saver(out);
        saver.saveMolecule(target);
    }
    else
    {
        SmilesSaver saver(out);
        saver.saveMolecule(target);
    }
    out.writeByte(0);
    return self.buffer.ptr();
}

CEXPORT const char* mangoSMILES(const char* target_buf, int target_buf_len, int canonical)
{
    profTimerStart(t0, "smiles");

    BINGO_BEGIN
    {
        return self.mangoSMILES(target_buf, target_buf_len, canonical);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoMolfile(const char* molecule, int molecule_len) {
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(molecule, molecule_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    ArrayOutput out(self.buffer);

    MolfileSaver saver(out);

    saver.saveMolecule(target);
    out.writeByte(0);
    return self.buffer.ptr();
}

CEXPORT const char* mangoMolfile(const char* molecule, int molecule_len) {
    BINGO_BEGIN {
        return self.mangoMolfile(molecule, molecule_len);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoCML(const char* molecule, int molecule_len) {
    // TODO: remove copy/paste in mangoCML, mangoMolfile and etc.
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(molecule, molecule_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    ArrayOutput out(self.buffer);

    CmlSaver saver(out);
    saver.saveMolecule(target);
    out.writeByte(0);
    return self.buffer.ptr();
}

CEXPORT const char* mangoCML(const char* molecule, int molecule_len) {
    BINGO_BEGIN {
        return self.mangoCML(molecule, molecule_len);
    }
    BINGO_END(0, 0)
}

void BingoCore::mangoGetQueryFingerprint(const char** query_fp, int* query_fp_len)
{
    if (self.mango_search_type == BingoCore::_UNDEF)
        throw BingoError("Undefined search type");

    if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;

        self.buffer.copy((const char*)substructure.getQueryFingerprint(), self.bingo_context->fp_parameters.fingerprintSize());
    }
    else if (self.mango_search_type == BingoCore::_TAUTOMER)
    {
        MangoTautomer& tautomer = self.mango_context->tautomer;
        self.buffer.copy((const char*)tautomer.getQueryFingerprint(), self.bingo_context->fp_parameters.fingerprintSize());
    }
    else if (self.mango_search_type == BingoCore::_SIMILARITY)
    {
        MangoSimilarity& similarity = self.mango_context->similarity;
        self.buffer.copy((const char*)similarity.getQueryFingerprint(), self.bingo_context->fp_parameters.fingerprintSize());
    }
    else
        throw BingoError("Invalid search type");

    *query_fp = self.buffer.ptr();
    *query_fp_len = self.buffer.size();
}

CEXPORT int mangoGetQueryFingerprint(const char** query_fp, int* query_fp_len)
{
    profTimerStart(t0, "match.query_fingerprint");

    BINGO_BEGIN
    {
        self.mangoGetQueryFingerprint(query_fp, query_fp_len);
    }
    BINGO_END(1, -2)
}

const char* BingoCore::mangoGetCountedElementName(int index){
    ArrayOutput output(self.buffer);
    output.printf("cnt_%s", Element::toString(MangoIndex::counted_elements[index]));
    self.buffer.push(0);

    return self.buffer.ptr();
}

CEXPORT const char* mangoGetCountedElementName(int index){
    BINGO_BEGIN{
        return self.mangoGetCountedElementName(index);
    }
    BINGO_END(0, 0)
}

int BingoCore::mangoNeedCoords()
{
    if (self.mango_search_type == BingoCore::_SUBSTRUCTRE)
    {
        MangoSubstructure& substructure = self.mango_context->substructure;
        return substructure.needCoords();
    }
    else if (self.mango_search_type == BingoCore::_EXACT)
    {
        MangoExact& exact = self.mango_context->exact;
        return exact.needCoords();
    }
    else if (self.mango_search_type == BingoCore::_TAUTOMER)
        return 0;
    else if (self.mango_search_type == BingoCore::_SIMILARITY)
        return 0;
    else
        throw BingoError("Invalid search type");
    return 0;
}

CEXPORT int mangoNeedCoords()
{
    profTimerStart(t0, "match.query_fingerprint");

    BINGO_BEGIN
    {
        return self.mangoNeedCoords();
    }
    BINGO_END(-2, -2)
}

byte BingoCore::mangoExactNeedComponentMatching(){
    MangoExact& exact = self.mango_context->exact;
    return exact.needComponentMatching();
}

CEXPORT byte mangoExactNeedComponentMatching(){
    BINGO_BEGIN {
        return self.mangoExactNeedComponentMatching();
    }
    BINGO_END(-2, -2)
}

const char* BingoCore::mangoTauGetQueryGross(){
    MangoTautomer& tautomer = self.mango_context->tautomer;
    return tautomer.getQueryGross();
}

CEXPORT const char* mangoTauGetQueryGross(){
    BINGO_BEGIN {
        return self.mangoTauGetQueryGross();
    }
    BINGO_END(0, 0)
}

void BingoCore::mangoMass(const char* target_buf, int target_buf_len, const char* type, float* out) {
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(target_buf, target_buf_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.skip_3d_chirality = true;
    loader.loadMolecule(target);

    MoleculeMass mass_calulator;
    mass_calulator.relative_atomic_mass_map = &self.bingo_context->relative_atomic_mass_map;

    if (type == 0 || strlen(type) == 0 || strcasecmp(type, "molecular-weight") == 0)
        *out = mass_calulator.molecularWeight(target);
    else if (strcasecmp(type, "most-abundant-mass") == 0)
        *out = mass_calulator.mostAbundantMass(target);
    else if (strcasecmp(type, "monoisotopic-mass") == 0)
        *out = mass_calulator.monoisotopicMass(target);
    else
        throw BingoError("unknown mass specifier: %s", type);
}

CEXPORT int mangoMass(const char* target_buf, int target_buf_len, const char* type, float* out){
    BINGO_BEGIN {
        self.mangoMass(target_buf, target_buf_len, type, out);
    }
    BINGO_END(1, -1)
}

int BingoCore::mangoMassD(const char* target_buf, int target_buf_len, const char* type, double* out) {
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(target_buf, target_buf_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.skip_3d_chirality = true;
    loader.loadMolecule(target);

    MoleculeMass mass_calulator;
    mass_calulator.relative_atomic_mass_map = &self.bingo_context->relative_atomic_mass_map;

    if (type == 0 || strlen(type) == 0 || strcasecmp(type, "molecular-weight") == 0)
        *out = mass_calulator.molecularWeight(target);
    else if (strcasecmp(type, "most-abundant-mass") == 0)
        *out = mass_calulator.mostAbundantMass(target);
    else if (strcasecmp(type, "monoisotopic-mass") == 0)
        *out = mass_calulator.monoisotopicMass(target);
    else
        throw BingoError("unknown mass specifier: %s", type);
    return 1;
}

CEXPORT int mangoMassD(const char* target_buf, int target_buf_len, const char* type, double* out){
    BINGO_BEGIN {
        return self.mangoMassD(target_buf, target_buf_len, type, out);
    }
    BINGO_END(-1, -1)
}

const char* BingoCore::mangoGross(const char* target_buf, int target_buf_len) {
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(target_buf, target_buf_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    QS_DEF(Array<int>, gross);
    MoleculeGrossFormula::collect(target, gross);
    MoleculeGrossFormula::toString(gross, self.buffer);
    self.buffer.push(0);

    return self.buffer.ptr();
}
CEXPORT const char* mangoGross(const char* target_buf, int target_buf_len) {
    BINGO_BEGIN{
        return mangoGross(target_buf, target_buf_len);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoGrossGetConditions() {
    if (self.bingo_context == 0) {
        throw BingoError("context not set");
    }

    if (self.mango_search_type != BingoCore::_GROSS)
        throw BingoError("Search type must be 'GROSS'");

    return self.mango_context->gross.getConditions();
}

CEXPORT const char* mangoGrossGetConditions() {
    BINGO_BEGIN {
        return self.mangoGrossGetConditions();
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoCheckMolecule(const char* molecule, int molecule_len) {
    _mangoCheckPseudoAndCBDM(self);

    TRY_READ_TARGET_MOL
    {
        QS_DEF(Molecule, mol);

        BufferScanner molecule_scanner(molecule, molecule_len);
        MoleculeAutoLoader loader(molecule_scanner);
        self.bingo_context->setLoaderSettings(loader);
        loader.loadMolecule(mol);
        Molecule::checkForConsistency(mol);
    }
    CATCH_READ_TARGET_MOL(self.buffer.readString(e.message(), true); return self.buffer.ptr())
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
    return 0;
}

CEXPORT const char* mangoCheckMolecule(const char* molecule, int molecule_len) {
    BINGO_BEGIN{
        return self.mangoCheckMolecule(molecule, molecule_len);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoICM(const char* molecule, int molecule_len, bool save_xyz, int* out_len) {
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(molecule, molecule_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    ArrayOutput out(self.buffer);

    if ((save_xyz != 0) && !target.have_xyz)
        throw BingoError("molecule has no XYZ");

    IcmSaver saver(out);
    saver.save_xyz = (save_xyz != 0);
    saver.saveMolecule(target);

    *out_len = self.buffer.size();
    return self.buffer.ptr();
}

CEXPORT const char* mangoICM(const char* molecule, int molecule_len, bool save_xyz, int* out_len) {
    BINGO_BEGIN{
        return self.mangoICM(molecule, molecule_len, save_xyz, out_len);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoFingerprint(const char* molecule, int molecule_len, const char* options, int* out_len){
    _mangoCheckPseudoAndCBDM(self);

    if (!self.bingo_context->fp_parameters_ready)
        throw BingoError("Fingerprint settings not ready");

    BufferScanner scanner(molecule, molecule_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    MoleculeFingerprintBuilder builder(target, self.bingo_context->fp_parameters);
    builder.parseFingerprintType(options, false);

    builder.process();

    const char* buf = (const char*)builder.get();
    int buf_len = self.bingo_context->fp_parameters.fingerprintSize();

    self.buffer.copy(buf, buf_len);

    *out_len = self.buffer.size();
    return self.buffer.ptr();
}

CEXPORT const char* mangoFingerprint(const char* molecule, int molecule_len, const char* options, int* out_len){
    BINGO_BEGIN{
        return self.mangoFingerprint(molecule, molecule_len, options, out_len);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoInChI(const char* molecule, int molecule_len, const char* options, int* out_len){
    _mangoCheckPseudoAndCBDM(self);

    BufferScanner scanner(molecule, molecule_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    InchiWrapper inchi;
    inchi.setOptions(options);
    inchi.saveMoleculeIntoInchi(target, self.buffer);

    *out_len = self.buffer.size();

    return self.buffer.ptr();
}

CEXPORT const char* mangoInChI(const char* molecule, int molecule_len, const char* options, int* out_len){
    BINGO_BEGIN{
        return self.mangoInChI(molecule, molecule_len, options, out_len);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoInChIKey(const char* inchi){
    InchiWrapper::InChIKey(inchi, self.buffer);
    return self.buffer.ptr();
}

CEXPORT const char* mangoInChIKey(const char* inchi){
    BINGO_BEGIN{
        return self.mangoInChIKey(inchi);
    }
    BINGO_END(0, 0)
}

const char* BingoCore::mangoStandardize(const char* molecule, int molecule_len, const char* options)
{
    BufferScanner scanner(molecule, molecule_len);

    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    self.bingo_context->setLoaderSettings(loader);
    loader.loadMolecule(target);

    StandardizeOptions st_options;
    st_options.parseFromString(options);

    MoleculeStandardizer::standardize(target, st_options);

    ArrayOutput out(self.buffer);

    MolfileSaver saver(out);

    saver.saveMolecule(target);
    out.writeByte(0);
    return self.buffer.ptr();
}

CEXPORT const char* mangoStandardize(const char* molecule, int molecule_len, const char* options)
{
    BINGO_BEGIN
    {
        return self.mangoStandardize(molecule, molecule_len, options);
    }
    BINGO_END(0, 0)
}
