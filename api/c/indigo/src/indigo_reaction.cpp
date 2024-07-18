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

#include "indigo_reaction.h"
#include "base_cpp/output.h"
#include "indigo_array.h"
#include "indigo_io.h"
#include "indigo_mapping.h"
#include "indigo_molecule.h"
#include "reaction/canonical_rsmiles_saver.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_automapper.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_saver.h"
#include <memory>

//
// IndigoBaseReaction
//
IndigoBaseReaction::IndigoBaseReaction(int type_) : IndigoObject(type_)
{
}

IndigoBaseReaction::~IndigoBaseReaction()
{
}

bool IndigoBaseReaction::is(IndigoObject& obj)
{
    int type = obj.type;

    if (type == REACTION || type == QUERY_REACTION || type == RDF_REACTION || type == SMILES_REACTION || type == CML_REACTION || type == JSON_REACTION || type == PATHWAY_REACTION)
        return true;

    if (type == ARRAY_ELEMENT)
        return is(((IndigoArrayElement&)obj).get());

    return false;
}

const char* IndigoBaseReaction::debugInfo() const
{
    return "<base reaction>";
}

//
// IndigoBaseReaction
//

IndigoReaction::IndigoReaction() : IndigoBaseReaction(REACTION)
{
	init();
}

const char* IndigoReaction::debugInfo() const
{
    return "<reaction>";
}

IndigoReaction::~IndigoReaction()
{
}

void IndigoReaction::init(std::unique_ptr<BaseReaction>&& reaction)
{
    type = dynamic_cast<Reaction*>(reaction.get()) ? IndigoObject::REACTION : IndigoObject::PATHWAY_REACTION;
    rxn = reaction ? std::move(reaction) : std::make_unique<Reaction>();
}

Reaction& IndigoReaction::getReaction()
{
	assert(rxn);
    return dynamic_cast<Reaction&>(*rxn);
}

BaseReaction& IndigoReaction::getBaseReaction()
{
	assert(rxn);
    return *rxn;
}

const char* IndigoReaction::getName()
{
    if (!rxn || rxn->name.ptr() == 0)
        return "";
    return rxn->name.ptr();
}

//
// IndigoQueryReaction
//

IndigoQueryReaction::IndigoQueryReaction() : IndigoBaseReaction(QUERY_REACTION)
{
}

const char* IndigoQueryReaction::debugInfo() const
{
    return "<query reaction>";
}

IndigoQueryReaction::~IndigoQueryReaction()
{
}

BaseReaction& IndigoQueryReaction::getBaseReaction()
{
    return rxn;
}

QueryReaction& IndigoQueryReaction::getQueryReaction()
{
    return rxn;
}

const char* IndigoQueryReaction::getName()
{
    if (rxn.name.ptr() == 0)
        return "";
    return rxn.name.ptr();
}

//
// IndigoReactionMolecule
//

IndigoReactionMolecule::IndigoReactionMolecule(BaseReaction& reaction, int index) : IndigoObject(REACTION_MOLECULE), rxn(reaction), idx(index)
{
}

IndigoReactionMolecule::IndigoReactionMolecule(BaseReaction& reaction, MonomersProperties& map, int index)
    : IndigoObject(REACTION_MOLECULE), rxn(reaction), idx(index)
{
    if (index < map.size())
    {
        _properties.copy(map.at(index));
    }
}

const char* IndigoReactionMolecule::debugInfo() const
{
    return "<reaction molecule>";
}

IndigoReactionMolecule::~IndigoReactionMolecule()
{
}

BaseMolecule& IndigoReactionMolecule::getBaseMolecule()
{
    return rxn.getBaseMolecule(idx);
}

Molecule& IndigoReactionMolecule::getMolecule()
{
    return rxn.getBaseMolecule(idx).asMolecule();
}

QueryMolecule& IndigoReactionMolecule::getQueryMolecule()
{
    return rxn.getBaseMolecule(idx).asQueryMolecule();
}

int IndigoReactionMolecule::getIndex()
{
    return idx;
}

IndigoObject* IndigoReactionMolecule::clone()
{
    if (rxn.isQueryReaction())
        return IndigoQueryMolecule::cloneFrom(*this);
    else
        return IndigoMolecule::cloneFrom(*this);
}

void IndigoReactionMolecule::remove()
{
    rxn.remove(idx);
}

//
// IndigoReactionIter
//

