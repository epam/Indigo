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

#ifndef __indigo_molecule__
#define __indigo_molecule__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/properties_map.h"
#include "graph/cycle_enumerator.h"
#include "graph/edge_subgraph_enumerator.h"
#include "graph/graph_subtree_enumerator.h"
#include "indigo_internal.h"
#include "molecule/molecule.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/query_molecule.h"

class DLLEXPORT IndigoBaseMolecule : public IndigoObject
{
public:
    explicit IndigoBaseMolecule(int type_);

    ~IndigoBaseMolecule() override;

    PropertiesMap& getProperties() override
    {
        return _properties;
    }

    const char* debugInfo() override;

    static bool is(IndigoObject& object);

    indigo::PropertiesMap _properties;
};

class DLLEXPORT IndigoQueryMolecule : public IndigoBaseMolecule
{
public:
    IndigoQueryMolecule();

    ~IndigoQueryMolecule() override;

    BaseMolecule& getBaseMolecule() override;
    QueryMolecule& getQueryMolecule() override;
    const char* getName() override;

    static IndigoQueryMolecule* cloneFrom(IndigoObject& obj);
    static void parseAtomConstraint(const char* type, const char* value, std::unique_ptr<QueryMolecule::Atom>&);
    static QueryMolecule::Atom* parseAtomSMARTS(const char* string);

    const char* debugInfo() override;

    IndigoObject* clone() override;

    const MoleculeAtomNeighbourhoodCounters& getNeiCounters();

    QueryMolecule qmol;

private:
    MoleculeAtomNeighbourhoodCounters _nei_counters;
    int _nei_counters_edit_revision;
};

class DLLEXPORT IndigoMolecule : public IndigoBaseMolecule
{
public:
    IndigoMolecule();

    ~IndigoMolecule() override;

    BaseMolecule& getBaseMolecule() override;
    Molecule& getMolecule() override;
    const char* getName() override;

    static IndigoMolecule* cloneFrom(IndigoObject& obj);

    const char* debugInfo() override;

    IndigoObject* clone() override;

    Molecule mol;
};

class DLLEXPORT IndigoAtom : public IndigoObject
{
public:
    IndigoAtom(BaseMolecule& mol_, int idx_);
    ~IndigoAtom() override;

    static bool is(IndigoObject& obj);
    static IndigoAtom& cast(IndigoObject& obj);
    void remove() override;

    IndigoObject* clone() override;

    BaseMolecule& mol;
    int idx;

    int getIndex() override;

    const char* debugInfo() override;
};

class DLLEXPORT IndigoRGroup : public IndigoObject
{
public:
    IndigoRGroup();
    ~IndigoRGroup() override;

    int getIndex() override;

    static IndigoRGroup& cast(IndigoObject& obj);

    BaseMolecule* mol;
    int idx;
};

class DLLEXPORT IndigoRGroupFragment : public IndigoObject
{
public:
    IndigoRGroupFragment(IndigoRGroup& rgp, int idx);
    IndigoRGroupFragment(BaseMolecule* mol, int rgroup_idx, int fragment_idx);

    ~IndigoRGroupFragment() override;

    QueryMolecule& getQueryMolecule() override;
    Molecule& getMolecule() override;
    BaseMolecule& getBaseMolecule() override;
    int getIndex() override;

    void remove() override;
    IndigoObject* clone() override;

    IndigoRGroup rgroup;
    int frag_idx;
};

class DLLEXPORT IndigoBond : public IndigoObject
{
public:
    IndigoBond(BaseMolecule& mol_, int idx_);
    ~IndigoBond() override;

    static bool is(IndigoObject& obj);
    static IndigoBond& cast(IndigoObject& obj);
    void remove() override;

    BaseMolecule& mol;
    int idx;

    int getIndex() override;
    const char* debugInfo() override;
};

class IndigoAtomNeighbor : public IndigoAtom
{
public:
    explicit IndigoAtomNeighbor(BaseMolecule& mol_, int atom_idx, int bond_idx);
    ~IndigoAtomNeighbor() override;

    int bond_idx;
};

class IndigoAtomNeighborsIter : public IndigoObject
{
public:
    IndigoAtomNeighborsIter(BaseMolecule& molecule, int atom_idx);

    ~IndigoAtomNeighborsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _atom_idx;
    int _nei_idx;
    BaseMolecule& _mol;
};

class IndigoRGroupsIter : public IndigoObject
{
public:
    IndigoRGroupsIter(BaseMolecule* mol);

    ~IndigoRGroupsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    BaseMolecule* _mol;
    int _idx;
};

class IndigoRGroupFragmentsIter : public IndigoObject
{
public:
    IndigoRGroupFragmentsIter(IndigoRGroup& rgroup);
    ~IndigoRGroupFragmentsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    BaseMolecule* _mol;
    int _rgroup_idx;
    int _frag_idx;
};

