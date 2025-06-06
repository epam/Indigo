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

#ifndef __base_reaction_h__
#define __base_reaction_h__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/auto_iter.h"
#include "base_cpp/non_copyable.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_pool.h"
#include "molecule/base_molecule.h"

namespace indigo
{

    class Reaction;
    class QueryReaction;
    class BaseReaction;
    class PathwayReaction;
    class KetDocument;

    struct SpecialCondition
    {
    public:
        SpecialCondition(int idx, const Rect2f& box) : meta_idx(idx), bbox(box)
        {
        }
        int meta_idx;
        Rect2f bbox;
    };

    class SideIter : public AutoIterator
    {
    public:
        DECL_ERROR;

        SideIter(BaseReaction& owner, int idx, int side);

        SideIter& operator++();

    private:
        BaseReaction& _owner;
        int _side;
    };

    class SideAuto
    {
    public:
        SideAuto(BaseReaction& owner, int side);

        SideIter begin();
        SideIter end();

    private:
        BaseReaction& _owner;
        int _side;
    };

    class ReactionBlock : public NonCopyable
    {
    public:
        void copy(const ReactionBlock& other)
        {
            reactants.copy(other.reactants);
            products.copy(other.products);
        }
        Array<int> reactants;
        Array<int> products;
    };

    class DLLEXPORT BaseReaction : public NonCopyable
    {
    public:
        enum
        {
            REACTANT = 1,
            PRODUCT = 2,
            INTERMEDIATE = 4,
            UNDEFINED = 8,
            CATALYST = 16
        };

        BaseReaction();
        virtual ~BaseReaction();

        MetaDataStorage& meta();
        PropertiesMap& properties();

        // 'neu' means 'new' in German
        virtual BaseReaction* neu() = 0;

        virtual int begin();
        virtual int end();
        virtual int next(int i);
        virtual int count();

        void remove(int i);

        int intermediateBegin()
        {
            return _nextElement(INTERMEDIATE, -1);
        }
        int intermediateNext(int index)
        {
            return _nextElement(INTERMEDIATE, index);
        }

        int intermediateEnd()
        {
            return _allMolecules.end();
        }

        int undefinedBegin()
        {
            return _nextElement(UNDEFINED, -1);
        }

        int undefinedNext(int index)
        {
            return _nextElement(UNDEFINED, index);
        }

        int undefinedEnd()
        {
            return _allMolecules.end();
        }

        int reactantBegin()
        {
            return _nextElement(REACTANT, -1);
        }
        int reactantNext(int index)
        {
            return _nextElement(REACTANT, index);
        }
        int reactantEnd()
        {
            return _allMolecules.end();
        }

        int productBegin()
        {
            return _nextElement(PRODUCT, -1);
        }
        int productNext(int index)
        {
            return _nextElement(PRODUCT, index);
        }
        int productEnd()
        {
            return _allMolecules.end();
        }

        int catalystBegin()
        {
            return _nextElement(CATALYST, -1);
        }
        int catalystNext(int index)
        {
            return _nextElement(CATALYST, index);
        }
        int catalystEnd()
        {
            return _allMolecules.end();
        }

        int sideBegin(int side)
        {
            return _nextElement(side, -1);
        }

        int sideNext(int side, int index)
        {
            return _nextElement(side, index);
        }

        virtual int reactionsCount()
        {
            return _reactionBlocks.size();
        }

        virtual int reactionBegin()
        {
            int i = 0;
            for (; i < _reactionBlocks.size(); ++i)
            {
                auto& rb = _reactionBlocks[i];
                if (rb.products.size() || rb.reactants.size())
                    break;
            }
            return i;
        }

        virtual int reactionEnd()
        {
            if (_reactionBlocks.size() == 0)
                return 1;
            return _reactionBlocks.size();
        }

        virtual int reactionNext(int i)
        {
            while (++i < _reactionBlocks.size())
            {
                auto& rb = _reactionBlocks[i];
                if (rb.products.size() || rb.reactants.size())
                    break;
            }
            return i;
        }

        // dkuzminov: we either need to have a parameter "side" for method sideEnd() or we should exclude the set of "different" xxxEnd methods for sake of the
        // single "end" method
        int sideEnd()
        {
            return _allMolecules.end();
        }

