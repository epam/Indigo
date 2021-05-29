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

#include "indigo_fingerprints.h"

#include "base_c/bitarray.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "indigo_io.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/reaction.h"
#include "reaction/reaction_fingerprint.h"
#include <math.h>

IndigoFingerprint::IndigoFingerprint() : IndigoObject(FINGERPRINT)
{
}

IndigoFingerprint::~IndigoFingerprint()
{
}

IndigoFingerprint& IndigoFingerprint::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::FINGERPRINT)
        return (IndigoFingerprint&)obj;
    throw IndigoError("%s is not a fingerprint", obj.debugInfo());
}

void _indigoParseMoleculeFingerprintType(MoleculeFingerprintBuilder& builder, const char* type, bool query)
{
    builder.query = query;

    if (type == 0 || *type == 0 || strcasecmp(type, "sim") == 0)
    {
        // similarity
        builder.skip_tau = true;
        builder.skip_ext = true;
        builder.skip_ord = true;
        builder.skip_any_atoms = true;
        builder.skip_any_bonds = true;
        builder.skip_any_atoms_bonds = true;
    }
    else if (strcasecmp(type, "sub") == 0)
    {
        // substructure
        builder.skip_sim = true;
        builder.skip_tau = true;
    }
    else if (strcasecmp(type, "sub-res") == 0)
    {
        // resonance substructure
        builder.skip_sim = true;
        builder.skip_tau = true;
        builder.skip_ord = true;
        builder.skip_any_atoms = true;
        builder.skip_ext_charge = true;
    }
    else if (strcasecmp(type, "sub-tau") == 0)
    {
        // tautomer
        builder.skip_ord = true;
        builder.skip_sim = true;

        // tautomer fingerprint part does already contain all necessary any-bits
        builder.skip_any_atoms = true;
        builder.skip_any_bonds = true;
        builder.skip_any_atoms_bonds = true;
    }
    else if (strcasecmp(type, "full") == 0)
    {
        if (query)
            throw IndigoError("there can not be 'full' fingerprint of a query molecule");
        // full (non-query) fingerprint, do not skip anything
    }
    else
        throw IndigoError("unknown molecule fingerprint type: %s", type);
}

void _indigoParseReactionFingerprintType(ReactionFingerprintBuilder& builder, const char* type, bool query)
{
    builder.query = query;

    if (type == 0 || *type == 0 || strcasecmp(type, "sim") == 0)
    {
        // similarity
        builder.skip_ext = true;
        builder.skip_ord = true;
    }
    else if (strcasecmp(type, "sub") == 0)
        // substructure
        builder.skip_sim = true;
    else if (strcasecmp(type, "full") == 0)
    {
        if (query)
            throw IndigoError("there can not be 'full' fingerprint of a query reaction");
        // full (non-query) fingerprint, do not skip anything
    }
    else
        throw IndigoError("unknown molecule fingerprint type: %s", type);
}

