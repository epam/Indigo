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
#include "indigo_ket_document.h"
#include "indigo_mapping.h"
#include "indigo_monomer_library.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/elements.h"
#include "molecule/hybridization.h"
#include "molecule/ket_document_json_loader.h"
#include "molecule/mm_expand.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molecule_mass.h"
#include "molecule/molecule_name_parser.h"
#include "molecule/molecule_savers.h"
#include "molecule/molfile_loader.h"
#include "molecule/query_molecule.h"
#include "molecule/sdf_loader.h"
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
    return indigoLoadMoleculeWithLib(source, -1);
}

CEXPORT int indigoLoadMoleculeWithLib(int source, int monomer_library)
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
        loader.smiles_loading_strict_aliphatic = self.smiles_loading_strict_aliphatic;
        loader.dearomatize_on_load = self.dearomatize_on_load;
        loader.arom_options = self.arom_options;

        std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();

        Molecule& mol = molptr->mol;

        MonomerTemplateLibrary* monomer_lib = nullptr;
        if (monomer_library >= 0)
            monomer_lib = &IndigoMonomerLibrary::get(self.getObject(monomer_library));
        loader.loadMolecule(mol, monomer_lib);
        molptr->getProperties().copy(loader.properties);

        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadMoleculeWithLibFromString(const char* string, int monomer_library)
{
    int source = indigoReadString(string);
    int result;

    if (source <= 0)
        return -1;

    result = indigoLoadMoleculeWithLib(source, monomer_library);
    indigoFree(source);
    return result;
}

CEXPORT int indigoLoadMoleculeWithLibFromFile(const char* filename, int monomer_library)
{
    int source = indigoReadFile(filename);
    int result;

    if (source < 0)
        return -1;

    result = indigoLoadMoleculeWithLib(source, monomer_library);
    indigoFree(source);
    return result;
}

CEXPORT int indigoLoadMoleculeWithLibFromBuffer(const char* buffer, int size, int monomer_library)
{
    int source = indigoReadBuffer(buffer, size);
    int result;

    if (source < 0)
        return -1;

    result = indigoLoadMoleculeWithLib(source, monomer_library);
    indigoFree(source);
    return result;
}

CEXPORT int indigoLoadQueryMolecule(int source)
{
    return indigoLoadQueryMoleculeWithLib(source, -1);
}

CEXPORT int indigoLoadQueryMoleculeWithLibFromString(const char* string, int monomer_library)
{
    int source = indigoReadString(string);
    int result;

    if (source <= 0)
        return -1;

    result = indigoLoadQueryMoleculeWithLib(source, monomer_library);
    indigoFree(source);
    return result;
}

CEXPORT int indigoLoadQueryMoleculeWithLibFromFile(const char* filename, int monomer_library)
{
    int source = indigoReadFile(filename);
    int result;

    if (source < 0)
        return -1;

    result = indigoLoadQueryMoleculeWithLib(source, monomer_library);
    indigoFree(source);
    return result;
}

CEXPORT int indigoLoadQueryMoleculeWithLibFromBuffer(const char* buffer, int size, int monomer_library)
{
    int source = indigoReadBuffer(buffer, size);
    int result;

    if (source < 0)
        return -1;

    result = indigoLoadQueryMoleculeWithLib(source, monomer_library);
    indigoFree(source);
    return result;
}

