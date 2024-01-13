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

#include "molecule/molecule_morgan_fingerprint_builder.h"
#include <algorithm>
#include <cmath>
#include <molecule/elements.h>
#include <set>
#include <vector>

using namespace indigo;

MoleculeMorganFingerprintBuilder::MoleculeMorganFingerprintBuilder(BaseMolecule& mol) : mol(mol)
{
}

void MoleculeMorganFingerprintBuilder::calculateDescriptorsECFP(int fp_depth, Array<dword>& res)
{
    initDescriptors(initialStateCallback_ECFP);
    buildDescriptors(fp_depth);

    res.clear();

    for (auto& feature : features)
    {
        res.push(feature.hash);
    }
}

void MoleculeMorganFingerprintBuilder::calculateDescriptorsFCFP(int fp_depth, Array<dword>& res)
{
    initDescriptors(initialStateCallback_FCFP);
    buildDescriptors(fp_depth);

    res.clear();

    for (auto& feature : features)
    {
        res.push(feature.hash);
    }
}

void MoleculeMorganFingerprintBuilder::packFingerprintECFP(int fp_depth, Array<byte>& res)
{
    int size = res.sizeInBytes();

    if (0 == size)
        throw Exception("Resulting array [res] must not be empty");

    initDescriptors(initialStateCallback_ECFP);
    buildDescriptors(fp_depth);

    res.zerofill();

    for (auto& feature : features)
    {
        setBits(feature.hash, res.ptr(), size);
    }
}

void MoleculeMorganFingerprintBuilder::packFingerprintFCFP(int fp_depth, Array<byte>& res)
{
    int size = res.sizeInBytes();

    if (0 == size)
        throw Exception("Resulting array [res] must not be empty");

    initDescriptors(initialStateCallback_FCFP);
    buildDescriptors(fp_depth);

    res.zerofill();

    for (auto& feature : features)
    {
        setBits(feature.hash, res.ptr(), size);
    }
}

void MoleculeMorganFingerprintBuilder::setBits(dword hash, byte* fp, int size)
{
    unsigned seed = hash;

    // fill random bits
    seed = seed * 0x8088405 + 1;

    // Uniformly distributed bits
    unsigned n = (unsigned)(((qword)(size * 8) * seed) / (unsigned)(-1));

    unsigned nByte = n / 8;
    unsigned nBit = n - nByte * 8;

    fp[nByte] = fp[nByte] | (byte)(1 << nBit);
}

void MoleculeMorganFingerprintBuilder::initDescriptors(InitialStateCallback initialStateCallback)
{
    features.clear();
    atom_descriptors.clear();

    for (int idx : mol.vertices())
    {
        AtomDescriptor atom_descriptor;
        atom_descriptor.descr.hash = initialStateCallback(mol, idx);

        const Vertex& vertex = mol.getVertex(idx);

        for (int nei_idx : vertex.neighbors())
        {
            int edge_idx = vertex.neiEdge(nei_idx);
            int vertex_idx = vertex.neiVertex(nei_idx);

            int bond_type = mol.getBondOrder(edge_idx);

            atom_descriptor.bond_descriptors.push_back(BondDescriptor{bond_type, vertex_idx, edge_idx});
        }

        atom_descriptors.push_back(atom_descriptor);
    }
}

void MoleculeMorganFingerprintBuilder::buildDescriptors(int fp_depth)
{
    for (int i = 0; i < fp_depth; i++)
    {
        calculateNewAtomDescriptors(i);

        // Update all atom descriptors simultaneously
        std::vector<FeatureDescriptor> new_features;
        for (auto& atom : atom_descriptors)
        {
            atom.descr = atom.new_descr;

            const auto& duplicate = std::find(new_features.begin(), new_features.end(), atom.descr);
            if (duplicate == new_features.end())
            {
                new_features.push_back(atom.descr);
            }
            else if (atom.descr.hash < duplicate->hash)
            { // the leaser hash is preferred
                new_features.erase(duplicate);
                new_features.push_back(atom.descr);
            }
        }

        // Features are sorted by their iteration number, then by their hash
        std::sort(new_features.begin(), new_features.end(), [](const FeatureDescriptor& fd1, const FeatureDescriptor& fd2) { return fd1.hash < fd2.hash; });

        // Update features
        for (auto& feature : new_features)
        {
            if (std::find(features.begin(), features.end(), feature) == features.end())
            {
                features.push_back(feature); // add unique
            }
        }
    }
}

