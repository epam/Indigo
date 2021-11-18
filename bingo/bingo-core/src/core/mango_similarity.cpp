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

#include "base_c/bitarray.h"
#include "base_cpp/scanner.h"
#include "bingo_context.h"
#include "bingo_error.h"
#include "mango_matchers.h"
#include "molecule/cmf_loader.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_fingerprint.h"

IMPL_ERROR(MangoSimilarity, "mango similarity");

MangoSimilarity::MangoSimilarity(BingoContext& context) : _context(context)
{
    metrics.type = BIT_METRICS_TANIMOTO;
    bottom = -0.1f; // to ensure that 0 is included
    top = 1.1f;     // to ensure that 1 is included

    _numerator_value = 0;
    _denominator_value = 0;
}

MangoSimilarity::Metrics MangoSimilarity::whichMetrics(const char* metrics_str)
{
    if (metrics_str == 0 || metrics_str[0] == 0)
        return Metrics(BIT_METRICS_TANIMOTO);

    // Expecting "tversky" or "tversky <alpha> <beta>"
    const char* TVERSKY = "tversky";
    if (strncasecmp(metrics_str, TVERSKY, strlen(TVERSKY)) == 0)
    {
        Metrics metrics(BIT_METRICS_TVERSKY);
        metrics.tversky_alpha = 0.5f;
        metrics.tversky_beta = 0.5f;
        if (strcasecmp(metrics_str, TVERSKY) != 0)
        {
            // Try to parse alpha and beta
            const char* params = metrics_str + strlen(TVERSKY);
            BufferScanner scanner(params);
            if (!scanner.tryReadFloat(metrics.tversky_alpha))
                throw Error("unknown metrics: %s", metrics_str);
            scanner.skipSpace();
            if (!scanner.tryReadFloat(metrics.tversky_beta))
                throw Error("unknown metrics: %s", metrics_str);
        }
        return metrics;
    }
    if (strcasecmp(metrics_str, "tanimoto") == 0)
        return Metrics(BIT_METRICS_TANIMOTO);
    if (strcasecmp(metrics_str, "euclid-sub") == 0)
        return Metrics(BIT_METRICS_EUCLID_SUB);

    throw Error("unknown metrics: %s", metrics_str);
}

void MangoSimilarity::setMetrics(const char* metrics_str)
{
    metrics = whichMetrics(metrics_str);
}

void MangoSimilarity::loadQuery(Scanner& scanner)
{
    QS_DEF(Molecule, query);

    MoleculeAutoLoader loader(scanner);

    _context.setLoaderSettings(loader);
    loader.loadMolecule(query);

    _initQuery(query);

    MoleculeFingerprintBuilder builder(query, _context.fp_parameters);

    builder.skip_tau = true;
    builder.skip_ext = true;
    builder.skip_ord = true;
    builder.skip_any_atoms = true;
    builder.skip_any_bonds = true;
    builder.skip_any_atoms_bonds = true;

    builder.process();
    _query_fp.copy(builder.get(), _context.fp_parameters.fingerprintSize());
    _query_ones = builder.countBits_Sim();
}

