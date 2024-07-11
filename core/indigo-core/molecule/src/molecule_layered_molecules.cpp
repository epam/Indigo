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

#include "molecule/molecule_layered_molecules.h"

#include "base_c/defs.h"
#include "base_cpp/output.h"
#include "graph/cycle_enumerator.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "molecule/molecule_standardize.h"

using namespace indigo;

LayeredMolecules::LayeredMolecules(BaseMolecule& molecule) : _layersAromatized(0)
{
    _proto.clone(molecule.asMolecule(), 0, 0);
    _proto.dearomatize(AromaticityOptions());

    cloneGraph(_proto, 0);

    for (auto e_idx : _proto.edges())
    {
        for (auto i = 0; i < BOND_TYPES_NUMBER; ++i)
        {
            _bond_masks[i].push();
            _bond_masks[i].top().resize(1);
        }

        _bond_masks[BOND_ZERO].top().reset(0);
        _bond_masks[BOND_SINGLE].top().reset(0);
        _bond_masks[BOND_DOUBLE].top().reset(0);
        _bond_masks[BOND_TRIPLE].top().reset(0);
        _bond_masks[BOND_AROMATIC].top().reset(0);
        _bond_masks[_proto.getBondOrder(e_idx)].top().set(0);
    }

    _mobilePositions.expandFill(_proto.vertexCount(), false);
    _mobilePositionsOccupied.expand(_proto.vertexCount());

    layers = 1;

    unsigned node = _trie.getRoot();
    for (auto i : _proto.edges())
    {
        bool stub;
        node = _trie.add(node, _proto.getBondOrder(i), stub);
    }
    _hashs.push(node);
}

LayeredMolecules::~LayeredMolecules()
{
}

void LayeredMolecules::constructMolecule(Molecule& molecule, int layer, bool aromatized) const
{
    molecule.clone(const_cast<Molecule&>(_proto), NULL, NULL);
    molecule.clearXyz();
    for (auto i : const_cast<Molecule&>(_proto).edges())
    {
        int order = BOND_ZERO;
        _bond_masks[BOND_SINGLE][i].get(layer) ? order = BOND_SINGLE : 0;
        _bond_masks[BOND_DOUBLE][i].get(layer) ? order = BOND_DOUBLE : 0;
        _bond_masks[BOND_TRIPLE][i].get(layer) ? order = BOND_TRIPLE : 0;
        molecule.setBondOrder(i, order);
    }
    for (auto i : const_cast<Molecule&>(_proto).vertices())
    {
        molecule.setAtomCharge(i, const_cast<Molecule&>(_proto).getAtomCharge(i));
    }
    // Actually I would prefer to aromatize the molecule manually (and much more effective) as far as I have the list of aromatic bonds already.
    // But I don't have any approprite molecule API to do it effectively... :(
    if (aromatized)
        molecule.aromatize(AromaticityOptions());
}

void LayeredMolecules::clear()
{
    BaseMolecule::clear();
}

const Dbitset& LayeredMolecules::getBondMask(int idx, int order) const
{
    return _bond_masks[order][idx];
}

bool LayeredMolecules::isMobilePosition(int idx) const
{
    return _mobilePositions[idx];
}

void LayeredMolecules::setMobilePosition(int idx, bool value)
{
    _mobilePositions[idx] = value;
}

const Dbitset& LayeredMolecules::getMobilePositionOccupiedMask(int idx) const
{
    return _mobilePositionsOccupied[idx];
}

void LayeredMolecules::setMobilePositionOccupiedMask(int idx, Dbitset& mask, bool value)
{
    if (value)
        _mobilePositionsOccupied[idx].orWith(mask);
    else
        _mobilePositionsOccupied[idx].andNotWith(mask);
}

