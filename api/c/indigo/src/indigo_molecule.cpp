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

#include "indigo_molecule.h"
#include "base_c/bitarray.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "indigo_array.h"
#include "indigo_io.h"
#include "indigo_mapping.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/elements.h"
#include "molecule/hybridization.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_mass.h"
#include "molecule/molecule_name_parser.h"
#include "molecule/molecule_savers.h"
#include "molecule/query_molecule.h"
#include "molecule/sequence_loader.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"

IndigoBaseMolecule::IndigoBaseMolecule(int type_) : IndigoObject(type_)
{
}

IndigoBaseMolecule::~IndigoBaseMolecule()
{
}

const char* IndigoBaseMolecule::debugInfo() const
{
    return "<base molecule>";
}

IndigoMolecule::IndigoMolecule() : IndigoBaseMolecule(MOLECULE)
{
}

bool IndigoBaseMolecule::is(const IndigoObject& object)
{
    switch (object.type)
    {
    case MOLECULE:
    case QUERY_MOLECULE:
    case REACTION_MOLECULE:
    case SCAFFOLD:
    case RGROUP_FRAGMENT:
    case RDF_MOLECULE:
    case SMILES_MOLECULE:
    case CML_MOLECULE:
    case JSON_MOLECULE:
    case CDX_MOLECULE:
    case SUBMOLECULE:
        return true;

    case ARRAY_ELEMENT:
        return is(((IndigoArrayElement&)object).get());
    }
    return false;
}

IndigoMolecule::~IndigoMolecule()
{
}

Molecule& IndigoMolecule::getMolecule()
{
    return mol;
}

const Molecule& IndigoMolecule::getMolecule() const
{
    return mol;
}

BaseMolecule& IndigoMolecule::getBaseMolecule()
{
    return mol;
}

const char* IndigoMolecule::getName()
{
    if (mol.name.ptr() == 0)
        return "";
    return mol.name.ptr();
}

IndigoMolecule* IndigoMolecule::cloneFrom(IndigoObject& obj)
{
    std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();
    QS_DEF(Array<int>, mapping);

    molptr->mol.clone(obj.getMolecule(), 0, &mapping);

    auto& props = obj.getProperties();
    molptr->copyProperties(props);

    return molptr.release();
}

IndigoQueryMolecule::IndigoQueryMolecule() : IndigoBaseMolecule(QUERY_MOLECULE)
{
    _nei_counters_edit_revision = -1;
}

IndigoQueryMolecule::~IndigoQueryMolecule()
{
}

QueryMolecule& IndigoQueryMolecule::getQueryMolecule()
{
    return qmol;
}

IndigoQueryMolecule* IndigoQueryMolecule::cloneFrom(IndigoObject& obj)
{
    std::unique_ptr<IndigoQueryMolecule> molptr = std::make_unique<IndigoQueryMolecule>();
    QS_DEF(Array<int>, mapping);

    molptr->qmol.clone(obj.getQueryMolecule(), 0, &mapping);

    auto& props = obj.getProperties();
    molptr->copyProperties(props);

    return molptr.release();
}

void IndigoQueryMolecule::parseAtomConstraint(const char* type, const char* value, std::unique_ptr<QueryMolecule::Atom>& atom)
{
    enum KeyType
    {
        Int,
        Bool
    };
    struct Mapping
    {
        const char* key;
        QueryMolecule::OpType value;
        KeyType key_type;
    };

    static Mapping mappingForKeys[] = {
        {"atomic-number", QueryMolecule::ATOM_NUMBER, Int},
        {"charge", QueryMolecule::ATOM_CHARGE, Int},
        {"isotope", QueryMolecule::ATOM_ISOTOPE, Int},
        {"radical", QueryMolecule::ATOM_RADICAL, Int},
        {"valence", QueryMolecule::ATOM_VALENCE, Int},
        {"connectivity", QueryMolecule::ATOM_CONNECTIVITY, Int},
        {"total-bond-order", QueryMolecule::ATOM_TOTAL_BOND_ORDER, Int},
        {"hydrogens", QueryMolecule::ATOM_TOTAL_H, Int},
        {"substituents", QueryMolecule::ATOM_SUBSTITUENTS, Int},
        {"ring", QueryMolecule::ATOM_SSSR_RINGS, Int},
        {"smallest-ring-size", QueryMolecule::ATOM_SMALLEST_RING_SIZE, Int},
        {"ring-bonds", QueryMolecule::ATOM_RING_BONDS, Int},
        {"rsite-mask", QueryMolecule::ATOM_RSITE, Int},
        {"highlighting", QueryMolecule::HIGHLIGHTING, Bool},
    };

    for (int i = 0; i < NELEM(mappingForKeys); i++)
    {
        if (strcasecmp(type, mappingForKeys[i].key) == 0)
        {
            int int_value = 0;
            if (value != NULL)
            {
                if (mappingForKeys[i].key_type == Int)
                {
                    BufferScanner buf_scanner(value);
                    int_value = buf_scanner.readInt();
                }
                else if (mappingForKeys[i].key_type == Bool)
                {
                    if (strcasecmp(value, "true") == 0)
                        int_value = 1;
                    else if (strcasecmp(value, "false") == 0)
                        int_value = 0;
                    else
                    {
                        BufferScanner buf_scanner(value);
                        int_value = buf_scanner.readInt();
                    }
                }
            }
            atom = std::make_unique<QueryMolecule::Atom>(mappingForKeys[i].value, int_value);
            return;
        }
    }

    if (strcasecmp(type, "rsite") == 0)
    {
        int int_value = 0;
        if (value != NULL)
        {
            BufferScanner buf_scanner(value);
            int_value = buf_scanner.readInt();
        }
        atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RSITE, 1 << int_value);
        return;
    }
    else if (strcasecmp(type, "smarts") == 0)
    {
        if (value == NULL)
            throw IndigoError("Internal error: value argument in parseAtomConstraint has null value");
        atom.reset(parseAtomSMARTS(value));
        return;
    }
    else if (strcasecmp(type, "aromaticity") == 0)
    {
        int int_value = 0;
        if (value != NULL)
        {
            if (strcasecmp(value, "aromatic") == 0)
                int_value = ATOM_AROMATIC;
            else if (strcasecmp(value, "aliphatic") == 0)
                int_value = ATOM_ALIPHATIC;
            else
                throw IndigoError("unsupported aromaticity type: %s", value);
        }
        atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_AROMATICITY, int_value);
        return;
    }

    throw IndigoError("unsupported constraint type: %s", type);
}

QueryMolecule::Atom* IndigoQueryMolecule::parseAtomSMARTS(const char* string)
{
    if (strlen(string) == 0)
        return new QueryMolecule::Atom();

    QS_DEF(QueryMolecule, qmol);
    qmol.clear();

    BufferScanner scanner(string);
    SmilesLoader loader(scanner);

    loader.loadSMARTS(qmol);
    if (qmol.vertexCount() != 1)
        throw IndigoError("cannot parse '%s' as a single-atom", string);

    return qmol.releaseAtom(qmol.vertexBegin());
}

BaseMolecule& IndigoQueryMolecule::getBaseMolecule()
{
    return qmol;
}

const char* IndigoQueryMolecule::getName()
{
    if (qmol.name.ptr() == 0)
        return "";
    return qmol.name.ptr();
}

IndigoAtom::IndigoAtom(BaseMolecule& mol_, int idx_) : IndigoObject(ATOM), mol(mol_)
{
    idx = idx_;
}

IndigoAtom::~IndigoAtom()
{
}

int IndigoAtom::getIndex()
{
    return idx;
}

const char* IndigoAtom::debugInfo() const
{
    return "<atom>";
}

bool IndigoAtom::is(IndigoObject& obj)
{
    if (obj.type == IndigoObject::ATOM || obj.type == IndigoObject::ATOM_NEIGHBOR)
        return true;
    if (obj.type == IndigoObject::ARRAY_ELEMENT)
        return is(((IndigoArrayElement&)obj).get());
    return false;
}

IndigoAtom& IndigoAtom::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::ATOM || obj.type == IndigoObject::ATOM_NEIGHBOR)
        return (IndigoAtom&)obj;
    if (obj.type == IndigoObject::ARRAY_ELEMENT)
        return cast(((IndigoArrayElement&)obj).get());
    throw IndigoError("%s does not represent an atom", obj.debugInfo());
}

void IndigoAtom::remove()
{
    mol.removeAtom(idx);
}

IndigoObject* IndigoAtom::clone()
{
    return new IndigoAtom(mol, idx);
}

IndigoAtomsIter::IndigoAtomsIter(BaseMolecule* mol, int type_) : IndigoObject(ATOMS_ITER)
{
    _mol = mol;
    _type = type_;
    _idx = -1;
}

IndigoAtomsIter::~IndigoAtomsIter()
{
}

int IndigoAtomsIter::_shift(int idx)
{
    if (_type == PSEUDO)
    {
        for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
            if (_mol->isPseudoAtom(idx))
                break;
    }
    else if (_type == RSITE)
    {
        for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
            if (_mol->isRSite(idx))
                break;
    }
    else if (_type == STEREOCENTER)
    {
        for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
            if (_mol->stereocenters.getType(idx) != 0)
                break;
    }
    else if (_type == ALLENE_CENTER)
    {
        for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
            if (_mol->allene_stereo.isCenter(idx))
                break;
    }

    return idx;
}

bool IndigoAtomsIter::hasNext()
{
    if (_idx == _mol->vertexEnd())
        return false;

    int next_idx;

    if (_idx == -1)
        next_idx = _shift(_mol->vertexBegin());
    else
        next_idx = _shift(_mol->vertexNext(_idx));

    return next_idx != _mol->vertexEnd();
}

IndigoObject* IndigoAtomsIter::next()
{
    if (_idx == -1)
        _idx = _mol->vertexBegin();
    else
        _idx = _mol->vertexNext(_idx);

    _idx = _shift(_idx);

    if (_idx == _mol->vertexEnd())
        return 0;

    std::unique_ptr<IndigoAtom> atom = std::make_unique<IndigoAtom>(*_mol, _idx);

    return atom.release();
}

IndigoBond::IndigoBond(BaseMolecule& mol_, int idx_) : IndigoObject(BOND), mol(mol_)
{
    idx = idx_;
}

IndigoBond::~IndigoBond()
{
}

int IndigoBond::getIndex()
{
    return idx;
}

const char* IndigoBond::debugInfo() const
{
    return "<bond>";
}

bool IndigoBond::is(IndigoObject& obj)
{
    if (obj.type == IndigoObject::BOND)
        return true;
    if (obj.type == IndigoObject::ARRAY_ELEMENT)
        return is(((IndigoArrayElement&)obj).get());
    return false;
}

IndigoBond& IndigoBond::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::BOND)
        return (IndigoBond&)obj;
    if (obj.type == IndigoObject::ARRAY_ELEMENT)
        return cast(((IndigoArrayElement&)obj).get());
    throw IndigoError("%s does not represent a bond", obj.debugInfo());
}

void IndigoBond::remove()
{
    mol.removeBond(idx);
}

IndigoBondsIter::IndigoBondsIter(BaseMolecule& mol) : IndigoObject(BONDS_ITER), _mol(mol)
{
    _idx = -1;
}

IndigoBondsIter::~IndigoBondsIter()
{
}

bool IndigoBondsIter::hasNext()
{
    if (_idx == _mol.edgeEnd())
        return false;

    int next_idx;

    if (_idx == -1)
        next_idx = _mol.edgeBegin();
    else
        next_idx = _mol.edgeNext(_idx);

    return next_idx != _mol.edgeEnd();
}

IndigoObject* IndigoBondsIter::next()
{
    if (_idx == -1)
        _idx = _mol.edgeBegin();
    else
        _idx = _mol.edgeNext(_idx);

    if (_idx == _mol.edgeEnd())
        return nullptr;
    return new IndigoBond(_mol, _idx);
}

