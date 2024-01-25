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

#include "molecule/molecule_dearom.h"

#include "base_c/bitarray.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "graph/filter.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom.h"
#include "molecule/query_molecule.h"

using namespace indigo;

// Exceptions
IMPL_EXCEPTION(indigo, DearomatizationException, "dearomatization");
IMPL_EXCEPTION2(indigo, NonUniqueDearomatizationException, DearomatizationException, "non-unique dearomatization");

//
// Indigo aromaticiy model remarks.
// C1=CC2=CC=CC=CC2=C1 is aromatized into c1:c2-c(:c:c:c:c:c:2):c:c:1 but not into
//    c1cc2cccccc2c1 because single bond is not aromatic.
//
// O=C1C=CC(=O)C2=C1SC=CS2 is not aromatized to O=c1ccc(=O)c2sccsc12 because
//    of double bond inside cycle that is prohibited. Big cycle has 4n+2 electrons
//    but is not treated as aromatic because of this double bond.
//

thread_local int _dearomatizationParams = Dearomatizer::PARAMS_SAVE_ONE_DEAROMATIZATION;

CP_DEF(Dearomatizer);

Dearomatizer::Dearomatizer(BaseMolecule& molecule, const int* atom_external_conn, const AromaticityOptions& options)
    : _graphMatching(molecule), _molecule(molecule), _options(options), _aromaticGroups(molecule, options.aromatize_skip_superatoms), CP_INIT,
      TL_CP_GET(_aromaticGroupData),
      // TL_CP_GET(_edgesFixed),
      // TL_CP_GET(_verticesFixed),
      TL_CP_GET(_submoleculeMapping)
{
    _isQueryMolecule = _molecule.isQueryMolecule();
    _edgesFixed.resize(_molecule.edgeEnd());
    _verticesFixed.resize(_molecule.vertexEnd());
    _verticesFixed.zeroFill();

    _connectivityGroups = _aromaticGroups.detectAromaticGroups(atom_external_conn);

    _initVertices();
    _initEdges();

    _graphMatching.setFixedInfo(&_edgesFixed, &_verticesFixed);
}

Dearomatizer::~Dearomatizer()
{
}

void Dearomatizer::setDearomatizationParams(int params)
{
    _dearomatizationParams = params;
}

// Enumerate all dearomatizations for all connectivity groups
void Dearomatizer::enumerateDearomatizations(DearomatizationsStorage& dearomatizations)
{
    dearomatizations.clear();
    if (_connectivityGroups == 0)
        return;
    _dearomatizations = &dearomatizations;

    Molecule submolecule;
    QueryMolecule qsubmolecule;

    dearomatizations.setGroupsCount(_connectivityGroups);
    dearomatizations.setDearomatizationParams(_dearomatizationParams);

    _aromaticGroups.constructGroups(dearomatizations, true);

    for (int group = 0; group < _connectivityGroups; group++)
    {
        _activeGroup = group;
        if (_isQueryMolecule)
            _prepareGroup(group, qsubmolecule);
        else
            _prepareGroup(group, submolecule);

        GrayCodesEnumerator grayCodes(_aromaticGroupData.heteroAtoms.size(), true);
        do
        {
            if (_graphMatching.findMatching())
                if (_isQueryMolecule)
                    _processMatching(qsubmolecule, group, grayCodes.getCode());
                else
                    _processMatching(submolecule, group, grayCodes.getCode());

            grayCodes.next();

            if (!grayCodes.isDone())
            {
                int heteroAtomToInvert = _aromaticGroupData.heteroAtoms[grayCodes.getBitChangeIndex()];
                _fixHeteratom(heteroAtomToInvert, !_verticesFixed.get(heteroAtomToInvert));
            }
        } while (!grayCodes.isDone());
    }
}

void Dearomatizer::_fixHeteratom(int atom_idx, bool toFix)
{

    if (!_verticesFixed.get(atom_idx))
    {
        if (_graphMatching.isVertexInMatching(atom_idx))
            _graphMatching.removeVertexFromMatching(atom_idx);

        _verticesFixed.set(atom_idx);
    }
    else
        _verticesFixed.reset(atom_idx);
    return;
}

void Dearomatizer::_initVertices(void)
{
    for (int v_idx = _molecule.vertexBegin(); v_idx < _molecule.vertexEnd(); v_idx = _molecule.vertexNext(v_idx))
    {
        if (_molecule.getAtomAromaticity(v_idx) == ATOM_ALIPHATIC)
            _verticesFixed.set(v_idx);
    }
}

// Find all aromatic bonds
void Dearomatizer::_initEdges(void)
{
    for (int e_idx = _molecule.edgeBegin(); e_idx < _molecule.edgeEnd(); e_idx = _molecule.edgeNext(e_idx))
    {
        bool non_aromatic = _molecule.getBondOrder(e_idx) != BOND_AROMATIC;
        if (non_aromatic && _isQueryMolecule)
            non_aromatic = !_molecule.asQueryMolecule().possibleAromaticBond(e_idx);
        _edgesFixed.set(e_idx, non_aromatic);
    }
}

