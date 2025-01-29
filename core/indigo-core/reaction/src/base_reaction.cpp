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

#include "reaction/base_reaction.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "molecule/ket_document.h"
#include "molecule/ket_document_json_loader.h"
#include "molecule/meta_commons.h"
#include "molecule/molecule_dearom.h"
#include "reaction/reaction_json_saver.h"

using namespace indigo;

IMPL_ERROR(BaseReaction, "reaction");
IMPL_ERROR(SideIter, "iterator");

SideIter::SideIter(BaseReaction& owner, int idx, int side) : _owner(owner), AutoIterator(idx), _side(side)
{
}

SideIter& SideIter::operator++()
{
    switch (_side)
    {
    case BaseReaction::REACTANT:
        _idx = _owner.reactantNext(_idx);
        break;
    case BaseReaction::PRODUCT:
        _idx = _owner.productNext(_idx);
        break;
    case BaseReaction::CATALYST:
        _idx = _owner.catalystNext(_idx);
        break;
    case BaseReaction::INTERMEDIATE:
        _idx = _owner.intermediateNext(_idx);
        break;
    case BaseReaction::UNDEFINED:
        _idx = _owner.undefinedNext(_idx);
        break;
    default:
        throw Error("Invalid BaseReaction side iterator type");
    }
    return *this;
}

SideAuto::SideAuto(BaseReaction& owner, int side) : _owner(owner), _side(side)
{
}

SideIter SideAuto::begin()
{
    int idx;

    switch (_side)
    {
    case BaseReaction::REACTANT:
        idx = _owner.reactantBegin();
        break;
    case BaseReaction::PRODUCT:
        idx = _owner.productBegin();
        break;
    case BaseReaction::CATALYST:
        idx = _owner.catalystBegin();
        break;
    case BaseReaction::INTERMEDIATE:
        idx = _owner.intermediateBegin();
        break;
    case BaseReaction::UNDEFINED:
        idx = _owner.catalystBegin();
        break;
    default:
        throw SideIter::Error("Invalid BaseReaction side iterator type");
    }

    return SideIter(_owner, idx, _side);
}

SideIter SideAuto::end()
{
    return SideIter(_owner, _owner.end(), _side);
}

BaseReaction::BaseReaction()
    : reactants(*this, REACTANT), catalysts(*this, CATALYST), products(*this, PRODUCT), intermediates(*this, INTERMEDIATE), undefined(*this, UNDEFINED),
      original_format(BaseMolecule::UNKNOWN), _document(nullptr)
{
    clear();
}

BaseReaction::~BaseReaction()
{
}

void BaseReaction::clear()
{
    _reactantCount = 0;
    _productCount = 0;
    _catalystCount = 0;
    _intermediateCount = 0;
    _undefinedCount = 0;
    _allMolecules.clear();
    _types.clear();
    name.clear();
    if (_document != nullptr)
    {
        delete _document;
        _document = nullptr;
    }
}

int BaseReaction::getAAM(int index, int atom)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_atom_mapping[atom];
}

int BaseReaction::getReactingCenter(int index, int bond)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_bond_reacting_center[bond];
}

int BaseReaction::getInversion(int index, int atom)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_atom_inversion[atom];
}

Array<int>& BaseReaction::getAAMArray(int index)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_atom_mapping;
}

Array<int>& BaseReaction::getReactingCenterArray(int index)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_bond_reacting_center;
}

Array<int>& BaseReaction::getInversionArray(int index)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_atom_inversion;
}

int BaseReaction::addReactant()
{
    return _addBaseMolecule(REACTANT);
}

int BaseReaction::addProduct()
{
    return _addBaseMolecule(PRODUCT);
}

int BaseReaction::addCatalyst()
{
    return _addBaseMolecule(CATALYST);
}

int BaseReaction::addIntermediate()
{
    return _addBaseMolecule(INTERMEDIATE);
}

int BaseReaction::addUndefined()
{
    return _addBaseMolecule(UNDEFINED);
}

void BaseReaction::_addedBaseMolecule(int idx, int side, BaseMolecule& mol)
{
    switch (side)
    {
    case REACTANT:
        _reactantCount++;
        break;
    case PRODUCT:
        _productCount++;
        break;
    case INTERMEDIATE:
        _intermediateCount++;
        break;
    case UNDEFINED:
        _undefinedCount++;
        break;
    case CATALYST:
        _catalystCount++;
        break;
    }
    _types.expand(idx + 1);
    _types[idx] = side;
}