CEXPORT int indigoLoadMolecule(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);

        MoleculeAutoLoader loader(IndigoScanner::get(obj));

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
        loader.ignore_noncritical_query_features = self.ignore_noncritical_query_features;
        loader.skip_3d_chirality = self.skip_3d_chirality;
        loader.ignore_closing_bond_direction_mismatch = self.ignore_closing_bond_direction_mismatch;
        loader.ignore_no_chiral_flag = self.ignore_no_chiral_flag;
        loader.treat_stereo_as = self.treat_stereo_as;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.dearomatize_on_load = self.dearomatize_on_load;
        loader.arom_options = self.arom_options;

        std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

        Molecule& mol = molptr->mol;

        loader.loadMolecule(mol);
        molptr->getProperties().copy(loader.properties);

        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadQueryMolecule(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        MoleculeAutoLoader loader(IndigoScanner::get(obj));

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
        loader.dearomatize_on_load = self.dearomatize_on_load;
        loader.arom_options = self.arom_options;

        std::unique_ptr<IndigoQueryMolecule> molptr = std::make_unique<IndigoQueryMolecule>();

        QueryMolecule& qmol = molptr->qmol;

        loader.loadMolecule(qmol);
        molptr->copyProperties(loader.properties);

        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSequence(int source, const char* seq_type)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        SequenceLoader loader(IndigoScanner::get(obj));

        std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

        Molecule& mol = molptr->mol;
        loader.loadSequence(mol, seq_type);
        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSequenceFromString(const char* string, const char* seq_type)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadSequence(source, seq_type);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSequenceFromFile(const char* filename, const char* seq_type)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadSequence(source, seq_type);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFasta(int source, const char* seq_type)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        SequenceLoader loader(IndigoScanner::get(obj));

        std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

        Molecule& mol = molptr->mol;
        loader.loadFasta(mol, seq_type);
        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFastaFromString(const char* string, const char* seq_type)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadFasta(source, seq_type);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFastaFromFile(const char* filename, const char* seq_type)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadFasta(source, seq_type);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadIDT(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        SequenceLoader loader(IndigoScanner::get(obj));

        std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

        Molecule& mol = molptr->mol;
        loader.loadIDT(mol);
        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadIDTFromString(const char* string)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadIDT(source);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadIDTFromFile(const char* filename)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadIDT(source);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSmarts(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        SmilesLoader loader(IndigoScanner::get(obj));

        std::unique_ptr<IndigoQueryMolecule> molptr = std::make_unique<IndigoQueryMolecule>();

        QueryMolecule& qmol = molptr->qmol;

        loader.loadSMARTS(qmol);
        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

static bool isReacton(const char* string)
{
    auto isIn = [](const char* source, const char* pattern) -> bool { return std::string(source).find(pattern) != std::string::npos; };

    auto startWith = [](const char* source, const char* pattern) -> bool { return strncmp(source, pattern, strlen(pattern)) == 0; };

    return isIn(string, ">>") || startWith(string, "$RXN") || isIn(string, "<reactantList>");
}

CEXPORT int indigoLoadStructureFromString(const char* string, const char* params)
{
    INDIGO_BEGIN
    {
        if (strncmp(string, "InChI", strlen("InChI")) == 0)
        {
            return indigoLoadMoleculeFromString(string);
        }

        const std::string strParams(params ? params : "");
        bool isQuery = (strParams.find("query") != std::string::npos) ? true : false;
        bool isSmarts = (strParams.find("smarts") != std::string::npos) ? true : false;
        bool isReaction = isReacton(string);

        if (isSmarts)
        {
            if (isReaction)
                return indigoLoadReactionSmartsFromString(string);
            else
                return indigoLoadSmartsFromString(string);
        }

        if (isQuery)
        {
            if (isReaction)
                return indigoLoadQueryReactionFromString(string);
            else
                return indigoLoadQueryMoleculeFromString(string);
        }

        try
        {
            if (isReaction)
                return indigoLoadReactionFromString(string);
            else
                return indigoLoadMoleculeFromString(string);
        }
        catch (Exception& e)
        {
            if (std::string(e.message()).find("query") == std::string::npos && std::string(e.message()).find("queries") == std::string::npos)
            {
                throw e;
            }

            if (isReaction)
                return indigoLoadQueryReactionFromString(string);
            else
                return indigoLoadQueryMoleculeFromString(string);
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadStructureFromBuffer(const byte* buff, int bufferSize, const char* params)
{
    BufferScanner scanner(buff, bufferSize);
    Array<char> arr;
    MoleculeAutoLoader::readAllDataToString(scanner, arr);

    return indigoLoadStructureFromString(arr.ptr(), params);
}

CEXPORT int indigoLoadStructureFromFile(const char* filename, const char* params)
{
    INDIGO_BEGIN
    {
        FileScanner scanner(self.filename_encoding, filename);
        Array<char> arr;
        MoleculeAutoLoader::readAllDataToString(scanner, arr);

        return indigoLoadStructureFromString(arr.ptr(), params);
    }
    INDIGO_END(-1);
}

IndigoMoleculeComponent::IndigoMoleculeComponent(BaseMolecule& mol_, int index_) : IndigoObject(COMPONENT), mol(mol_)
{
    index = index_;
}

IndigoMoleculeComponent::~IndigoMoleculeComponent()
{
}

int IndigoMoleculeComponent::getIndex()
{
    return index;
}

IndigoObject* IndigoMoleculeComponent::clone()
{
    std::unique_ptr<IndigoBaseMolecule> res;
    BaseMolecule* newmol;

    if (mol.isQueryMolecule())
    {
        res = std::make_unique<IndigoQueryMolecule>();
        newmol = &(((IndigoQueryMolecule*)res.get())->qmol);
    }
    else
    {
        res = std::make_unique<IndigoMolecule>();
        newmol = &(((IndigoMolecule*)res.get())->mol);
    }

    Filter filter(mol.getDecomposition().ptr(), Filter::EQ, index);
    newmol->makeSubmolecule(mol, filter, 0, 0);
    for (auto it = newmol->properties().begin(); it != newmol->properties().end(); ++it)
    {
        auto& props = newmol->properties().value(it);
        res->getProperties().merge(props);
    }

    return res.release();
}

IndigoComponentsIter::IndigoComponentsIter(BaseMolecule& mol_) : IndigoObject(COMPONENT), mol(mol_)
{
    _idx = -1;
}

IndigoComponentsIter::~IndigoComponentsIter()
{
}

bool IndigoComponentsIter::hasNext()
{
    return _idx + 1 < mol.countComponents();
}

IndigoObject* IndigoComponentsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    return new IndigoMoleculeComponent(mol, _idx);
}

IndigoSGroupAtomsIter::IndigoSGroupAtomsIter(BaseMolecule& mol, SGroup& sgroup) : IndigoObject(SGROUP_ATOMS_ITER), _mol(mol), _sgroup(sgroup)
{
    _idx = -1;
}

IndigoSGroupAtomsIter::~IndigoSGroupAtomsIter()
{
}

bool IndigoSGroupAtomsIter::hasNext()
{
    return _idx + 1 < _sgroup.atoms.size();
}

IndigoObject* IndigoSGroupAtomsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    return new IndigoAtom(_mol, _sgroup.atoms[_idx]);
}

IndigoSGroupBondsIter::IndigoSGroupBondsIter(BaseMolecule& mol, SGroup& sgroup) : IndigoObject(SGROUP_ATOMS_ITER), _mol(mol), _sgroup(sgroup)
{
    _idx = -1;
}

IndigoSGroupBondsIter::~IndigoSGroupBondsIter()
{
}

bool IndigoSGroupBondsIter::hasNext()
{
    return _idx + 1 < _sgroup.bonds.size();
}

IndigoObject* IndigoSGroupBondsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    return new IndigoBond(_mol, _sgroup.bonds[_idx]);
}

int _indigoIterateAtoms(Indigo& self, int molecule, int type)
{
    return self.addObject(new IndigoAtomsIter(&self.getObject(molecule).getBaseMolecule(), type));
}

CEXPORT int indigoIterateAtoms(int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);

        if (obj.type == IndigoObject::COMPONENT)
        {
            IndigoMoleculeComponent& mc = (IndigoMoleculeComponent&)obj;
            return self.addObject(new IndigoComponentAtomsIter(mc.mol, mc.index));
        }
        if (obj.type == IndigoObject::SUBMOLECULE)
        {
            IndigoSubmolecule& sm = (IndigoSubmolecule&)obj;
            return self.addObject(new IndigoSubmoleculeAtomsIter(sm));
        }
        if (obj.type == IndigoObject::DATA_SGROUP)
        {
            IndigoDataSGroup& dsg = IndigoDataSGroup::cast(obj);
            return self.addObject(new IndigoSGroupAtomsIter(dsg.mol, dsg.mol.sgroups.getSGroup(dsg.idx)));
        }
        if (obj.type == IndigoObject::SUPERATOM)
        {
            IndigoSuperatom& sa = IndigoSuperatom::cast(obj);
            return self.addObject(new IndigoSGroupAtomsIter(sa.mol, sa.mol.sgroups.getSGroup(sa.idx)));
        }
        if (obj.type == IndigoObject::REPEATING_UNIT)
        {
            IndigoRepeatingUnit& ru = IndigoRepeatingUnit::cast(obj);
            return self.addObject(new IndigoSGroupAtomsIter(ru.mol, ru.mol.sgroups.getSGroup(ru.idx)));
        }
        if (obj.type == IndigoObject::MULTIPLE_GROUP)
        {
            IndigoMultipleGroup& mr = IndigoMultipleGroup::cast(obj);
            return self.addObject(new IndigoSGroupAtomsIter(mr.mol, mr.mol.sgroups.getSGroup(mr.idx)));
        }
        if (obj.type == IndigoObject::GENERIC_SGROUP)
        {
            IndigoGenericSGroup& gg = IndigoGenericSGroup::cast(obj);
            return self.addObject(new IndigoSGroupAtomsIter(gg.mol, gg.mol.sgroups.getSGroup(gg.idx)));
        }

        return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::ALL);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateBonds(int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);

        if (obj.type == IndigoObject::COMPONENT)
        {
            IndigoMoleculeComponent& mc = (IndigoMoleculeComponent&)obj;
            return self.addObject(new IndigoComponentBondsIter(mc.mol, mc.index));
        }
        if (obj.type == IndigoObject::SUBMOLECULE)
        {
            IndigoSubmolecule& sm = (IndigoSubmolecule&)obj;
            return self.addObject(new IndigoSubmoleculeBondsIter(sm));
        }
        if (obj.type == IndigoObject::DATA_SGROUP)
        {
            IndigoDataSGroup& dsg = IndigoDataSGroup::cast(obj);
            return self.addObject(new IndigoSGroupBondsIter(dsg.mol, dsg.mol.sgroups.getSGroup(dsg.idx)));
        }
        if (obj.type == IndigoObject::SUPERATOM)
        {
            IndigoSuperatom& sa = IndigoSuperatom::cast(obj);
            return self.addObject(new IndigoSGroupBondsIter(sa.mol, sa.mol.sgroups.getSGroup(sa.idx)));
        }
        if (obj.type == IndigoObject::REPEATING_UNIT)
        {
            IndigoRepeatingUnit& ru = IndigoRepeatingUnit::cast(obj);
            return self.addObject(new IndigoSGroupBondsIter(ru.mol, ru.mol.sgroups.getSGroup(ru.idx)));
        }
        if (obj.type == IndigoObject::MULTIPLE_GROUP)
        {
            IndigoMultipleGroup& mr = IndigoMultipleGroup::cast(obj);
            return self.addObject(new IndigoSGroupBondsIter(mr.mol, mr.mol.sgroups.getSGroup(mr.idx)));
        }
        if (obj.type == IndigoObject::GENERIC_SGROUP)
        {
            IndigoGenericSGroup& gg = IndigoGenericSGroup::cast(obj);
            return self.addObject(new IndigoSGroupBondsIter(gg.mol, gg.mol.sgroups.getSGroup(gg.idx)));
        }
        BaseMolecule& mol = obj.getBaseMolecule();
        return self.addObject(new IndigoBondsIter(mol));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountAtoms(int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);

        if (obj.type == IndigoObject::COMPONENT)
        {
            IndigoMoleculeComponent& mc = (IndigoMoleculeComponent&)obj;
            return mc.mol.countComponentVertices(mc.index);
        }
        if (obj.type == IndigoObject::SUBMOLECULE)
        {
            IndigoSubmolecule& sm = (IndigoSubmolecule&)obj;
            return sm.vertices.size();
        }
        if (obj.type == IndigoObject::DATA_SGROUP)
        {
            IndigoDataSGroup& dsg = IndigoDataSGroup::cast(obj);
            return dsg.get().atoms.size();
        }
        if (obj.type == IndigoObject::SUPERATOM)
        {
            IndigoSuperatom& sa = IndigoSuperatom::cast(obj);
            return sa.get().atoms.size();
        }

        BaseMolecule& mol = obj.getBaseMolecule();

        return mol.vertexCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountBonds(int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);

        if (obj.type == IndigoObject::COMPONENT)
        {
            IndigoMoleculeComponent& mc = (IndigoMoleculeComponent&)obj;
            return mc.mol.countComponentEdges(mc.index);
        }
        if (obj.type == IndigoObject::SUBMOLECULE)
        {
            IndigoSubmolecule& sm = (IndigoSubmolecule&)obj;
            return sm.edges.size();
        }
        if (obj.type == IndigoObject::DATA_SGROUP)
        {
            IndigoDataSGroup& dsg = IndigoDataSGroup::cast(obj);
            return dsg.get().bonds.size();
        }
        if (obj.type == IndigoObject::SUPERATOM)
        {
            IndigoSuperatom& sa = IndigoSuperatom::cast(obj);
            return sa.get().bonds.size();
        }

        BaseMolecule& mol = obj.getBaseMolecule();

        return mol.edgeCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountPseudoatoms(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int i, res = 0;

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            if (mol.isPseudoAtom(i))
                res++;

        return res;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountRSites(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int i, res = 0;

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            if (mol.isRSite(i))
                res++;

        return res;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIteratePseudoatoms(int molecule)
{
    INDIGO_BEGIN
    {
        return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::PSEUDO);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateRSites(int molecule)
{
    INDIGO_BEGIN
    {
        return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::RSITE);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateStereocenters(int molecule)
{
    INDIGO_BEGIN
    {
        return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::STEREOCENTER);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateAlleneCenters(int molecule)
{
    INDIGO_BEGIN
    {
        return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::ALLENE_CENTER);
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoSymbol(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        auto& tmp = self.getThreadTmpData();
        ia.mol.getAtomSymbol(ia.idx, tmp.string);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoIsPseudoatom(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.isPseudoAtom(ia.idx))
            return 1;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIsRSite(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.isRSite(ia.idx))
            return 1;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIsTemplateAtom(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.isTemplateAtom(ia.idx))
            return 1;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSingleAllowedRGroup(int rsite)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(rsite));

        return ia.mol.getSingleAllowedRGroup(ia.idx);
    }
    INDIGO_END(-1);
}

IndigoRGroup::IndigoRGroup() : IndigoObject(RGROUP)
{
}

IndigoRGroup::~IndigoRGroup()
{
}

int IndigoRGroup::getIndex()
{
    return idx;
}

IndigoRGroup& IndigoRGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::RGROUP)
        return (IndigoRGroup&)obj;
    throw IndigoError("%s is not an rgroup", obj.debugInfo());
}

IndigoRGroupsIter::IndigoRGroupsIter(BaseMolecule* mol) : IndigoObject(RGROUPS_ITER)
{
    _mol = mol;
    _idx = 0;
}

IndigoRGroupsIter::~IndigoRGroupsIter()
{
}

CEXPORT int indigoIterateRGroups(int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& mol = obj.getBaseMolecule();

            return self.addObject(new IndigoRGroupsIter(&mol));
        }

        throw IndigoError("%s can not have r-groups", obj.debugInfo());
    }
    INDIGO_END(-1);
}

IndigoRGroupFragment::IndigoRGroupFragment(IndigoRGroup& rgp, int idx) : IndigoObject(RGROUP_FRAGMENT)
{
    rgroup.idx = rgp.idx;
    rgroup.mol = rgp.mol;
    frag_idx = idx;
}

IndigoRGroupFragment::IndigoRGroupFragment(BaseMolecule* mol, int rgroup_idx, int fragment_idx) : IndigoObject(RGROUP_FRAGMENT)
{
    rgroup.mol = mol;
    rgroup.idx = rgroup_idx;
    frag_idx = fragment_idx;
}

IndigoRGroupFragment::~IndigoRGroupFragment()
{
}

int IndigoRGroupFragment::getIndex()
{
    return frag_idx;
}

void IndigoRGroupFragment::remove()
{
    rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments.remove(frag_idx);
}

QueryMolecule& IndigoRGroupFragment::getQueryMolecule()
{
    return rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx]->asQueryMolecule();
}

Molecule& IndigoRGroupFragment::getMolecule()
{
    return rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx]->asMolecule();
}