void Dearomatizer::_enumerateMatching(void)
{
    // Find strong edge in alternating circle
    const Edge* edge = 0;
    int e_idx;
    bool found = false;
    for (int i = 0; i < _aromaticGroupData.bonds.size(); i++)
    {
        e_idx = _aromaticGroupData.bonds[i];
        if (!_edgesFixed.get(e_idx) && _graphMatching.isEdgeMatching(e_idx))
        {
            edge = &(_molecule.getEdge(e_idx));

            if (_graphMatching.findAlternatingPath(edge->beg, edge->end, false, false))
            {
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        _handleMatching();
        return;
    }

    const int MAX_PATH_SIZE = 100;
    int pathSize = _graphMatching.getPathSize();
    int path[MAX_PATH_SIZE];
    memcpy(path, _graphMatching.getPath(), sizeof(int) * pathSize);

    // Enumerate all matching with this strong edge
    _verticesFixed.set(edge->beg);
    _verticesFixed.set(edge->end);
    _enumerateMatching();
    _verticesFixed.reset(edge->beg);
    _verticesFixed.reset(edge->end);

    // Enumerate all matching without this strong edge

    _graphMatching.setPath(path, pathSize);
    _graphMatching.setEdgeMatching(e_idx, false);
    _graphMatching.processPath();
    _edgesFixed.set(e_idx);

    _enumerateMatching();

    _edgesFixed.reset(e_idx);
    _graphMatching.setPath(path, pathSize);
    _graphMatching.processPath();
    _graphMatching.setEdgeMatching(e_idx, true);
}

void Dearomatizer::_handleMatching(void)
{
    // Add dearomatizations
    _dearomatizations->addGroupDearomatization(_activeGroup, _graphMatching.getEdgesState());
}

static void dearomatizeQueryBond(QueryMolecule& qmol, int idx, int order)
{
    qmol.resetBond(idx, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order));
    // Clear 'Aromatic' at bond vertex. Disabled for #1472.
    // Edge edg = qmol.getEdge(idx);
    // if (qmol.getAtom(edg.beg).hasConstraintWithValue(QueryMolecule::ATOM_AROMATICITY, ATOM_AROMATIC))
    //     qmol.getAtom(edg.beg).removeConstraints(QueryMolecule::ATOM_AROMATICITY);
    // if (qmol.getAtom(edg.end).hasConstraintWithValue(QueryMolecule::ATOM_AROMATICITY, ATOM_AROMATIC))
    //     qmol.getAtom(edg.end).removeConstraints(QueryMolecule::ATOM_AROMATICITY);
}

void Dearomatizer::_processMatching(BaseMolecule& submolecule, int group, const byte* hetroAtomsState)
{
    bool isQuery = submolecule.isQueryMolecule();
    // Copy bonds
    for (int e_idx = submolecule.edgeBegin(); e_idx < submolecule.edgeEnd(); e_idx = submolecule.edgeNext(e_idx))
    {
        if (submolecule.getBondTopology(e_idx) != TOPOLOGY_RING)
            // Do not change any bond orders that are not in rings
            continue;
        const Edge& edge = submolecule.getEdge(e_idx);
        int supIdx = _molecule.findEdgeIndex(_submoleculeMapping[edge.beg], _submoleculeMapping[edge.end]);

        if (_graphMatching.isEdgeMatching(supIdx))
            if (isQuery)
                dearomatizeQueryBond(submolecule.asQueryMolecule(), e_idx, BOND_DOUBLE);
            else
                submolecule.asMolecule().setBondOrder(e_idx, BOND_DOUBLE);
        else if (isQuery)
            dearomatizeQueryBond(submolecule.asQueryMolecule(), e_idx, BOND_SINGLE);
        else
            submolecule.asMolecule().setBondOrder(e_idx, BOND_SINGLE);
    }

    // Check aromaticity
    bool valid = true;
    if (_options.dearomatize_check)
    {
        // Check configuration only if antiaromaticity is not allowed
        // For example structure c1ccc1 is not aromatic but antiaromatic, and
        // kekulized form is C1=CC=C1
        // Dearomatization without verification can be used for finding kekulized form
        // that is not necessary aromatic
        if (isQuery)
            QueryMoleculeAromatizer::aromatizeBonds(submolecule.asQueryMolecule(), _options);
        else
            MoleculeAromatizer::aromatizeBonds(submolecule.asMolecule(), _options);
        for (int e_idx = submolecule.edgeBegin(); e_idx < submolecule.edgeEnd(); e_idx = submolecule.edgeNext(e_idx))
        {
            if (submolecule.getBondTopology(e_idx) == TOPOLOGY_RING && submolecule.getBondOrder(e_idx) != BOND_AROMATIC)
            {
                valid = false;
                break;
            }
        }
    }

    if (valid)
    {
        if (_dearomatizationParams == PARAMS_SAVE_ALL_DEAROMATIZATIONS)
            // Enumerate all equivalent dearomatizations
            _enumerateMatching();
        else if (_dearomatizationParams == PARAMS_SAVE_ONE_DEAROMATIZATION)
            _handleMatching();
        else if (_dearomatizationParams == PARAMS_SAVE_JUST_HETERATOMS)
            _dearomatizations->addGroupHeteroAtomsState(group, hetroAtomsState);
    }
}

void Dearomatizer::_prepareGroup(int group, BaseMolecule& submolecule)
{
    _aromaticGroups.getGroupData(group, DearomatizationsGroups::GET_VERTICES_FILTER | DearomatizationsGroups::GET_HETERATOMS_INDICES, &_aromaticGroupData);

    Filter filter(_aromaticGroupData.verticesFilter.ptr(), Filter::EQ, 1);
    submolecule.makeSubmolecule(_molecule, filter, &_submoleculeMapping, NULL, SKIP_ALL);
    // Remove non-aromatic bonds
    for (int e_idx = submolecule.edgeBegin(); e_idx < submolecule.edgeEnd(); e_idx = submolecule.edgeNext(e_idx))
    {
        // Keep double bonds too
        if (submolecule.getBondOrder(e_idx) == BOND_SINGLE)
            submolecule.removeEdge(e_idx);
    }

    for (int i = 0; i < _aromaticGroupData.vertices.size(); i++)
    {
        int v_idx = _aromaticGroupData.vertices[i];
        if (!_aromaticGroups.isAcceptDoubleBond(v_idx))
            _verticesFixed.set(v_idx);
        else
            _verticesFixed.reset(v_idx);
    }
    for (int i = 0; i < _aromaticGroupData.heteroAtoms.size(); i++)
    {
        int hetero_idx = _aromaticGroupData.heteroAtoms[i];
        _verticesFixed.set(hetero_idx);
    }

    _graphMatching.reset();
    _graphMatching.setEdgesMappingPtr(_aromaticGroupData.bondsInvMapping.ptr());
    _graphMatching.setVerticesSetPtr(_aromaticGroupData.vertices.ptr(), _aromaticGroupData.vertices.size());
}

//
// Dearomatizer::DearomatizerGraphMatching
//

bool Dearomatizer::GraphMatchingFixed::checkVertex(int v_idx)
{
    return !_verticesFixed->get(v_idx);
}

bool Dearomatizer::GraphMatchingFixed::checkEdge(int e_idx)
{
    return !_edgesFixed->get(e_idx);
}

void Dearomatizer::GraphMatchingFixed::setFixedInfo(const Dbitset* edgesFixed, const Dbitset* verticesFixed)
{
    _edgesFixed = edgesFixed;
    _verticesFixed = verticesFixed;
}

Dearomatizer::GraphMatchingFixed::GraphMatchingFixed(BaseMolecule& molecule) : GraphPerfectMatching(molecule, USE_VERTICES_SET | USE_EDGES_MAPPING)
{
}

//
// Dearomatizations
//

void DearomatizationsStorage::clear(void)
{
    _heteroAtomsStateArray.clear();
    _aromaticGroups.clear();
    clearIndices();
    clearBondsState();
    _dearomParams = Dearomatizer::PARAMS_NO_DEAROMATIZATIONS;
}

void DearomatizationsStorage::clearIndices(void)
{
    _aromBondsArray.clear();
    _heteroAtomsIndicesArray.clear();
}

void DearomatizationsStorage::clearBondsState(void)
{
    _dearomBondsStateArray.clear();
    for (int i = 0; i < _aromaticGroups.size(); i++)
    {
        _aromaticGroups[i].dearomBondsState.count = 0;
        _aromaticGroups[i].dearomBondsState.offset = 0;
    }
}

void DearomatizationsStorage::setGroupsCount(int groupsCount)
{
    _aromaticGroups.resize(groupsCount);
    _aromaticGroups.zerofill();
}

void DearomatizationsStorage::setGroup(int group, int boundsCount, const int* bondsPtr, int heteroAtomsCount, const int* hetroAtoms)
{
    _aromaticGroups[group].aromBondsIndices.count = boundsCount;
    _aromaticGroups[group].aromBondsIndices.offset = _aromBondsArray.size();

    if (_dearomParams == Dearomatizer::PARAMS_SAVE_JUST_HETERATOMS)
    {
        _aromaticGroups[group].heteroAtomsIndices.count = heteroAtomsCount;
        _aromaticGroups[group].heteroAtomsIndices.offset = _heteroAtomsIndicesArray.size();
        for (int i = 0; i < heteroAtomsCount; i++)
            _heteroAtomsIndicesArray.push(hetroAtoms[i]);
    }
    else
    {
        _aromaticGroups[group].heteroAtomsIndices.count = 0;
        _aromaticGroups[group].heteroAtomsIndices.offset = _heteroAtomsIndicesArray.size();
    }

    for (int i = 0; i < boundsCount; i++)
        _aromBondsArray.push(bondsPtr[i]);
}

void DearomatizationsStorage::addGroupDearomatization(int group, const byte* dearomBondsState)
{
    // Check group
    int dearomStateSize = bitGetSize(_aromaticGroups[group].aromBondsIndices.count);
    int expectedOffset = _dearomBondsStateArray.size() - dearomStateSize * _aromaticGroups[group].dearomBondsState.count;

    if (_aromaticGroups[group].dearomBondsState.count != 0 && _aromaticGroups[group].dearomBondsState.offset != expectedOffset)
        throw Error("Dearomatizations::addGroupDearomatization: unable to add dearomatization");

    if (_aromaticGroups[group].dearomBondsState.count == 0)
        _aromaticGroups[group].dearomBondsState.offset = _dearomBondsStateArray.size();

    // Add dearomatization to group
    for (int i = 0; i < dearomStateSize; i++)
        _dearomBondsStateArray.push(dearomBondsState[i]);
    _aromaticGroups[group].dearomBondsState.count++;
}

void DearomatizationsStorage::addGroupHeteroAtomsState(int group, const byte* heteroAtomsState)
{
    // Check group
    int heteroAtomsSize = bitGetSize(_aromaticGroups[group].heteroAtomsIndices.count);
    int expectedOffset = _heteroAtomsStateArray.size() - heteroAtomsSize * _aromaticGroups[group].heteroAtomsState.count;

    if (_aromaticGroups[group].heteroAtomsState.count != 0 && _aromaticGroups[group].heteroAtomsState.offset != expectedOffset)
        throw Error("Dearomatizations::addGroupHeteroAtomsState: unable to add heteroatoms state");

    if (_aromaticGroups[group].heteroAtomsState.count == 0)
        _aromaticGroups[group].heteroAtomsState.offset = _heteroAtomsStateArray.size();

    for (int i = 0; i < heteroAtomsSize; i++)
        _heteroAtomsStateArray.push(heteroAtomsState[i]);
    _aromaticGroups[group].heteroAtomsState.count++;
}

// Bonds state for dearomatization
int DearomatizationsStorage::getGroupDearomatizationsCount(int group) const
{
    return _aromaticGroups[group].dearomBondsState.count;
}

byte* DearomatizationsStorage::getGroupDearomatization(int group, int dearomatizationIndex)
{
    int offset = _aromaticGroups[group].dearomBondsState.offset + dearomatizationIndex * bitGetSize(_aromaticGroups[group].aromBondsIndices.count);

    if (offset >= _dearomBondsStateArray.size())
        return 0;
    return &_dearomBondsStateArray[offset];
}

const int* DearomatizationsStorage::getGroupBonds(int group) const
{
    int offset = _aromaticGroups[group].aromBondsIndices.offset;
    if (offset >= _aromBondsArray.size())
        return 0;
    return &_aromBondsArray[offset];
}

int DearomatizationsStorage::getGroupBondsCount(int group) const
{
    return _aromaticGroups[group].aromBondsIndices.count;
}

int DearomatizationsStorage::getGroupsCount(void) const
{
    return _aromaticGroups.size();
}

// Heteroatoms
int DearomatizationsStorage::getGroupHeterAtomsStateCount(int group) const
{
    return _aromaticGroups[group].heteroAtomsState.count;
}

const byte* DearomatizationsStorage::getGroupHeterAtomsState(int group, int index) const
{
    int offset = _aromaticGroups[group].heteroAtomsState.offset + index * bitGetSize(_aromaticGroups[group].heteroAtomsIndices.count);
    return _heteroAtomsStateArray.ptr() + offset;
}

const int* DearomatizationsStorage::getGroupHeteroAtoms(int group) const
{
    return _heteroAtomsIndicesArray.ptr() + _aromaticGroups[group].heteroAtomsIndices.offset;
}

int DearomatizationsStorage::getGroupHeteroAtomsCount(int group) const
{
    return _aromaticGroups[group].heteroAtomsIndices.count;
}

// I/O
void DearomatizationsStorage::saveBinary(Output& output) const
{
    output.writeByte(_dearomParams);
    output.writePackedShort(_aromaticGroups.size());
    if (_dearomParams != Dearomatizer::PARAMS_SAVE_JUST_HETERATOMS)
    {
        for (int i = 0; i < _aromaticGroups.size(); i++)
        {
            int expectedOffset = 0;
            if (i != 0)
                expectedOffset = _aromaticGroups[i - 1].dearomBondsState.offset +
                                 _aromaticGroups[i - 1].dearomBondsState.count * bitGetSize(_aromaticGroups[i - 1].aromBondsIndices.count);
            if (i != 0 && _aromaticGroups[i].dearomBondsState.offset != expectedOffset)
                throw Error("DearomatizationsStorage::saveBinary: invalid data order #1");
            output.writePackedShort(_aromaticGroups[i].dearomBondsState.count);
        }
        output.writePackedShort(_dearomBondsStateArray.size());
        if (_dearomBondsStateArray.size() != 0)
            output.write(_dearomBondsStateArray.ptr(), _dearomBondsStateArray.size() * sizeof(byte));
    }
    else
    {
        for (int i = 0; i < _aromaticGroups.size(); i++)
        {
            int expectedOffset = 0;
            if (i != 0)
                expectedOffset = _aromaticGroups[i - 1].heteroAtomsState.offset +
                                 _aromaticGroups[i - 1].heteroAtomsState.count * bitGetSize(_aromaticGroups[i - 1].heteroAtomsIndices.count);
            if (i != 0 && _aromaticGroups[i].heteroAtomsState.offset != expectedOffset)
                throw Error("DearomatizationsStorage::saveBinary: invalid data order #2");
            output.writePackedShort(_aromaticGroups[i].heteroAtomsState.count);
        }

        output.writePackedShort(_heteroAtomsStateArray.size());
        if (_heteroAtomsStateArray.size() != 0)
            output.write(_heteroAtomsStateArray.ptr(), _heteroAtomsStateArray.size() * sizeof(byte));
    }
}

void DearomatizationsStorage::loadBinary(Scanner& scanner)
{
    clear();

    _dearomParams = scanner.readChar();
    short groupsCount = scanner.readPackedShort();
    _aromaticGroups.resize(groupsCount);
    _aromaticGroups.zerofill();

    if (_dearomParams != Dearomatizer::PARAMS_SAVE_JUST_HETERATOMS)
    {
        for (int i = 0; i < groupsCount; i++)
        {
            short count = scanner.readPackedShort();
            if (i != 0)
                _aromaticGroups[i].dearomBondsState.offset = _aromaticGroups[i - 1].dearomBondsState.offset + count;
            _aromaticGroups[i].dearomBondsState.count = count;
        }
        short bondsStateSize = scanner.readPackedShort();
        _dearomBondsStateArray.resize(bondsStateSize);
        if (bondsStateSize != 0)
            scanner.read(bondsStateSize, _dearomBondsStateArray.ptr());
    }
    else
    {
        for (int i = 0; i < groupsCount; i++)
        {
            short count = scanner.readPackedShort();
            if (i != 0)
                _aromaticGroups[i].heteroAtomsState.offset = _aromaticGroups[i - 1].heteroAtomsState.offset + count;
            _aromaticGroups[i].heteroAtomsState.count = count;
        }

        short heteroAtomsStateSize = scanner.readPackedShort();
        _heteroAtomsStateArray.resize(heteroAtomsStateSize);
        if (heteroAtomsStateSize)
            scanner.read(heteroAtomsStateSize, _heteroAtomsStateArray.ptr());
    }
}

IMPL_ERROR2(DearomatizationsStorage, DearomatizationException, "Dearomatization storage");

DearomatizationsStorage::DearomatizationsStorage(void)
{
    _dearomParams = Dearomatizer::PARAMS_NO_DEAROMATIZATIONS;
}

//
// DearomatizationsGroups
//

IMPL_ERROR2(DearomatizationsGroups, DearomatizationException, "Dearomatization groups");
CP_DEF(DearomatizationsGroups);

DearomatizationsGroups::DearomatizationsGroups(BaseMolecule& molecule, bool skip_superatoms)
    : _molecule(molecule), CP_INIT, TL_CP_GET(_vertexAromaticGroupIndex), TL_CP_GET(_vertexIsAcceptDoubleEdge), TL_CP_GET(_vertexIsAcceptSingleEdge),
      TL_CP_GET(_vertexProcessed), TL_CP_GET(_groupVertices), TL_CP_GET(_groupEdges), TL_CP_GET(_groupHeteroAtoms), TL_CP_GET(_groupData)
{
    _isQueryMolecule = _molecule.isQueryMolecule();
    // collect superatoms
    if (skip_superatoms)
        for (int i = molecule.sgroups.begin(); i != molecule.sgroups.end(); i = molecule.sgroups.next(i))
        {
            SGroup& sgroup = molecule.sgroups.getSGroup(i);
            if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                for (int i = 0; i < sgroup.atoms.size(); i++)
                    _inside_superatoms.find_or_insert(sgroup.atoms[i]);
            }
        }
}

