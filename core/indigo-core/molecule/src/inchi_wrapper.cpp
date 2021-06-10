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

#include <algorithm>

#include "molecule/inchi_wrapper.h"

#include "base_cpp/obj.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_dearom.h"
// #include "mode.h"


using namespace indigo;

// Inchi doesn't seem to support multithreading
static OsLock inchi_lock;

namespace indigo
{
    // Structure that matches both inchi_OutputStruct and tagINCHI_Input
    struct InchiOutput
    {
        inchi_Atom* atom;
        inchi_Stereo0D* stereo0D;
        AT_NUM num_atoms;
        AT_NUM num_stereo0D;
    };
} // namespace indigo

IMPL_ERROR(InchiWrapper, "inchi-wrapper")

template <typename T> class InchiMemObject
{
public:
    typedef void (*DestructorT)(T* obj);

    InchiMemObject(DestructorT destructor) : destructor(destructor)
    {
    }

    ~InchiMemObject()
    {
        destructor(&obj);
    }

    T& ref()
    {
        return obj;
    }

private:
    T obj;
    DestructorT destructor;
};

const char* InchiWrapper::version()
{
    return "1.03";
}

InchiWrapper::InchiWrapper()
{
    clear();
}

void InchiWrapper::clear()
{
    // Set option to process empty structure by default
    setOptions("/WarnOnEmptyStructure");

    warning.clear();
    log.clear();
    auxInfo.clear();
}

void InchiWrapper::setOptions(const char* opt)
{
    options.readString(opt, true);
    // Replace '/' and '-' according to InChI manual:
    //   "(use - instead of / for O.S. other than MS Windows)"
#ifdef _WIN32
    for (int i = 0; i < options.size(); i++)
        if (options[i] == '-')
            options[i] = '/';
#else
    for (int i = 0; i < options.size(); i++)
        if (options[i] == '/')
            options[i] = '-';
#endif
}

void InchiWrapper::getOptions(ArrayChar& value)
{
    options.copy(value);
}

static inchi_BondType getInchiBondType(int bond_order)
{
    switch (bond_order)
    {
    case BOND_SINGLE:
        return INCHI_BOND_TYPE_SINGLE;
    case BOND_DOUBLE:
        return INCHI_BOND_TYPE_DOUBLE;
    case BOND_TRIPLE:
        return INCHI_BOND_TYPE_TRIPLE;
    case BOND_AROMATIC:
        return INCHI_BOND_TYPE_ALTERN;
    }
    throw InchiWrapper::Error("unexpected bond order %d", bond_order);
}

void InchiWrapper::loadMoleculeFromAux(const char* aux, Molecule& mol)
{
    // lock
    OsLocker locker(inchi_lock);

    InchiMemObject<inchi_Input> data_inp_obj(Free_inchi_Input);
    inchi_Input& data_inp = data_inp_obj.ref();

    memset(&data_inp, 0, sizeof(data_inp));
    InchiInpData data;
    memset(&data, 0, sizeof(data));
    data.pInp = &data_inp;

    int retcode = Get_inchi_Input_FromAuxInfo((char*)aux, 0, 0, &data);
    if (retcode != inchi_Ret_OKAY && retcode != inchi_Ret_WARNING)
        throw Error("Indigo-InChI: Aux InChI loading failed: %s. Code: %d.", data.szErrMsg, retcode);

    InchiOutput output;
    output.atom = data_inp.atom;
    output.stereo0D = data_inp.stereo0D;
    output.num_atoms = data_inp.num_atoms;
    output.num_stereo0D = data_inp.num_stereo0D;
    parseInchiOutput(output, mol);
}