bool LayeredMolecules::addLayersWithInvertedPath(const Dbitset& mask, const Array<int>& edgesPath, int beg, int end, bool forward)
// mask: the mask of layers used as prototypes;
// edgesPath: the path of single-double bonds to be inverted
// edgesPath: a sequence of edges with intercganging single-double bonds that need to be inverted
// beg, end: the mobile positions of hydrogen to swap
// forward: the direction to move the hydrogen
{
    QS_DEF(Array<bool>, edgeIsOnPath); // indicates if an edge is on path that needs to be inverted
    edgeIsOnPath.clear();
    edgeIsOnPath.expandFill(edgeCount(), false);
    for (auto i = 0; i < edgesPath.size(); ++i)
    {
        edgeIsOnPath[edgesPath[i]] = true;
    }

    QS_DEF(Dbitset, maskCopy);
    maskCopy.copy(mask);

    int newTautomerIndex = -1;
    while (!maskCopy.isEmpty())
    {
        newTautomerIndex = layers;

        int prototypeIndex = maskCopy.nextSetBit(0);

        _resizeLayers(newTautomerIndex + 1);

        unsigned node = _trie.getRoot();
        bool unique = false;

        for (auto i = 0; i < edgeCount(); ++i)
        {
            int order = 0;
            if (_bond_masks[BOND_SINGLE][i].get(prototypeIndex))
                order = 1;
            else if (_bond_masks[BOND_DOUBLE][i].get(prototypeIndex))
                order = 2;

            if (edgeIsOnPath[i])
            {
                order = (order == 1 ? 2 : 1);
            }

            bool newlyAdded;
            node = _trie.add(node, order, newlyAdded);
            unique = (newlyAdded ? true : unique);

            _bond_masks[order][i].set(newTautomerIndex);
            _bond_masks[BOND_TRIPLE][i].reset(newTautomerIndex);
            _bond_masks[BOND_AROMATIC][i].reset(newTautomerIndex);
            _bond_masks[order == 1 ? BOND_DOUBLE : BOND_SINGLE][i].reset(newTautomerIndex);
        }
        if (!unique)
        {
            maskCopy.reset(prototypeIndex);
            continue;
        }

        for (auto i = 0; i < _mobilePositionsOccupied.size(); ++i)
        {
            if (_mobilePositionsOccupied[i].get(prototypeIndex))
                _mobilePositionsOccupied[i].set(newTautomerIndex);
        }

        _hashs.push(node);
        ++layers;
        maskCopy.reset(prototypeIndex);
        _mobilePositionsOccupied[forward ? beg : end].reset(newTautomerIndex);
        _mobilePositionsOccupied[forward ? end : beg].set(newTautomerIndex);
    }

    if (newTautomerIndex == layers)
    {
        // This means that we avoided adding non-unique layer, and we need to reduce the size of bitsets.
        _resizeLayers(layers);
        return false;
    }

    return true;
}

bool LayeredMolecules::addLayerFromMolecule(const Molecule& molecule, Array<int>& aam)
{
    Array<int> aam_inverse;
    aam_inverse.expandFill(aam.size(), -1);
    for (int i = 0; i < aam.size(); ++i)
    {
        if (aam[i] != -1)
            aam_inverse[aam[i]] = i;
    }

    unsigned newTautomerIndex = layers;
    _resizeLayers(newTautomerIndex + 1);
    for (auto e1_idx : edges())
    {
        _bond_masks[BOND_ZERO][e1_idx].reset(newTautomerIndex);
        _bond_masks[BOND_SINGLE][e1_idx].reset(newTautomerIndex);
        _bond_masks[BOND_DOUBLE][e1_idx].reset(newTautomerIndex);
        _bond_masks[BOND_TRIPLE][e1_idx].reset(newTautomerIndex);
        _bond_masks[BOND_AROMATIC][e1_idx].reset(newTautomerIndex);
    }

    unsigned node = _trie.getRoot();
    bool unique = false;

    for (auto e2_idx : const_cast<Molecule&>(molecule).edges())
    {
        auto e2 = molecule.getEdge(e2_idx);
        int u2 = e2.beg;
        int v2 = e2.end;
        int u1 = aam_inverse[u2];
        int v1 = aam_inverse[v2];
        if (u1 == -1 || v1 == -1)
            continue;
        int e1_idx = findEdgeIndex(u1, v1);
        if (e1_idx == -1)
        {
            e1_idx = addEdge(u1, v1);
            _proto.addEdge(u1, v1);
            _proto.setBondOrder(e1_idx, BOND_ZERO, false);
            _bond_masks[BOND_ZERO].resize(e1_idx + 1);
            _bond_masks[BOND_SINGLE].resize(e1_idx + 1);
            _bond_masks[BOND_DOUBLE].resize(e1_idx + 1);
            _bond_masks[BOND_TRIPLE].resize(e1_idx + 1);
            _bond_masks[BOND_AROMATIC].resize(e1_idx + 1);
        }
        int order = const_cast<Molecule&>(molecule).getBondOrder(e2_idx);
        _bond_masks[order][e1_idx].set(newTautomerIndex);
    }

    for (auto e1_idx : edges())
    {
        int order = BOND_ZERO;
        if (_bond_masks[BOND_SINGLE][e1_idx].get(newTautomerIndex))
            order = BOND_SINGLE;
        else if (_bond_masks[BOND_DOUBLE][e1_idx].get(newTautomerIndex))
            order = BOND_DOUBLE;
        else if (_bond_masks[BOND_TRIPLE][e1_idx].get(newTautomerIndex))
            order = BOND_TRIPLE;

        bool newlyAdded;
        node = _trie.add(node, order, newlyAdded);
        unique = (newlyAdded ? true : unique);
    }

    if (unique)
    {
        ++layers;
        return true;
    }

    // This means that we avoided adding non-unique layer, and we need to reduce the size of bitsets.
    _resizeLayers(layers);
    return false;
}

