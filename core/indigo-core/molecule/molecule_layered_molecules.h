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

#ifndef __molecule_layered_molecules_h__
#define __molecule_layered_molecules_h__

#include "common/base_cpp/d_bitset.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT LayeredMolecules : public BaseMolecule
    {
    public:
        enum
        {
            BOND_TYPES_NUMBER = 5,
            MAX_CYCLE_LENGTH = 22
        };

        LayeredMolecules(BaseMolecule& molecule);
        ~LayeredMolecules() override;

        // This method returns a bitmask of all layers that contain a bond idx of a specific order:
        const Dbitset& getBondMask(int idx, int order) const;

        // These methods are used for tracking if the atom is a possible position for a mobile hydrogen:
        bool isMobilePosition(int idx) const;
        void setMobilePosition(int idx, bool value);

        // These methods are used for tracking if the mobile position is occupied already.
        // The bitmask is the layers where the position is occupied.
        const Dbitset& getMobilePositionOccupiedMask(int idx) const;
        void setMobilePositionOccupiedMask(int idx, Dbitset& mask, bool value);

        // mask: the mask of layers used as prototypes;
        // edgesPath: the path of single-double bonds to be inverted
        // beg, end: the mobile positions of hydrogen to swap
        // forward: the direction to move the hydrogen
        // returns true if at least one new layer was added, false otherwise
        bool addLayersWithInvertedPath(const Dbitset& mask, const Array<int>& path, int beg, int end, bool forward);
        bool addLayerFromMolecule(const Molecule& molecule, Array<int>& aam);

        bool aromatize(int layerFrom, int layerTo, const AromaticityOptions& options);

        // construct a molecule that is represented as a layer
        void constructMolecule(Molecule& molecule, int layer, bool aromatized) const;

        unsigned getHash(int layer, bool aromatized)
        {
            if (aromatized)
                return _hashsAromatized[layer];
            return _hashs[layer];
        }

        void clear() override;

        BaseMolecule* neu() override;

        int getAtomNumber(int idx) override;
        int getAtomCharge(int idx) override;
        int getAtomIsotope(int idx) override;
        int getAtomRadical(int idx) override;
        int getAtomAromaticity(int idx) override;
        int getExplicitValence(int idx) override;
        int getAtomValence(int idx) override;
        int getAtomSubstCount(int idx) override;
        int getAtomRingBondsCount(int idx) override;
        int getAtomConnectivity(int idx) override;

        int getAtomMaxH(int idx) override;
        int getAtomMinH(int idx) override;
        int getAtomTotalH(int idx) override;

        bool isPseudoAtom(int idx) override;
        const char* getPseudoAtom(int idx) override;

        bool isTemplateAtom(int idx) override;
        const char* getTemplateAtom(int idx) override;
        const int getTemplateAtomSeqid(int idx) override;
        const int getTemplateAtomTemplateIndex(int idx) override;

        const char* getTemplateAtomClass(int idx) override;
        const int getTemplateAtomDisplayOption(int idx) override;
        void getTemplatesMap(std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates_map) override;
        void getTemplateAtomDirectionsMap(std::unordered_map<int, std::map<int, int>>& directions_map) override;

        bool isRSite(int atom_idx) override;
        dword getRSiteBits(int atom_idx) override;
        void allowRGroupOnRSite(int atom_idx, int rg_idx) override;

        int getBondOrder(int idx) const override;
        int getBondTopology(int idx) override;

        bool atomNumberBelongs(int idx, const int* numbers, int count) override;
        bool possibleAtomNumber(int idx, int number) override;
        bool possibleAtomNumberAndCharge(int idx, int number, int charge) override;
        bool possibleAtomNumberAndIsotope(int idx, int number, int isotope) override;
        bool possibleAtomIsotope(int idx, int isotope) override;
        bool possibleAtomCharge(int idx, int charge) override;
        void getAtomDescription(int idx, Array<char>& description) override;
        void getBondDescription(int idx, Array<char>& description) override;
        bool possibleBondOrder(int idx, int order) override;

        bool isSaturatedAtom(int idx) override;

        bool bondStereoCare(int idx) override;

        bool aromatize(const AromaticityOptions& options) override;
        bool dearomatize(const AromaticityOptions& options) override;

        int addAtom(int label) override;
        int addBond(int beg, int end, int order) override;

        int getImplicitH(int idx, bool impl_h_no_throw) override;
        void setImplicitH(int idx, int impl_h) override;

        int layers;

    protected:
        struct AromatizationContext
        {
            LayeredMolecules* self;
            int layerFrom;
            int layerTo;
            bool result;
        };

        Molecule _proto;
        ObjArray<Dbitset> _bond_masks[BOND_TYPES_NUMBER];
        Array<bool> _mobilePositions;
        ObjArray<Dbitset> _mobilePositionsOccupied;

        void _mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags) override;

        static bool _cb_handle_cycle(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

        void _resizeLayers(int newSize);
        void _calcConnectivity(int layerFrom, int layerTo);
        void _calcPiLabels(int layerFrom, int layerTo);
        bool _handleCycle(int layerFrom, int layerTo, const Array<int>& path);
        bool _isCycleAromaticInLayer(const int* cycle, int cycle_len, int layer);
        void _aromatizeCycle(const Array<int>& cycle, const Dbitset& mask);
        void _registerAromatizedLayers(int layerFrom, int layerTo);

    private:
        LayeredMolecules(const LayeredMolecules&); // no implicit copy
        ObjArray<Array<int>> _piLabels;
        ObjArray<Array<int>> _connectivity;
        int _layersAromatized;

        struct TrieNode
        {
            static const int ALPHABET_SIZE = 5;

            TrieNode()
            {
                for (auto& n : next)
                    n = -1;
            }
            int next[ALPHABET_SIZE];
        };

        class Trie
        {
        public:
            Trie()
            {
                // Adding root (index == 0)
                _nodes->add();
            }
            unsigned getRoot()
            {
                return 0;
            }
            unsigned follow(unsigned nodeInd, unsigned key)
            {
                return _nodes->at(nodeInd).next[key];
            }
            unsigned add(unsigned nodeInd, unsigned key, bool& newlyAdded)
            {
                if (_nodes->at(nodeInd).next[key] != -1)
                {
                    newlyAdded = false;
                    return _nodes->at(nodeInd).next[key];
                }
                newlyAdded = true;
                int ind = _nodes->add();
                _nodes->at(nodeInd).next[key] = ind;
                return ind;
            }

        private:
            std::unique_ptr<ObjPool<TrieNode>> _nodes = std::make_unique<ObjPool<TrieNode>>();
        };

        Trie _trie;
        Array<unsigned> _hashs;
        Array<unsigned> _hashsAromatized;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