void InchiWrapper::loadMoleculeFromInchi(const char* inchi_string, Molecule& mol)
{
    // lock
    OsLocker locker(inchi_lock);

    inchi_InputINCHI inchi_input;
    inchi_input.szInChI = (char*)inchi_string;
    inchi_input.szOptions = (char*)options.ptr();

    InchiMemObject<inchi_OutputStruct> inchi_output_obj(FreeStructFromINCHI);
    inchi_OutputStruct& inchi_output = inchi_output_obj.ref();

    int retcode = GetStructFromINCHI(&inchi_input, &inchi_output);

    if (inchi_output.szMessage)
        warning.readString(inchi_output.szMessage, true);
    if (inchi_output.szLog)
        log.readString(inchi_output.szLog, true);

    if (retcode != inchi_Ret_OKAY && retcode != inchi_Ret_WARNING)
        throw Error("Indigo-InChI: InChI loading failed: %s. Code: %d.", inchi_output.szMessage, retcode);

    // Check stereo options
    _stereo_opt = _STEREO_ABS;
    std::string str(inchi_string);
    if (str.find("/s2") != std::string::npos)
        _stereo_opt = _STEREO_REL;
    else if (str.find("/s3") != std::string::npos)
        _stereo_opt = _STEREO_RAC;

    InchiOutput output;
    output.atom = inchi_output.atom;
    output.stereo0D = inchi_output.stereo0D;
    output.num_atoms = inchi_output.num_atoms;
    output.num_stereo0D = inchi_output.num_stereo0D;
    parseInchiOutput(output, mol);
}

void InchiWrapper::neutralizeV5Nitrogen(Molecule& mol)
{
    // Initial structure C[C@H](O)[C@H](COC)/N=[N+](\[O-])/C=CCCCCCC
    // is loaded via InChI as CCCCCCC=CN(=O)=N[C@@H](COC)[C@H](C)O
    // and we cannot restore cis-trans configuration for O=N=N-C bond
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
        if (mol.isNitrogenV5(v))
        {
            const Vertex& vertex = mol.getVertex(v);
            for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
            {
                int nei_edge = vertex.neiEdge(nei);
                if (mol.getBondOrder(nei_edge) != BOND_DOUBLE)
                    continue;

                int nei_idx = vertex.neiVertex(nei);
                int number = mol.getAtomNumber(nei_idx);
                int charge = mol.getAtomCharge(nei_idx);
                int radical = mol.getAtomRadical(nei_idx);
                if ((number == ELEM_O || number == ELEM_S) && charge == 0 && radical == 0)
                {
                    mol.setAtomCharge(v, 1);
                    mol.setAtomCharge(nei_idx, -1);
                    mol.setBondOrder(nei_edge, BOND_SINGLE);
                    break;
                }
            }
        }
}