void MangoSimilarity::loadQuery(const Array<char>& buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

void MangoSimilarity::loadQuery(const char* str)
{
    BufferScanner scanner(str);

    loadQuery(scanner);
}

void MangoSimilarity::_initQuery(Molecule& query)
{
    MoleculeAromatizer::aromatizeBonds(query, AromaticityOptions::BASIC);
}

double MangoSimilarity::_similarity(int ones1, int ones2, int ones_common, Metrics metrics, double& numerator, double& denominator)
{
    numerator = _numerator(ones1, ones2, ones_common, metrics);

    if (numerator < 1e-6f)
        return 0;

    denominator = _denominator(ones1, ones2, ones_common, metrics);

    if (denominator < 1e-6f)
        throw Error("attempt to divide by zero");

    return (double)numerator / denominator;
}

double MangoSimilarity::getSimilarityScore()
{
    return (double)_numerator_value / _denominator_value;
}

double MangoSimilarity::_numerator(int ones1, int ones2, int ones_common, Metrics metrics)
{
    switch (metrics.type)
    {
    case BIT_METRICS_TANIMOTO:
    case BIT_METRICS_EUCLID_SUB:
        return (double)ones_common;
    case BIT_METRICS_TVERSKY:
        return (double)ones_common;
    default:
        throw Error("bad metrics: %d", metrics.type);
    }
}

double MangoSimilarity::_denominator(int ones1, int ones2, int ones_common, Metrics metrics)
{
    switch (metrics.type)
    {
    case BIT_METRICS_TANIMOTO:
        return (double)(ones1 + ones2 - ones_common);
    case BIT_METRICS_TVERSKY:
        return metrics.tversky_alpha * ones1 + metrics.tversky_beta * ones2 + (1 - metrics.tversky_alpha - metrics.tversky_beta) * ones_common;
    case BIT_METRICS_EUCLID_SUB:
        return (double)ones1;
    default:
        throw Error("bad metrics: %d", metrics.type);
    }
}

int MangoSimilarity::getLowerBound(int target_ones)
{
    switch (metrics.type)
    {
    case BIT_METRICS_TVERSKY:
        return (int)(ceil((metrics.tversky_alpha * _query_ones + metrics.tversky_beta * target_ones) * bottom /
                          (1 - bottom * (1 - metrics.tversky_alpha - metrics.tversky_beta))));
    case BIT_METRICS_TANIMOTO:
        if (bottom < -0.5)
            throw Error("bad lower bound: %lf", bottom);
        return (int)(ceil((_query_ones + target_ones) * bottom / (bottom + 1.0)));
    case BIT_METRICS_EUCLID_SUB:
        return (int)(ceil(_query_ones * bottom));
    default:
        throw Error("bad metrics: %d", metrics.type);
    }
}

int MangoSimilarity::getUpperBound(int target_ones)
{
    switch (metrics.type)
    {
    case BIT_METRICS_TVERSKY:
        return (int)(floor((metrics.tversky_alpha * _query_ones + metrics.tversky_beta * target_ones) * top /
                           (1 - top * (1 - metrics.tversky_alpha - metrics.tversky_beta))));
    case BIT_METRICS_TANIMOTO:
        return (int)(floor((_query_ones + target_ones) * top / (top + 1.0)));
    case BIT_METRICS_EUCLID_SUB:
        return (int)(floor(_query_ones * top));
    default:
        throw Error("bad metrics: %d", metrics.type);
    }
}

bool MangoSimilarity::match(int ones_target, int ones_common)
{
    _numerator_value = _numerator(_query_ones, ones_target, ones_common, metrics);
    _denominator_value = _denominator(_query_ones, ones_target, ones_common, metrics);

    if (_denominator_value < 1e-6f && _numerator_value > 1e-6f)
        throw Error("attempt to divide by zero");

    double top_lim = top * _denominator_value;

    if (include_top && _numerator_value > top_lim + 1e-6f)
        return false;

    if (!include_top && _numerator_value > top_lim - 1e-6f)
        return false;

    double bottom_lim = bottom * _denominator_value;

    if (_numerator_value < 1e-6 && _denominator_value < 1e-6)
    {
        // Similarity is zero
        if (bottom > 0 || (!include_bottom && bottom >= 0))
            return false;
        return true;
    }

    if (include_bottom && _numerator_value < bottom_lim - 1e-6f)
        return false;

    if (!include_bottom && _numerator_value < bottom_lim + 1e-6f)
        return false;

    return true;
}

double MangoSimilarity::calc(Scanner& scanner)
{
    QS_DEF(Molecule, target);

    MoleculeAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadMolecule(target);

    MoleculeAromatizer::aromatizeBonds(target, AromaticityOptions::BASIC);

    QS_DEF(Array<byte>, target_fp);

    MoleculeFingerprintBuilder builder(target, _context.fp_parameters);

    builder.skip_tau = true;
    builder.skip_ext = true;
    builder.skip_ord = true;
    builder.skip_any_atoms = true;
    builder.skip_any_bonds = true;
    builder.skip_any_atoms_bonds = true;

    builder.process();
    target_fp.copy(builder.get(), _context.fp_parameters.fingerprintSize());

    int target_ones = builder.countBits_Sim();

    int common = bitCommonOnes(_query_fp.ptr(), target_fp.ptr(), _context.fp_parameters.fingerprintSize());

    return _similarity(_query_ones, target_ones, common, metrics, _numerator_value, _denominator_value);
}

double MangoSimilarity::calc(const Array<char>& target_buf)
{
    BufferScanner scanner(target_buf);

    return calc(scanner);
}

bool MangoSimilarity::matchBinary(Scanner& scanner)
{
    QS_DEF(Molecule, target);

    CmfLoader loader(_context.cmf_dict, scanner);

    loader.skip_cistrans = true;
    loader.skip_stereocenters = true;
    loader.skip_valence = true;

    loader.loadMolecule(target);

    MoleculeFingerprintBuilder builder(target, _context.fp_parameters);

    builder.skip_tau = true;
    builder.skip_ext = true;
    builder.skip_ord = true;
    builder.skip_any_atoms = true;
    builder.skip_any_bonds = true;
    builder.skip_any_atoms_bonds = true;

    builder.process();

    int common_ones = bitCommonOnes(builder.get(), _query_fp.ptr(), _context.fp_parameters.fingerprintSize());

    int target_ones = builder.countBits_Sim();

    return match(target_ones, common_ones);
}

const byte* MangoSimilarity::getQueryFingerprint()
{
    return _query_fp.ptr();
}