void MoleculeMorganFingerprintBuilder::calculateNewAtomDescriptors(int iterationNumber)
{
    for (auto& atom : atom_descriptors)
    {
        std::sort(atom.bond_descriptors.begin(), atom.bond_descriptors.end(),
                  [&](const BondDescriptor& bd1, const BondDescriptor& bd2) { return bondDescriptorCmp(bd1, bd2) < 0; });

        atom.new_descr.hash = (dword)iterationNumber * MAGIC_HASH_NUMBER + atom.descr.hash;
        atom.new_descr.bond_set.clear();

        for (auto& bond : atom.bond_descriptors)
        {
            FeatureDescriptor& descr = atom_descriptors[bond.vertex_idx].descr;

            atom.new_descr.hash = MAGIC_HASH_NUMBER * atom.new_descr.hash + bond.bond_type;
            atom.new_descr.hash = MAGIC_HASH_NUMBER * atom.new_descr.hash + descr.hash;

            atom.new_descr.bond_set.insert(bond.edge_idx);
            atom.new_descr.bond_set.insert(descr.bond_set.begin(), descr.bond_set.end());
        }
    }
}

dword MoleculeMorganFingerprintBuilder::initialStateCallback_ECFP(BaseMolecule& mol, int idx)
{
    int nonhydrogen_neighbors = 0;
    const Vertex& vertex = mol.getVertex(idx);
    for (auto i : vertex.neighbors())
    {
        int nei_idx = vertex.neiVertex(i);
        if (mol.getAtomNumber(nei_idx) != ELEM_H)
            nonhydrogen_neighbors += 1;
    }

    double atomic_weight = Element::getStandardAtomicWeight(mol.getAtomNumber(idx));

    dword key = 1;
    key = key * MAGIC_HASH_NUMBER + nonhydrogen_neighbors;
    key = key * MAGIC_HASH_NUMBER + mol.getAtomValence(idx) - mol.getAtomTotalH(idx);
    key = key * MAGIC_HASH_NUMBER + mol.getAtomNumber(idx);
    key = key * MAGIC_HASH_NUMBER + (int)std::round(atomic_weight);
    key = key * MAGIC_HASH_NUMBER + mol.getAtomCharge(idx);
    key = key * MAGIC_HASH_NUMBER + mol.getAtomTotalH(idx);
    key = key * MAGIC_HASH_NUMBER + mol.vertexInRing(idx);

    return key;
}

dword MoleculeMorganFingerprintBuilder::initialStateCallback_FCFP(BaseMolecule& /* mol */, int /* idx */)
{
    throw Exception("FCFP is not implemented"); // TODO: implement ionizability

    /*
    dword key = 0;

    key |= (mol.getAtomAromaticity(0) == ATOM_AROMATIC) << 4;
    key |= Element::isHalogen(mol.getAtomNumber(idx)) << 5;

    return key;
    //*/
}

int MoleculeMorganFingerprintBuilder::bondDescriptorCmp(const BondDescriptor& bd1, const BondDescriptor& bd2)
{
    if (bd1.bond_type != bd2.bond_type)
        return bd1.bond_type < bd2.bond_type ? -1 : 1;

    AtomDescriptor& ad1 = atom_descriptors[bd1.vertex_idx];
    AtomDescriptor& ad2 = atom_descriptors[bd2.vertex_idx];

    return ad1.descr.hash < ad2.descr.hash ? -1 : 1;
}

bool MoleculeMorganFingerprintBuilder::FeatureDescriptor::operator==(const FeatureDescriptor& rhs) const
{
    return bond_set == rhs.bond_set;
}

bool MoleculeMorganFingerprintBuilder::FeatureDescriptor::operator<(const MoleculeMorganFingerprintBuilder::FeatureDescriptor& rhs) const
{
    return bond_set < rhs.bond_set;
}