void InchiWrapper::parseInchiOutput(const InchiOutput& inchi_output, Molecule& mol)
{
    mol.clear();

    ArrayInt atom_indices;
    atom_indices.clear();

    // Add atoms
    for (AT_NUM i = 0; i < inchi_output.num_atoms; i++)
    {
        const inchi_Atom& inchi_atom = inchi_output.atom[i];

        int idx = mol.addAtom(Element::fromString(inchi_atom.elname));
        atom_indices.push(idx);
    }

    // Add bonds
    for (AT_NUM i = 0; i < inchi_output.num_atoms; i++)
    {
        const inchi_Atom& inchi_atom = inchi_output.atom[i];
        for (AT_NUM bi = 0; bi < inchi_atom.num_bonds; bi++)
        {
            AT_NUM nei = inchi_atom.neighbor[bi];
            if (i > nei)
                // Add bond only once
                continue;
            int bond_order = inchi_atom.bond_type[bi];
            int bond_stereo = abs(inchi_atom.bond_stereo[bi]);
            if (bond_order == INCHI_BOND_TYPE_NONE)
                throw Molecule::Error("Indigo-InChI: NONE-typed bonds are not supported");
            if (bond_order >= INCHI_BOND_TYPE_ALTERN)
                throw Molecule::Error("Indigo-InChI: ALTERN-typed bonds are not supported");
            int bond_idx = mol.addBond(atom_indices[i], atom_indices[nei], bond_order);

            if (bond_stereo == 1)
                mol.setBondDirection(bond_idx, BOND_UP);
            else if (bond_stereo == 6)
                mol.setBondDirection(bond_idx, BOND_DOWN);
            else if (bond_stereo == 4)
                mol.setBondDirection(bond_idx, BOND_EITHER);
            else if (bond_stereo == 3)
                mol.cis_trans.ignore(bond_idx);
            else if (bond_stereo != 0)
                throw Error("unknown number for bond stereo: %d", bond_stereo);
        }
    }

    // Add Hydrogen isotope atoms at the end to preserver
    // the same atom ordering
    for (AT_NUM i = 0; i < inchi_output.num_atoms; i++)
    {
        const inchi_Atom& inchi_atom = inchi_output.atom[i];

        int root_atom = atom_indices[i];
        for (int iso = 1; iso <= NUM_H_ISOTOPES; iso++)
        {
            int count = inchi_atom.num_iso_H[iso];
            while (count-- > 0)
            {
                int h = mol.addAtom(ELEM_H);
                mol.setAtomIsotope(h, iso);
                mol.addBond(root_atom, h, BOND_SINGLE);
            }
        }
    }

    // Set atom charges, radicals and etc.
    for (int i = 0; i < inchi_output.num_atoms; i++)
    {
        const inchi_Atom& inchi_atom = inchi_output.atom[i];

        int idx = atom_indices[i];
        mol.setAtomCharge(idx, inchi_atom.charge);

        mol.setAtomXyz(idx, inchi_atom.x, inchi_atom.y, inchi_atom.z);

        if (inchi_atom.isotopic_mass)
        {
            int default_iso = Element::getDefaultIsotope(mol.getAtomNumber(idx));
            mol.setAtomIsotope(idx, default_iso + inchi_atom.isotopic_mass - ISOTOPIC_SHIFT_FLAG);
        }
        if (inchi_atom.radical)
            mol.setAtomRadical(idx, inchi_atom.radical);
        mol.setImplicitH(idx, inchi_atom.num_iso_H[0]);
    }

    neutralizeV5Nitrogen(mol);

    // Process stereoconfiguration
    for (int i = 0; i < inchi_output.num_stereo0D; i++)
    {
        inchi_Stereo0D& stereo0D = inchi_output.stereo0D[i];
        if (stereo0D.type == INCHI_StereoType_DoubleBond)
        {
            if (stereo0D.parity != INCHI_PARITY_ODD && stereo0D.parity != INCHI_PARITY_EVEN)
                continue;

            int bond = mol.findEdgeIndex(stereo0D.neighbor[1], stereo0D.neighbor[2]);

            // Just ignore cis/trans info non-double bonds
            if (mol.getBondOrder(bond) != BOND_DOUBLE)
                continue;

            bool valid = mol.cis_trans.registerBondAndSubstituents(bond);
            if (!valid)
                throw Error("Indigo-InChI: Unsupported cis-trans configuration for "
                            "bond %d (atoms %d-%d-%d-%d)",
                            bond, stereo0D.neighbor[0], stereo0D.neighbor[1], stereo0D.neighbor[2], stereo0D.neighbor[3]);

            int vb, ve;
            const Edge& edge = mol.getEdge(bond);
            if (edge.beg == stereo0D.neighbor[1])
            {
                vb = stereo0D.neighbor[0];
                ve = stereo0D.neighbor[3];
            }
            else if (edge.beg == stereo0D.neighbor[2])
            {
                vb = stereo0D.neighbor[3];
                ve = stereo0D.neighbor[0];
            }
            else
                throw Error("Indigo-InChI: Internal error: cannot find cis-trans bond indices");

            const int* subst = mol.cis_trans.getSubstituents(bond);
            bool same_side;
            if (subst[0] == vb)
                same_side = (subst[2] == ve);
            else if (subst[1] == vb)
                same_side = (subst[3] == ve);
            else
                throw Error("Indigo-InChI: Internal error: cannot find cis-trans bond indices (#2)");

            if (stereo0D.parity == INCHI_PARITY_EVEN)
                same_side = !same_side;

            mol.cis_trans.setParity(bond, same_side ? MoleculeCisTrans::CIS : MoleculeCisTrans::TRANS);
        }
        else if (stereo0D.type == INCHI_StereoType_Tetrahedral)
        {

            if (stereo0D.parity == INCHI_PARITY_NONE)
                continue;

            int pyramid[4];
            if (stereo0D.central_atom == stereo0D.neighbor[0])
            {
                pyramid[1] = stereo0D.neighbor[1];
                pyramid[0] = stereo0D.neighbor[2];
                pyramid[2] = stereo0D.neighbor[3];
                pyramid[3] = -1;
            }
            else
            {
                pyramid[0] = stereo0D.neighbor[0];
                pyramid[1] = stereo0D.neighbor[1];
                pyramid[2] = stereo0D.neighbor[2];
                pyramid[3] = stereo0D.neighbor[3];
            }
            if (stereo0D.parity == INCHI_PARITY_ODD)
                std::swap(pyramid[0], pyramid[1]);

            if (stereo0D.parity == INCHI_PARITY_ODD || stereo0D.parity == INCHI_PARITY_EVEN)
            {
                if (_stereo_opt == _STEREO_ABS)
                    mol.stereocenters.add(stereo0D.central_atom, MoleculeStereocenters::ATOM_ABS, 0, pyramid);
                else if (_stereo_opt == _STEREO_REL)
                    mol.stereocenters.add(stereo0D.central_atom, MoleculeStereocenters::ATOM_OR, 0, pyramid);
                else if (_stereo_opt == _STEREO_RAC)
                    mol.stereocenters.add(stereo0D.central_atom, MoleculeStereocenters::ATOM_AND, 0, pyramid);
            }
            else if (stereo0D.parity == INCHI_PARITY_UNKNOWN || stereo0D.parity == INCHI_PARITY_UNDEFINED)
                mol.stereocenters.add(stereo0D.central_atom, MoleculeStereocenters::ATOM_ANY, 0, pyramid);
        }
    }
}