BaseMolecule& IndigoRGroupFragment::getBaseMolecule()
{
    return *rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx];
}

IndigoObject* IndigoRGroupFragment::clone()
{
    BaseMolecule* mol = rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx];

    std::unique_ptr<IndigoBaseMolecule> molptr;

    if (mol->isQueryMolecule())
    {
        molptr = std::make_unique<IndigoQueryMolecule>();
        molptr->getQueryMolecule().clone(*mol, 0, 0);
    }
    else
    {
        molptr = std::make_unique<IndigoMolecule>();
        molptr->getMolecule().clone(*mol, 0, 0);
    }

    return molptr.release();
}

IndigoRGroupFragmentsIter::IndigoRGroupFragmentsIter(IndigoRGroup& rgp) : IndigoObject(RGROUP_FRAGMENTS_ITER)
{
    _mol = rgp.mol;
    _rgroup_idx = rgp.idx;
    _frag_idx = -1;
}

IndigoRGroupFragmentsIter::~IndigoRGroupFragmentsIter()
{
}

bool IndigoRGroupFragmentsIter::hasNext()
{
    PtrPool<BaseMolecule>& frags = _mol->rgroups.getRGroup(_rgroup_idx).fragments;

    if (_frag_idx == -1)
        return frags.begin() != frags.end();
    return frags.next(_frag_idx) != frags.end();
}

IndigoObject* IndigoRGroupFragmentsIter::next()
{
    if (!hasNext())
        return nullptr;

    PtrPool<BaseMolecule>& frags = _mol->rgroups.getRGroup(_rgroup_idx).fragments;

    if (_frag_idx == -1)
        _frag_idx = frags.begin();
    else
        _frag_idx = frags.next(_frag_idx);
    return new IndigoRGroupFragment(_mol, _rgroup_idx, _frag_idx);
}