class IndigoAtomsIter : public IndigoObject
{
public:
    enum
    {
        ALL,
        PSEUDO,
        RSITE,
        STEREOCENTER,
        ALLENE_CENTER
    };

    IndigoAtomsIter(BaseMolecule* molecule, int type);

    ~IndigoAtomsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _shift(int idx);

    int _type;
    int _idx;
    BaseMolecule* _mol;
};

class IndigoBondsIter : public IndigoObject
{
public:
    IndigoBondsIter(BaseMolecule& molecule);

    ~IndigoBondsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
};

class IndigoTGroup : public IndigoObject
{
public:
    IndigoTGroup(BaseMolecule& mol_, int idx_);
    ~IndigoTGroup() override;

    int getIndex() override;
    void remove() override;

    const char* debugInfo() override;

    static IndigoTGroup& cast(IndigoObject& obj);
    TGroup& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoTGroupsIter : public IndigoObject
{
public:
    IndigoTGroupsIter(BaseMolecule& molecule);

    ~IndigoTGroupsIter() override;

    const char* debugInfo() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
};

class IndigoSGroup : public IndigoObject
{
public:
    IndigoSGroup(BaseMolecule& mol_, int idx_);
    ~IndigoSGroup() override;

    int getIndex() override;
    void remove() override;

    const char* debugInfo() override;

    static IndigoSGroup& cast(IndigoObject& obj);
    SGroup& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoSGroupsIter : public IndigoObject
{
public:
    IndigoSGroupsIter(BaseMolecule& molecule, Array<int>&& sgs);

    ~IndigoSGroupsIter() override;

    const char* debugInfo() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
    Array<int> _refs;
};

class IndigoDataSGroup : public IndigoObject
{
public:
    IndigoDataSGroup(BaseMolecule& mol_, int idx_);
    ~IndigoDataSGroup() override;

    int getIndex() override;
    void remove() override;

    static IndigoDataSGroup& cast(IndigoObject& obj);
    DataSGroup& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoDataSGroupsIter : public IndigoObject
{
public:
    IndigoDataSGroupsIter(BaseMolecule& molecule, Array<int>&& refs);
    ~IndigoDataSGroupsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
    Array<int> _refs;
};

class IndigoSuperatom : public IndigoObject
{
public:
    IndigoSuperatom(BaseMolecule& mol_, int idx_);
    ~IndigoSuperatom() override;

    int getIndex() override;
    void remove() override;

    const char* getName() override;

    static IndigoSuperatom& cast(IndigoObject& obj);
    Superatom& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoSuperatomsIter : public IndigoObject
{
public:
    IndigoSuperatomsIter(BaseMolecule& molecule, Array<int>&& refs);
    ~IndigoSuperatomsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
    Array<int> _refs;
};

class IndigoRepeatingUnit : public IndigoObject
{
public:
    IndigoRepeatingUnit(BaseMolecule& mol_, int idx_);
    ~IndigoRepeatingUnit() override;

    int getIndex() override;
    void remove() override;

    static IndigoRepeatingUnit& cast(IndigoObject& obj);
    RepeatingUnit& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoRepeatingUnitsIter : public IndigoObject
{
public:
    IndigoRepeatingUnitsIter(BaseMolecule& molecule, Array<int>&& refs);
    ~IndigoRepeatingUnitsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
    Array<int> _refs;
};

class IndigoMultipleGroup : public IndigoObject
{
public:
    IndigoMultipleGroup(BaseMolecule& mol_, int idx_);
    ~IndigoMultipleGroup() override;

    int getIndex() override;
    void remove() override;

    static IndigoMultipleGroup& cast(IndigoObject& obj);
    MultipleGroup& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoMultipleGroupsIter : public IndigoObject
{
public:
    IndigoMultipleGroupsIter(BaseMolecule& molecule, Array<int>&& refs);
    ~IndigoMultipleGroupsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
    Array<int> _refs;
};

class IndigoGenericSGroup : public IndigoObject
{
public:
    IndigoGenericSGroup(BaseMolecule& mol_, int idx_);
    ~IndigoGenericSGroup() override;

    int getIndex() override;
    void remove() override;

    static IndigoGenericSGroup& cast(IndigoObject& obj);
    SGroup& get();