void InchiWrapper::generateInchiInput(Molecule& mol, inchi_Input& input, Array<inchi_Atom>& atoms, Array<inchi_Stereo0D>& stereo)
{
    QS_DEF(ArrayInt, mapping);
    mapping.clear_resize(mol.vertexEnd());
    mapping.fffill();
    int index = 0;
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
        mapping[v] = index++;
    atoms.clear_resize(index);
    atoms.zerofill();

    stereo.clear();
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        inchi_Atom& atom = atoms[mapping[v]];

        int atom_number = mol.getAtomNumber(v);
        if (atom_number == ELEM_PSEUDO)
            throw Error("Molecule with pseudoatom (%s) cannot be converted into InChI", mol.getPseudoAtom(v));
        if (atom_number == ELEM_RSITE)
            throw Error("Molecule with RGroups cannot be converted into InChI");
        strncpy(atom.elname, Element::toString(atom_number), ATOM_EL_LEN);

        Vec3f& c = mol.getAtomXyz(v);
        atom.x = c.x;
        atom.y = c.y;
        atom.z = c.z;

        // connectivity
        const Vertex& vtx = mol.getVertex(v);
        int nei_idx = 0;
        for (int nei = vtx.neiBegin(); nei != vtx.neiEnd(); nei = vtx.neiNext(nei))
        {
            int v_nei = vtx.neiVertex(nei);
            atom.neighbor[nei_idx] = mapping[v_nei];
            int edge_idx = vtx.neiEdge(nei);
            atom.bond_type[nei_idx] = getInchiBondType(mol.getBondOrder(edge_idx));

            int bond_stereo = INCHI_BOND_STEREO_NONE;

            int direction1 = mol.getBondDirection2(v, v_nei);
            int direction2 = mol.getBondDirection2(v_nei, v);

            if (mol.cis_trans.isIgnored(edge_idx))
                bond_stereo = INCHI_BOND_STEREO_DOUBLE_EITHER;
            else
            {
                if (direction1 == BOND_EITHER)
                    bond_stereo = INCHI_BOND_STEREO_SINGLE_1EITHER;
                else if (direction2 == BOND_EITHER)
                    bond_stereo = INCHI_BOND_STEREO_SINGLE_2EITHER;
                else if (direction1 == BOND_UP)
                    bond_stereo = INCHI_BOND_STEREO_SINGLE_1UP;
                else if (direction2 == BOND_UP)
                    bond_stereo = INCHI_BOND_STEREO_SINGLE_2UP;
                else if (direction1 == BOND_DOWN)
                    bond_stereo = INCHI_BOND_STEREO_SINGLE_1DOWN;
                else if (direction2 == BOND_DOWN)
                    bond_stereo = INCHI_BOND_STEREO_SINGLE_2DOWN;
            }
            atom.bond_stereo[nei_idx] = bond_stereo;
            nei_idx++;
        }
        atom.num_bonds = vtx.degree();

        // Other properties
        atom.isotopic_mass = mol.getAtomIsotope(v);
        atom.radical = mol.getAtomRadical(v);
        atom.charge = mol.getAtomCharge(v);

        // Hydrogens
        int hcount = -1;
        if (Molecule::shouldWriteHCount(mol, v) || mol.isExplicitValenceSet(v) || mol.isImplicitHSet(v))
        {
            if (mol.getAtomAromaticity(v) == ATOM_AROMATIC && atom_number == ELEM_C && atom.charge == 0 && atom.radical == 0)
            {
                // Do not set number of implicit hydrogens here as InChI throws an exception on
                // the molecule B1=CB=c2cc3B=CC=c3cc12
                ;
            }
            else
                // set -1 to tell InChI add implicit hydrogens automatically
                hcount = mol.getImplicitH_NoThrow(v, -1);
        }

        if ((mol.getAtomNumber(v) == ELEM_S) && (mol.getAtomValence(v) == 4))
        {
            hcount = mol.getImplicitH_NoThrow(v, -1);
        }

        atom.num_iso_H[0] = hcount;
    }

    // Process cis-trans bonds
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
    {
        if (mol.cis_trans.getParity(e) == 0)
            continue;

        int subst[4];
        mol.cis_trans.getSubstituents_All(e, subst);

        const Edge& edge = mol.getEdge(e);

        inchi_Stereo0D& st = stereo.push();

        // Write it as
        // #0 - #1 = #2 - #3
        st.neighbor[0] = mapping[subst[0]];
        st.neighbor[1] = mapping[edge.beg];
        st.neighbor[2] = mapping[edge.end];
        st.neighbor[3] = mapping[subst[2]];

        if (mol.cis_trans.getParity(e) == MoleculeCisTrans::CIS)
            st.parity = INCHI_PARITY_ODD;
        else
            st.parity = INCHI_PARITY_EVEN;

        st.central_atom = NO_ATOM;
        st.type = INCHI_StereoType_DoubleBond;
    }

    // Process tetrahedral stereocenters
    for (int i = mol.stereocenters.begin(); i != mol.stereocenters.end(); i = mol.stereocenters.next(i))
    {
        int v = mol.stereocenters.getAtomIndex(i);

        int type, group, pyramid[4];
        mol.stereocenters.get(v, type, group, pyramid);
        if (type == MoleculeStereocenters::ATOM_ANY)
            continue;
        if (type == MoleculeStereocenters::ATOM_AND)
            setOptions("/SRac");
        else if (type == MoleculeStereocenters::ATOM_OR)
            setOptions("/SRel");

        for (int i = 0; i < 4; i++)
            if (pyramid[i] != -1)
                pyramid[i] = mapping[pyramid[i]];

        inchi_Stereo0D& st = stereo.push();

        /*
           4 neighbors

                    X                    neighbor[4] : {#W, #X, #Y, #Z}
                    |                    central_atom: #A
                 W--A--Y                 type        : INCHI_StereoType_Tetrahedral
                    |
                    Z
           parity: if (X,Y,Z) are clockwize when seen from W then parity is 'e' otherwise 'o'
           Example (see AXYZW above): if W is above the plane XYZ then parity = 'e'

           3 neighbors

                      Y          Y       neighbor[4] : {#A, #X, #Y, #Z}
                     /          /        central_atom: #A
                 X--A  (e.g. O=S   )     type        : INCHI_StereoType_Tetrahedral
                     \          \
                      Z          Z
        */
        int offset = 0;
        if (pyramid[3] == -1)
            offset = 1;

        st.neighbor[offset] = mapping[pyramid[0]];
        st.neighbor[offset + 1] = mapping[pyramid[1]];
        st.neighbor[offset + 2] = mapping[pyramid[2]];
        if (offset == 0)
            st.neighbor[3] = mapping[pyramid[3]];
        else
            st.neighbor[0] = mapping[v];

        st.parity = INCHI_PARITY_ODD;
        if (offset != 0)
            st.parity = INCHI_PARITY_ODD;
        else
            st.parity = INCHI_PARITY_EVEN;
        st.central_atom = mapping[v];
        st.type = INCHI_StereoType_Tetrahedral;
    }

    input.atom = atoms.ptr();
    input.num_atoms = atoms.size();
    input.stereo0D = stereo.ptr();
    input.num_stereo0D = stereo.size();
    input.szOptions = options.ptr();
}