int BaseReaction::findAtomByAAM(int mol_idx, int aam)
{
    BaseMolecule& mol = *_allMolecules.at(mol_idx);

    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        if (getAAM(mol_idx, i) == aam)
            return i;

    return -1;
}

int BaseReaction::findAamNumber(BaseMolecule* mol, int atom_number)
{
    for (int i = begin(); i < end(); i = next(i))
        if (mol == _allMolecules.at(i))
            return getAAM(i, atom_number);

    throw Error("cannot find aam number");
}

int BaseReaction::findReactingCenter(BaseMolecule* mol, int bond_number)
{
    for (int i = begin(); i < end(); i = next(i))
        if (mol == _allMolecules.at(i))
            return getReactingCenter(i, bond_number);

    throw Error("cannot find reacting center");
}

void BaseReaction::markStereocenterBonds()
{
    for (int i = begin(); i < end(); i = next(i))
    {
        _allMolecules[i]->clearBondDirections();
        _allMolecules[i]->markBondsStereocenters();
        _allMolecules[i]->markBondsAlleneStereo();
    }
}

bool BaseReaction::haveCoord(BaseReaction& reaction)
{
    for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
        if (!reaction.getBaseMolecule(i).have_xyz)
            return false;
    return true;
}

int BaseReaction::_nextElement(int type, int index)
{
    if (index == -1)
        index = _allMolecules.begin();
    else
        index = _allMolecules.next(index);

    for (; index != _allMolecules.end(); index = _allMolecules.next(index))
    {
        if (_types[index] & type)
            break;
    }
    return index;
}

void BaseReaction::clearAAM()
{
    for (int i = begin(); i < end(); i = next(i))
    {
        BaseMolecule& mol = *_allMolecules.at(i);
        mol.reaction_atom_mapping.zerofill();
    }
}

int BaseReaction::addSpecialCondition(int meta_idx, const Rect2f& bbox)
{
    _specialConditions.push(SpecialCondition(meta_idx, bbox));
    return _specialConditions.size() - 1;
}

void BaseReaction::clearSpecialConditions()
{
    _specialConditions.clear();
}

const SpecialCondition& BaseReaction::specialCondition(int meta_idx) const
{
    return _specialConditions[meta_idx];
}

int BaseReaction::addReactantCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping)
{
    int idx = _allMolecules.add(mol.neu());

    _allMolecules[idx]->clone(mol, mapping, inv_mapping);
    _addedBaseMolecule(idx, REACTANT, *_allMolecules[idx]);
    return idx;
}

int BaseReaction::addProductCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping)
{
    int idx = _allMolecules.add(mol.neu());

    _allMolecules[idx]->clone(mol, mapping, inv_mapping);
    _addedBaseMolecule(idx, PRODUCT, *_allMolecules[idx]);
    return idx;
}

int BaseReaction::addCatalystCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping)
{
    int idx = _allMolecules.add(mol.neu());

    _allMolecules[idx]->clone(mol, mapping, inv_mapping);
    _addedBaseMolecule(idx, CATALYST, *_allMolecules[idx]);
    return idx;
}

int BaseReaction::addIntermediateCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping)
{
    int idx = _allMolecules.add(mol.neu());

    _allMolecules[idx]->clone(mol, mapping, inv_mapping);
    _addedBaseMolecule(idx, INTERMEDIATE, *_allMolecules[idx]);
    return idx;
}

int BaseReaction::addUndefinedCopy(BaseMolecule& mol, Array<int>* mapping, Array<int>* inv_mapping)
{
    int idx = _allMolecules.add(mol.neu());

    _allMolecules[idx]->clone(mol, mapping, inv_mapping);
    _addedBaseMolecule(idx, UNDEFINED, *_allMolecules[idx]);
    return idx;
}