int LayeredMolecules::getAtomNumber(int idx)
{
    return _proto.getAtomNumber(idx);
}

int LayeredMolecules::getAtomCharge(int idx)
{
    return _proto.getAtomCharge(idx);
}

int LayeredMolecules::getAtomIsotope(int idx)
{
    return _proto.getAtomIsotope(idx);
}

int LayeredMolecules::getAtomRadical(int idx)
{
    return _proto.getAtomRadical(idx);
}

int LayeredMolecules::getAtomAromaticity(int /* idx */)
{
    //   return _proto.getAtomAromaticity(idx);
    return true;
}

int LayeredMolecules::getExplicitValence(int idx)
{
    return _proto.getExplicitValence(idx);
}

int LayeredMolecules::getAtomValence(int idx)
{
    return _proto.getAtomValence(idx);
}

int LayeredMolecules::getAtomSubstCount(int idx)
{
    return _proto.getAtomSubstCount(idx);
}

int LayeredMolecules::getAtomRingBondsCount(int idx)
{
    return _proto.getAtomSubstCount(idx);
}

int LayeredMolecules::getAtomConnectivity(int /* idx */)
{
    return 0;
}

int LayeredMolecules::getAtomMaxH(int idx)
{
    return getAtomTotalH(idx);
}

int LayeredMolecules::getAtomMinH(int idx)
{
    return getAtomTotalH(idx);
}

int LayeredMolecules::getAtomTotalH(int /* idx */)
{
    throw Error("getAtomTotalH method has no sense for LayeredMolecules");
}

bool LayeredMolecules::isPseudoAtom(int idx)
{
    return _proto.isPseudoAtom(idx);
}

const char* LayeredMolecules::getPseudoAtom(int idx)
{
    return _proto.getPseudoAtom(idx);
}

bool LayeredMolecules::isTemplateAtom(int idx)
{
    return _proto.isTemplateAtom(idx);
}

const char* LayeredMolecules::getTemplateAtom(int idx)
{
    return _proto.getTemplateAtom(idx);
}

const int LayeredMolecules::getTemplateAtomSeqid(int idx)
{
    return _proto.getTemplateAtomSeqid(idx);
}

const char* LayeredMolecules::getTemplateAtomSeqName(int idx)
{
    return _proto.getTemplateAtomSeqName(idx);
}

const int LayeredMolecules::getTemplateAtomTemplateIndex(int idx)
{
    return _proto.getTemplateAtomTemplateIndex(idx);
}

const char* LayeredMolecules::getTemplateAtomClass(int idx)
{
    return _proto.getTemplateAtomClass(idx);
}

const int LayeredMolecules::getTemplateAtomDisplayOption(int idx)
{
    return _proto.getTemplateAtomDisplayOption(idx);
}

bool LayeredMolecules::isRSite(int idx)
{
    return _proto.isRSite(idx);
}

dword LayeredMolecules::getRSiteBits(int idx)
{
    return _proto.getRSiteBits(idx);
}

void LayeredMolecules::allowRGroupOnRSite(int /* atom_idx */, int /* rg_idx */)
{
    throw Error("allowRGroupOnRSite method is not implemented in LayeredMolecules class");
}

int LayeredMolecules::getBondOrder(int /* idx */) const
{
    throw Error("getBondOrder method has no sense for LayeredMolecules");
}

int LayeredMolecules::getBondTopology(int /* idx */)
{
    throw Error("getBondTopology method is not implemented in LayeredMolecules class");
}