IndigoReactionIter::IndigoReactionIter(BaseReaction& rxn, MonomersProperties& map, int subtype) : IndigoObject(REACTION_ITER), _rxn(rxn), _map(&map)
{
    _subtype = subtype;
    _idx = -1;
}

IndigoReactionIter::IndigoReactionIter(BaseReaction& rxn, int subtype) : IndigoObject(REACTION_ITER), _rxn(rxn), _map(nullptr)
{
    _subtype = subtype;
    _idx = -1;
}

const char* IndigoReactionIter::debugInfo() const
{
    return "<reaction molecule iterator>";
}

IndigoReactionIter::~IndigoReactionIter()
{
}

int IndigoReactionIter::_begin()
{
    if (_subtype == REACTANTS)
        return _rxn.reactantBegin();
    if (_subtype == PRODUCTS)
        return _rxn.productBegin();
    if (_subtype == CATALYSTS)
        return _rxn.catalystBegin();

    return _rxn.begin();
}

int IndigoReactionIter::_end()
{
    if (_subtype == REACTANTS)
        return _rxn.reactantEnd();
    if (_subtype == PRODUCTS)
        return _rxn.productEnd();
    if (_subtype == CATALYSTS)
        return _rxn.catalystEnd();

    return _rxn.end();
}

int IndigoReactionIter::_next(int i)
{
    if (_subtype == REACTANTS)
        return _rxn.reactantNext(i);
    if (_subtype == PRODUCTS)
        return _rxn.productNext(i);
    if (_subtype == CATALYSTS)
        return _rxn.catalystNext(i);

    return _rxn.next(i);
}

IndigoObject* IndigoReactionIter::next()
{
    if (_idx == -1)
    {
        _idx = _begin();
    }
    else
        _idx = _next(_idx);

    if (_idx == _end())
        return 0;

    if (_map)
    {
        return new IndigoReactionMolecule(_rxn, *_map, _idx);
    }
    else
    {
        return new IndigoReactionMolecule(_rxn, _idx);
    }
}

bool IndigoReactionIter::hasNext()
{
    if (_idx == -1)
        return _begin() != _end();

    return _next(_idx) != _end();
}

IndigoReaction* IndigoReaction::cloneFrom(IndigoObject& obj)
{
    Reaction& rxn = obj.getReaction();
    std::unique_ptr<IndigoReaction> rxnptr = std::make_unique<IndigoReaction>();
    rxnptr->rxn->clone(rxn, 0, 0, 0);
    try
    {
        MonomersProperties& mprops = obj.getMonomersProperties();
        for (auto i = 0; i < mprops.size(); i++)
        {
            rxnptr->_monomersProperties.push().copy(mprops[i]);
        }
    }
    catch (Exception&)
    {
    }

    auto& props = obj.getProperties();
    rxnptr->copyProperties(props);
    return rxnptr.release();
}

IndigoQueryReaction* IndigoQueryReaction::cloneFrom(IndigoObject& obj)
{
    QueryReaction& rxn = obj.getQueryReaction();

    std::unique_ptr<IndigoQueryReaction> rxnptr = std::make_unique<IndigoQueryReaction>();
    rxnptr->rxn.clone(rxn, 0, 0, 0);

    try
    {
        MonomersProperties& mprops = obj.getMonomersProperties();
        for (auto i = 0; i < mprops.size(); i++)
        {
            rxnptr->_monomersProperties.push().copy(mprops[i]);
        }
    }
    catch (Exception&)
    {
    }

    auto& props = obj.getProperties();
    rxnptr->copyProperties(props);
    return rxnptr.release();
}

IndigoObject* IndigoReaction::clone()
{
    return cloneFrom(*this);
}

IndigoObject* IndigoQueryReaction::clone()
{
    return cloneFrom(*this);
}