void DearomatizationsGroups::getGroupData(int group, int flags, DearomatizationsGroups::GROUP_DATA* data)
{
    data->bonds.clear();
    data->bondsInvMapping.resize(_molecule.edgeEnd());
    data->heteroAtoms.clear();
    data->vertices.clear();

    if (flags & GET_VERTICES_FILTER)
    {
        data->verticesFilter.resize(_molecule.vertexEnd());
        data->verticesFilter.zerofill();
    }
    for (int v_idx = _molecule.vertexBegin(); v_idx < _molecule.vertexEnd(); v_idx = _molecule.vertexNext(v_idx))
    {
        if (_vertexAromaticGroupIndex[v_idx] != group)
            continue;

        data->vertices.push(v_idx);
        if (flags & GET_VERTICES_FILTER)
            data->verticesFilter[v_idx] = 1;

        if (flags & GET_HETERATOMS_INDICES)
        {
            // Check if atom have lone pair or vacant orbital
            int lonepairs;

            int label = _molecule.getAtomNumber(v_idx);
            int charge = _molecule.getAtomCharge(v_idx);
            int radical = _molecule.getAtomRadical_NoThrow(v_idx, -1);

            // Treat unset charge and radical as zero;
            // We have checked before in detectAromaticGroups()
            if (charge == CHARGE_UNKNOWN)
                charge = 0;
            if (radical == -1)
                radical = 0;

            if (label == -1)
                continue;

            int max_conn = Element::getMaximumConnectivity(label, charge, radical, false);

            int atom_group = Element::group(_molecule.getAtomNumber(v_idx));

            int vac = _molecule.getVacantPiOrbitals(atom_group, charge, radical, max_conn, &lonepairs);

            if (_vertexIsAcceptDoubleEdge[v_idx] && _vertexIsAcceptSingleEdge[v_idx] && (vac > 0 || lonepairs > 0))
                data->heteroAtoms.push(v_idx);
        }
    }

    memset(data->bondsInvMapping.ptr(), -1, sizeof(int) * data->bondsInvMapping.size());
    for (int e_idx = _molecule.edgeBegin(); e_idx < _molecule.edgeEnd(); e_idx = _molecule.edgeNext(e_idx))
    {
        const Edge& edge = _molecule.getEdge(e_idx);
        int bond_order = _molecule.getBondOrder(e_idx);

        if (bond_order < 0 && _isQueryMolecule && _molecule.asQueryMolecule().possibleAromaticBond(e_idx))
            bond_order = BOND_AROMATIC;

        if (bond_order == BOND_AROMATIC && _vertexAromaticGroupIndex[edge.beg] == group)
        {
            data->bonds.push(e_idx);
            data->bondsInvMapping[e_idx] = data->bonds.size() - 1;
        }
    }
}