bool LayeredMolecules::atomNumberBelongs(int idx, const int* numbers, int count)
{
    return _proto.atomNumberBelongs(idx, numbers, count);
}

bool LayeredMolecules::possibleAtomNumber(int idx, int number)
{
    return _proto.possibleAtomNumber(idx, number);
}

bool LayeredMolecules::possibleAtomNumberAndCharge(int idx, int number, int charge)
{
    return _proto.possibleAtomNumberAndCharge(idx, number, charge);
}

bool LayeredMolecules::possibleAtomNumberAndIsotope(int idx, int number, int isotope)
{
    return _proto.possibleAtomNumberAndIsotope(idx, number, isotope);
}

bool LayeredMolecules::possibleAtomIsotope(int idx, int isotope)
{
    return _proto.possibleAtomIsotope(idx, isotope);
}

bool LayeredMolecules::possibleAtomCharge(int idx, int charge)
{
    return _proto.possibleAtomCharge(idx, charge);
}

void LayeredMolecules::getAtomDescription(int idx, Array<char>& description)
{
    return _proto.getAtomDescription(idx, description);
}

void LayeredMolecules::getBondDescription(int /* idx */, Array<char>& /* description */)
{
    throw Error("getBondDescription method is not implemented in LayeredMolecules class");
}

bool LayeredMolecules::possibleBondOrder(int /* idx */, int /* order */)
{
    throw Error("possibleBondOrder method has no sense for LayeredMolecules");
}

bool LayeredMolecules::isSaturatedAtom(int /* idx */)
{
    throw Error("isSaturatedAtom method is not implemented in LayeredMolecules class");
}

bool LayeredMolecules::bondStereoCare(int /* idx */)
{
    throw Error("bondStereoCare method is not implemented in LayeredMolecules class");
}

bool LayeredMolecules::aromatize(const AromaticityOptions& options)
{
    return aromatize(_layersAromatized, layers, options);
}

bool LayeredMolecules::dearomatize(const AromaticityOptions& options)
{
    return _proto.dearomatize(options);
}

int LayeredMolecules::addAtom(int label)
{
    return _proto.addAtom(label);
}

int LayeredMolecules::addBond(int beg, int end, int order)
{
    return _proto.addBond(beg, end, order);
}

int LayeredMolecules::getImplicitH(int idx, bool impl_h_no_throw)
{
    return _proto.getImplicitH(idx, impl_h_no_throw);
}

void LayeredMolecules::setImplicitH(int idx, int impl_h)
{
    return _proto.setImplicitH(idx, impl_h);
}

void LayeredMolecules::_mergeWithSubmolecule(BaseMolecule& /* bmol */, const Array<int>& /* vertices */, const Array<int>* /* edges */,
                                             const Array<int>& /* mapping */, int /* skip_flags */)
{
    throw Error("_mergeWithSubmolecule method is not implemented in LayeredMolecules class");
}

BaseMolecule* LayeredMolecules::neu()
{
    throw Error("neu method is not implemented in LayeredMolecules class");
}

void LayeredMolecules::_resizeLayers(int newSize)
{
    for (auto i : _proto.edges())
    {
        _bond_masks[BOND_ZERO][i].resize(newSize);
        _bond_masks[BOND_SINGLE][i].resize(newSize);
        _bond_masks[BOND_DOUBLE][i].resize(newSize);
        _bond_masks[BOND_TRIPLE][i].resize(newSize);
        _bond_masks[BOND_AROMATIC][i].resize(newSize);
    }
    for (auto i : _proto.vertices())
    {
        _mobilePositionsOccupied[i].resize(newSize);
    }
}

void LayeredMolecules::_calcConnectivity(int layerFrom, int layerTo)
{
    _connectivity.resize(_proto.vertexEnd());
    for (auto v_idx : _proto.vertices())
    {
        _connectivity[v_idx].expandFill(layerTo, 0);
    }

    for (auto bond_idx : _proto.edges())
    {
        const Edge& edge = _proto.getEdge(bond_idx);
        const Dbitset& bs1 = _bond_masks[BOND_SINGLE][bond_idx];
        const Dbitset& bs2 = _bond_masks[BOND_DOUBLE][bond_idx];
        const Dbitset& bs3 = _bond_masks[BOND_TRIPLE][bond_idx];
        for (auto l = layerFrom; l < layerTo; ++l)
        {
            int order = 0;
            if (bs1.get(l))
                order = 1;
            else if (bs2.get(l))
                order = 2;
            if (bs3.get(l))
                order = 3;
            _connectivity[edge.beg][l] += order;
            _connectivity[edge.end][l] += order;
        }
    }
}