void InchiWrapper::saveMoleculeIntoInchi(Molecule& mol, ArrayChar& inchi)
{
    inchi_Input input;
    QS_DEF(Array<inchi_Atom>, atoms);
    QS_DEF(Array<inchi_Stereo0D>, stereo);

    // Check if structure has aromatic bonds
    bool has_aromatic = false;
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
        if (mol.getBondOrder(e) == BOND_AROMATIC)
        {
            has_aromatic = true;
            break;
        }

    Molecule* target = &mol;
    Obj<Molecule> dearom;
    if (has_aromatic)
    {
        dearom.create();
        dearom->clone(mol, 0, 0);
        try
        {
            AromaticityOptions arom_options;
            arom_options.method = AromaticityOptions::GENERIC;
            arom_options.unique_dearomatization = true;
            dearom->dearomatize(arom_options);
        }
        catch (NonUniqueDearomatizationException& ex)
        {
            // Do not allow non-unique dearomatizations
            throw;
        }
        catch (DearomatizationException&)
        {
        }
        target = dearom.get();
    }
    generateInchiInput(*target, input, atoms, stereo);

    InchiMemObject<inchi_Output> inchi_output_obj(FreeINCHI);
    inchi_Output& output = inchi_output_obj.ref();

    // lock
    OsLocker locker(inchi_lock);

    int ret = GetINCHI(&input, &output);

    if (output.szMessage)
        warning.readString(output.szMessage, true);
    if (output.szLog)
    {
        const char* unrec_opt_prefix = "Unrecognized option:";
        if (strncmp(output.szLog, unrec_opt_prefix, strlen(unrec_opt_prefix)) == 0)
        {
            size_t i;
            for (i = 0; i < strlen(output.szLog); i++)
                if (output.szLog[i] == '\n')
                    break;
            ArrayChar unrec_opt;
            if (i > 0)
                unrec_opt.copy(output.szLog, i - 1);
            unrec_opt.push(0);

            throw Error("Indigo-InChI: %s.", unrec_opt.ptr());
            ;
        }

        log.readString(output.szLog, true);
    }
    if (output.szAuxInfo)
        auxInfo.readString(output.szAuxInfo, true);

    if (ret != inchi_Ret_OKAY && ret != inchi_Ret_WARNING)
    {
        // Construct error before dispoing inchi output to preserve error message
        Error error("Indigo-InChI: InChI generation failed: %s. Code: %d.", output.szMessage, ret);
        throw error;
    }

    inchi.readString(output.szInChI, true);
}

void InchiWrapper::InChIKey(const char* inchi, ArrayChar& output)
{
    // lock
    OsLocker locker(inchi_lock);

    output.resize(28);
    output.zerofill();
    int ret = GetINCHIKeyFromINCHI(inchi, 0, 0, output.ptr(), 0, 0);
    if (ret != INCHIKEY_OK)
    {
        if (ret == INCHIKEY_UNKNOWN_ERROR)
            throw Error("INCHIKEY_UNKNOWN_ERROR");
        else if (ret == INCHIKEY_EMPTY_INPUT)
            throw Error("INCHIKEY_EMPTY_INPUT");
        else if (ret == INCHIKEY_INVALID_INCHI_PREFIX)
            throw Error("INCHIKEY_INVALID_INCHI_PREFIX");
        else if (ret == INCHIKEY_NOT_ENOUGH_MEMORY)
            throw Error("INCHIKEY_NOT_ENOUGH_MEMORY");
        else if (ret == INCHIKEY_INVALID_INCHI)
            throw Error("INCHIKEY_INVALID_INCHI");
        else if (ret == INCHIKEY_INVALID_STD_INCHI)
            throw Error("INCHIKEY_INVALID_STD_INCHI");
        else
            throw Error("Undefined error");
    }
}
