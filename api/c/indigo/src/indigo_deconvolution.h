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

#ifndef __indigo_deconvolution__
#define __indigo_deconvolution__

#include "base_cpp/obj_list.h"
#include "base_cpp/properties_map.h"
#include "indigo_internal.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/query_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

class IndigoDeconvolutionElem;
class IndigoDecompositionMatch;

class DLLEXPORT IndigoDeconvolution : public IndigoObject
{
private:
    enum
    {
        SHIFT_IDX = 2
    };

public:
    IndigoDeconvolution();
    ~IndigoDeconvolution() override
    {
    }

    void addMolecule(Molecule& mol, PropertiesMap& props, int idx);

    void setScaffold(QueryMolecule& scaffold);
    void makeRGroups(QueryMolecule& scaffold);
    void makeRGroup(IndigoDeconvolutionElem& elem, bool all_matches, bool change_scaffold);

    QueryMolecule& getDecomposedScaffold()
    {
        return _fullScaffold;
    }
    ObjArray<IndigoDeconvolutionElem>& getItems()
    {
        return _deconvolutionElems;
    }
    /*
     * Save AP as sepearate atoms
     */
    bool save_ap_bond_orders;
    /*
     * Ignore match errors
     */
    bool ignore_errors;
    /*
     * Aromatize
     */
    bool aromatize;

    int (*cbEmbedding)(const int* sub_vert_map, const int* sub_edge_map, const void* info, void* userdata);
    void* embeddingUserdata;

public:
    class DecompositionEnumerator
    {
    public:
        DecompositionEnumerator() : all_matches(false), remove_rsites(false), deco(0)
        {
        }
        ~DecompositionEnumerator()
        {
        }

        AutoPtr<AromaticityMatcher> am;
        AutoPtr<MoleculeSubstructureMatcher::FragmentMatchCache> fmcache;

        void calculateAutoMaps(Graph& sub);
        bool shouldContinue(int* map, int size);
        void addMatch(IndigoDecompositionMatch& match, Graph& sub, Graph& super);

        bool all_matches;
        bool remove_rsites;
        IndigoDeconvolution* deco;
        ObjArray<IndigoDecompositionMatch> contexts;

    private:
        DecompositionEnumerator(const DecompositionEnumerator&); // no implicit copy
        bool _foundOrder(ObjArray<ArrayNew<int>>& rsite_orders, ArrayNew<int>& swap_order);
        void _swapIndexes(IndigoDecompositionMatch&, int old_idx, int new_idx);
        void _refineAutoMaps(ObjList<ArrayNew<int>>& auto_maps, Graph& sub, Graph& super, ArrayNew<int>& scaf_map);
        void _addAllRsites(QueryMolecule&, IndigoDecompositionMatch&, RedBlackMap<int, int>&);

        static bool _cbAutoCheckAutomorphism(Graph& graph, const ArrayNew<int>& mapping, const void* context);
        ObjList<ArrayNew<int>> _autoMaps;
        ObjList<ArrayNew<int>> _scafAutoMaps;
    };

    void addCompleteRGroup(IndigoDecompositionMatch& emb_context, bool change_scaffold, ArrayNew<int>* rg_map);
    void createRgroups(IndigoDecompositionMatch& emb_context, bool change_scaffold);

    DECL_ERROR;

private:
    void _parseOptions(const char* options);

    void _addFullRGroup(IndigoDecompositionMatch& deco_match, ArrayNew<int>& auto_map, int rg_idx, int new_rg_idx);

    static int _rGroupsEmbedding(Graph& g1, Graph& g2, int* core1, int* core2, void* userdata);

    static bool _matchAtoms(Graph& g1, Graph& g2, const int*, int sub_idx, int super_idx, void* userdata);
    static bool _matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
    static void _addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
    static void _removeAtom(Graph& subgraph, int sub_idx, void* userdata);
    void _makeInvertMap(ArrayNew<int>& map, ArrayNew<int>& invmap);
    int _createRgMap(IndigoDecompositionMatch& deco_match, int aut_idx, RedBlackStringObjMap<ArrayNew<int>>& match_rgroups, ArrayNew<int>* rg_map_buf,
                     bool change_scaffold);
    int _getRgScore(ArrayNew<int>& rg_map) const;

    QueryMolecule _scaffold;
    QueryMolecule _fullScaffold;
    bool _userDefinedScaffold;
    ObjArray<IndigoDeconvolutionElem> _deconvolutionElems;
};

class DLLEXPORT IndigoDeconvolutionElem : public IndigoObject
{
public:
    IndigoDeconvolutionElem(Molecule& mol);
    IndigoDeconvolutionElem(Molecule& mol, PropertiesMap& props);
    IndigoDeconvolutionElem(Molecule& mol, int* index);
    IndigoDeconvolutionElem(IndigoDeconvolutionElem& elem);

    ~IndigoDeconvolutionElem() override;

    int getIndex() override
    {
        return idx;
    }
    indigo::PropertiesMap& getProperties() override
    {
        return _properties;
    }
    int idx;

    Molecule mol_in;
    IndigoDeconvolution::DecompositionEnumerator deco_enum;

    indigo::PropertiesMap _properties;
};

class DLLEXPORT IndigoDecompositionMatch : public IndigoObject
{
public:
    IndigoDecompositionMatch();
    ArrayNew<int> visitedAtoms;
    ArrayNew<int> scaffoldBonds;
    ArrayNew<int> scaffoldAtoms;
    ArrayNew<int> lastMapping;
    ArrayNew<int> lastInvMapping;
    ObjArray<ArrayNew<int>> attachmentOrder;
    ObjArray<ArrayNew<int>> attachmentIndex;
    ObjList<ArrayNew<int>> scafAutoMaps;

    int getRgroupNumber() const
    {
        return attachmentIndex.size() - 1;
    }

    void renumber(ArrayNew<int>& map, ArrayNew<int>& inv_map);
    void copy(IndigoDecompositionMatch& other);
    void removeRsitesFromMaps(Graph& query_graph);
    void copyScafAutoMaps(ObjList<ArrayNew<int>>& autoMaps);
    void completeScaffold();

    Molecule mol_out;
    Molecule rgroup_mol;
    Molecule mol_scaffold;

    IndigoDeconvolution* deco;

private:
    IndigoDecompositionMatch(const IndigoDecompositionMatch&); // no implicit copy

    bool _completeScaffold;
};

class DLLEXPORT IndigoDeconvolutionIter : public IndigoObject
{
public:
    IndigoDeconvolutionIter(ObjArray<IndigoDeconvolutionElem>& items);
    ~IndigoDeconvolutionIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    int _index;
    ObjArray<IndigoDeconvolutionElem>& _items;
};
class DLLEXPORT IndigoDecompositionMatchIter : public IndigoObject
{
public:
    IndigoDecompositionMatchIter(ObjArray<IndigoDecompositionMatch>& matches);
    ~IndigoDecompositionMatchIter() override
    {
    }

    IndigoObject* next() override;
    bool hasNext() override;
    int getIndex() override
    {
        return _index;
    }

protected:
    int _index;
    ObjArray<IndigoDecompositionMatch>& _matches;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
