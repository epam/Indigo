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

CEXPORT int mangoIndexProcessSingleRecord(){BINGO_BEGIN{BufferScanner scanner(self.index_record_data.ref());

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
}
BINGO_END(1, -1)
}

CEXPORT int mangoIndexReadPreparedMolecule(int* id, const char** cmf_buf, int* cmf_buf_len, const char** xyz_buf, int* xyz_buf_len, const char** gross_str,
                                           const char** counter_elements_str, const char** fingerprint_buf, int* fingerprint_buf_len,
                                           const char** fingerprint_sim_str, float* mass,
                                           int* sim_fp_bits_count){BINGO_BEGIN{if (id) * id = self.index_record_data_id;

const ArrayChar& cmf = self.mango_index->getCmf();
const ArrayChar& xyz = self.mango_index->getXyz();

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
BINGO_END(-2, -2)
}

CEXPORT int mangoGetHash(bool for_index, int index, int* count, dword* hash)
{
    BINGO_BEGIN
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

CEXPORT int mangoGetAtomCount(const char* target_buf, int target_buf_len){BINGO_BEGIN{BufferScanner scanner(target_buf, target_buf_len);

QS_DEF(Molecule, target);

MoleculeAutoLoader loader(scanner);
loader.loadMolecule(target);

return target.vertexCount();
}
BINGO_END(-1, -1)
}

CEXPORT int mangoGetBondCount(const char* target_buf, int target_buf_len){BINGO_BEGIN{BufferScanner scanner(target_buf, target_buf_len);

QS_DEF(Molecule, target);

MoleculeAutoLoader loader(scanner);
loader.loadMolecule(target);

return target.edgeCount();
}
BINGO_END(-1, -1)
}

CEXPORT int mangoSetupMatch(const char* search_type, const char* query, const char* options)
{
    profTimerStart(t0, "match.setup_match");

    BINGO_BEGIN
    {
        _mangoCheckPseudoAndCBDM(self);

        TRY_READ_TARGET_MOL
        {
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
        CATCH_READ_TARGET_MOL(self.error.readString(e.message(), 1); return -1;);
    }
    BINGO_END(-2, -2)
}

CEXPORT int mangoSimilarityGetBitMinMaxBoundsArray(int count, int* target_ones, int** min_bound_ptr, int** max_bound_ptr){
    BINGO_BEGIN{if (self.mango_search_type != BingoCore::_SIMILARITY) throw BingoError("Undefined search type");
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
BINGO_END(1, -2)
}

CEXPORT int mangoSimilarityGetScore(float* score){BINGO_BEGIN{if (self.mango_search_type != BingoCore::_SIMILARITY) throw BingoError("Undefined search type");
MangoSimilarity& similarity = self.mango_context->similarity;
*score = similarity.getSimilarityScore();
}
BINGO_END(-2, 1)
}

CEXPORT int mangoSimilaritySetMinMaxBounds(float min_bound, float max_bound){
    BINGO_BEGIN{if (self.mango_search_type != BingoCore::_SIMILARITY) throw BingoError("Undefined search type");
MangoSimilarity& similarity = self.mango_context->similarity;
similarity.bottom = min_bound;
similarity.top = max_bound;
similarity.include_bottom = true;
similarity.include_top = true;
}
BINGO_END(1, -2)
}

// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int mangoMatchTarget(const char* target, int target_buf_len)
{
    profTimerStart(t0, "match.match_target");

    BINGO_BEGIN
    {
        if (self.mango_search_type == BingoCore::_UNDEF)
            throw BingoError("Undefined search type");

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
    }
    BINGO_END(-2, -2)
}

// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int mangoMatchTargetBinary(const char* target_bin, int target_bin_len, const char* target_xyz, int target_xyz_len)
{
    profTimerStart(t0, "match.match_target_binary");

    BINGO_BEGIN_TIMEOUT
    {
        if (self.mango_search_type == BingoCore::_UNDEF)
            throw BingoError("Undefined search type");

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
    }
    BINGO_END(-2, -2)
}

CEXPORT int mangoLoadTargetBinaryXyz(const char* target_xyz, int target_xyz_len)
{
    profTimerStart(t0, "match.match_target_binary");

    BINGO_BEGIN
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
    BINGO_END(1, -2)
}

CEXPORT int mangoSetHightlightingMode(int enable){BINGO_BEGIN{if (self.mango_context == 0) throw BingoError("mango_context not set");

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
BINGO_END(1, -2)
}

CEXPORT const char* mangoGetHightlightedMolecule(){BINGO_BEGIN{if (self.mango_context == 0) throw BingoError("mango_context not set");

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
BINGO_END(0, 0)
}

CEXPORT const char* mangoSMILES(const char* target_buf, int target_buf_len, int canonical)
{
    profTimerStart(t0, "smiles");

    BINGO_BEGIN_TIMEOUT
    {
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
    BINGO_END(0, 0)
}

CEXPORT const char* mangoMolfile(const char* molecule, int molecule_len){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
BINGO_END(0, 0)
}

CEXPORT const char* mangoCML(const char* molecule, int molecule_len){BINGO_BEGIN{// TODO: remove copy/paste in mangoCML, mangoMolfile and etc.
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
BINGO_END(0, 0)
}

CEXPORT int mangoGetQueryFingerprint(const char** query_fp, int* query_fp_len)
{
    profTimerStart(t0, "match.query_fingerprint");

    BINGO_BEGIN
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
    BINGO_END(1, -2)
}

CEXPORT const char* mangoGetCountedElementName(int index){BINGO_BEGIN{ArrayOutput output(self.buffer);
output.printf("cnt_%s", Element::toString(MangoIndex::counted_elements[index]));
self.buffer.push(0);

return self.buffer.ptr();
}
BINGO_END(0, 0)
}

CEXPORT int mangoNeedCoords()
{
    profTimerStart(t0, "match.query_fingerprint");

    BINGO_BEGIN
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
    }
    BINGO_END(-2, -2)
}

CEXPORT byte mangoExactNeedComponentMatching(){BINGO_BEGIN{MangoExact& exact = self.mango_context->exact;
return exact.needComponentMatching();
}
BINGO_END(-2, -2)
}

CEXPORT const char* mangoTauGetQueryGross(){BINGO_BEGIN{MangoTautomer& tautomer = self.mango_context->tautomer;
return tautomer.getQueryGross();
}
BINGO_END(0, 0)
}

CEXPORT int mangoMass(const char* target_buf, int target_buf_len, const char* type, float* out){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
BINGO_END(-1, -1)
}

CEXPORT int mangoMassD(const char* target_buf, int target_buf_len, const char* type, double* out){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
BINGO_END(-1, -1)
}

CEXPORT const char* mangoGross(const char* target_buf, int target_buf_len){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

BufferScanner scanner(target_buf, target_buf_len);

QS_DEF(Molecule, target);

MoleculeAutoLoader loader(scanner);
self.bingo_context->setLoaderSettings(loader);
loader.loadMolecule(target);

QS_DEF(ArrayInt, gross);
MoleculeGrossFormula::collect(target, gross);
MoleculeGrossFormula::toString(gross, self.buffer);
self.buffer.push(0);

return self.buffer.ptr();
}
BINGO_END(0, 0)
}

CEXPORT const char* mangoGrossGetConditions(){BINGO_BEGIN{if (self.bingo_context == 0) throw BingoError("context not set");

if (self.mango_search_type != BingoCore::_GROSS)
    throw BingoError("Search type must be 'GROSS'");

return self.mango_context->gross.getConditions();
}
BINGO_END(0, 0)
}

CEXPORT const char* mangoCheckMolecule(const char* molecule, int molecule_len){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
}
BINGO_END(0, 0)
}

CEXPORT const char* mangoICM(const char* molecule, int molecule_len, bool save_xyz, int* out_len){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
BINGO_END(0, 0)
}

CEXPORT const char* mangoFingerprint(const char* molecule, int molecule_len, const char* options, int* out_len){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
BINGO_END(0, 0)
}

CEXPORT const char* mangoInChI(const char* molecule, int molecule_len, const char* options, int* out_len){BINGO_BEGIN{_mangoCheckPseudoAndCBDM(self);

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
BINGO_END(0, 0)
}

CEXPORT const char* mangoInChIKey(const char* inchi){BINGO_BEGIN{InchiWrapper::InChIKey(inchi, self.buffer);
return self.buffer.ptr();
}
BINGO_END(0, 0)
}

CEXPORT const char* mangoStandardize(const char* molecule, int molecule_len, const char* options)
{
    BINGO_BEGIN
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
    BINGO_END(0, 0)
}