void BaseReaction::clone(BaseReaction& other, Array<int>* mol_mapping, ObjArray<Array<int>>* mappings, ObjArray<Array<int>>* inv_mappings)
{
    clear();

    int i, index = 0;
    QS_DEF(ObjArray<Array<int>>, tmp_mappings);

    if (mol_mapping != 0)
    {
        mol_mapping->clear_resize(other.end());
        mol_mapping->fffill();
    }

    if (mappings == 0)
        mappings = &tmp_mappings;
    mappings->clear();
    for (i = 0; i < other.end(); ++i)
        mappings->push();

    if (inv_mappings != 0)
        inv_mappings->clear();

    for (int i = other.begin(); i < other.end(); i = other.next(i))
    {
        BaseMolecule& rmol = other.getBaseMolecule(i);
        QS_DEF(Array<int>, inv_mapping);

        switch (other._types[i])
        {
        case REACTANT:
            index = addReactantCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
        case PRODUCT:
            index = addProductCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
        case CATALYST:
            index = addCatalystCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
        case INTERMEDIATE:
            index = addIntermediateCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
        case UNDEFINED:
            index = addUndefinedCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
        }

        if (inv_mappings != 0)
        {
            inv_mappings->expand(index + 1);
            inv_mappings->at(index).copy(inv_mapping);
        }
        if (mol_mapping != 0)
            mol_mapping->at(i) = index;

        // subclass' stuff
        _clone(other, index, i, mappings);
    }

    name.copy(other.name);
    _meta.clone(other._meta);
    _properties.copy(other._properties);
    isRetrosynthetic = other.isRetrosynthetic;
    _cloneSub(other);
}

void BaseReaction::_clone(BaseReaction& other, int index, int i, ObjArray<Array<int>>* mol_mappings)
{
}

void BaseReaction::_cloneSub(BaseReaction& other)
{
}

Reaction& BaseReaction::asReaction()
{
    throw Error("asReaction(): not a Reaction");
}

QueryReaction& BaseReaction::asQueryReaction()
{
    throw Error("asQueryReaction(): not a QueryReaction");
}

PathwayReaction& BaseReaction::asPathwayReaction()
{
    throw Error("asPathwayReaction(): not a PathwayReaction");
}

bool BaseReaction::isQueryReaction()
{
    return false;
}

bool BaseReaction::isPathwayReaction()
{
    return false;
}

void BaseReaction::remove(int i)
{
    int side = _types[i];
    switch (side)
    {
    case REACTANT:
        _reactantCount--;
        break;
    case PRODUCT:
        _productCount--;
        break;
    case INTERMEDIATE:
        _intermediateCount--;
        break;
    case UNDEFINED:
        _undefinedCount--;
        break;
    case CATALYST:
        _catalystCount--;
        break;
    }
    _allMolecules.remove(i);
}

int BaseReaction::begin()
{
    return _nextElement(REACTANT | PRODUCT | CATALYST | INTERMEDIATE | UNDEFINED, -1);
}

int BaseReaction::end()
{
    return _allMolecules.end();
}

int BaseReaction::next(int index)
{
    return _nextElement(REACTANT | PRODUCT | CATALYST | INTERMEDIATE | UNDEFINED, index);
}

int BaseReaction::count()
{
    return _allMolecules.size();
}

int BaseReaction::findMolecule(BaseMolecule* mol)
{
    for (int i = begin(); i != end(); i = next(i))
        if (&getBaseMolecule(i) == mol)
            return i;

    return -1;
}

bool BaseReaction::dearomatize(const AromaticityOptions& options)
{
    bool all_dearomatized = true;
    for (int i = begin(); i < end(); i = next(i))
    {
        all_dearomatized &= MoleculeDearomatizer::dearomatizeMolecule(*_allMolecules[i], options);
    }
    return all_dearomatized;
}

void BaseReaction::unfoldHydrogens()
{
    QS_DEF(Array<int>, markers);
    int i;

    for (i = begin(); i != end(); i = next(i))
    {
        BaseMolecule& b_mol = getBaseMolecule(i);
        b_mol.unfoldHydrogens(&markers, -1);
    }
}

MetaDataStorage& BaseReaction::meta()
{
    return _meta;
}

PropertiesMap& BaseReaction::properties()
{
    return _properties;
}

int BaseReaction::multitaleCount() const
{
    return _meta.getMetaCount(ReactionMultitailArrowObject::CID);
}

KetDocument& BaseReaction::getKetDocument()
{
    if (_document == nullptr)
    {
        // save to ket
        std::string json;
        StringOutput out(json);
        ReactionJsonSaver saver(out);
        saver.saveReaction(*this);
        // load document from ket
        rapidjson::Document data;
        std::ignore = data.Parse(json.c_str());
        _document = new KetDocument;
        KetDocumentJsonLoader loader{};
        loader.parseJson(json, *_document);
    }
    return *_document;
}