CEXPORT int indigoIterateRGroupFragments(int rgroup)
{
    INDIGO_BEGIN
    {
        IndigoRGroup& rgp = IndigoRGroup::cast(self.getObject(rgroup));
        return self.addObject(new IndigoRGroupFragmentsIter(rgp));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountRGroups(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.rgroups.getRGroupCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCopyRGroups(int molecule_from, int molecule_to)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol_from = self.getObject(molecule_from).getBaseMolecule();
        BaseMolecule& mol_to = self.getObject(molecule_to).getBaseMolecule();
        mol_from.rgroups.copyRGroupsFromMolecule(mol_to.rgroups);
        return 0;
    }
    INDIGO_END(-1);
}

bool IndigoRGroupsIter::hasNext()
{
    bool result = false;
    /*
     * Skip empty fragments
     */
    while ((_idx < _mol->rgroups.getRGroupCount()) && (_mol->rgroups.getRGroup(_idx + 1).fragments.size() == 0))
    {
        ++_idx;
    }

    if (_idx < _mol->rgroups.getRGroupCount())
        result = true;

    return result;
}

IndigoObject* IndigoRGroupsIter::next()
{
    if (!hasNext())
        return 0;

    _idx += 1;
    std::unique_ptr<IndigoRGroup> rgroup = std::make_unique<IndigoRGroup>();

    rgroup->mol = _mol;
    rgroup->idx = _idx;
    return rgroup.release();
}

CEXPORT int indigoCountAttachmentPoints(int rgroup)
{
    INDIGO_BEGIN
    {
        IndigoObject& object = self.getObject(rgroup);
        if (IndigoBaseMolecule::is(object))
            return object.getBaseMolecule().attachmentPointCount();

        IndigoRGroup& rgp = IndigoRGroup::cast(object);

        return rgp.mol->rgroups.getRGroup(rgp.idx).fragments[0]->attachmentPointCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoDegree(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        return ia.mol.getVertex(ia.idx).degree();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetCharge(int atom, int* charge)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        int ch = ia.mol.getAtomCharge(ia.idx);
        if (ch == CHARGE_UNKNOWN)
        {
            *charge = 0;
            return 0;
        }
        *charge = ch;
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoValence(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        return ia.mol.asMolecule().getAtomValence(ia.idx);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetHybridization(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        auto& molecule = ia.mol.asMolecule();
        return static_cast<int>(HybridizationCalculator::calculate(molecule, ia.idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCheckValence(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.isPseudoAtom(ia.idx) || ia.mol.isRSite(ia.idx) || ia.mol.isTemplateAtom(ia.idx))
            return 1;

        int res = ia.mol.getAtomValence_NoThrow(ia.idx, -100);

        return res == -100 ? 1 : 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetExplicitValence(int atom, int* valence)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        int val = ia.mol.getExplicitValence(ia.idx);
        if (val == -1)
        {
            *valence = 0;
            return 0;
        }
        *valence = val;
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetExplicitValence(int atom, int valence)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        ia.mol.asMolecule().setExplicitValence(ia.idx, valence);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIsotope(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        int iso = ia.mol.getAtomIsotope(ia.idx);
        return iso == -1 ? 0 : iso;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAtomicNumber(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.isPseudoAtom(ia.idx))
            throw IndigoError("indigoAtomicNumber() called on a pseudoatom");
        if (ia.mol.isRSite(ia.idx))
            throw IndigoError("indigoAtomicNumber() called on an R-site");

        int num = ia.mol.getAtomNumber(ia.idx);
        return num == -1 ? 0 : num;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetRadicalElectrons(int atom, int* electrons)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        int rad = ia.mol.getAtomRadical(ia.idx);

        if (rad == -1)
        {
            *electrons = 0;
            return 0;
        }
        *electrons = Element::radicalElectrons(rad);
        return 1;
    }
    INDIGO_END(-1);
}

static int mapRadicalToIndigoRadical(int radical)
{
    switch (radical)
    {
    case 0:
        return 0;
    case RADICAL_SINGLET:
        return INDIGO_SINGLET;
    case RADICAL_DOUBLET:
        return INDIGO_DOUBLET;
    case RADICAL_TRIPLET:
        return INDIGO_TRIPLET;
    default:
        throw IndigoError("Unknown radical type");
    }
}

static int mapIndigoRadicalToRadical(int indigo_radical)
{
    switch (indigo_radical)
    {
    case 0:
        return 0;
    case INDIGO_SINGLET:
        return RADICAL_SINGLET;
    case INDIGO_DOUBLET:
        return RADICAL_DOUBLET;
    case INDIGO_TRIPLET:
        return RADICAL_TRIPLET;
    default:
        throw IndigoError("Unknown radical type");
    }
}

CEXPORT int indigoGetRadical(int atom, int* radical)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        int rad = ia.mol.getAtomRadical(ia.idx);

        if (rad == -1)
        {
            *radical = 0;
            return 0;
        }
        *radical = mapRadicalToIndigoRadical(rad);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetRadical(int atom, int radical)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        ia.mol.asMolecule().setAtomRadical(ia.idx, mapIndigoRadicalToRadical(radical));
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT float* indigoXYZ(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        Vec3f& pos = mol.getAtomXyz(ia.idx);
        auto& tmp = self.getThreadTmpData();
        tmp.xyz[0] = pos.x;
        tmp.xyz[1] = pos.y;
        tmp.xyz[2] = pos.z;
        return tmp.xyz;
    }
    INDIGO_END(0);
}

CEXPORT int indigoSetXYZ(int atom, float x, float y, float z)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        Vec3f& pos = mol.getAtomXyz(ia.idx);
        pos.set(x, y, z);
        return 1;
    }
    INDIGO_END(0);
}

CEXPORT int indigoResetCharge(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        if (mol.isQueryMolecule())
            mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_CHARGE);
        else
            mol.asMolecule().setAtomCharge(ia.idx, 0);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetExplicitValence(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        if (mol.isQueryMolecule())
            mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_VALENCE);
        else
            mol.asMolecule().resetExplicitValence(ia.idx);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetRadical(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        if (mol.isQueryMolecule())
            mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_RADICAL);
        else
            mol.asMolecule().setAtomRadical(ia.idx, 0);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetIsotope(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        if (mol.isQueryMolecule())
            mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_ISOTOPE);
        else
            mol.asMolecule().setAtomIsotope(ia.idx, 0);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetRsite(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        BaseMolecule& mol = ia.mol;

        mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_RSITE);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetAttachmentPoint(int atom, int order)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        ia.mol.addAttachmentPoint(order, ia.idx);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearAttachmentPoints(int item)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(item).getBaseMolecule();
        mol.removeAttachmentPoints();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRemoveConstraints(int item, const char* str_type)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(item));
        QueryMolecule& qmol = ia.mol.asQueryMolecule();

        if (strcasecmp(str_type, "smarts") == 0)
            throw IndigoError("indigoRemoveConstraints(): type 'smarts' is not supported", str_type);

        std::unique_ptr<QueryMolecule::Atom> atom;
        IndigoQueryMolecule::parseAtomConstraint(str_type, NULL, atom);

        if (atom->children.size() != 0)
            throw IndigoError("indigoRemoveConstraints(): can not parse type: %s", str_type);

        qmol.getAtom(ia.idx).removeConstraints(atom->type);
        qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddConstraint(int atom, const char* type, const char* value)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        QueryMolecule& qmol = ia.mol.asQueryMolecule();
        std::unique_ptr<QueryMolecule::Atom> atom_constraint;

        IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

        qmol.resetAtom(ia.idx, QueryMolecule::Atom::und(qmol.releaseAtom(ia.idx), atom_constraint.release()));
        qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddConstraintNot(int atom, const char* type, const char* value)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        QueryMolecule& qmol = ia.mol.asQueryMolecule();
        std::unique_ptr<QueryMolecule::Atom> atom_constraint;

        IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

        qmol.resetAtom(ia.idx, QueryMolecule::Atom::und(qmol.releaseAtom(ia.idx), QueryMolecule::Atom::nicht(atom_constraint.release())));
        qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddConstraintOr(int atom, const char* type, const char* value)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        QueryMolecule& qmol = ia.mol.asQueryMolecule();
        std::unique_ptr<QueryMolecule::Atom> atom_constraint;

        IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

        qmol.resetAtom(ia.idx, QueryMolecule::Atom::oder(qmol.releaseAtom(ia.idx), atom_constraint.release()));
        qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

        return 1;
    }
    INDIGO_END(-1);
}

/*
CEXPORT int indigoAddConstraintOrNot(int atom, const char* type, const char* value)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      BaseMolecule *mol = ia.mol;
      QueryMolecule& qmol = mol->asQueryMolecule();
      std::unique_ptr<QueryMolecule::Atom> atom_constraint;

      IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

      qmol.resetAtom(ia.idx, QueryMolecule::Atom::oder(qmol.releaseAtom(ia.idx), QueryMolecule::Atom::nicht(atom_constraint.release())));

      return 1;
   }
   INDIGO_END(-1);
}
 * */

CEXPORT const int* indigoSymmetryClasses(int molecule, int* count_out)
{
    INDIGO_BEGIN
    {
        Molecule& mol = self.getObject(molecule).getMolecule();

        QS_DEF(Molecule, m2);
        m2.clone_KeepIndices(mol);
        m2.aromatize(self.arom_options);

        QS_DEF(Array<int>, ignored);
        ignored.clear_resize(m2.vertexEnd());
        ignored.zerofill();

        for (int i = m2.vertexBegin(); i < m2.vertexEnd(); i = m2.vertexNext(i))
            if (m2.convertableToImplicitHydrogen(i))
                ignored[i] = 1;

        MoleculeAutomorphismSearch of;

        QS_DEF(Array<int>, orbits);
        of.find_canonical_ordering = true;
        of.ignored_vertices = ignored.ptr();
        of.process(m2);
        of.getCanonicallyOrderedOrbits(orbits);

        auto& tmp = self.getThreadTmpData();
        tmp.string.resize(orbits.sizeInBytes());
        tmp.string.copy((char*)orbits.ptr(), orbits.sizeInBytes());

        if (count_out != 0)
            *count_out = orbits.size();

        return (const int*)tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoLayeredCode(int molecule)
{
    INDIGO_BEGIN
    {
        Molecule& mol = self.getObject(molecule).getMolecule();

        auto& tmp = self.getThreadTmpData();
        ArrayOutput output(tmp.string);

        MoleculeInChI inchi_saver(output);
        inchi_saver.outputInChI(mol);

        tmp.string.push(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoCreateSubmolecule(int molecule, int nvertices, int* vertices)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        QS_DEF(Array<int>, vertices_arr);

        vertices_arr.copy(vertices, nvertices);

        if (mol.isQueryMolecule())
        {
            std::unique_ptr<IndigoQueryMolecule> molptr = std::make_unique<IndigoQueryMolecule>();

            molptr->qmol.makeSubmolecule(mol, vertices_arr, 0, 0);
            return self.addObject(molptr.release());
        }
        else
        {
            std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

            molptr->mol.makeSubmolecule(mol, vertices_arr, 0, 0);
            return self.addObject(molptr.release());
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSubmolecule(int molecule, int nvertices, int* vertices)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        QS_DEF(Array<int>, vertices_arr);
        vertices_arr.copy(vertices, nvertices);

        // Collect edges by vertices
        QS_DEF(Array<int>, vertices_mask);
        vertices_mask.clear_resize(mol.vertexEnd());
        vertices_mask.zerofill();
        for (int i = 0; i < nvertices; i++)
            vertices_mask[vertices[i]] = 1;

        QS_DEF(Array<int>, edges);
        edges.clear();
        for (int i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
        {
            const Edge& edge = mol.getEdge(i);
            if (vertices_mask[edge.beg] && vertices_mask[edge.end])
                edges.push(i);
        }
        return self.addObject(new IndigoSubmolecule(mol, vertices_arr, edges));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCreateEdgeSubmolecule(int molecule, int nvertices, int* vertices, int nedges, int* edges)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        QS_DEF(Array<int>, vertices_arr);
        QS_DEF(Array<int>, edges_arr);

        vertices_arr.copy(vertices, nvertices);
        edges_arr.copy(edges, nedges);

        if (mol.isQueryMolecule())
        {
            std::unique_ptr<IndigoQueryMolecule> molptr = std::make_unique<IndigoQueryMolecule>();

            molptr->qmol.makeEdgeSubmolecule(mol, vertices_arr, edges_arr, 0, 0);
            return self.addObject(molptr.release());
        }
        else
        {
            std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

            molptr->mol.makeEdgeSubmolecule(mol, vertices_arr, edges_arr, 0, 0);
            return self.addObject(molptr.release());
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRemoveAtoms(int molecule, int nvertices, int* vertices)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        QS_DEF(Array<int>, indices);

        indices.copy(vertices, nvertices);

        mol.removeAtoms(indices);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRemoveBonds(int molecule, int nbonds, int* bonds)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        QS_DEF(Array<int>, indices);

        indices.copy(bonds, nbonds);

        mol.removeBonds(indices);
        return 1;
    }
    INDIGO_END(-1);
}

IndigoObject* IndigoMolecule::clone()
{
    return cloneFrom(*this);
}

const char* IndigoMolecule::debugInfo() const
{
    return "<molecule>";
}

IndigoObject* IndigoQueryMolecule::clone()
{
    return cloneFrom(*this);
}

const char* IndigoQueryMolecule::debugInfo() const
{
    return "<query molecule>";
}

const MoleculeAtomNeighbourhoodCounters& IndigoQueryMolecule::getNeiCounters()
{
    // TODO: implement query.getAtomEdit(...) instead of getAtom(...) to update nei counters
    // automatically. Current approach is too complictated because
    // we need to call updateEditRevision manually after changing an atom.
    // if (_nei_counters_edit_revision != qmol.getEditRevision())
    {
        _nei_counters.calculate(qmol);
        _nei_counters_edit_revision = qmol.getEditRevision();
    }
    return _nei_counters;
}

CEXPORT int indigoIsChiral(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.isChiral();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoBondOrder(int bond)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(bond));

        int num = ib.mol.getBondOrder(ib.idx);
        return num == -1 ? 0 : num;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoTopology(int bond)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(bond));

        int topology = ib.mol.getBondTopology(ib.idx);
        if (topology == TOPOLOGY_RING)
            return INDIGO_RING;
        if (topology == TOPOLOGY_CHAIN)
            return INDIGO_CHAIN;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetAtom(int molecule, int idx)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoAtom(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetBond(int molecule, int idx)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoBond(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSource(int bond)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(bond));
        return self.addObject(new IndigoAtom(ib.mol, ib.mol.getEdge(ib.idx).beg));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoDestination(int bond)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(bond));
        return self.addObject(new IndigoAtom(ib.mol, ib.mol.getEdge(ib.idx).end));
    }
    INDIGO_END(-1);
}

IndigoAtomNeighbor::IndigoAtomNeighbor(BaseMolecule& mol_, int atom_idx, int bond_idx_) : IndigoAtom(mol_, atom_idx)
{
    type = ATOM_NEIGHBOR;

    bond_idx = bond_idx_;
}

IndigoAtomNeighbor::~IndigoAtomNeighbor()
{
}

IndigoAtomNeighborsIter::IndigoAtomNeighborsIter(BaseMolecule& molecule, int atom_idx) : IndigoObject(ATOM_NEIGHBORS_ITER), _mol(molecule)
{
    _atom_idx = atom_idx;
    _nei_idx = -1;
}

IndigoAtomNeighborsIter::~IndigoAtomNeighborsIter()
{
}

IndigoObject* IndigoAtomNeighborsIter::next()
{
    const Vertex& vertex = _mol.getVertex(_atom_idx);

    if (_nei_idx == -1)
        _nei_idx = vertex.neiBegin();
    else if (_nei_idx != vertex.neiEnd())
        _nei_idx = vertex.neiNext(_nei_idx);

    if (_nei_idx == vertex.neiEnd())
        return 0;

    return new IndigoAtomNeighbor(_mol, vertex.neiVertex(_nei_idx), vertex.neiEdge(_nei_idx));
}

bool IndigoAtomNeighborsIter::hasNext()
{
    const Vertex& vertex = _mol.getVertex(_atom_idx);

    if (_nei_idx == -1)
        return vertex.neiBegin() != vertex.neiEnd();

    if (_nei_idx == vertex.neiEnd())
        return false;

    return vertex.neiNext(_nei_idx) != vertex.neiEnd();
}

CEXPORT int indigoIterateNeighbors(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        return self.addObject(new IndigoAtomNeighborsIter(ia.mol, ia.idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoBond(int nei)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(nei);

        if (obj.type != IndigoObject::ATOM_NEIGHBOR)
            throw IndigoError("indigoBond(): not applicable to %s", obj.debugInfo());

        IndigoAtomNeighbor& atomnei = (IndigoAtomNeighbor&)obj;

        return self.addObject(new IndigoBond(atomnei.mol, atomnei.bond_idx));
    }
    INDIGO_END(-1);
}

CEXPORT float indigoAlignAtoms(int molecule, int natoms, int* atom_ids, float* desired_xyz)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        QS_DEF(Array<Vec3f>, points);
        QS_DEF(Array<Vec3f>, goals);
        int i;

        if (natoms < 1)
            throw IndigoError("indigoAlignAtoms(): can not align %d atoms", natoms);

        if (atom_ids == 0 || desired_xyz == 0)
            throw IndigoError("indigoAlignAtoms(): zero pointer given as input");

        points.clear();
        goals.clear();

        for (i = 0; i < natoms; i++)
        {
            points.push(mol.getAtomXyz(atom_ids[i]));
            goals.push(Vec3f(desired_xyz[i * 3], desired_xyz[i * 3 + 1], desired_xyz[i * 3 + 2]));
        }

        if (points.size() < 1)
            return true;

        float sqsum;
        Transform3f matr;

        if (!matr.bestFit(points.size(), points.ptr(), goals.ptr(), &sqsum))
            return false;

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            mol.getAtomXyz(i).transformPoint(matr);

        return (float)(sqrt(sqsum / natoms));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearXYZ(int molecule)
{
    INDIGO_BEGIN
    {
        self.getObject(molecule).getBaseMolecule().clearXyz();
        return molecule;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountSuperatoms(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SUP);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountDataSGroups(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.sgroups.getSGroupCount(SGroup::SG_TYPE_DAT);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountRepeatingUnits(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountMultipleGroups(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.sgroups.getSGroupCount(SGroup::SG_TYPE_MUL);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountGenericSGroups(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return mol.sgroups.getSGroupCount(SGroup::SG_TYPE_GEN);
    }
    INDIGO_END(-1);
}

IndigoDataSGroupsIter::IndigoDataSGroupsIter(BaseMolecule& molecule, Array<int>&& refs)
    : IndigoObject(DATA_SGROUPS_ITER), _mol(molecule), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoDataSGroupsIter::~IndigoDataSGroupsIter()
{
}

bool IndigoDataSGroupsIter::hasNext()
{
    if (_idx == -1)
        return _refs.size() > 0;
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoDataSGroupsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoDataSGroup> sgroup = std::make_unique<IndigoDataSGroup>(_mol, _refs[_idx]);
    return sgroup.release();
}

CEXPORT int indigoIterateDataSGroups(int molecule)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<int>, sgs);
        sgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        mol.sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_DAT, sgs);
        return self.addObject(new IndigoDataSGroupsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

IndigoDataSGroup::IndigoDataSGroup(BaseMolecule& mol_, int idx_) : IndigoObject(DATA_SGROUP), mol(mol_)
{
    idx = idx_;
}

IndigoDataSGroup& IndigoDataSGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::DATA_SGROUP)
        return (IndigoDataSGroup&)obj;

    throw IndigoError("%s is not a data sgroup", obj.debugInfo());
}

DataSGroup& IndigoDataSGroup::get()
{
    return (DataSGroup&)mol.sgroups.getSGroup(idx);
}

void IndigoDataSGroup::remove()
{
    mol.removeSGroup(idx);
}

IndigoDataSGroup::~IndigoDataSGroup()
{
}

int IndigoDataSGroup::getIndex()
{
    return idx;
}

IndigoSuperatom::IndigoSuperatom(BaseMolecule& mol_, int idx_) : IndigoObject(SUPERATOM), mol(mol_)
{
    idx = idx_;
}

IndigoSuperatom::~IndigoSuperatom()
{
}

int IndigoSuperatom::getIndex()
{
    return idx;
}

void IndigoSuperatom::remove()
{
    mol.removeSGroup(idx);
}

const char* IndigoSuperatom::getName()
{
    return ((Superatom&)mol.sgroups.getSGroup(idx)).subscript.ptr();
}

IndigoSuperatom& IndigoSuperatom::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::SUPERATOM)
        return (IndigoSuperatom&)obj;

    throw IndigoError("%s is not a superatom", obj.debugInfo());
}

Superatom& IndigoSuperatom::get()
{
    return (Superatom&)mol.sgroups.getSGroup(idx);
}

IndigoSuperatomsIter::IndigoSuperatomsIter(BaseMolecule& molecule, Array<int>&& refs) : IndigoObject(SUPERATOMS_ITER), _mol(molecule), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoSuperatomsIter::~IndigoSuperatomsIter()
{
}

bool IndigoSuperatomsIter::hasNext()
{
    if (_idx == -1)
        return _refs.size() > 0;
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoSuperatomsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoSuperatom> sgroup = std::make_unique<IndigoSuperatom>(_mol, _refs[_idx]);
    return sgroup.release();
}

IndigoRepeatingUnit::IndigoRepeatingUnit(BaseMolecule& mol_, int idx_) : IndigoObject(REPEATING_UNIT), mol(mol_)
{
    idx = idx_;
}

IndigoRepeatingUnit::~IndigoRepeatingUnit()
{
}

int IndigoRepeatingUnit::getIndex()
{
    return idx;
}

void IndigoRepeatingUnit::remove()
{
    mol.removeSGroup(idx);
}

IndigoRepeatingUnit& IndigoRepeatingUnit::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::REPEATING_UNIT)
        return (IndigoRepeatingUnit&)obj;

    throw IndigoError("%s is not a repeating unit", obj.debugInfo());
}

RepeatingUnit& IndigoRepeatingUnit::get()
{
    return (RepeatingUnit&)mol.sgroups.getSGroup(idx);
}

IndigoRepeatingUnitsIter::IndigoRepeatingUnitsIter(BaseMolecule& molecule, Array<int>&& refs)
    : IndigoObject(REPEATING_UNITS_ITER), _mol(molecule), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoRepeatingUnitsIter::~IndigoRepeatingUnitsIter()
{
}

bool IndigoRepeatingUnitsIter::hasNext()
{
    if (_idx == -1)
        return _refs.size() > 0;
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoRepeatingUnitsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoRepeatingUnit> sgroup = std::make_unique<IndigoRepeatingUnit>(_mol, _refs[_idx]);
    return sgroup.release();
}

IndigoMultipleGroup::IndigoMultipleGroup(BaseMolecule& mol_, int idx_) : IndigoObject(MULTIPLE_GROUP), mol(mol_)
{
    idx = idx_;
}

IndigoMultipleGroup::~IndigoMultipleGroup()
{
}

int IndigoMultipleGroup::getIndex()
{
    return idx;
}

void IndigoMultipleGroup::remove()
{
    mol.removeSGroup(idx);
}

IndigoMultipleGroup& IndigoMultipleGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::MULTIPLE_GROUP)
        return (IndigoMultipleGroup&)obj;

    throw IndigoError("%s is not a multiple group", obj.debugInfo());
}