CEXPORT int indigoLoadQueryMoleculeWithLib(int source, int monomer_library)
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

        MonomerTemplateLibrary* monomer_lib = nullptr;
        if (monomer_library >= 0)
            monomer_lib = &IndigoMonomerLibrary::get(self.getObject(monomer_library));
        loader.loadMolecule(qmol, monomer_lib);
        molptr->copyProperties(loader.properties);

        return self.addObject(molptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadMonomerLibrary(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        std::unique_ptr<IndigoMonomerLibrary> libptr = std::make_unique<IndigoMonomerLibrary>();
        try // try to load as json
        {
            MoleculeJsonLoader loader(IndigoScanner::get(obj));
            loader.stereochemistry_options.ignore_errors = true;
            loader.loadMonomerLibrary(libptr->get());
        }
        catch (Exception& e) // trying as SDF
        {
            try
            {
                SdfLoader sdf_loader(IndigoScanner::get(obj));
                PropertiesMap properties;
                while (!sdf_loader.isEOF())
                {
                    sdf_loader.readNext();
                    // Copy properties
                    properties.copy(sdf_loader.properties);
                    BufferScanner scanner2(sdf_loader.data);
                    MolfileLoader loader(scanner2, &libptr->get());
                    loader.stereochemistry_options = self.stereochemistry_options;
                    loader.ignore_noncritical_query_features = self.ignore_noncritical_query_features;
                    loader.skip_3d_chirality = self.skip_3d_chirality;
                    loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
                    loader.ignore_no_chiral_flag = self.ignore_no_chiral_flag;
                    loader.treat_stereo_as = self.treat_stereo_as;
                    Molecule mol;
                    loader.loadMolecule(mol);
                    // now we have molecule and its properties. we need to add it to library.
                    if (mol.tgroups.getTGroupCount())
                        libptr->get().addMonomersFromMolecule(mol, properties);
                }
            }
            catch (Exception& e)
            {
                throw IndigoError("Error loading monomer library: %s", e.message());
            }
        }
        return self.addObject(libptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadKetDocument(int source)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        std::string json_str;
        if (IndigoBaseMolecule::is(obj))
        {
            json_str = indigoJson(source);
        }
        else
        {
            auto& scanner = IndigoScanner::get(obj);
            scanner.readAll(json_str);
        }
        std::unique_ptr<IndigoKetDocument> docptr = std::make_unique<IndigoKetDocument>();
        KetDocumentJsonLoader loader{};
        loader.parseJson(json_str, docptr->get());
        return self.addObject(docptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSequence(int source, const char* seq_type, int library)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        IndigoObject& lib_obj = self.getObject(library);
        SequenceLoader loader(IndigoScanner::get(obj), IndigoMonomerLibrary::get(lib_obj));

        std::unique_ptr<IndigoKetDocument> docptr = std::make_unique<IndigoKetDocument>();

        loader.loadSequence(docptr->get(), seq_type);
        return self.addObject(docptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSequenceFromString(const char* string, const char* seq_type, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadSequence(source, seq_type, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadSequenceFromFile(const char* filename, const char* seq_type, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadSequence(source, seq_type, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFasta(int source, const char* seq_type, int library)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        IndigoObject& lib_obj = self.getObject(library);
        SequenceLoader loader(IndigoScanner::get(obj), IndigoMonomerLibrary::get(lib_obj));

        std::unique_ptr<IndigoKetDocument> docptr = std::make_unique<IndigoKetDocument>();

        loader.loadFasta(docptr->get(), seq_type);
        return self.addObject(docptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFastaFromString(const char* string, const char* seq_type, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadFasta(source, seq_type, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadFastaFromFile(const char* filename, const char* seq_type, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadFasta(source, seq_type, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadIdt(int source, int library)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        IndigoObject& lib_obj = self.getObject(library);
        MonomerTemplateLibrary& lib = IndigoMonomerLibrary::get(lib_obj);
        SequenceLoader loader(IndigoScanner::get(obj), lib);

        std::unique_ptr<IndigoKetDocument> docptr = std::make_unique<IndigoKetDocument>();

        loader.loadIdt(docptr->get());
        return self.addObject(docptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadIdtFromString(const char* string, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadIdt(source, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadIdtFromFile(const char* filename, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadIdt(source, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadHelm(int source, int library)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        IndigoObject& lib_obj = self.getObject(library);
        SequenceLoader loader(IndigoScanner::get(obj), IndigoMonomerLibrary::get(lib_obj));

        std::unique_ptr<IndigoKetDocument> docptr = std::make_unique<IndigoKetDocument>();

        loader.loadHELM(docptr->get());
        return self.addObject(docptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadHelmFromString(const char* string, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadHelm(source, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadHelmFromFile(const char* filename, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadHelm(source, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadAxoLabs(int source, int library)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(source);
        IndigoObject& lib_obj = self.getObject(library);
        MonomerTemplateLibrary& lib = IndigoMonomerLibrary::get(lib_obj);
        SequenceLoader loader(IndigoScanner::get(obj), lib);

        std::unique_ptr<IndigoKetDocument> docptr = std::make_unique<IndigoKetDocument>();

        loader.loadAxoLabs(docptr->get());
        return self.addObject(docptr.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadAxoLabsFromString(const char* string, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadString(string);
        int result;

        if (source <= 0)
            return -1;

        result = indigoLoadAxoLabs(source, library);
        indigoFree(source);
        return result;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadAxoLabsFromFile(const char* filename, int library)
{
    INDIGO_BEGIN
    {
        int source = indigoReadFile(filename);
        int result;

        if (source < 0)
            return -1;

        result = indigoLoadAxoLabs(source, library);
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
        loader.strict_aliphatic = self.smiles_loading_strict_aliphatic;

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