// Construct bondsInvMapping, vertices and heteroAtomsInvMapping
void DearomatizationsGroups::getGroupDataFromStorage(DearomatizationsStorage& storage, int group, GROUP_DATA* data)
{
    data->bondsInvMapping.resize(_molecule.edgeEnd());
    data->vertices.clear();
    data->heteroAtomsInvMapping.resize(_molecule.vertexEnd());
    _vertexProcessed.resize(_molecule.vertexEnd());
    _vertexProcessed.zerofill();

    memset(data->bondsInvMapping.ptr(), -1, sizeof(int) * data->bondsInvMapping.size());
    memset(data->heteroAtomsInvMapping.ptr(), -1, sizeof(int) * data->heteroAtomsInvMapping.size());

    int bondsCount = storage.getGroupBondsCount(group);
    const int* bonds = storage.getGroupBonds(group);
    for (int i = 0; i < bondsCount; i++)
    {
        int e_idx = bonds[i];
        data->bondsInvMapping[e_idx] = i;
        const Edge& edge = _molecule.getEdge(e_idx);

        if (!_vertexProcessed[edge.beg])
        {
            data->vertices.push(edge.beg);
            _vertexProcessed[edge.beg] = 1;
        }
        if (!_vertexProcessed[edge.end])
        {
            data->vertices.push(edge.end);
            _vertexProcessed[edge.end] = 1;
        }
    }

    int heteroAtomsCount = storage.getGroupHeteroAtomsCount(group);
    const int* heteroAtoms = storage.getGroupHeteroAtoms(group);
    for (int i = 0; i < heteroAtomsCount; i++)
    {
        int h_idx = heteroAtoms[i];
        data->heteroAtomsInvMapping[h_idx] = i;
    }
}

int DearomatizationsGroups::detectAromaticGroups(const int* atom_external_conn)
{
    _vertexAromaticGroupIndex.resize(_molecule.vertexEnd());
    _vertexIsAcceptDoubleEdge.resize(_molecule.vertexEnd());
    _vertexIsAcceptSingleEdge.resize(_molecule.vertexEnd());
    memset(_vertexAromaticGroupIndex.ptr(), -1, _vertexAromaticGroupIndex.size() * sizeof(int));

    int currentAromaticGroup = 0;

    QueryMolecule* qmol = nullptr;

    if (_isQueryMolecule)
        qmol = &_molecule.asQueryMolecule();

    for (int v_idx = _molecule.vertexBegin(); v_idx < _molecule.vertexEnd(); v_idx = _molecule.vertexNext(v_idx))
    {
        if (_inside_superatoms.find(v_idx))
            continue;

        if (_vertexAromaticGroupIndex[v_idx] != -1)
            continue;

        if ((_molecule.getAtomAromaticity(v_idx) == ATOM_ALIPHATIC) || _molecule.isPseudoAtom(v_idx) || _molecule.isTemplateAtom(v_idx))
            continue;

        if (_molecule.getAtomNumber(v_idx) == -1)
            continue;

        if (qmol != 0 && qmol->getAtom(v_idx).hasConstraint(QueryMolecule::ATOM_CHARGE) && qmol->getAtomCharge(v_idx) == CHARGE_UNKNOWN)
            continue;

        if (qmol != 0 && qmol->getAtom(v_idx).hasConstraint(QueryMolecule::ATOM_RADICAL) && qmol->getAtomCharge(v_idx) == -1)
            continue;

        _vertexAromaticGroupIndex[v_idx] = currentAromaticGroup++;
        _detectAromaticGroups(v_idx, atom_external_conn);
    }

    // Add atoms that are connected to the aromaic group with double bonds
    // like in O=C1NC=CC=C1
    for (int e_idx = _molecule.edgeBegin(); e_idx < _molecule.edgeEnd(); e_idx = _molecule.edgeNext(e_idx))
    {
        const Edge& e = _molecule.getEdge(e_idx);
        int& g1 = _vertexAromaticGroupIndex[e.beg];
        int& g2 = _vertexAromaticGroupIndex[e.end];
        if (g1 == g2)
            continue;
        if (_molecule.getBondOrder(e_idx) == BOND_DOUBLE)
        {
            int dangling_v;
            if (g1 != -1)
            {
                g2 = g1;
                dangling_v = e.end;
            }
            else
            {
                g1 = g2;
                dangling_v = e.beg;
            }

            // Handle tricky case with 5-valence Nitrogen: CC1=CC=CC=[N]1=C
            _vertexIsAcceptDoubleEdge[dangling_v] = false;
            _vertexIsAcceptSingleEdge[dangling_v] = true;
        }
    }

    _aromaticGroups = currentAromaticGroup;
    return _aromaticGroups;
}

// Construct group structure in DearomatizationsStorage
void DearomatizationsGroups::constructGroups(DearomatizationsStorage& storage, bool needHeteroAtoms)
{
    if (storage.getGroupsCount() == 0 && _aromaticGroups != 0)
        storage.setGroupsCount(_aromaticGroups);
    storage.clearIndices();

    for (int group = 0; group < _aromaticGroups; group++)
    {
        int flags = 0;
        if (needHeteroAtoms)
            flags = GET_HETERATOMS_INDICES;
        getGroupData(group, flags, &_groupData);
        storage.setGroup(group, _groupData.bonds.size(), _groupData.bonds.ptr(), _groupData.heteroAtoms.size(), _groupData.heteroAtoms.ptr());
    }
}