    BaseMolecule& mol;
    int idx;
};

class IndigoGenericSGroupsIter : public IndigoObject
{
public:
    IndigoGenericSGroupsIter(BaseMolecule& molecule, Array<int>&& refs);
    ~IndigoGenericSGroupsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _idx;
    BaseMolecule& _mol;
    Array<int> _refs;
};

class IndigoSGroupAtomsIter : public IndigoObject
{
public:
    IndigoSGroupAtomsIter(BaseMolecule& mol, SGroup& sgroup);
    ~IndigoSGroupAtomsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    BaseMolecule& _mol;
    SGroup& _sgroup;
    int _idx;
};

class IndigoSGroupBondsIter : public IndigoObject
{
public:
    IndigoSGroupBondsIter(BaseMolecule& mol, SGroup& sgroup);
    ~IndigoSGroupBondsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    BaseMolecule& _mol;
    SGroup& _sgroup;
    int _idx;
};

class IndigoMoleculeComponent : public IndigoObject
{
public:
    IndigoMoleculeComponent(BaseMolecule& mol_, int index_);
    ~IndigoMoleculeComponent() override;

    int getIndex() override;
    IndigoObject* clone() override;

    int index;
    BaseMolecule& mol;
};

class IndigoComponentsIter : public IndigoObject
{
public:
    IndigoComponentsIter(BaseMolecule& mol_);
    ~IndigoComponentsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

    BaseMolecule& mol;

protected:
    int _idx;
};

class IndigoComponentAtomsIter : public IndigoObject
{
public:
    IndigoComponentAtomsIter(BaseMolecule& mol, int cidx);
    ~IndigoComponentAtomsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _next();

    BaseMolecule& _mol;
    int _cidx;
    int _idx;
};

class IndigoComponentBondsIter : public IndigoObject
{
public:
    IndigoComponentBondsIter(BaseMolecule& mol, int cidx);
    ~IndigoComponentBondsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _next();

    BaseMolecule& _mol;
    int _cidx;
    int _idx;
};

class IndigoSubmolecule : public IndigoObject
{
public:
    IndigoSubmolecule(BaseMolecule& mol_, Array<int>& vertices_, Array<int>& edges_);
    IndigoSubmolecule(BaseMolecule& mol_, List<int>& vertices_, List<int>& edges_);
    ~IndigoSubmolecule() override;

    BaseMolecule& getBaseMolecule() override;

    int getIndex() override;
    IndigoObject* clone() override;

    BaseMolecule& getOriginalMolecule()
    {
        return _mol;
    };

    int idx; // not really a submolecule property, but included for convenience
             // of iterators that return submolecules

    Array<int> vertices;
    Array<int> edges;

protected:
    BaseMolecule& _mol;
    std::unique_ptr<BaseMolecule> _submol;
    int _submol_revision;
    void _createSubMolecule();
};

class IndigoSubmoleculeAtomsIter : public IndigoObject
{
public:
    IndigoSubmoleculeAtomsIter(IndigoSubmolecule& submol);
    ~IndigoSubmoleculeAtomsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    IndigoSubmolecule& _submol;
    int _idx;
};

class IndigoSubmoleculeBondsIter : public IndigoObject
{
public:
    IndigoSubmoleculeBondsIter(IndigoSubmolecule& submol);
    ~IndigoSubmoleculeBondsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    IndigoSubmolecule& _submol;
    int _idx;
};

class IndigoSSSRIter : public IndigoObject
{
public:
    IndigoSSSRIter(BaseMolecule& mol);
    ~IndigoSSSRIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    BaseMolecule& _mol;
    int _idx;
};

class IndigoSubtreesIter : public IndigoObject
{
public:
    IndigoSubtreesIter(BaseMolecule& mol, int min_vertices, int max_vertices);
    ~IndigoSubtreesIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    static void _handleTree(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

    BaseMolecule& _mol;
    GraphSubtreeEnumerator _enumerator;
    int _idx;
    ObjArray<Array<int>> _vertices;
    ObjArray<Array<int>> _edges;
};

class IndigoRingsIter : public IndigoObject
{
public:
    IndigoRingsIter(BaseMolecule& mol, int min_vertices, int max_vertices);
    ~IndigoRingsIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    static bool _handleCycle(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

    BaseMolecule& _mol;
    CycleEnumerator _enumerator;
    int _idx;
    ObjArray<Array<int>> _vertices;
    ObjArray<Array<int>> _edges;
};

class IndigoEdgeSubmoleculeIter : public IndigoObject
{
public:
    IndigoEdgeSubmoleculeIter(BaseMolecule& mol, int min_edges, int max_edges);
    ~IndigoEdgeSubmoleculeIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    static void _handleSubgraph(Graph& graph, const int* v_mapping, const int* e_mapping, void* context);

    BaseMolecule& _mol;
    EdgeSubgraphEnumerator _enumerator;
    int _idx;
    ObjArray<Array<int>> _vertices;
    ObjArray<Array<int>> _edges;
};

class IndigoAttachmentPointsIter : public IndigoObject
{
public:
    IndigoAttachmentPointsIter(BaseMolecule& mol, int order);

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    BaseMolecule& _mol;
    int _order, _index;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