MultipleGroup& IndigoMultipleGroup::get()
{
    return (MultipleGroup&)mol.sgroups.getSGroup(idx);
}

IndigoMultipleGroupsIter::IndigoMultipleGroupsIter(BaseMolecule& molecule, Array<int>&& refs)
    : IndigoObject(MULTIPLE_GROUPS_ITER), _mol(molecule), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoMultipleGroupsIter::~IndigoMultipleGroupsIter()
{
}

bool IndigoMultipleGroupsIter::hasNext()
{
    if (_idx == -1)
        return _refs.size() > 0;
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoMultipleGroupsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoMultipleGroup> sgroup = std::make_unique<IndigoMultipleGroup>(_mol, _refs[_idx]);
    return sgroup.release();
}

IndigoGenericSGroup::IndigoGenericSGroup(BaseMolecule& mol_, int idx_) : IndigoObject(GENERIC_SGROUP), mol(mol_)
{
    idx = idx_;
}

IndigoGenericSGroup::~IndigoGenericSGroup()
{
}

int IndigoGenericSGroup::getIndex()
{
    return idx;
}

void IndigoGenericSGroup::remove()
{
    mol.removeSGroup(idx);
}

IndigoGenericSGroup& IndigoGenericSGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::GENERIC_SGROUP)
        return (IndigoGenericSGroup&)obj;

    throw IndigoError("%s is not a generic sgroup", obj.debugInfo());
}

SGroup& IndigoGenericSGroup::get()
{
    return (SGroup&)mol.sgroups.getSGroup(idx);
}

IndigoGenericSGroupsIter::IndigoGenericSGroupsIter(BaseMolecule& molecule, Array<int>&& refs)
    : IndigoObject(GENERIC_SGROUPS_ITER), _mol(molecule), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoGenericSGroupsIter::~IndigoGenericSGroupsIter()
{
}

bool IndigoGenericSGroupsIter::hasNext()
{
    if (_idx == -1)
        return _refs.size() > 0;
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoGenericSGroupsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoGenericSGroup> sgroup = std::make_unique<IndigoGenericSGroup>(_mol, _refs[_idx]);
    return sgroup.release();
}

CEXPORT int indigoIterateGenericSGroups(int molecule)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<int>, sgs);
        sgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        mol.sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_GEN, sgs);
        return self.addObject(new IndigoGenericSGroupsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

IndigoSGroup::IndigoSGroup(BaseMolecule& mol_, int sg_idx_) : IndigoObject(SGROUP), mol(mol_)
{
    idx = sg_idx_;
}

IndigoSGroup::~IndigoSGroup()
{
}

const char* IndigoSGroup::debugInfo() const
{
    return "<sgroup>";
}

int IndigoSGroup::getIndex()
{
    return idx;
}

void IndigoSGroup::remove()
{
    mol.removeSGroup(idx);
}

IndigoSGroup& IndigoSGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::SGROUP || obj.type == IndigoObject::DATA_SGROUP || obj.type == IndigoObject::SUPERATOM ||
        obj.type == IndigoObject::REPEATING_UNIT || obj.type == IndigoObject::MULTIPLE_GROUP || obj.type == IndigoObject::GENERIC_SGROUP)
        return (IndigoSGroup&)obj;

    throw IndigoError("%s is not a sgroup", obj.debugInfo());
}

SGroup& IndigoSGroup::get()
{
    return (SGroup&)mol.sgroups.getSGroup(idx);
}

IndigoSGroupsIter::IndigoSGroupsIter(BaseMolecule& molecule, Array<int>&& sg_refs) : IndigoObject(SGROUPS_ITER), _mol(molecule), _refs(std::move(sg_refs))
{
    _idx = -1;
}

IndigoSGroupsIter::~IndigoSGroupsIter()
{
}

const char* IndigoSGroupsIter::debugInfo() const
{
    return "<sgroups iterator>";
}

bool IndigoSGroupsIter::hasNext()
{
    if (_idx == -1)
        return _refs.size() > 0;
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoSGroupsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoSGroup> sgroup = std::make_unique<IndigoSGroup>(_mol, _refs[_idx]);
    return sgroup.release();
}

CEXPORT int indigoIterateSGroups(int molecule)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<int>, sgs);
        sgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        for (auto i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            sgs.push(i);
        }
        return self.addObject(new IndigoSGroupsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

IndigoTGroup::IndigoTGroup(BaseMolecule& mol_, int tg_idx_) : IndigoObject(TGROUP), mol(mol_)
{
    idx = tg_idx_;
}

IndigoTGroup::~IndigoTGroup()
{
}

const char* IndigoTGroup::debugInfo() const
{
    return "<tgroup>";
}

int IndigoTGroup::getIndex()
{
    return idx;
}

void IndigoTGroup::remove()
{
    mol.tgroups.remove(idx);
}

IndigoTGroup& IndigoTGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::TGROUP)
        return (IndigoTGroup&)obj;

    throw IndigoError("%s is not a tgroup", obj.debugInfo());
}

TGroup& IndigoTGroup::get()
{
    return (TGroup&)mol.tgroups.getTGroup(idx);
}

IndigoTGroupsIter::IndigoTGroupsIter(BaseMolecule& molecule) : IndigoObject(TGROUPS_ITER), _mol(molecule)
{
    _idx = -1;
}

IndigoTGroupsIter::~IndigoTGroupsIter()
{
}

const char* IndigoTGroupsIter::debugInfo() const
{
    return "<tgroups iterator>";
}

bool IndigoTGroupsIter::hasNext()
{
    if (_idx == -1)
        return _mol.tgroups.getTGroupCount() > 0;
    return _idx + 1 < _mol.tgroups.getTGroupCount();
}

IndigoObject* IndigoTGroupsIter::next()
{
    if (!hasNext())
        return 0;

    if (_idx == -1)
        _idx = 0;
    else
        _idx++;

    std::unique_ptr<IndigoTGroup> tgroup = std::make_unique<IndigoTGroup>(_mol, _idx);
    return tgroup.release();
}