int _indigoIterateReaction(int reaction, int subtype)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reaction);
        BaseReaction& rxn = obj.getBaseReaction();

        try
        {
            MonomersProperties& map = obj.getMonomersProperties();
            return self.addObject(new IndigoReactionIter(rxn, map, subtype));
        }
        catch (Exception&)
        {
            return self.addObject(new IndigoReactionIter(rxn, subtype));
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadReaction(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        Scanner& scanner = IndigoScanner::get(obj);

        ReactionAutoLoader loader(scanner);

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
        loader.ignore_noncritical_query_features = self.ignore_noncritical_query_features;
        loader.dearomatize_on_load = self.dearomatize_on_load;
        loader.arom_options = self.arom_options;

        std::unique_ptr<IndigoReaction> rxnptr = std::make_unique<IndigoReaction>();
        rxnptr->init(loader.loadReaction(false));
        return self.addObject(rxnptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadQueryReaction(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        Scanner& scanner = IndigoScanner::get(obj);

        ReactionAutoLoader loader(scanner);

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
        loader.dearomatize_on_load = self.dearomatize_on_load;
        loader.arom_options = self.arom_options;

        std::unique_ptr<IndigoQueryReaction> rxnptr = std::make_unique<IndigoQueryReaction>();
        loader.loadReaction(rxnptr->rxn);
        return self.addObject(rxnptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateReactants(int reaction)
{
    return _indigoIterateReaction(reaction, IndigoReactionIter::REACTANTS);
}

CEXPORT int indigoIterateProducts(int reaction)
{
    return _indigoIterateReaction(reaction, IndigoReactionIter::PRODUCTS);
}

CEXPORT int indigoIterateCatalysts(int reaction)
{
    return _indigoIterateReaction(reaction, IndigoReactionIter::CATALYSTS);
}

CEXPORT int indigoIterateMolecules(int reaction)
{
    return _indigoIterateReaction(reaction, IndigoReactionIter::MOLECULES);
}

CEXPORT int indigoCreateReaction(void)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoReaction());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCreateQueryReaction(void)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoQueryReaction());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddReactant(int reaction, int molecule)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();

        rxn.addReactantCopy(self.getObject(molecule).getBaseMolecule(), 0, 0);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddProduct(int reaction, int molecule)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();

        rxn.addProductCopy(self.getObject(molecule).getBaseMolecule(), 0, 0);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddCatalyst(int reaction, int molecule)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();

        rxn.addCatalystCopy(self.getObject(molecule).getBaseMolecule(), 0, 0);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountReactants(int reaction)
{
    INDIGO_BEGIN
    {
        return self.getObject(reaction).getBaseReaction().reactantsCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountProducts(int reaction)
{
    INDIGO_BEGIN
    {
        return self.getObject(reaction).getBaseReaction().productsCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountCatalysts(int reaction)
{
    INDIGO_BEGIN
    {
        return self.getObject(reaction).getBaseReaction().catalystCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountMolecules(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseReaction::is(obj))
            return obj.getBaseReaction().count();

        throw IndigoError("can not count molecules of %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetMolecule(int reaction, int index)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reaction);
        BaseReaction& rxn = obj.getBaseReaction();

        try
        {
            MonomersProperties& map = obj.getMonomersProperties();
            return self.addObject(new IndigoReactionMolecule(rxn, map, index));
        }
        catch (Exception&)
        {
            return self.addObject(new IndigoReactionMolecule(rxn, index));
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoMapMolecule(int handle, int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);
        if (obj.type != IndigoObject::REACTION_MAPPING)
            throw IndigoError("%s is not a reaction mapping object", obj.debugInfo());
        IndigoReactionMapping& mapping = (IndigoReactionMapping&)obj;

        IndigoObject& mol_obj = self.getObject(molecule);
        if (mol_obj.type != IndigoObject::REACTION_MOLECULE)
            throw IndigoError("%s is not a reaction molecule object", mol_obj.debugInfo());
        IndigoReactionMolecule& mol = (IndigoReactionMolecule&)mol_obj;

        if (&mol.rxn != &mapping.from)
            throw IndigoError("%s molecule doesn't correspond to a mapping %s", mol.debugInfo(), mapping.debugInfo());

        int target_index = mapping.mol_mapping[mol.getIndex()];

        return self.addObject(new IndigoReactionMolecule(mapping.to, target_index));
    }
    INDIGO_END(-1);
}

static int readAAMOptions(const char* mode, ReactionAutomapper& ram)
{
    int nmode = ReactionAutomapper::AAM_REGEN_DISCARD;

    if (mode == 0 || mode[0] == 0)
        return nmode;

    QS_DEF(Array<char>, word);
    BufferScanner scanner(mode);

    while (1)
    {
        scanner.skipSpace();

        if (scanner.isEOF())
            break;

        scanner.readWord(word, 0);

        if (strcasecmp(word.ptr(), "discard") == 0)
            nmode = ReactionAutomapper::AAM_REGEN_DISCARD;
        else if (strcasecmp(word.ptr(), "alter") == 0)
            nmode = ReactionAutomapper::AAM_REGEN_ALTER;
        else if (strcasecmp(word.ptr(), "keep") == 0)
            nmode = ReactionAutomapper::AAM_REGEN_KEEP;
        else if (strcasecmp(word.ptr(), "clear") == 0)
            nmode = ReactionAutomapper::AAM_REGEN_CLEAR;
        else if (strcasecmp(word.ptr(), "ignore_charges") == 0)
            ram.ignore_atom_charges = true;
        else if (strcasecmp(word.ptr(), "ignore_isotopes") == 0)
            ram.ignore_atom_isotopes = true;
        else if (strcasecmp(word.ptr(), "ignore_radicals") == 0)
            ram.ignore_atom_radicals = true;
        else if (strcasecmp(word.ptr(), "ignore_valence") == 0)
            ram.ignore_atom_valence = true;
        else
            throw IndigoError("indigoAutomap(): unknown mode: %s", word.ptr());
    }

    return nmode;
}

CEXPORT int indigoAutomap(int reaction, const char* mode)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        ReactionAutomapper ram(rxn);
        ram.arom_options = self.arom_options;
        /*
         * Read options
         */
        int nmode = readAAMOptions(mode, ram);
        /*
         * Clear AAM if required
         */
        if (nmode == ReactionAutomapper::AAM_REGEN_CLEAR)
        {
            rxn.clearAAM();
            return 0;
        }
        /*
         * Set timeout
         */
        std::shared_ptr<TimeoutCancellationHandler> timeout(nullptr);
        if (self.aam_cancellation_timeout > 0)
        {
            timeout = std::make_shared<TimeoutCancellationHandler>(self.aam_cancellation_timeout);
        }
        /*
         * Set cancellation handler
         */
        AAMCancellationWrapper aam_timeout(timeout);
        /*
         * Launch automap
         */
        ram.automap(nmode);

        aam_timeout.reset();

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetAtomMappingNumber(int reaction, int reaction_atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(reaction_atom));

        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        int mol_idx = rxn.findMolecule(&ia.mol);

        if (mol_idx == -1)
            throw IndigoError("indigoGetAtomMapping(): input atom not found in the reaction");

        return rxn.getAAM(mol_idx, ia.idx);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetAtomMappingNumber(int reaction, int reaction_atom, int number)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(reaction_atom));

        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        int mol_idx = rxn.findMolecule(&ia.mol);

        if (mol_idx == -1)
            throw IndigoError("indigoSetAtomMapping(): input atom not found in the reaction");
        if (number < 0)
            throw IndigoError("indigoSetAtomMapping(): mapping number cannot be negative");

        rxn.getAAMArray(mol_idx).at(ia.idx) = number;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetReactingCenter(int reaction, int reaction_bond, int* rc)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(reaction_bond));

        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        int mol_idx = rxn.findMolecule(&ib.mol);

        if (mol_idx == -1)
            throw IndigoError("indigoGetReactingCenter(): input bond not found in the reaction");

        *rc = rxn.getReactingCenter(mol_idx, ib.idx);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetReactingCenter(int reaction, int reaction_bond, int rc)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(reaction_bond));

        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        int mol_idx = rxn.findMolecule(&ib.mol);

        if (mol_idx == -1)
            throw IndigoError("indigoSetReactingCenter(): input bond not found in the reaction");
        if (rc < -1 || rc > RC_TOTAL)
            throw IndigoError("indigoSetReactingCenter(): invalid or unsupported reacting center: %d", rc);

        rxn.getReactingCenterArray(mol_idx).at(ib.idx) = rc;
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearAAM(int reaction)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        rxn.clearAAM();
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCorrectReactingCenters(int reaction)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        ReactionAutomapper ram(rxn);
        ram.arom_options = self.arom_options;
        ram.correctReactingCenters(true);
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadReactionSmarts(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        RSmilesLoader loader(IndigoScanner::get(obj));

        std::unique_ptr<IndigoQueryReaction> rxnptr = std::make_unique<IndigoQueryReaction>();

        QueryReaction& qrxn = rxnptr->rxn;

        loader.smarts_mode = true;
        loader.loadQueryReaction(qrxn);
        return self.addObject(rxnptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoCanonicalRSmiles(int reaction)
{
    INDIGO_BEGIN
    {
        Reaction& react = self.getObject(reaction).getReaction();

        auto& tmp = self.getThreadTmpData();
        ArrayOutput output(tmp.string);
        CanonicalRSmilesSaver saver(output);

        saver.saveReaction(react);
        tmp.string.push(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}