        int getSideType(int index)
        {
            return _types[index];
        }

        int undefinedCount() const
        {
            return _undefinedCount;
        }

        int intermediateCount() const
        {
            return _intermediateCount;
        }

        int multitaleCount() const;

        int reactantsCount() const
        {
            return _reactantCount;
        }
        int productsCount() const
        {
            return _productCount;
        }
        int catalystCount() const
        {
            return _catalystCount;
        }

        int specialConditionsCount() const
        {
            return _specialConditions.size();
        }

        int reactionBlocksCount() const
        {
            return _reactionBlocks.size();
        }

        ReactionBlock& reactionBlock(int index)
        {
            return _reactionBlocks[index];
        }

        ReactionBlock& addReactionBlock()
        {
            auto& rb = _reactionBlocks.push();
            rb.copy(ReactionBlock());
            return rb;
        }

        void clearReactionBlocks()
        {
            _reactionBlocks.clear();
        }

        SideAuto reactants;
        SideAuto catalysts;
        SideAuto products;
        SideAuto intermediates;
        SideAuto undefined;

        virtual void clear();

        // Returns true if some bonds were changed
        virtual bool aromatize(const AromaticityOptions& options) = 0;
        // Returns true if all bonds were dearomatized
        virtual bool dearomatize(const AromaticityOptions& options);
        void unfoldHydrogens();

        // poor man's dynamic casting
        virtual Reaction& asReaction();
        virtual QueryReaction& asQueryReaction();
        virtual PathwayReaction& asPathwayReaction();
        virtual bool isQueryReaction();
        virtual bool isPathwayReaction();

        virtual BaseMolecule& getBaseMolecule(int index);

        virtual std::unique_ptr<BaseReaction> getBaseReaction(int index) = 0;

        int getAAM(int index, int atom);
        int getReactingCenter(int index, int bond);
        int getInversion(int index, int atom);

        Array<int>& getAAMArray(int index);
        Array<int>& getReactingCenterArray(int index);
        Array<int>& getInversionArray(int index);

        void clearAAM();

        int addReactant();
        int addProduct();
        int addCatalyst();
        int addIntermediate();
        int addUndefined();
        int addSpecialCondition(int meta_idx, const Rect2f& bbox);
        void clearSpecialConditions();
        const SpecialCondition& specialCondition(int meta_idx) const;

        int addReactantCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);
        int addProductCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);
        int addCatalystCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);
        int addIntermediateCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);
        int addUndefinedCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);

        int findAtomByAAM(int mol_idx, int aam);
        int findAamNumber(BaseMolecule* mol, int atom_number);
        int findReactingCenter(BaseMolecule* mol, int bond_number);

        int findMolecule(BaseMolecule* mol);

        void markStereocenterBonds();

        static bool haveCoord(BaseReaction& reaction);

        void clone(BaseReaction& other, Array<int>* mol_mapping = nullptr, ObjArray<Array<int>>* mappings = nullptr,
                   ObjArray<Array<int>>* inv_mappings = nullptr);

        Array<char> name;

        int original_format;

        bool isRetrosyntetic()
        {
            return isRetrosynthetic;
        };

        void setIsRetrosyntetic()
        {
            isRetrosynthetic = true;
        };

        KetDocument& getKetDocument();

        DECL_ERROR;

    protected:
        virtual int _addBaseMolecule(int side) = 0;

        virtual void _addedBaseMolecule(int idx, int side, BaseMolecule& mol);

        PtrPool<BaseMolecule> _allMolecules;

        ObjArray<ReactionBlock> _reactionBlocks; // for multistep reactions only

        Array<int> _types;
        Array<SpecialCondition> _specialConditions;
        PropertiesMap _properties;
        int _reactantCount;
        int _productCount;
        int _catalystCount;
        int _intermediateCount;
        int _undefinedCount;
        int _specialCount;

        int _nextElement(int type, int index);

        MetaDataStorage _meta;
        bool isRetrosynthetic = false;

        virtual void _clone(BaseReaction& other, int index, int i, ObjArray<Array<int>>* mol_mappings);
        virtual void _cloneSub(BaseReaction& other);

        KetDocument* _document;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
