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

    class DLLEXPORT BaseReaction : public NonCopyable
    {
    public:
        enum
        {
            REACTANT = 1,
            PRODUCT = 2,
            CATALYST = 4
        };

        BaseReaction();
        virtual ~BaseReaction();

        // 'neu' means 'new' in German
        virtual BaseReaction* neu() = 0;

        int begin();
        int end();
        int next(int i);
        int count();

        void remove(int i);

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

        SideAuto reactants;
        SideAuto catalysts;
        SideAuto products;

        virtual void clear();

        // Returns true if some bonds were changed
        virtual bool aromatize(const AromaticityOptions& options) = 0;
        // Returns true if all bonds were dearomatized
        virtual bool dearomatize(const AromaticityOptions& options) = 0;

        // poor man's dynamic casting
        virtual Reaction& asReaction();
        virtual QueryReaction& asQueryReaction();
        virtual bool isQueryReaction();

        BaseMolecule& getBaseMolecule(int index)
        {
            return *_allMolecules.at(index);
        }

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

        int addReactantCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);
        int addProductCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);
        int addCatalystCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping);

        int findAtomByAAM(int mol_idx, int aam);
        int findAamNumber(BaseMolecule* mol, int atom_number);
        int findReactingCenter(BaseMolecule* mol, int bond_number);

        int findMolecule(BaseMolecule* mol);

        void markStereocenterBonds();

        static bool haveCoord(BaseReaction& reaction);

        void clone(BaseReaction& other, Array<int>* mol_mapping, ObjArray<Array<int>>* mappings, ObjArray<Array<int>>* inv_mappings);

        Array<char> name;

        DECL_ERROR;

    protected:
        virtual int _addBaseMolecule(int side) = 0;

        virtual void _addedBaseMolecule(int idx, int side, BaseMolecule& mol);

        PtrPool<BaseMolecule> _allMolecules;

        Array<int> _types;

        int _reactantCount;
        int _productCount;
        int _catalystCount;

        int _nextElement(int type, int index);

        virtual void _clone(BaseReaction& other, int index, int i, ObjArray<Array<int>>* mol_mappings);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