void LayeredMolecules::_calcPiLabels(int layerFrom, int layerTo)
{
    _piLabels.resize(_proto.vertexEnd());
    QS_DEF(Dbitset, skip);
    skip.resize(layers);
    QS_DEF(Array<int>, non_arom_conn);
    QS_DEF(Array<int>, arom_bonds);
    QS_DEF(Array<int>, n_double_ext);
    QS_DEF(Array<int>, n_double_ring);
    non_arom_conn.resize(layers);
    arom_bonds.resize(layers);
    n_double_ext.resize(layers);
    n_double_ring.resize(layers);

    for (auto v_idx : _proto.vertices())
    {
        skip.clear();

        _piLabels[v_idx].expandFill(layers, -1);

        if (!_proto.vertexInRing(v_idx) || !Element::canBeAromatic(_proto.getAtomNumber(v_idx)))
        {
            _piLabels[v_idx].fill(-1);
            continue;
        }

        const Vertex& vertex = _proto.getVertex(v_idx);

        non_arom_conn.fill(0);
        arom_bonds.fill(0);
        n_double_ext.fill(0);
        n_double_ring.fill(0);

        for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        {
            int bond_idx = vertex.neiEdge(i);
            // const Dbitset &bs1 = _bond_masks[BOND_SINGLE][bond_idx];
            const Dbitset& bs2 = _bond_masks[BOND_DOUBLE][bond_idx];
            const Dbitset& bs3 = _bond_masks[BOND_TRIPLE][bond_idx];
            const Dbitset& bsArom = _bond_masks[BOND_AROMATIC][bond_idx];

            for (auto l = layerFrom; l < layerTo; ++l)
            {
                if (bs3.get(l))
                {
                    skip.set(l);
                    continue;
                }
                if (bs2.get(l))
                {
                    if (_proto.getBondTopology(bond_idx) == TOPOLOGY_RING) // needs to be layer by layer
                    {
                        ++n_double_ring[l];
                    }
                    else
                    {
                        if (!_proto.isNitrogenV5(v_idx))
                        // needs to be checked layer by layer
                        {
                            skip.set(l);
                            continue;
                        }
                        else
                            ++n_double_ext[l];
                    }
                }

                if (bsArom.get(l))
                    ++arom_bonds[l];
                else
                    ++non_arom_conn[l];
            }
        }

        for (auto l = layerFrom; l < layerTo; ++l)
        {
            if (skip.get(l))
                continue;
            if (arom_bonds[l] == 0)
            {
                // Verify that this atom has valid valence
                // TBD
            }
            if (n_double_ring[l] > 0)
                _piLabels[v_idx][l] = 1;

            if (n_double_ext[l] > 1)
            {
                _piLabels[v_idx][l] = -1;
                skip.set(l);
            }
            else if (n_double_ext[l] == 1)
            {
                // Only a single external double bond that was accepted in _acceptOutgoingDoubleBond
                // It means that it is C=S, C=O, or C=N, like in O=C1NC=CC(=O)N1
                int atom_number = _proto.getAtomNumber(v_idx);
                if (atom_number == ELEM_S)
                    _piLabels[v_idx][l] = 2;
                _piLabels[v_idx][l] = 0;
            }

            if (_piLabels[v_idx][l] != -1)
                continue;

            int conn = _connectivity[v_idx][l];
            int valence;
            int impl_h;
            Element::calcValence(_proto.getAtomNumber(v_idx), _proto.getAtomCharge(v_idx), 0, conn, valence, impl_h, false);
            conn += impl_h;

            if (arom_bonds[l] != 0)
            {
                // Atom is already aromatic and in general number of hydrogens
                // cannot be deduced. But if atom can have one single or onle
                // double bond while being aromatic then pi label can be calculated

                // Currently not implemented
            }
            int group = Element::group(_proto.getAtomNumber(v_idx));
            int charge = 0;  // getAtomCharge(atom_idx);
            int radical = 0; // getAtomRadical(atom_idx);
            int lonepairs = 0;
            if (BaseMolecule::getVacantPiOrbitals(group, charge, radical, conn, &lonepairs) > 0)
                _piLabels[v_idx][l] = 0;
            else if (lonepairs > 0)
                _piLabels[v_idx][l] = 2;
        }
    }
}