CEXPORT int indigoFingerprint(int item, const char* type)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& mol = obj.getBaseMolecule();
            MoleculeFingerprintBuilder builder(mol, self.fp_params);

            _indigoParseMoleculeFingerprintType(builder, type, mol.isQueryMolecule());
            builder.process();
            AutoPtr<IndigoFingerprint> fp(new IndigoFingerprint());
            fp->bytes.copy(builder.get(), self.fp_params.fingerprintSize());
            return self.addObject(fp.release());
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& rxn = obj.getBaseReaction();
            ReactionFingerprintBuilder builder(rxn, self.fp_params);

            _indigoParseReactionFingerprintType(builder, type, rxn.isQueryReaction());
            builder.process();
            AutoPtr<IndigoFingerprint> fp(new IndigoFingerprint());
            fp->bytes.copy(builder.get(), self.fp_params.fingerprintSizeExtOrdSim() * 2);
            return self.addObject(fp.release());
        }
        else
            throw IndigoError("indigoFingerprint(): accepting only molecules and reactions, got %s", obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFingerprintFromBuffer(const byte* buffer, int size)
{
    INDIGO_BEGIN
    {
        AutoPtr<IndigoFingerprint> fp(new IndigoFingerprint());
        fp->bytes.copy(buffer, size);
        return self.addObject(fp.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFingerprintFromDescriptors(const double* arr, int arr_len, int size, double density)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<byte>, data);
        data.resize(size);
        data.zerofill();

        const int bit_size = 8 * size;

        for (int i = 0; i < arr_len; i++)
        {
            int set_bits_num = (int)round(arr[i] * (density * 10) * bit_size / arr_len);

            int hash = i;
            for (auto cnt = 0; cnt < set_bits_num; cnt++)
            {
                hash = abs(hash * 0x8088405 + 1) % bit_size;
                bitSetBit(data.ptr(), hash, 1);
            }
        }

        AutoPtr<IndigoFingerprint> fp(new IndigoFingerprint());
        fp->bytes.copy(data.ptr(), size);

        return self.addObject(fp.release());
    }
    INDIGO_END(-1);
}

void IndigoFingerprint::toString(std::string& str)
{
    StringOutput output(str);
    int i;

    for (i = 0; i < bytes.size(); i++)
        output.printf("%02x", bytes[i]);
}

void IndigoFingerprint::toBuffer(std::string& buf)
{
    buf.copy((char*)bytes.ptr(), bytes.size());
}

static float _indigoSimilarity2(const byte* arr1, const byte* arr2, int size, const char* metrics)
{
    int ones1 = bitGetOnesCount(arr1, size);
    int ones2 = bitGetOnesCount(arr2, size);
    int common_ones = bitCommonOnes(arr1, arr2, size);

    if (metrics == 0 || metrics[0] == 0 || strcasecmp(metrics, "tanimoto") == 0)
    {
        if (common_ones == 0)
            return 0.f;

        return (float)common_ones / (ones1 + ones2 - common_ones);
    }
    else if (strlen(metrics) >= 7 && strncasecmp(metrics, "tversky", 7) == 0)
    {
        float alpha = 0.5f, beta = 0.5f;

        const char* params = metrics + 7;

        if (*params != 0)
        {
            BufferScanner scanner(params);
            if (!scanner.tryReadFloat(alpha))
                throw IndigoError("unknown metrics: %s", metrics);
            scanner.skipSpace();
            if (!scanner.tryReadFloat(beta))
                throw IndigoError("unknown metrics: %s", metrics);
        }
        if (common_ones == 0)
            return 0;

        float denom = (ones1 - common_ones) * alpha + (ones2 - common_ones) * beta + common_ones;

        if (denom < 1e-6f)
            throw IndigoError("bad denominator");

        return common_ones / denom;
    }
    else if (strcasecmp(metrics, "euclid-sub") == 0)
    {
        if (common_ones == 0)
            return 0;

        return (float)common_ones / ones1;
    }
    else
        throw IndigoError("unknown metrics: %s", metrics);
}

static float _indigoSimilarity(Array<byte>& arr1, Array<byte>& arr2, const char* metrics)
{
    int size = arr1.size();

    if (size != arr2.size())
        throw IndigoError("fingerprint sizes do not match (%d and %d)", arr1.size(), arr2.size());

    return _indigoSimilarity2(arr1.ptr(), arr2.ptr(), size, metrics);
}

static void _collectAtomFeatures(BaseMolecule& m, RedBlackStringMap<int>& counters, bool with_degrees)
{
    QS_DEF(std::string, symbol);
    for (int i = m.vertexBegin(); i != m.vertexEnd(); i = m.vertexNext(i))
    {
        int iso = 0, charge = 0, radical = 0;
        if (!m.isRSite(i) && !m.isPseudoAtom(i))
        {
            iso = m.getAtomIsotope(i);
            charge = m.getAtomCharge(i);
            radical = m.getAtomRadical_NoThrow(i, -1);
        }
        int degree = 0;
        if (with_degrees)
            degree = m.getVertex(i).degree();

        m.getAtomSymbol(i, symbol);

        char key[100];
        snprintf(key, NELEM(key), "e:%s i:%d c:%d r:%d d:%d", symbol.ptr(), iso, charge, radical, degree);

        int* ptr = counters.at2(key);
        if (ptr)
            (*ptr)++;
        else
            counters.insert(key, 1);
    }
}

static void _collectBondFeatures(BaseMolecule& m, RedBlackStringMap<int>& counters, bool with_degrees)
{
    QS_DEF(std::string, symbol);
    for (int i = m.edgeBegin(); i != m.edgeEnd(); i = m.edgeNext(i))
    {
        const Edge& e = m.getEdge(i);
        int d1 = m.getVertex(e.beg).degree();
        int d2 = m.getVertex(e.end).degree();

        const char* stereo = "";

        int parity = m.cis_trans.getParity(i);
        if (parity)
        {
            // Set cis-trans parity only there is one substituent
            // In other case atoms ordering is important
            if (d1 == 2 && d2 == 2)
                stereo = (parity == MoleculeCisTrans::CIS) ? "cis" : "trans";
        }

        int dir = m.getBondDirection(i);
        if (dir == BOND_UP)
            stereo = "up";
        else if (dir == BOND_DOWN)
            stereo = "down";
        else if (dir == BOND_EITHER)
            stereo = "up-down";

        if (!with_degrees)
            d1 = d2 = 0;

        char key[100];
        snprintf(key, NELEM(key), "o:%d s:%s d:%d %d", m.getBondOrder(i), stereo, __min(d1, d2), __max(d1, d2));

        int* ptr = counters.at2(key);
        if (ptr)
            (*ptr)++;
        else
            counters.insert(key, 1);
    }
}

static void _getCountersDifference(RedBlackStringMap<int>& c1, RedBlackStringMap<int>& c2, int& common, int& diff1, int& diff2)
{
    common = 0;
    diff1 = 0;
    diff2 = 0;

    int* diff[2] = {&diff1, &diff2};
    RedBlackStringMap<int>* cnt[2] = {&c1, &c2};
    for (int i = 0; i < 2; i++)
    {
        RedBlackStringMap<int>&a = *cnt[i], &b = *cnt[1 - i];

        for (int node = a.begin(); node != a.end(); node = a.next(node))
        {
            const char* key = a.key(node);
            int val1 = a.value(node);
            int* val2_ptr = b.at2(key);
            int val2 = val2_ptr ? *val2_ptr : 0;

            int c = __min(val1, val2);
            common += c;
            *diff[i] += val1 - c;
        }
    }

    // Common values were counted twice
    common /= 2;
}

static float _indigoSimilarityNormalizedEdit(BaseMolecule& mol1, BaseMolecule& mol2)
{
    QS_DEF(RedBlackStringMap<int>, c1);
    QS_DEF(RedBlackStringMap<int>, c2);
    QS_DEF(RedBlackStringMap<int>, c1b);
    QS_DEF(RedBlackStringMap<int>, c2b);

    for (int iter = 0; iter < 2; iter++)
    {
        c1.clear();
        c2.clear();

        bool need_degree = (iter == 1);

        _collectAtomFeatures(mol1, c1, need_degree);
        _collectAtomFeatures(mol2, c2, need_degree);
        _collectBondFeatures(mol1, c1, need_degree);
        _collectBondFeatures(mol2, c2, need_degree);

        int common, diff1, diff2;
        _getCountersDifference(c1, c2, common, diff1, diff2);
        if (diff1 != 0 || diff2 != 0)
        {
            // Use Tversky Index
            float alpha = 0.7f, beta = 0.7f;

            float sim = common / (alpha * diff1 + beta * diff2 + common);
            float scaling = sim * sim * (3 - 2 * sim);
            return sim * scaling;
        }
    }

    return 1;
}

CEXPORT float indigoSimilarity(int item1, int item2, const char* metrics)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj1 = self.getObject(item1);
        IndigoObject& obj2 = self.getObject(item2);

        if (strcasecmp(metrics, "normalized-edit") == 0)
        {
            return _indigoSimilarityNormalizedEdit(obj1.getBaseMolecule(), obj2.getBaseMolecule());
        }

        if (IndigoBaseMolecule::is(obj1))
        {
            Molecule& mol1 = obj1.getMolecule();
            Molecule& mol2 = obj2.getMolecule();

            MoleculeFingerprintBuilder builder1(mol1, self.fp_params);
            MoleculeFingerprintBuilder builder2(mol2, self.fp_params);

            _indigoParseMoleculeFingerprintType(builder1, "sim", false);
            _indigoParseMoleculeFingerprintType(builder2, "sim", false);

            builder1.process();
            builder2.process();

            return _indigoSimilarity2(builder1.getSim(), builder2.getSim(), self.fp_params.fingerprintSizeSim(), metrics);
        }
        else if (IndigoBaseReaction::is(obj1))
        {
            Reaction& rxn1 = obj1.getReaction();
            Reaction& rxn2 = obj2.getReaction();

            ReactionFingerprintBuilder builder1(rxn1, self.fp_params);
            ReactionFingerprintBuilder builder2(rxn2, self.fp_params);

            _indigoParseReactionFingerprintType(builder1, "sim", false);
            _indigoParseReactionFingerprintType(builder2, "sim", false);

            builder1.process();
            builder2.process();

            return _indigoSimilarity2(builder1.getSim(), builder2.getSim(), self.fp_params.fingerprintSizeSim() * 2, metrics);
        }
        else if (self.getObject(item1).type == IndigoObject::FINGERPRINT)
        {
            IndigoFingerprint& fp1 = IndigoFingerprint::cast(obj1);
            IndigoFingerprint& fp2 = IndigoFingerprint::cast(obj2);

            return _indigoSimilarity(fp1.bytes, fp2.bytes, metrics);
        }
        else
            throw IndigoError("indigoSimilarity(): can not accept %s", obj1.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountBits(int fingerprint)
{
    INDIGO_BEGIN
    {
        IndigoFingerprint& fp = IndigoFingerprint::cast(self.getObject(fingerprint));
        return bitGetOnesCount(fp.bytes.ptr(), fp.bytes.size());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCommonBits(int fingerprint1, int fingerprint2)
{
    INDIGO_BEGIN
    {
        Array<byte>& fp1 = IndigoFingerprint::cast(self.getObject(fingerprint1)).bytes;
        Array<byte>& fp2 = IndigoFingerprint::cast(self.getObject(fingerprint2)).bytes;

        if (fp1.size() != fp2.size())
            throw IndigoError("fingerprint sizes do not match (%d and %d)", fp1.size(), fp2.size());

        return bitCommonOnes(fp1.ptr(), fp2.ptr(), fp1.size());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoOneBitsList(int fingerprint)
{
    INDIGO_BEGIN
    {
        Array<byte>& fp = IndigoFingerprint::cast(self.getObject(fingerprint)).bytes;
        auto& tmp = self.getThreadTmpData();
        StringOutput out(tmp.string);
        tmp.string.clear();
        for (int i = 0; i < fp.sizeInBytes() * 8; ++i)
        {
            if (bitGetBit(fp.ptr(), i) > 0)
            {
                if (tmp.string.size() > 0)
                {
                    out.writeString(" ");
                }
                out.printf("%d", i);
            }
        }

        tmp.string.push(0);

        return tmp.string.ptr();
    }
    INDIGO_END(0);
}