CEXPORT int indigoIterateTGroups(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        return self.addObject(new IndigoTGroupsIter(mol));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateRepeatingUnits(int molecule)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<int>, sgs);
        sgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        mol.sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_SRU, sgs);
        return self.addObject(new IndigoRepeatingUnitsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateMultipleGroups(int molecule)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<int>, sgs);
        sgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        mol.sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_MUL, sgs);
        return self.addObject(new IndigoMultipleGroupsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateSuperatoms(int molecule)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<int>, sgs);
        sgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        mol.sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_SUP, sgs);
        return self.addObject(new IndigoSuperatomsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSuperatom(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (index < 0 || index >= mol.sgroups.end())
            throw IndigoError("Invalid Sgroup index %d", index);

        SGroup& sg = mol.sgroups.getSGroup(index);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            return self.addObject(new IndigoSuperatom(mol, index));

        throw IndigoError("Sgroup with index %d is not a Superatom", index);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetDataSGroup(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (index < 0 || index >= mol.sgroups.end())
            throw IndigoError("Invalid Sgroup index %d", index);

        SGroup& sg = mol.sgroups.getSGroup(index);
        if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            return self.addObject(new IndigoDataSGroup(mol, index));

        throw IndigoError("Sgroup with index %d is not a DataSGroup", index);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetGenericSGroup(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (index < 0 || index >= mol.sgroups.end())
            throw IndigoError("Invalid Sgroup index %d", index);

        SGroup& sg = mol.sgroups.getSGroup(index);
        if (sg.sgroup_type == SGroup::SG_TYPE_GEN)
            return self.addObject(new IndigoGenericSGroup(mol, index));

        throw IndigoError("Sgroup with index %d is not a GenericSGroup", index);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetMultipleGroup(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (index < 0 || index >= mol.sgroups.end())
            throw IndigoError("Invalid Sgroup index %d", index);

        SGroup& sg = mol.sgroups.getSGroup(index);
        if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
            return self.addObject(new IndigoMultipleGroup(mol, index));

        throw IndigoError("Sgroup with index %d is not a MultipleGroup", index);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetRepeatingUnit(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (index < 0 || index >= mol.sgroups.end())
            throw IndigoError("Invalid Sgroup index %d", index);

        SGroup& sg = mol.sgroups.getSGroup(index);
        if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
            return self.addObject(new IndigoRepeatingUnit(mol, index));

        throw IndigoError("Sgroup with index %d is not a RepeatingUnit", index);
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoDescription(int data_sgroup)
{
    INDIGO_BEGIN
    {
        IndigoDataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(data_sgroup));
        if (dsg.get().name.size() < 1)
            return "";
        return dsg.get().name.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoData(int data_sgroup)
{
    INDIGO_BEGIN
    {
        IndigoDataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(data_sgroup));
        if (dsg.get().data.size() < 1)
            return "";
        return dsg.get().data.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoAddDataSGroup(int molecule, int natoms, int* atoms, int nbonds, int* bonds, const char* name, const char* data)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT);
        DataSGroup& dsg = (DataSGroup&)mol.sgroups.getSGroup(idx);
        int i;
        if (atoms != nullptr)
            dsg.atoms.concat(atoms, natoms);

        if (bonds != nullptr)
            dsg.bonds.concat(bonds, nbonds);

        if (data != nullptr)
            dsg.data.readString(data, true);
        if (name != nullptr)
            dsg.name.readString(name, true);

        return self.addObject(new IndigoDataSGroup(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddSuperatom(int molecule, int natoms, int* atoms, const char* name)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_SUP);
        Superatom& satom = (Superatom&)mol.sgroups.getSGroup(idx);
        satom.subscript.appendString(name, true);
        if (atoms == nullptr)
            throw IndigoError("indigoAddSuperatom(): atoms were not specified");

        else
            satom.atoms.concat(atoms, natoms);

        return self.addObject(new IndigoSuperatom(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetDataSGroupXY(int sgroup, float x, float y, const char* options)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        dsg.display_pos.x = x;
        dsg.display_pos.y = y;
        dsg.detached = true;

        if (options != 0 && options[0] != 0)
        {
            if (strcasecmp(options, "absolute") == 0)
                dsg.relative = false;
            else if (strcasecmp(options, "relative") == 0)
                dsg.relative = true;
            else
                throw IndigoError("indigoSetDataSGroupXY(): invalid options string");
        }

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupData(int sgroup, const char* data)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (data != 0)
            dsg.data.readString(data, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupCoords(int sgroup, float x, float y)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        dsg.display_pos.x = x;
        dsg.display_pos.y = y;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupDescription(int sgroup, const char* description)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (description != 0)
            dsg.description.readString(description, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupFieldName(int sgroup, const char* name)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (name != 0)
            dsg.name.readString(name, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupQueryCode(int sgroup, const char* querycode)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (querycode != 0)
            dsg.querycode.readString(querycode, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupQueryOper(int sgroup, const char* queryoper)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (queryoper != 0)
            dsg.queryoper.readString(queryoper, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupDisplay(int sgroup, const char* option)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (option != 0 && option[0] != 0)
        {
            if (strcasecmp(option, "attached") == 0)
                dsg.detached = false;
            else if (strcasecmp(option, "detached") == 0)
                dsg.detached = true;
            else
                throw IndigoError("indigoSetSgroupDisplay(): invalid option string");
        }

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupLocation(int sgroup, const char* option)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (option != 0 && option[0] != 0)
        {
            if (strcasecmp(option, "absolute") == 0)
                dsg.relative = false;
            else if (strcasecmp(option, "relative") == 0)
                dsg.relative = true;
            else
                throw IndigoError("indigoSetSgroupLocation(): invalid option string");
        }

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupTag(int sgroup, const char* tag)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (tag != 0 && tag[0] != 0)
        {
            dsg.tag = tag[0];
        }

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupTagAlign(int sgroup, int tag_align)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (tag_align > 0 && tag_align < 10)
        {
            dsg.dasp_pos = tag_align;
        }

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupDataType(int sgroup, const char* data_type)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        if (data_type != 0 && data_type[0] != 0)
        {
            dsg.type.readString(data_type, true);
        }

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupXCoord(int sgroup, float x)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        dsg.display_pos.x = x;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupYCoord(int sgroup, float y)
{
    INDIGO_BEGIN
    {
        DataSGroup& dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

        dsg.display_pos.y = y;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCreateSGroup(const char* type, int mapping, const char* name)
{
    INDIGO_BEGIN
    {
        IndigoMapping& map = IndigoMapping::cast(self.getObject(mapping));
        BaseMolecule& mol = map.to;
        BaseMolecule& temp = map.from;
        Array<int>& m = map.mapping;
        int idx = mol.sgroups.addSGroup(type);
        if (idx != -1)
        {
            SGroup& sgroup = mol.sgroups.getSGroup(idx);

            for (auto i : temp.vertices())
            {
                sgroup.atoms.push(m[i]);
            }

            for (auto i : mol.edges())
            {
                const Edge& edge = mol.getEdge(i);
                if (((sgroup.atoms.find(edge.beg) != -1) && (sgroup.atoms.find(edge.end) == -1)) ||
                    ((sgroup.atoms.find(edge.end) != -1) && (sgroup.atoms.find(edge.beg) == -1)))
                {
                    sgroup.bonds.push(i);
                }
            }

            if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& sa = (Superatom&)sgroup;
                sa.subscript.appendString(name, true);
                return self.addObject(new IndigoSuperatom(mol, idx));
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_SRU)
            {
                RepeatingUnit& ru = (RepeatingUnit&)sgroup;
                ru.subscript.appendString(name, true);
                return self.addObject(new IndigoRepeatingUnit(mol, idx));
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
            {
                return self.addObject(new IndigoMultipleGroup(mol, idx));
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                return self.addObject(new IndigoDataSGroup(mol, idx));
            }
            else
            {
                return self.addObject(new IndigoGenericSGroup(mol, idx));
            }
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupClass(int sgroup, const char* sgclass)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        sup.sa_class.readString(sgclass, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetSGroupClass(int sgroup)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        if (sup.sa_class.size() < 1)
            return "";
        return sup.sa_class.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoSetSGroupName(int sgroup, const char* sgname)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        sup.subscript.readString(sgname, true);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetSGroupName(int sgroup)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        if (sup.subscript.size() < 1)
            return "";
        return sup.subscript.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoGetSGroupNumCrossBonds(int sgroup)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        return sup.bonds.size();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddSGroupAttachmentPoint(int sgroup, int aidx, int lvidx, const char* apid)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        int ap_idx = sup.attachment_points.add();
        Superatom::_AttachmentPoint& ap = sup.attachment_points.at(ap_idx);
        ap.aidx = aidx;
        ap.lvidx = lvidx;
        ap.apid.readString(apid, true);
        return ap_idx;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoDeleteSGroupAttachmentPoint(int sgroup, int ap_idx)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        sup.attachment_points.remove(ap_idx);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSGroupDisplayOption(int sgroup)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        if (sup.contracted > DisplayOption::Undefined)
            return (int)sup.contracted;

        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupDisplayOption(int sgroup, int option)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        sup.contracted = (DisplayOption)option;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSGroupSeqId(int sgroup)
{
    INDIGO_BEGIN
    {
        Superatom& sup = IndigoSuperatom::cast(self.getObject(sgroup)).get();
        if (sup.seqid != -1)
            return sup.seqid;
        return 0;
    }
    INDIGO_END(0);
}

CEXPORT float* indigoGetSGroupCoords(int sgroup)
{
    INDIGO_BEGIN
    {
        IndigoDataSGroup& ds = IndigoDataSGroup::cast(self.getObject(sgroup));

        auto& tmp = self.getThreadTmpData();
        auto& xy = ds.get().display_pos;
        tmp.xyz[0] = xy.x;
        tmp.xyz[1] = xy.y;
        tmp.xyz[2] = 0.f;
        return tmp.xyz;
    }
    INDIGO_END(0);
}

CEXPORT int indigoGetSGroupMultiplier(int sgroup)
{
    INDIGO_BEGIN
    {
        MultipleGroup& mg = IndigoMultipleGroup::cast(self.getObject(sgroup)).get();
        return mg.multiplier;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetRepeatingUnitSubscript(int sgroup)
{
    INDIGO_BEGIN
    {
        RepeatingUnit& ru = IndigoRepeatingUnit::cast(self.getObject(sgroup)).get();
        return ru.subscript.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoGetRepeatingUnitConnectivity(int sgroup)
{
    INDIGO_BEGIN
    {
        RepeatingUnit& ru = IndigoRepeatingUnit::cast(self.getObject(sgroup)).get();
        return ru.connectivity;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupMultiplier(int sgroup, int multiplier)
{
    INDIGO_BEGIN
    {
        MultipleGroup& mg = IndigoMultipleGroup::cast(self.getObject(sgroup)).get();
        mg.multiplier = multiplier;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupBrackets(int sgroup, int brk_style, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    INDIGO_BEGIN
    {
        SGroup* psg = 0;

        if (self.getObject(sgroup).type == IndigoObject::GENERIC_SGROUP)
            psg = &(IndigoGenericSGroup::cast(self.getObject(sgroup)).get());
        else if (self.getObject(sgroup).type == IndigoObject::REPEATING_UNIT)
            psg = &(IndigoRepeatingUnit::cast(self.getObject(sgroup)).get());
        else if (self.getObject(sgroup).type == IndigoObject::MULTIPLE_GROUP)
            psg = &(IndigoMultipleGroup::cast(self.getObject(sgroup)).get());
        else
            throw IndigoError("indigoSetSgroupBrackets(): brackets properties are not supported for this Sgroup type");

        psg->brk_style = brk_style;
        psg->brackets.clear();
        Vec2f* brackets = psg->brackets.push();
        brackets[0].set(x1, y1);
        brackets[1].set(x2, y2);
        brackets = psg->brackets.push();
        brackets[0].set(x3, y3);
        brackets[1].set(x4, y4);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoFindSGroups(int item, const char* property, const char* value)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(item).getBaseMolecule();
        QS_DEF(Array<int>, sgs);
        sgs.clear();

        mol.sgroups.findSGroups(property, value, sgs);

        return self.addObject(new IndigoSGroupsIter(mol, std::move(sgs)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSGroupType(int sgroup)
{
    INDIGO_BEGIN
    {
        IndigoSGroup& sg = IndigoSGroup::cast(self.getObject(sgroup));
        return sg.get().sgroup_type;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSGroupIndex(int sgroup)
{
    INDIGO_BEGIN
    {
        IndigoSGroup& sg = IndigoSGroup::cast(self.getObject(sgroup));
        return sg.idx;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSGroupOriginalId(int sgroup)
{
    INDIGO_BEGIN
    {
        IndigoSGroup& sg = IndigoSGroup::cast(self.getObject(sgroup));
        return sg.get().original_group;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupOriginalId(int sgroup, int new_original)
{
    INDIGO_BEGIN
    {
        IndigoSGroup& sgr = IndigoSGroup::cast(self.getObject(sgroup));

        for (auto i = sgr.mol.sgroups.begin(); i != sgr.mol.sgroups.end(); i = sgr.mol.sgroups.next(i))
        {
            SGroup& sg = sgr.mol.sgroups.getSGroup(i);
            if (sg.original_group == new_original && i != sgr.idx)
                throw IndigoError("indigoSetSGroupOriginalId: duplicated sgroup id %d )", new_original);
        }

        int old_original = sgr.get().original_group;
        if (old_original > 0)
        {
            for (auto i = sgr.mol.sgroups.begin(); i != sgr.mol.sgroups.end(); i = sgr.mol.sgroups.next(i))
            {
                SGroup& sg = sgr.mol.sgroups.getSGroup(i);
                if (sg.parent_group == old_original)
                    sg.parent_group = new_original;
            }
        }
        sgr.get().original_group = new_original;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetSGroupParentId(int sgroup)
{
    INDIGO_BEGIN
    {
        IndigoSGroup& sg = IndigoSGroup::cast(self.getObject(sgroup));
        return sg.get().parent_group;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetSGroupParentId(int sgroup, int parent)
{
    INDIGO_BEGIN
    {
        IndigoSGroup& sgr = IndigoSGroup::cast(self.getObject(sgroup));

        bool original_found = false;
        for (auto i = sgr.mol.sgroups.begin(); i != sgr.mol.sgroups.end(); i = sgr.mol.sgroups.next(i))
        {
            SGroup& sg = sgr.mol.sgroups.getSGroup(i);
            if (sg.original_group == parent)
                original_found = true;
        }
        if (!original_found)
            throw IndigoError("indigoSetSGroupParentId: sgroup with original id %d is not found)", parent);

        sgr.get().parent_group = parent;

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddTemplate(int molecule, int templates, const char* tname)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        BaseMolecule& temp = self.getObject(templates).getBaseMolecule();
        int tgidx = temp.tgroups.findTGroup(tname);
        if (tgidx != -1)
        {
            TGroup& tg = temp.tgroups.getTGroup(tgidx);
            int idx = mol.addTemplate(tg);
            return idx + 1;
        }
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRemoveTemplate(int molecule, const char* tname)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int tgidx = mol.tgroups.findTGroup(tname);
        if (tgidx != -1)
        {
            mol.tgroups.remove(tgidx);
        }
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoFindTemplate(int molecule, const char* tname)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int tgidx = mol.tgroups.findTGroup(tname);
        if (tgidx != -1)
        {
            return tgidx + 1;
        }
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetTGroupClass(int tgroup)
{
    INDIGO_BEGIN
    {
        TGroup& tg = IndigoTGroup::cast(self.getObject(tgroup)).get();
        if (tg.tgroup_class.size() < 1)
            return "";
        return tg.tgroup_class.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoGetTGroupName(int tgroup)
{
    INDIGO_BEGIN
    {
        TGroup& tg = IndigoTGroup::cast(self.getObject(tgroup)).get();
        if (tg.tgroup_name.size() < 1)
            return "";
        return tg.tgroup_name.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoGetTGroupAlias(int tgroup)
{
    INDIGO_BEGIN
    {
        TGroup& tg = IndigoTGroup::cast(self.getObject(tgroup)).get();
        if (tg.tgroup_alias.size() < 1)
            return "";
        return tg.tgroup_alias.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoTransformSCSRtoCTAB(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        mol.transformSCSRtoFullCTAB();

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoTransformCTABtoSCSR(int molecule, int templates)
{
    INDIGO_BEGIN
    {
        QS_DEF(ObjArray<TGroup>, tgs);
        tgs.clear();
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        BaseMolecule& temp = self.getObject(templates).getBaseMolecule();
        for (auto i = temp.tgroups.begin(); i != temp.tgroups.end(); i = temp.tgroups.next(i))
        {
            TGroup& tg = tgs.push();
            tg.copy(temp.tgroups.getTGroup(i));
        }

        mol.ignore_chem_templates = self.scsr_ignore_chem_templates;
        mol.transformFullCTABtoSCSR(tgs);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountHeavyAtoms(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        int i, cnt = 0;

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            if (!mol.possibleAtomNumber(i, ELEM_H))
                cnt++;

        return cnt;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountComponents(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        return bm.countComponents();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCloneComponent(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();
        if (index < 0 || index >= bm.countComponents())
            throw IndigoError("indigoCloneComponent(): bad index %d (0-%d allowed)", index, bm.countComponents() - 1);

        Filter filter(bm.getDecomposition().ptr(), Filter::EQ, index);
        std::unique_ptr<IndigoMolecule> im = std::make_unique<IndigoMolecule>();
        im->mol.makeSubmolecule(bm, filter, 0, 0);
        return self.addObject(im.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoComponentIndex(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        return ia.mol.vertexComponent(ia.idx);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoComponent(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        if (index < 0 || index >= bm.countComponents())
            throw IndigoError("indigoComponent(): bad index %d (0-%d allowed)", index, bm.countComponents() - 1);

        return self.addObject(new IndigoMoleculeComponent(bm, index));
    }
    INDIGO_END(-1);
}

IndigoComponentAtomsIter::IndigoComponentAtomsIter(BaseMolecule& mol, int cidx) : IndigoObject(COMPONENT_ATOMS_ITER), _mol(mol)
{
    if (cidx < 0 || cidx >= mol.countComponents())
        throw IndigoError("%d is not a valid component number (0-%d allowed)", cidx, _mol.countComponents() - 1);
    _idx = -1;
    _cidx = cidx;
}

IndigoComponentAtomsIter::~IndigoComponentAtomsIter()
{
}

bool IndigoComponentAtomsIter::hasNext()
{
    return _next() != _mol.vertexEnd();
}

IndigoObject* IndigoComponentAtomsIter::next()
{
    int idx = _next();

    if (idx == _mol.vertexEnd())
        return 0;
    _idx = idx;
    return new IndigoAtom(_mol, idx);
}

int IndigoComponentAtomsIter::_next()
{
    int idx;

    if (_idx == -1)
        idx = _mol.vertexBegin();
    else
        idx = _mol.vertexNext(_idx);

    for (; idx != _mol.vertexEnd(); idx = _mol.vertexNext(idx))
        if (_mol.vertexComponent(idx) == _cidx)
            break;
    return idx;
}

IndigoComponentBondsIter::IndigoComponentBondsIter(BaseMolecule& mol, int cidx) : IndigoObject(COMPONENT_BONDS_ITER), _mol(mol)
{
    if (cidx < 0 || cidx >= _mol.countComponents())
        throw IndigoError("%d is not a valid component number (0-%d allowed)", cidx, _mol.countComponents() - 1);
    _idx = -1;
    _cidx = cidx;
}

IndigoComponentBondsIter::~IndigoComponentBondsIter()
{
}

bool IndigoComponentBondsIter::hasNext()
{
    return _next() != _mol.edgeEnd();
}

IndigoObject* IndigoComponentBondsIter::next()
{
    int idx = _next();

    if (idx == _mol.edgeEnd())
        return 0;
    _idx = idx;
    return new IndigoBond(_mol, idx);
}

int IndigoComponentBondsIter::_next()
{
    int idx;

    if (_idx == -1)
        idx = _mol.edgeBegin();
    else
        idx = _mol.edgeNext(_idx);

    for (; idx != _mol.edgeEnd(); idx = _mol.edgeNext(idx))
    {
        const Edge& edge = _mol.getEdge(idx);

        int comp = _mol.vertexComponent(edge.beg);

        if (comp != _mol.vertexComponent(edge.end))
            throw IndigoError("internal: edge ends belong to different components");

        if (comp == _cidx)
            break;
    }
    return idx;
}

CEXPORT int indigoIterateComponents(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoComponentsIter(bm));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateComponentAtoms(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoComponentAtomsIter(bm, index));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateComponentBonds(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoComponentBondsIter(bm, index));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountComponentAtoms(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        return bm.countComponentVertices(index);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountComponentBonds(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& bm = self.getObject(molecule).getBaseMolecule();

        return bm.countComponentEdges(index);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCreateMolecule()
{
    INDIGO_BEGIN
    {
        std::unique_ptr<IndigoMolecule> obj = std::make_unique<IndigoMolecule>();
        return self.addObject(obj.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCreateQueryMolecule()
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoQueryMolecule());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoMerge(int where, int what)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol_where = self.getObject(where).getBaseMolecule();
        BaseMolecule& mol_what = self.getObject(what).getBaseMolecule();

        std::unique_ptr<IndigoMapping> res = std::make_unique<IndigoMapping>(mol_what, mol_where);

        mol_where.mergeWithMolecule(mol_what, &res->mapping, 0);

        return self.addObject(res.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddAtom(int molecule, const char* symbol)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);
        BaseMolecule& bmol = obj.getBaseMolecule();

        int idx;
        if (bmol.isQueryMolecule())
        {
            QueryMolecule& qmol = bmol.asQueryMolecule();
            idx = qmol.addAtom(IndigoQueryMolecule::parseAtomSMARTS(symbol));
        }
        else
        {
            Molecule& mol = bmol.asMolecule();
            int elem = Element::fromString2(symbol);

            if (elem > 0)
                idx = mol.addAtom(elem);
            else
            {
                idx = mol.addAtom(ELEM_PSEUDO);
                mol.setPseudoAtom(idx, symbol);
            }
        }

        return self.addObject(new IndigoAtom(bmol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetAtom(int atom, const char* symbol)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        BaseMolecule& bmol = ia.mol;

        if (bmol.isQueryMolecule())
        {
            QueryMolecule& qmol = bmol.asQueryMolecule();
            qmol.resetAtom(ia.idx, IndigoQueryMolecule::parseAtomSMARTS(symbol));
        }
        else
        {
            Molecule& mol = ia.mol.asMolecule();

            int elem = Element::fromString2(symbol);

            if (elem > 0)
                mol.resetAtom(ia.idx, elem);
            else if (!mol.isTemplateAtom(ia.idx))
            {
                mol.resetAtom(ia.idx, ELEM_PSEUDO);
                mol.setPseudoAtom(ia.idx, symbol);
            }
            else
            {
                mol.setTemplateAtomName(ia.idx, symbol);
            }
        }
        bmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ATOM_NUMBER);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetTemplateAtomClass(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        BaseMolecule& bmol = ia.mol;
        Molecule& mol = ia.mol.asMolecule();

        if (mol.isTemplateAtom(ia.idx))
        {
            return ia.mol.getTemplateAtomClass(ia.idx);
        }
        else
            throw IndigoError("indigoGetTemplateAtomClass(): atom %d is not template atom", ia.idx);

        return "";
    }
    INDIGO_END(0);
}

CEXPORT int indigoSetTemplateAtomClass(int atom, const char* name)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        BaseMolecule& bmol = ia.mol;
        Molecule& mol = ia.mol.asMolecule();

        if (mol.isTemplateAtom(ia.idx))
        {
            mol.setTemplateAtomClass(ia.idx, name);
        }
        else
            throw IndigoError("indigoSetTemplateAtomClass(): atom %d is not template atom", ia.idx);

        return 1;
    }
    INDIGO_END(-1);
}

static void _parseRSites(const char* name, Array<int>& rsites)
{
    BufferScanner scanner(name);
    rsites.clear();
    while (!scanner.isEOF())
    {
        scanner.skipSpace();
        if (scanner.lookNext() != 'R')
            throw IndigoError("indigoAddRSite(): cannot parse '%s' as r-site name(s)", name);
        scanner.readChar();
        if (scanner.isEOF())
            break;
        if (isdigit(scanner.lookNext()))
        {
            int idx = scanner.readInt();
            rsites.push(idx);
        }

        scanner.skipSpace();
        if (scanner.lookNext() == ',' || scanner.lookNext() == ';')
            scanner.readChar();
    }
}

static void _indigoSetRSite(Molecule& mol, int atom_index, const char* name)
{
    // Parse r-sites
    QS_DEF(Array<int>, rsites);
    _parseRSites(name, rsites);
    mol.resetAtom(atom_index, ELEM_RSITE);
    mol.setRSiteBits(atom_index, 0);
    for (int i = 0; i < rsites.size(); i++)
        mol.allowRGroupOnRSite(atom_index, rsites[i]);
}

CEXPORT int indigoAddRSite(int molecule, const char* name)
{
    INDIGO_BEGIN
    {
        Molecule& mol = self.getObject(molecule).getMolecule();

        int idx = mol.addAtom(ELEM_RSITE);
        try
        {
            _indigoSetRSite(mol, idx, name);
        }
        catch (...)
        {
            // Remove atom if there is an exception (r-site index is very big for example)
            mol.removeAtom(idx);
            throw;
        }

        return self.addObject(new IndigoAtom(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetRSite(int atom, const char* name)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        Molecule& mol = ia.mol.asMolecule();

        _indigoSetRSite(mol, ia.idx, name);

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetCharge(int atom, int charge)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        ia.mol.asMolecule().setAtomCharge(ia.idx, charge);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetIsotope(int atom, int isotope)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        ia.mol.asMolecule().setAtomIsotope(ia.idx, isotope);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetImplicitHCount(int atom, int impl_h)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        ia.mol.asMolecule().setImplicitH(ia.idx, impl_h);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddBond(int source, int destination, int order)
{
    INDIGO_BEGIN
    {
        IndigoAtom& s_atom = IndigoAtom::cast(self.getObject(source));
        IndigoAtom& d_atom = IndigoAtom::cast(self.getObject(destination));

        if (&s_atom.mol != &d_atom.mol)
            throw IndigoError("indigoAddBond(): molecules do not match");

        int idx;
        if (s_atom.mol.isQueryMolecule())
            idx = s_atom.mol.asQueryMolecule().addBond(s_atom.idx, d_atom.idx, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order));
        else
            idx = s_atom.mol.asMolecule().addBond(s_atom.idx, d_atom.idx, order);

        return self.addObject(new IndigoBond(s_atom.mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetBondOrder(int bond, int order)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(bond));

        ib.mol.asMolecule().setBondOrder(ib.idx, order, false);
        return 1;
    }
    INDIGO_END(-1);
}

IndigoSubmolecule::IndigoSubmolecule(BaseMolecule& mol_, Array<int>& vertices_, Array<int>& edges_) : IndigoObject(SUBMOLECULE), _mol(mol_)
{
    vertices.copy(vertices_);
    edges.copy(edges_);
    idx = -1;
}

IndigoSubmolecule::IndigoSubmolecule(BaseMolecule& mol_, List<int>& vertices_, List<int>& edges_) : IndigoObject(SUBMOLECULE), _mol(mol_)
{
    int i;

    vertices.clear();
    edges.clear();

    for (i = vertices_.begin(); i != vertices_.end(); i = vertices_.next(i))
        vertices.push(vertices_[i]);

    for (i = edges_.begin(); i != edges_.end(); i = edges_.next(i))
        edges.push(edges_[i]);

    idx = -1;
}

IndigoSubmolecule::~IndigoSubmolecule()
{
}

void IndigoSubmolecule::_createSubMolecule()
{
    if (_submol.get() != 0 && _submol_revision == _mol.getEditRevision())
    {
        return;
    }
    if (_mol.isQueryMolecule())
    {
        _submol = std::make_unique<QueryMolecule>();
    }
    else
    {
        _submol = std::make_unique<Molecule>();
    }
    _submol->makeEdgeSubmolecule(_mol, vertices, edges, 0, 0);
    _submol_revision = _mol.getEditRevision();
}

BaseMolecule& IndigoSubmolecule::getBaseMolecule()
{
    _createSubMolecule();
    return *_submol;
}

int IndigoSubmolecule::getIndex()
{
    if (idx == -1)
        throw IndigoError("index not set");

    return idx;
}

IndigoObject* IndigoSubmolecule::clone()
{
    std::unique_ptr<IndigoObject> res;
    BaseMolecule* newmol;

    if (_mol.isQueryMolecule())
    {
        res = std::make_unique<IndigoQueryMolecule>();
        newmol = &(((IndigoQueryMolecule*)res.get())->qmol);
    }
    else
    {
        res = std::make_unique<IndigoMolecule>();
        newmol = &(((IndigoMolecule*)res.get())->mol);
    }

    newmol->makeEdgeSubmolecule(_mol, vertices, edges, 0, 0);
    return res.release();
}

IndigoSubmoleculeAtomsIter::IndigoSubmoleculeAtomsIter(IndigoSubmolecule& submol) : IndigoObject(SUBMOLECULE_ATOMS_ITER), _submol(submol)
{
    _idx = -1;
}

IndigoSubmoleculeAtomsIter::~IndigoSubmoleculeAtomsIter()
{
}

bool IndigoSubmoleculeAtomsIter::hasNext()
{
    return _idx + 1 < _submol.vertices.size();
}

IndigoObject* IndigoSubmoleculeAtomsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;

    return new IndigoAtom(_submol.getOriginalMolecule(), _submol.vertices[_idx]);
}

IndigoSubmoleculeBondsIter::IndigoSubmoleculeBondsIter(IndigoSubmolecule& submol) : IndigoObject(SUBMOLECULE_BONDS_ITER), _submol(submol)
{
    _idx = -1;
}

IndigoSubmoleculeBondsIter::~IndigoSubmoleculeBondsIter()
{
}

bool IndigoSubmoleculeBondsIter::hasNext()
{
    return _idx + 1 < _submol.edges.size();
}

IndigoObject* IndigoSubmoleculeBondsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;

    return new IndigoBond(_submol.getOriginalMolecule(), _submol.edges[_idx]);
}

CEXPORT int indigoCountSSSR(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return mol.sssrCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateSSSR(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoSSSRIter(mol));
    }
    INDIGO_END(-1);
}

IndigoSSSRIter::IndigoSSSRIter(BaseMolecule& mol) : IndigoObject(SSSR_ITER), _mol(mol)
{
    _idx = -1;
}

IndigoSSSRIter::~IndigoSSSRIter()
{
}

bool IndigoSSSRIter::hasNext()
{
    return _idx + 1 < _mol.sssrCount();
}

IndigoObject* IndigoSSSRIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    List<int>& vertices = _mol.sssrVertices(_idx);
    List<int>& edges = _mol.sssrEdges(_idx);

    std::unique_ptr<IndigoSubmolecule> res = std::make_unique<IndigoSubmolecule>(_mol, vertices, edges);
    res->idx = _idx;
    return res.release();
}

IndigoSubtreesIter::IndigoSubtreesIter(BaseMolecule& mol, int min_vertices, int max_vertices) : IndigoObject(SUBTREES_ITER), _mol(mol), _enumerator(mol)
{
    _enumerator.min_vertices = min_vertices;
    _enumerator.max_vertices = max_vertices;
    _enumerator.context = this;
    _enumerator.callback = _handleTree;
    _enumerator.process();
    _idx = -1;
}

IndigoSubtreesIter::~IndigoSubtreesIter()
{
}

void IndigoSubtreesIter::_handleTree(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context)
{
    IndigoSubtreesIter* self = (IndigoSubtreesIter*)context;

    Array<int>& self_vertices = self->_vertices.push();
    Array<int>& self_edges = self->_edges.push();
    self_vertices.copy(vertices);
    self_edges.copy(edges);
}

bool IndigoSubtreesIter::hasNext()
{
    return _idx + 1 < _vertices.size();
}

IndigoObject* IndigoSubtreesIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    std::unique_ptr<IndigoSubmolecule> res = std::make_unique<IndigoSubmolecule>(_mol, _vertices[_idx], _edges[_idx]);
    res->idx = _idx;
    return res.release();
}

CEXPORT int indigoIterateSubtrees(int molecule, int min_atoms, int max_atoms)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoSubtreesIter(mol, min_atoms, max_atoms));
    }
    INDIGO_END(-1);
}

IndigoRingsIter::IndigoRingsIter(BaseMolecule& mol, int min_vertices, int max_vertices) : IndigoObject(RINGS_ITER), _mol(mol), _enumerator(mol)
{
    _enumerator.min_length = min_vertices;
    _enumerator.max_length = max_vertices;
    _enumerator.context = this;
    _enumerator.cb_handle_cycle = _handleCycle;
    _enumerator.process();
    _idx = -1;
}

IndigoRingsIter::~IndigoRingsIter()
{
}

bool IndigoRingsIter::_handleCycle(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context)
{
    IndigoRingsIter* self = (IndigoRingsIter*)context;

    self->_vertices.push().copy(vertices);
    self->_edges.push().copy(edges);
    return true;
}

bool IndigoRingsIter::hasNext()
{
    return _idx + 1 < _vertices.size();
}

IndigoObject* IndigoRingsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    std::unique_ptr<IndigoSubmolecule> res = std::make_unique<IndigoSubmolecule>(_mol, _vertices[_idx], _edges[_idx]);
    res->idx = _idx;
    return res.release();
}

CEXPORT int indigoIterateRings(int molecule, int min_atoms, int max_atoms)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoRingsIter(mol, min_atoms, max_atoms));
    }
    INDIGO_END(-1);
}

IndigoEdgeSubmoleculeIter::IndigoEdgeSubmoleculeIter(BaseMolecule& mol, int min_edges, int max_edges)
    : IndigoObject(EDGE_SUBMOLECULE_ITER), _mol(mol), _enumerator(mol)
{
    _enumerator.min_edges = min_edges;
    _enumerator.max_edges = max_edges;
    _enumerator.userdata = this;
    _enumerator.cb_subgraph = _handleSubgraph;
    _enumerator.process();
    _idx = -1;
}

IndigoEdgeSubmoleculeIter::~IndigoEdgeSubmoleculeIter()
{
}

void IndigoEdgeSubmoleculeIter::_handleSubgraph(Graph& graph, const int* v_mapping, const int* e_mapping, void* context)
{
    IndigoEdgeSubmoleculeIter* self = (IndigoEdgeSubmoleculeIter*)context;

    Array<int>& vertices = self->_vertices.push();
    Array<int>& edges = self->_edges.push();

    Graph::filterVertices(graph, v_mapping, FILTER_NEQ, -1, vertices);
    Graph::filterEdges(graph, e_mapping, FILTER_NEQ, -1, edges);
}

bool IndigoEdgeSubmoleculeIter::hasNext()
{
    return _idx + 1 < _vertices.size();
}

IndigoObject* IndigoEdgeSubmoleculeIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    std::unique_ptr<IndigoSubmolecule> res = std::make_unique<IndigoSubmolecule>(_mol, _vertices[_idx], _edges[_idx]);
    res->idx = _idx;
    return res.release();
}

CEXPORT int indigoIterateEdgeSubmolecules(int molecule, int min_bonds, int max_bonds)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoEdgeSubmoleculeIter(mol, min_bonds, max_bonds));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountHydrogens(int item, int* hydro)
{
    INDIGO_BEGIN
    {
        if (hydro == 0)
            throw IndigoError("indigoCountHydrogens(): null pointer");

        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            int res = ia.mol.getAtomTotalH(ia.idx);

            if (res == -1)
                return 0;

            *hydro = res;
        }
        else if (IndigoBaseMolecule::is(obj))
        {
            Molecule& mol = obj.getMolecule();
            *hydro = 0;

            for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            {
                if (mol.getAtomNumber(i) == ELEM_H)
                    (*hydro)++;
                else if (!mol.isPseudoAtom(i) && !mol.isRSite(i))
                    (*hydro) += mol.getImplicitH(i);
            }
        }
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountImplicitHydrogens(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            return ia.mol.asMolecule().getImplicitH(ia.idx);
        }
        else if (IndigoBaseMolecule::is(obj))
        {
            Molecule& mol = obj.getMolecule();
            int i, sum = 0;

            for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
                sum += mol.getImplicitH(i);
            return sum;
        }
        else
            throw IndigoError("indigoCountImplicitHydrogens: %s is not a molecule nor an atom", obj.debugInfo());
    }
    INDIGO_END(-1);
}

//
// IndigoAttachmentPointsIter
//

IndigoAttachmentPointsIter::IndigoAttachmentPointsIter(BaseMolecule& mol, int order) : IndigoObject(ATTACHMENT_POINTS_ITER), _mol(mol)
{
    _order = order;
    _index = -1;
}

IndigoObject* IndigoAttachmentPointsIter::next()
{
    if (!hasNext())
        return 0;
    _index++;
    int atom_index = _mol.getAttachmentPoint(_order, _index);
    if (atom_index == -1)
        throw IndigoError("Internal error in IndigoAttachmentPointsIter::next");
    return new IndigoAtom(_mol, atom_index);
}

bool IndigoAttachmentPointsIter::hasNext()
{
    return _mol.getAttachmentPoint(_order, _index + 1) != -1;
}

CEXPORT int indigoIterateAttachmentPoints(int molecule, int order)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return self.addObject(new IndigoAttachmentPointsIter(mol, order));
    }
    INDIGO_END(-1);
}

/*
Converts a chemical name into a corresponding structure
Returns -1 if parsing fails or no structure is found
Parameters:
name - a name to parse
params - a string containing parsing options or nullptr if no options are changed
*/
CEXPORT int indigoNameToStructure(const char* name, const char* params)
{
    INDIGO_BEGIN
    {
        if (name == nullptr)
        {
            throw IndigoError("indigoNameToStructure: invalid parameter");
        }

        MoleculeNameParser parser;
        if (params)
        {
            /*
            Duplicate params string as we call destructive function strtok() on callee side
            We can get rid of it if we have sustainable options in the future
            */
            char* options = ::strdup(params);
            if (options)
            {
                parser.setOptions(options);
                ::free(options);
            }
        }

        std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

        Molecule& mol = molptr->mol;
        parser.parseMolecule(name, mol);

        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCheckRGroups(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            if (indigoCountRSites(item) || indigoCountAttachmentPoints(item) || indigoCountRGroups(item))
            {
                return 1;
            }
        }
        else
        {
            throw IndigoError("%s is not a base molecule", obj.debugInfo());
        }
        return 0;
    }
    INDIGO_END(-1);
}