int DearomatizationsGroups::_getFixedConnectivitySpecific(int elem, int charge, int min_conn, int n_arom)
{
    if (elem == ELEM_Se && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 2;      // common case
            if (min_conn == 3) // one external bond
                return 4;      // CID 10262587
            if (min_conn == 4)
                // CID 21204858, two single external bonds
                // CID 14984497, one double aromatic bond
                return 4;
        }
    }
    else if (elem == ELEM_Se && charge == 1)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 3;      // CID 10872228
            if (min_conn == 3) // one external bond
                return 3;      // CID 11115581
        }
    }
    else if (elem == ELEM_As && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 3;      // CID 136132
            if (min_conn == 3) // one external bond
                return 3;      // CID 237687
                               // no other cases known from PubChem
        }
    }
    else if (elem == ELEM_S && charge == 0)
    {
        if (n_arom == 2)
        {
            if (min_conn == 3)
                return 4; // CS1=[As]C=N[AsH]1
            if (min_conn == 4)
                return 4; // O=S1N=CC=N1
        }
    }
    else if (elem == ELEM_N && charge == 0)
    {
        if (n_arom == 2 && min_conn == 4)
            return 5; // 5-valence Nitrogen: CC(c1cc[n](=O)cc1)=O
    }
    return -1;
}

void DearomatizationsGroups::_detectAromaticGroups(int v_idx, const int* atom_external_conn)
{
    int non_aromatic_conn = 0;
    if (atom_external_conn != nullptr)
        non_aromatic_conn = atom_external_conn[v_idx];

    const Vertex& vertex = _molecule.getVertex(v_idx);
    int n_arom = 0;
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int e_idx = vertex.neiEdge(i);
        int bond_order = _molecule.getBondOrder(e_idx);

        if (bond_order == -1)
            if (!_isQueryMolecule)
                // Ignore such bonds.
                // It may be zero bonds from TautomerSuperStructure
                continue;
            else if (_molecule.asQueryMolecule().possibleAromaticBond(e_idx))
                bond_order = BOND_AROMATIC;
        if (bond_order != BOND_AROMATIC)
        {
            non_aromatic_conn += bond_order;
            continue;
        }
        non_aromatic_conn++;
        n_arom++;

        int vn_idx = vertex.neiVertex(i);
        if (_vertexAromaticGroupIndex[vn_idx] != -1)
            continue;

        _vertexAromaticGroupIndex[vn_idx] = _vertexAromaticGroupIndex[v_idx];
        _detectAromaticGroups(vn_idx, atom_external_conn);
    }

    bool impl_h_fixed = false;
    if (_isQueryMolecule)
    {
        QueryMolecule& qm = _molecule.asQueryMolecule();
        if (atom_external_conn == nullptr)
        {
            int impl_h = -1;
            if (qm.getAtom(v_idx).sureValue(QueryMolecule::ATOM_IMPLICIT_H, impl_h))
            {
                non_aromatic_conn += impl_h;
                impl_h_fixed = true;
            }
        }
    }
    else
    {
        Molecule& m = _molecule.asMolecule();
        // Check if number of hydrogens are fixed
        if (atom_external_conn == nullptr)
        {
            int impl_h = m.getImplicitH_NoThrow(v_idx, -1);
            if (impl_h != -1)
            {
                non_aromatic_conn += impl_h;
                impl_h_fixed = true;
            }
        }
    }

    int label = _molecule.getAtomNumber(v_idx);
    int charge = _molecule.getAtomCharge(v_idx);
    if (_isQueryMolecule && charge == CHARGE_UNKNOWN)
    {
        charge = 0;
    }
    // If radical is undefined then treat it as there are not radical
    // Because if there were a radical it should have been explicitly marked
    int radical = _molecule.getAtomRadical_NoThrow(v_idx, 0);
    if (_isQueryMolecule && radical < 0)
    {
        radical = 0;
    }

    int max_connectivity = -1;
    if (_isQueryMolecule)
    {
        if (_molecule.asQueryMolecule().possibleNitrogenV5(v_idx))
            max_connectivity = 5;
    }
    else
    {
        Molecule& m = _molecule.asMolecule();
        if (atom_external_conn == 0)
        {
            if (m.isNitrogenV5(v_idx))
                max_connectivity = 5;
        }
        else
        {
            if (m.isNitrogenV5ForConnectivity(v_idx, non_aromatic_conn))
                max_connectivity = non_aromatic_conn;
            if (m.isNitrogenV5ForConnectivity(v_idx, non_aromatic_conn + 1))
                max_connectivity = non_aromatic_conn + 1;
        }
    }

    // Explicit values for specific configurations
    if (max_connectivity == -1)
    {
        max_connectivity = _getFixedConnectivitySpecific(label, charge, non_aromatic_conn, n_arom);
        if (max_connectivity != -1)
            impl_h_fixed = true;
    }

    // Apply general purpose method
    if (max_connectivity == -1)
        max_connectivity = Element::getMaximumConnectivity(label, charge, radical, false);

    int atom_aromatic_connectivity = max_connectivity - non_aromatic_conn;
    if (atom_aromatic_connectivity < 0) // recalc with use_d_orbital=true
        max_connectivity = Element::getMaximumConnectivity(label, charge, radical, true);

    atom_aromatic_connectivity = max_connectivity - non_aromatic_conn;
    if (atom_aromatic_connectivity < 0)
    {
        _vertexIsAcceptSingleEdge[v_idx] = false;
        _vertexIsAcceptDoubleEdge[v_idx] = false;
    }
    else
    {
        _vertexIsAcceptSingleEdge[v_idx] = true;
        if (atom_aromatic_connectivity > 0)
        {
            _vertexIsAcceptDoubleEdge[v_idx] = true;
            // If number of implicit hydrogens are fixed and double bond is possible then
            // double bond must exist
            if (impl_h_fixed)
                _vertexIsAcceptSingleEdge[v_idx] = false;
        }
        else
            _vertexIsAcceptDoubleEdge[v_idx] = false;
    }
}

bool* DearomatizationsGroups::getAcceptDoubleBonds(void)
{
    return _vertexIsAcceptDoubleEdge.ptr();
}

bool DearomatizationsGroups::isAcceptDoubleBond(int atom)
{
    return _vertexIsAcceptDoubleEdge[atom];
}

//
// DearomatizationMatcher
//

IMPL_ERROR2(DearomatizationMatcher, DearomatizationException, "Dearomatization matcher");

CP_DEF(DearomatizationMatcher);

DearomatizationMatcher::DearomatizationMatcher(DearomatizationsStorage& dearomatizations, BaseMolecule& molecule, const int* atom_external_conn,
                                               bool skip_superatoms)
    : _molecule(molecule), _dearomatizations(dearomatizations), _graphMatchingFixedEdges(molecule), _aromaticGroups(molecule, skip_superatoms), CP_INIT,
      TL_CP_GET(_matchedEdges), TL_CP_GET(_matchedEdgesState), TL_CP_GET(_groupExInfo), TL_CP_GET(_verticesInGroup), TL_CP_GET(_verticesAdded),
      TL_CP_GET(_edges2GroupMapping), TL_CP_GET(_edges2IndexInGroupMapping), TL_CP_GET(_correctEdgesArray), TL_CP_GET(_verticesFixCount),
      TL_CP_GET(_aromaticGroupsData)
{
    _needPrepare = true;
    _aromaticGroups.detectAromaticGroups(atom_external_conn);
}