bool LayeredMolecules::_cb_handle_cycle(Graph& /* graph */, const Array<int>& vertices, const Array<int>& /* edges */, void* context)
{
    AromatizationContext* aromatizationContext = (AromatizationContext*)context;
    LayeredMolecules* self = aromatizationContext->self;
    self->_handleCycle(aromatizationContext->layerFrom, aromatizationContext->layerTo, vertices);
    return true;
}

bool LayeredMolecules::_handleCycle(int layerFrom, int layerTo, const Array<int>& path)
{
    // Check Huckel's rule
    QS_DEF(Dbitset, satisfiesRule);
    satisfiesRule.resize(layerTo);
    satisfiesRule.clear();
    for (auto l = layerFrom; l < layerTo; ++l)
    {
        if (_isCycleAromaticInLayer(path.ptr(), path.size(), l))
        {
            satisfiesRule.set(l);
        }
    }
    if (!satisfiesRule.isEmpty())
    {
        _aromatizeCycle(path, satisfiesRule);
        return true;
    }
    return false;
}

bool LayeredMolecules::_isCycleAromaticInLayer(const int* cycle, int cycle_len, int layer)
{
    int count = 0;
    // Check Huckel's rule
    for (int i = 0; i < cycle_len; ++i)
        count += _piLabels[cycle[i]][layer];

    if (((count - 2) % 4) != 0)
        return false;
    return true;
}

void LayeredMolecules::_aromatizeCycle(const Array<int>& cycle, const Dbitset& mask)
{
    for (auto i = 0; i < cycle.size(); ++i)
    {
        const Vertex& vertex = _proto.getVertex(cycle[i]);
        for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
        {
            int bond_idx = vertex.neiEdge(j);
            _bond_masks[BOND_AROMATIC][bond_idx].orWith(mask);
            // We are able to store both aromatic and non-aromatic bonds. But in case we need to store only one type, uncomment next lines.
            //_bond_masks[BOND_ZERO][bond_idx].andNotWith(mask);
            //_bond_masks[BOND_SINGLE][bond_idx].andNotWith(mask);
            //_bond_masks[BOND_DOUBLE][bond_idx].andNotWith(mask);
            //_bond_masks[BOND_TRIPLE][bond_idx].andNotWith(mask);
        }
    }
}

void LayeredMolecules::_registerAromatizedLayers(int layerFrom, int layerTo)
{
    _hashsAromatized.resize(layerTo);
    for (auto l = layerFrom; l < layerTo; ++l)
    {
        unsigned node = _trie.getRoot();
        bool unique = false;
        bool aromatic = false;
        for (auto i : _proto.edges())
        {
            int order = 0;
            if (_bond_masks[BOND_AROMATIC][i].get(l))
            {
                order = 4;
                aromatic = true;
            }
            else
            {
                if (_bond_masks[BOND_SINGLE][i].get(l))
                    order = 1;
                else if (_bond_masks[BOND_DOUBLE][i].get(l))
                    order = 2;
                else if (_bond_masks[BOND_TRIPLE][i].get(l))
                    order = 3;
            }

            node = _trie.add(node, order, unique);
        }
        if (aromatic)
        {
            for (auto i : _proto.vertices())
            {
                int piLabel = 0;
                if (_piLabels[i][l] != -1)
                {
                    piLabel = _piLabels[i][l];
                }
                node = _trie.add(node, piLabel, unique);
            }
            _hashsAromatized[l] = node;
        }
        else
        {
            _hashsAromatized[l] = 0;
        }
    }
}

bool LayeredMolecules::aromatize(int layerFrom, int layerTo, const AromaticityOptions& /* options */)
{
    if (layerFrom == layerTo)
        return false;

    _calcConnectivity(layerFrom, layerTo);
    _calcPiLabels(layerFrom, layerTo);

    CycleEnumerator cycle_enumerator(_proto);

    cycle_enumerator.cb_handle_cycle = _cb_handle_cycle;
    cycle_enumerator.max_length = 22;
    AromatizationContext context;
    context.self = this;
    context.layerFrom = layerFrom;
    context.layerTo = layerTo;
    context.result = false;
    cycle_enumerator.context = &context;
    cycle_enumerator.process();

    _registerAromatizedLayers(layerFrom, layerTo);

    if (layerFrom <= _layersAromatized && _layersAromatized < layerTo)
        _layersAromatized = layerTo;
    return context.result;
}