bool DearomatizationMatcher::isAbleToFixBond(int edge_idx, int type)
{
    if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_NO_DEAROMATIZATIONS)
        return false;
    _prepare();

    int group = _edges2GroupMapping[edge_idx];
    if (group == -1)
        return false;

    if (type == BOND_TRIPLE)
        return false; // Triple bonds aren't supported

    _prepareGroup(group);
    if (_dearomatizations.getGroupDearomatizationsCount(group) == 0)
        return false;

    int offset = _groupExInfo[group].offsetInEdgesState;
    byte* groupFixedEdgesPtr = _matchedEdges.ptr() + offset;

    int indexInGroup = _edges2IndexInGroupMapping[edge_idx];
    byte* groupFixedEdgesStatePtr = _matchedEdgesState.ptr() + offset;

    if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_SAVE_ALL_DEAROMATIZATIONS)
    {
        bitSetBit(groupFixedEdgesPtr, indexInGroup, 1);
        bitSetBit(groupFixedEdgesStatePtr, indexInGroup, type - 1);

        // Try to find dearomatization with the same edges in all dearomatizations
        int count = _dearomatizations.getGroupDearomatizationsCount(group);
        int activeEdgeState = _groupExInfo[group].activeEdgeState;
        int i;
        for (i = 0; i < count; i++)
        {
            const byte* dearomState = _dearomatizations.getGroupDearomatization(group, (i + activeEdgeState) % count);
            int nbits = _dearomatizations.getGroupBondsCount(group);
            if (bitTestEqualityByMask(dearomState, groupFixedEdgesStatePtr, groupFixedEdgesPtr, nbits))
            {
                _groupExInfo[group].activeEdgeState = i;
                break; // Dearomatization was found
            }
        }
        if (i != count)
        {
            _lastAcceptedEdge = edge_idx;
            _lastAcceptedEdgeType = type;
        }

        bitSetBit(groupFixedEdgesPtr, indexInGroup, 0);
        if (i != count)
            return true;
    }
    else
    {
        // Try to use active dearomatizations
        byte* activeDearom = _dearomatizations.getGroupDearomatization(group, _groupExInfo[group].activeEdgeState);

        if (bitGetBit(activeDearom, indexInGroup) == type - 1)
        {
            bitSetBit(groupFixedEdgesStatePtr, indexInGroup, type - 1);
            _lastAcceptedEdge = edge_idx;
            _lastAcceptedEdgeType = type;
            return true;
        }

        // Try to modify current dearomatization
        _graphMatchingFixedEdges.setEdgesMappingPtr(_edges2IndexInGroupMapping.ptr());
        _graphMatchingFixedEdges.setMatchingEdgesPtr(activeDearom);
        _graphMatchingFixedEdges.setExtraInfo(groupFixedEdgesPtr);

        if (_fixBondInMatching(group, indexInGroup, type))
        {
            bitSetBit(groupFixedEdgesStatePtr, indexInGroup, type - 1);
            _lastAcceptedEdge = edge_idx;
            _lastAcceptedEdgeType = type;
            return true;
        }

        // Try to modify other dearomatizations
        bitSetBit(groupFixedEdgesPtr, indexInGroup, 1);
        bitSetBit(groupFixedEdgesStatePtr, indexInGroup, type - 1);

        int count = _dearomatizations.getGroupDearomatizationsCount(group);
        for (int i = 0; i < count - 1; i++)
        {
            int dearom_idx = (i + 1 + _groupExInfo[group].activeEdgeState) % count;
            // Get difference between current state and dearomatization state in group
            if (_tryToChangeActiveIndex(dearom_idx, group, groupFixedEdgesPtr, groupFixedEdgesStatePtr))
            {
                bitSetBit(groupFixedEdgesPtr, indexInGroup, 0);
                _groupExInfo[group].activeEdgeState = dearom_idx;
                _lastAcceptedEdge = edge_idx;
                _lastAcceptedEdgeType = type;
                return true;
            }
        }

        bitSetBit(groupFixedEdgesPtr, indexInGroup, 0);
        return false;
    }

    return false;
}

bool DearomatizationMatcher::fixBond(int edge_idx, int type)
{
    if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_NO_DEAROMATIZATIONS)
        return false;
    _prepare();

    int group = _edges2GroupMapping[edge_idx];
    if (group == -1)
        return false;

    if (_lastAcceptedEdge != edge_idx || _lastAcceptedEdgeType != type)
    {
        if (!isAbleToFixBond(edge_idx, type))
            return false;
        if (_lastAcceptedEdge != edge_idx || _lastAcceptedEdgeType != type)
            throw Error("DearomatizationMatcher::fixBond: internal error");
    }

    int offset = _groupExInfo[group].offsetInEdgesState;
    byte* groupFixedEdgesPtr = _matchedEdges.ptr() + offset;
    byte* groupFixedEdgesStatePtr = _matchedEdgesState.ptr() + offset;

    int indexInGroup = _edges2IndexInGroupMapping[edge_idx];
    bitSetBit(groupFixedEdgesPtr, indexInGroup, 1);
    if (bitGetBit(groupFixedEdgesStatePtr, indexInGroup) != type - 1)
        throw Error("DearomatizationMatcher::fixBond: internal error #2");

    const Edge& edge = _molecule.getEdge(edge_idx);
    _verticesFixCount[edge.beg]++;
    _verticesFixCount[edge.end]++;

    _lastAcceptedEdge = -1;
    return true;
}

void DearomatizationMatcher::unfixBond(int edge_idx)
{
    if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_NO_DEAROMATIZATIONS)
        return;
    _prepare();

    int group = _edges2GroupMapping[edge_idx];
    if (group == -1)
        return;

    byte* groupFixedEdgesPtr = _matchedEdges.ptr() + _groupExInfo[group].offsetInEdgesState;
    bitSetBit(groupFixedEdgesPtr, _edges2IndexInGroupMapping[edge_idx], 0);

    const Edge& edge = _molecule.getEdge(edge_idx);
    _verticesFixCount[edge.beg]--;
    _verticesFixCount[edge.end]--;
}

void DearomatizationMatcher::unfixBondByAtom(int atom_idx)
{
    if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_NO_DEAROMATIZATIONS)
        return;
    _prepare();
    if (_verticesFixCount[atom_idx] == 0)
        return;

    const Vertex& vertex = _molecule.getVertex(atom_idx);
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        unfixBond(vertex.neiEdge(i));
}

void DearomatizationMatcher::_prepare(void)
{
    if (!_needPrepare)
        return;

    if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_SAVE_JUST_HETERATOMS)
    {
        _dearomatizations.clearBondsState();
        _aromaticGroups.constructGroups(_dearomatizations, true);
    }
    else
        _aromaticGroups.constructGroups(_dearomatizations, false);

    int offset = 0;
    _groupExInfo.resize(_dearomatizations.getGroupsCount());
    _edges2IndexInGroupMapping.resize(_molecule.edgeEnd());
    _edges2GroupMapping.resize(_molecule.edgeEnd());
    memset(_edges2IndexInGroupMapping.ptr(), -1, sizeof(int) * _edges2IndexInGroupMapping.size());
    memset(_edges2GroupMapping.ptr(), -1, sizeof(int) * _edges2GroupMapping.size());

    _verticesFixCount.resize(_molecule.vertexEnd());
    _verticesFixCount.zerofill();

    int maxGroupDearomatizations = 0;
    for (int group = 0; group < _dearomatizations.getGroupsCount(); group++)
    {
        _groupExInfo[group].offsetInEdgesState = offset;
        _groupExInfo[group].activeEdgeState = 0;

        if (_dearomatizations.getDearomatizationParams() == Dearomatizer::PARAMS_SAVE_JUST_HETERATOMS)
            _groupExInfo[group].needPrepare = true;
        else
            _groupExInfo[group].needPrepare = false;

        maxGroupDearomatizations = std::max(maxGroupDearomatizations, _dearomatizations.getGroupDearomatizationsCount(group));
        maxGroupDearomatizations = std::max(maxGroupDearomatizations, _dearomatizations.getGroupHeterAtomsStateCount(group));

        int edgesInGroup = _dearomatizations.getGroupBondsCount(group);
        const int* edges = _dearomatizations.getGroupBonds(group);
        for (int i = 0; i < edgesInGroup; i++)
        {
            int edge_idx = edges[i];
            _edges2GroupMapping[edge_idx] = group;
            _edges2IndexInGroupMapping[edge_idx] = i;
        }

        offset += bitGetSize(edgesInGroup);
    }

    _matchedEdges.resize(offset);
    _matchedEdges.zerofill();
    _matchedEdgesState.resize(_matchedEdges.size());
    _correctEdgesArray.resize(_matchedEdges.size());

    if (_dearomatizations.getDearomatizationParams() != Dearomatizer::PARAMS_SAVE_ALL_DEAROMATIZATIONS)
    {
        _verticesInGroup.reserve(_molecule.vertexEnd());
        _verticesAdded.resize(_molecule.vertexEnd());
        _verticesAdded.zeroFill();

        _generateUsedVertices();
        _graphMatchingFixedEdges.setAllVerticesInMatching();
    }
    _lastAcceptedEdge = -1;
    _lastAcceptedEdgeType = -1;

    _needPrepare = false;
}

// Generate used vertices per each group
void DearomatizationMatcher::_generateUsedVertices()
{
    for (int group = 0; group < _dearomatizations.getGroupsCount(); group++)
    {
        _groupExInfo[group].offsetInVertices = _verticesInGroup.size();
        const int* groupBonds = _dearomatizations.getGroupBonds(group);
        int count = _dearomatizations.getGroupBondsCount(group);
        for (int i = 0; i < count; i++)
        {
            const Edge& edge = _molecule.getEdge(groupBonds[i]);
            if (!_verticesAdded.get(edge.beg))
            {
                _verticesInGroup.push(edge.beg);
                _verticesAdded.set(edge.beg);
            }
            if (!_verticesAdded.get(edge.end))
            {
                _verticesInGroup.push(edge.end);
                _verticesAdded.set(edge.end);
            }
        }
        _groupExInfo[group].verticesUsed = _verticesInGroup.size() - _groupExInfo[group].offsetInVertices;
    }
}

// Try to modify dearomatizations to have the same fixed bonds
bool DearomatizationMatcher::_tryToChangeActiveIndex(int dearom_idx, int group, byte* groupFixedEdgesPtr, byte* groupFixedEdgesStatePtr)
{
    int bondsCount = _dearomatizations.getGroupBondsCount(group);
    byte* dearomState = _dearomatizations.getGroupDearomatization(group, dearom_idx);

    bitGetAandBxorNotC(groupFixedEdgesPtr, groupFixedEdgesStatePtr, dearomState, _correctEdgesArray.ptr(), bondsCount);
    _graphMatchingFixedEdges.setExtraInfo(_correctEdgesArray.ptr());
    _graphMatchingFixedEdges.setMatchingEdgesPtr(dearomState);

    int bytesCount = bitGetSize(bondsCount);
    for (int i = 0; i < bytesCount; i++)
    {
        byte dif = groupFixedEdgesPtr[i] & (groupFixedEdgesStatePtr[i] ^ dearomState[i]);
        while (dif != 0)
        {
            int indexInGroup = bitGetOneLOIndex(dif) + i * 8;
            if (indexInGroup > bondsCount)
                return true;

            if (!_fixBondInMatching(group, indexInGroup, bitGetBit(groupFixedEdgesStatePtr, indexInGroup) + 1))
                return false;

            // Update correct edges
            _correctEdgesArray[i] = groupFixedEdgesPtr[i] & (groupFixedEdgesStatePtr[i] ^ ~dearomState[i]);
            dif = groupFixedEdgesPtr[i] & (groupFixedEdgesStatePtr[i] ^ dearomState[i]);
        }
    }

    return true;
}

bool DearomatizationMatcher::_fixBondInMatching(int group, int indexInGroup, int type)
{
    const int* aromEdges = _dearomatizations.getGroupBonds(group);
    const Edge& edge = _molecule.getEdge(aromEdges[indexInGroup]);
    bool found = _graphMatchingFixedEdges.findAlternatingPath(edge.beg, edge.end, type != BOND_SINGLE, type != BOND_SINGLE);
    if (found)
    {
        if (type == BOND_SINGLE)
        {
            _graphMatchingFixedEdges.setEdgeMatching(aromEdges[indexInGroup], false);
            _graphMatchingFixedEdges.processPath();
        }
        else
        {
            _graphMatchingFixedEdges.processPath();
            _graphMatchingFixedEdges.setEdgeMatching(aromEdges[indexInGroup], true);
        }
        return true;
    }
    return false;
}

void DearomatizationMatcher::_prepareGroup(int group)
{
    if (!_groupExInfo[group].needPrepare)
        return;

    _groupExInfo[group].needPrepare = false;
    if (_dearomatizations.getGroupHeteroAtomsCount(group) != 0 && _dearomatizations.getGroupHeterAtomsStateCount(group) == 0)
        return;
    // Create mapping from local hetero-atoms indices to atom indices in molecule
    _aromaticGroups.getGroupDataFromStorage(_dearomatizations, group, &_aromaticGroupsData);

    GraphMatchingVerticesFixed graphMatchingFixedVertices(_molecule);

    graphMatchingFixedVertices.setEdgesMappingPtr(_aromaticGroupsData.bondsInvMapping.ptr());
    graphMatchingFixedVertices.setVerticesSetPtr(_aromaticGroupsData.vertices.ptr(), _aromaticGroupsData.vertices.size());

    graphMatchingFixedVertices.setVerticesMapping(_aromaticGroupsData.heteroAtomsInvMapping.ptr());
    graphMatchingFixedVertices.setVerticesAccept(_aromaticGroups.getAcceptDoubleBonds());

    // Generate one dearomatization for each hetero-atoms configuration
    int count = _dearomatizations.getGroupHeterAtomsStateCount(group);
    int index = 0;
    do
    {
        if (count != 0)
        {
            const byte* heteroAtomsState = _dearomatizations.getGroupHeterAtomsState(group, index++);
            graphMatchingFixedVertices.setVerticesState(heteroAtomsState);
        }
        if (!graphMatchingFixedVertices.findMatching())
            throw Error("DearomatizationMatcher::_prepareGroup: internal error");

        _dearomatizations.addGroupDearomatization(group, graphMatchingFixedVertices.getEdgesState());

        graphMatchingFixedVertices.reset();
    } while (index < count);
}

//
// DearomatizationMatcher::GraphMatchingEdgeFixed
//

void DearomatizationMatcher::GraphMatchingEdgeFixed::setExtraInfo(byte* edgesEdges)
{
    _edgesState = edgesEdges;
}

bool DearomatizationMatcher::GraphMatchingEdgeFixed::checkEdge(int e_idx)
{
    return !bitGetBit(_edgesState, _edgesMapping[e_idx]);
}

DearomatizationMatcher::GraphMatchingEdgeFixed::GraphMatchingEdgeFixed(BaseMolecule& molecule)
    : GraphPerfectMatching(molecule, USE_EXTERNAL_EDGES_PTR | USE_EDGES_MAPPING | USE_VERTICES_SET)
{
    _edgesState = NULL;
}

//
// DearomatizationMatcher::GraphMatchingVerticesFixed
//

bool DearomatizationMatcher::GraphMatchingVerticesFixed::checkVertex(int v_idx)
{
    if (_verticesMapping[v_idx] != -1)
        return bitGetBit(_verticesState, _verticesMapping[v_idx]) == 1;
    return _verticesAcceptDoubleBond[v_idx];
}

void DearomatizationMatcher::GraphMatchingVerticesFixed::setVerticesState(const byte* verticesState)
{
    _verticesState = verticesState;
}

void DearomatizationMatcher::GraphMatchingVerticesFixed::setVerticesMapping(int* verticesMapping)
{
    _verticesMapping = verticesMapping;
}

void DearomatizationMatcher::GraphMatchingVerticesFixed::setVerticesAccept(bool* verticesAcceptDoubleBond)
{
    _verticesAcceptDoubleBond = verticesAcceptDoubleBond;
}

DearomatizationMatcher::GraphMatchingVerticesFixed::GraphMatchingVerticesFixed(BaseMolecule& molecule)
    : GraphPerfectMatching(molecule, USE_EDGES_MAPPING | USE_VERTICES_SET)
{
    _verticesState = NULL;
    _verticesMapping = NULL;
    _verticesAcceptDoubleBond = NULL;
}

//
// MoleculeDearomatizer
//

CP_DEF(MoleculeDearomatizer);

MoleculeDearomatizer::MoleculeDearomatizer(BaseMolecule& mol, DearomatizationsStorage& dearom)
    : _dearomatizations(dearom), _mol(mol), CP_INIT, TL_CP_GET(vertex_connectivity)
{
    _isQueryMolecule = _mol.isQueryMolecule();
}

void MoleculeDearomatizer::dearomatizeGroup(int group, int dearomatization_index)
{
    byte* bondsState = _dearomatizations.getGroupDearomatization(group, dearomatization_index);
    const int* bondsMap = _dearomatizations.getGroupBonds(group);
    int bondsCount = _dearomatizations.getGroupBondsCount(group);

    for (int i = 0; i < bondsCount; i++)
    {
        if (bitGetBit(bondsState, i))
            if (_isQueryMolecule)
                dearomatizeQueryBond(_mol.asQueryMolecule(), bondsMap[i], BOND_DOUBLE);
            else
                _mol.asMolecule().setBondOrder(bondsMap[i], BOND_DOUBLE, true);
        else if (_isQueryMolecule)
            dearomatizeQueryBond(_mol.asQueryMolecule(), bondsMap[i], BOND_SINGLE);
        else
            _mol.asMolecule().setBondOrder(bondsMap[i], BOND_SINGLE, true);
    }
}

int MoleculeDearomatizer::_getBestDearomatization(int group)
{
    // Select group with more double bonds that means less hydrogens
    // For example for c1cnn2nnnc2c1 one should select C1=CC2=NN=NN2N=C1
    // instead of N1NN2NC=CC=C2N1
    int groups = _dearomatizations.getGroupDearomatizationsCount(group);
    int best_index = -1, best_count = -1;
    for (int i = 0; i < groups; i++)
    {
        int cnt = _countDoubleBonds(group, i);
        if (cnt > best_count)
        {
            best_count = cnt;
            best_index = i;
        }
    }
    return best_index;
}

int MoleculeDearomatizer::_countDoubleBonds(int group, int dearomatization_index)
{
    byte* bondsState = _dearomatizations.getGroupDearomatization(group, dearomatization_index);
    int bondsCount = _dearomatizations.getGroupBondsCount(group);

    int count = 0;

    for (int i = 0; i < bondsCount; i++)
        if (bitGetBit(bondsState, i))
            count++;
    return count;
}

void MoleculeDearomatizer::restoreHydrogens(int group, int dearomatization_index)
{
    byte* bondsState = _dearomatizations.getGroupDearomatization(group, dearomatization_index);
    const int* bondsMap = _dearomatizations.getGroupBonds(group);
    int bondsCount = _dearomatizations.getGroupBondsCount(group);

    for (int i = 0; i < bondsCount; i++)
    {
        const Edge& edge = _mol.getEdge(bondsMap[i]);
        int order = bitGetBit(bondsState, i) ? 2 : 1;
        int v_indices[2] = {edge.beg, edge.end};
        for (int j = 0; j < 2; j++)
        {
            int v = v_indices[j];
            if (vertex_connectivity[j] == 0)
            {
                // Compute non-aromatic connectivity
                const Vertex& vertex = _mol.getVertex(v);
                for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
                {
                    int nei_edge = vertex.neiEdge(nei);
                    int nei_order = _mol.getBondOrder(nei_edge);
                    if (nei_order != BOND_AROMATIC)
                        vertex_connectivity[v] += nei_order;
                }
            }
        }
        vertex_connectivity[edge.beg] += order;
        vertex_connectivity[edge.end] += order;
    }
}

bool MoleculeDearomatizer::dearomatizeMolecule(BaseMolecule& mol, const AromaticityOptions& options)
{
    DearomatizationsStorage dst;
    Dearomatizer dearomatizer(mol, 0, options);
    dearomatizer.setDearomatizationParams(Dearomatizer::PARAMS_SAVE_ONE_DEAROMATIZATION);
    dearomatizer.enumerateDearomatizations(dst);
    MoleculeDearomatizer mol_dearom(mol, dst);

    bool all_dearomatzied = true;
    for (int i = 0; i < dst.getGroupsCount(); ++i)
    {
        int cnt = dst.getGroupDearomatizationsCount(i);
        if (cnt == 0)
            all_dearomatzied = false;
        else if (cnt > 1 && options.unique_dearomatization)
            throw NonUniqueDearomatizationException("Dearomatization is not unique");
        else
            mol_dearom.dearomatizeGroup(i, mol_dearom._getBestDearomatization(i));
    }

    // Dearomatize RGroups
    int n_rgroups = mol.rgroups.getRGroupCount();
    for (int i = 1; i <= n_rgroups; i++)
    {
        PtrPool<BaseMolecule>& frags = mol.rgroups.getRGroup(i).fragments;

        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        {
            Molecule& fragment = frags[j]->asMolecule();
            dearomatizeMolecule(fragment, options);
        }
    }
    return all_dearomatzied;
}

bool MoleculeDearomatizer::restoreHydrogens(BaseMolecule& mol, const AromaticityOptions& options)
{
    bool found_invalid_aromatic_h = false;
    bool _isQueryMolecule = mol.isQueryMolecule();
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mol.isRSite(i) || mol.isPseudoAtom(i) || mol.isTemplateAtom(i))
            continue;

        if (_isQueryMolecule)
        {
            // TODO QDEAROM
        }
        else if (mol.asMolecule().getImplicitH_NoThrow(i, -1) == -1 && mol.getAtomAromaticity(i) == ATOM_AROMATIC)
            found_invalid_aromatic_h = true;
    }
    if (!found_invalid_aromatic_h)
        return false;

    DearomatizationsStorage dst;
    Dearomatizer dearomatizer(mol, 0, options);
    dearomatizer.setDearomatizationParams(Dearomatizer::PARAMS_SAVE_ONE_DEAROMATIZATION);
    dearomatizer.enumerateDearomatizations(dst);
    MoleculeDearomatizer mol_dearom(mol, dst);

    mol_dearom.vertex_connectivity.clear_resize(mol.vertexEnd());
    mol_dearom.vertex_connectivity.zerofill();

    bool all_dearomatzied = true;
    for (int i = 0; i < dst.getGroupsCount(); ++i)
    {
        int cnt = dst.getGroupDearomatizationsCount(i);
        if (cnt == 0)
            all_dearomatzied = false;
        else if (cnt > 1 && options.unique_dearomatization)
            throw NonUniqueDearomatizationException("Dearomatization is not unique. Cannot restore hydrogens.");
        else
            mol_dearom.restoreHydrogens(i, mol_dearom._getBestDearomatization(i));
    }

    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        int conn = mol_dearom.vertex_connectivity[i];
        if (mol.isRSite(i) || mol.isPseudoAtom(i) || mol.isTemplateAtom(i))
            continue;

        if (!_isQueryMolecule && (mol.asMolecule().getImplicitH_NoThrow(i, -1) == -1 && conn > 0))
        {
            int h = mol.asMolecule().calcImplicitHForConnectivity(i, conn);
            mol.asMolecule().setImplicitH(i, h);
        }
    }
    return all_dearomatzied;
}

bool MoleculeDearomatizer::restoreHydrogens(BaseMolecule& mol, bool unambiguous_only)
{
    AromaticityOptions options;
    options.method = AromaticityOptions::GENERIC;
    options.unique_dearomatization = unambiguous_only;
    return MoleculeDearomatizer::restoreHydrogens(mol, options);
}
