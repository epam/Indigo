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

#include <memory>

#include "../layout/molecule_layout.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "layout/sequence_layout.h"
#include "molecule/elements.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/monomer_commons.h"
#include "molecule/parse_utils.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

#define STRCMP(a, b) strncmp((a), (b), strlen(b))

using namespace indigo;

void MolfileLoader::_readCtab2000()
{
    _init();

    QS_DEF(Array<char>, str);

    // read atoms
    for (int k = 0; k < _atoms_num; k++)
    {
        // read each atom line to buffer
        _scanner.readLine(str, false);
        BufferScanner atom_line(str);

        // read coordinates
        float x = atom_line.readFloatFix(10);
        float y = atom_line.readFloatFix(10);
        float z = atom_line.readFloatFix(10);

        atom_line.skip(1);

        char atom_label_array[4] = {0};
        int label = 0;
        int isotope = 0;

        int& atom_type = _atom_types.push();

        _hcount.push(0);

        atom_type = _ATOM_ELEMENT;

        // read atom label and mass difference
        int read_chars_atom_label = atom_line.readCharsFlexible(3, atom_label_array);

        // Atom label can be both left-bound or right-bound: "  N", "N  " or even " N ".
        char* buf = _strtrim(atom_label_array);

        // #349: make 'isotope' field optional
        try
        {
            atom_line.skip(3 - read_chars_atom_label);
            isotope = atom_line.readIntFix(2);
        }
        catch (Scanner::Error&)
        {
        }

        if (buf[0] == 0)
            throw Error("Empty atom label");
        else if (buf[0] == 'R' && (buf[1] == '#' || buf[1] == 0))
        {
            atom_type = _ATOM_R;
            label = ELEM_RSITE;
        }
        else if (buf[0] == 'A' && buf[1] == 0)
        {
            atom_type = _ATOM_A; // will later become 'any atom' or pseudo atom
        }
        else if (buf[0] == 'A' && buf[1] == 'H' && buf[2] == 0)
        {
            if (_qmol == 0)
                throw Error("'AH' label is allowed only for queries");
            atom_type = _ATOM_AH;
        }
        else if (buf[0] == 'X' && buf[1] == 0 && !treat_x_as_pseudoatom) // TODO: treat_x_as_pseudoatom should be checked only for mol?
        {
            if (_qmol == 0)
                throw Error("'X' label is allowed only for queries");
            atom_type = _ATOM_X;
        }
        else if (buf[0] == 'X' && buf[1] == 'H' && buf[2] == 0)
        {
            if (_qmol == 0)
                throw Error("'XH' label is allowed only for queries");
            atom_type = _ATOM_XH;
        }
        else if (buf[0] == 'Q' && buf[1] == 0)
        {
            if (_qmol == 0)
                throw Error("'Q' label is allowed only for queries");
            atom_type = _ATOM_Q;
        }
        else if (buf[0] == 'Q' && buf[1] == 'H' && buf[2] == 0)
        {
            if (_qmol == 0)
                throw Error("'QH' label is allowed only for queries");
            atom_type = _ATOM_QH;
        }
        else if (buf[0] == 'M' && buf[1] == 0)
        {
            if (_qmol == 0)
                throw Error("'M' label is allowed only for queries");
            atom_type = _ATOM_M;
        }
        else if (buf[0] == 'M' && buf[1] == 'H' && buf[2] == 0)
        {
            if (_qmol == 0)
                throw Error("'MH' label is allowed only for queries");
            atom_type = _ATOM_MH;
        }
        else if (buf[0] == 'L' && buf[1] == 0)
        {
            if (_qmol == 0)
                throw Error("atom lists are allowed only for queries");
            atom_type = _ATOM_LIST;
        }
        else if (buf[0] == 'D' && buf[1] == 0)
        {
            label = ELEM_H;
            isotope = DEUTERIUM;
        }
        else if (buf[0] == 'T' && buf[1] == 0)
        {
            label = ELEM_H;
            isotope = TRITIUM;
        }
        else if (_qmol && buf[0] == '*' && buf[1] == 0)
        {
            atom_type = _ATOM_STAR;
        }
        else
        {
            label = _getElement(buf);

            if (label == -1)
            {
                atom_type = _ATOM_PSEUDO;
                if (isotope != 0)
                    throw Error("isotope number not allowed on pseudo-atoms");
            }

            if (isotope != 0)
                isotope = Element::getDefaultIsotope(label) + isotope;
        }

        int stereo_care = 0, valence = 0;
        int aam = 0, irflag = 0, ecflag = 0;
        int charge = 0, radical = 0;

        // #349: make 'charge' field optional
        try
        {
            _convertCharge(atom_line.readIntFix(3), charge, radical);
        }
        catch (Scanner::Error&)
        {
        }

        try
        {

            atom_line.skip(3); // skip atom stereo parity
            _hcount[k] = atom_line.readIntFix(3);

            if (_hcount[k] > 0 && _qmol == 0)
                if (!ignore_noncritical_query_features)
                    throw Error("only a query can have H count value");

            stereo_care = atom_line.readIntFix(3);

            if (stereo_care > 0 && _qmol == 0)
                if (!ignore_noncritical_query_features)
                    throw Error("only a query can have stereo care box");

            valence = atom_line.readIntFix(3);
            atom_line.skip(9);                // skip "HO designator" and 2 unused fields
            aam = atom_line.readIntFix(3);    // atom-to-atom mapping number
            irflag = atom_line.readIntFix(3); // inversion/retension flag,
            ecflag = atom_line.readIntFix(3); // exact change flag
        }
        catch (Scanner::Error&)
        {
        }

        int idx;

        if (_mol != 0)
        {
            idx = _mol->addAtom(label);

            if (atom_type == _ATOM_PSEUDO)
                _mol->setPseudoAtom(idx, buf);

            _mol->setAtomCharge_Silent(idx, charge);
            _mol->setAtomIsotope(idx, isotope);
            _mol->setAtomRadical(idx, radical);

            if (valence > 0 && valence <= 14)
                _mol->setExplicitValence(idx, valence);
            if (valence == 15)
                _mol->setExplicitValence(idx, 0);

            _bmol->setAtomXyz(idx, x, y, z);
        }
        else
        {
            std::unique_ptr<QueryMolecule::Atom> atom;

            if (atom_type == _ATOM_ELEMENT)
                atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, label);
            else if (atom_type == _ATOM_PSEUDO)
                atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, buf);
            else if (atom_type == _ATOM_A)
                atom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
            else if (atom_type == _ATOM_AH)
            {
                atom = std::make_unique<QueryMolecule::Atom>();
            }
            else if (atom_type == _ATOM_STAR)
            {
                atom = std::make_unique<QueryMolecule::Atom>();
                atom->type = QueryMolecule::ATOM_STAR;
            }
            else if (atom_type == _ATOM_QH)
                atom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
            else if (atom_type == _ATOM_Q)
                atom.reset(QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                    QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
            else if (atom_type == _ATOM_X)
            {
                atom = std::make_unique<QueryMolecule::Atom>();
                atom->type = QueryMolecule::OP_OR;
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
            }
            else if (atom_type == _ATOM_XH)
            {
                atom = std::make_unique<QueryMolecule::Atom>();
                atom->type = QueryMolecule::OP_OR;
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
                atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H));
            }
            else if (atom_type == _ATOM_MH)
            {
                atom = std::make_unique<QueryMolecule::Atom>();
                atom->type = QueryMolecule::OP_AND;
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));
            }
            else if (atom_type == _ATOM_M)
            {
                atom = std::make_unique<QueryMolecule::Atom>();
                atom->type = QueryMolecule::OP_AND;
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));
                atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
            }
            else if (atom_type == _ATOM_R)
                atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RSITE, 0);
            else // _ATOM_LIST
                atom = std::make_unique<QueryMolecule::Atom>();

            if (charge != 0)
                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, charge)));
            if (isotope != 0)
                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, isotope)));
            if (radical != 0)
                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));
            if (valence > 0)
            {
                if (valence == 15)
                    valence = 0;
                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_BOND_ORDER, valence)));
            }

            idx = _qmol->addAtom(atom.release());
            _bmol->setAtomXyz(idx, x, y, z);
        }

        if (stereo_care)
            _stereo_care_atoms[idx] = 1;

        _bmol->reaction_atom_mapping[idx] = aam;
        _bmol->reaction_atom_inversion[idx] = irflag;
        _bmol->reaction_atom_exact_change[idx] = ecflag;
    }

    // read bonds

    for (int bond_idx = 0; bond_idx < _bonds_num; bond_idx++)
    {
        // read each bond line to buffer
        _scanner.readLine(str, false);
        BufferScanner bond_line(str);

        int beg = bond_line.readIntFix(3);
        int end = bond_line.readIntFix(3);
        int order = bond_line.readIntFix(3);
        int stereo = 0;
        int topology = 0;
        int rcenter = 0;

        // #349: rest fields are optional
        try
        {

            stereo = bond_line.readIntFix(3);

            bond_line.skip(3); // not used

            topology = bond_line.readIntFix(3);

            if (topology != 0 && _qmol == 0)
                if (!ignore_noncritical_query_features)
                    throw Error("bond topology is allowed only for queries");

            rcenter = bond_line.readIntFix(3);
        }
        catch (Scanner::Error&)
        {
        }

        if (_mol != 0)
        {
            if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC || order == _BOND_HYDROGEN ||
                order == _BOND_COORDINATION)
                _mol->addBond_Silent(beg - 1, end - 1, order);
            else if (order == _BOND_SINGLE_OR_DOUBLE)
                throw Error("'single or double' bonds are allowed only for queries");
            else if (order == _BOND_SINGLE_OR_AROMATIC)
                throw Error("'single or aromatic' bonds are allowed only for queries");
            else if (order == _BOND_DOUBLE_OR_AROMATIC)
                throw Error("'double or aromatic' bonds are allowed only for queries");
            else if (order == _BOND_ANY)
                throw Error("'any' bonds are allowed only for queries");
            else
                throw Error("unknown bond type: %d", order);
        }
        else
        {
            int direction = BOND_ZERO;
            if (stereo == BIOVIA_STEREO_UP)
                direction = BOND_UP;
            else if (stereo == BIOVIA_STEREO_DOWN)
                direction = BOND_DOWN;
            _qmol->addBond(beg - 1, end - 1, QueryMolecule::createQueryMoleculeBond(order, topology, direction));
        }

        if (stereo == BIOVIA_STEREO_UP)
            _bmol->setBondDirection(bond_idx, BOND_UP);
        else if (stereo == BIOVIA_STEREO_DOWN)
            _bmol->setBondDirection(bond_idx, BOND_DOWN);
        else if (stereo == BIOVIA_STEREO_ETHER)
            _bmol->setBondDirection(bond_idx, BOND_EITHER);
        else if (stereo == BIOVIA_STEREO_DOUBLE_CISTRANS)
            _ignore_cistrans[bond_idx] = 1;
        else if (stereo != BIOVIA_STEREO_NO)
            throw Error("unknown number for bond stereo: %d", stereo);

        _bmol->reaction_bond_reacting_center[bond_idx] = rcenter;
    }

    int n_3d_features = -1;

    // read groups
    while (!_scanner.isEOF())
    {
        char c = _scanner.readChar();

        if (c == 'G')
        {
            _scanner.skipLine();
            _scanner.skipLine();
            continue;
        }
        if (c == 'M')
        {
            _scanner.skip(2);
            char chars[4] = {0, 0, 0, 0};

            _scanner.readCharsFix(3, chars);

            if (strncmp(chars, "END", 3) == 0)
            {
                _scanner.skipLine();
                break;
            }
            // atom list
            else if (strncmp(chars, "ALS", 3) == 0)
            {
                if (_qmol == 0)
                    throw Error("atom lists are allowed only for queries");

                int i;

                _scanner.skip(1);
                int atom_idx = _scanner.readIntFix(3);
                int list_size = _scanner.readIntFix(3);
                _scanner.skip(1);
                char excl_char = _scanner.readChar();
                _scanner.skip(1);

                atom_idx--;

                std::unique_ptr<QueryMolecule::Atom> atomlist;

                _scanner.readLine(str, false);
                BufferScanner rest(str);

                for (i = 0; i < list_size; i++)
                {
                    int j;

                    memset(chars, 0, sizeof(chars));

                    for (j = 0; j < 4; j++)
                    {
                        // can not read 4 characters at once because
                        // sqlplus cuts the trailing spaces
                        if (!rest.isEOF())
                            chars[j] = rest.readChar();
                        else
                            break;
                    }

                    if (j < 1)
                        throw Error("atom list: can not read element #%d", i);

                    for (j = 0; j < 4; j++)
                        if (chars[j] == ' ')
                            memset(chars + j, 0, 4 - j);

                    if (chars[3] != 0)
                        throw Error("atom list: invalid element '%c%c%c%c'", chars[0], chars[1], chars[2], chars[3]);

                    if (chars[0] == 'A' && chars[1] == 0)
                    {
                        if (list_size == 1)
                            atomlist.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
                        else
                            throw Error("'A' inside atom list, if present, must be single");
                    }
                    else if (chars[0] == 'Q' && chars[1] == 0)
                    {
                        if (list_size == 1)
                            atomlist.reset(QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                                    QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
                        else
                            throw Error("'Q' inside atom list, if present, must be single");
                    }
                    else
                    {
                        _appendQueryAtom(chars, atomlist);
                    }
                }

                if (excl_char == 'T')
                    atomlist.reset(QueryMolecule::Atom::nicht(atomlist.release()));

                _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), atomlist.release()));
            }
            // atom charge
            else if (strncmp(chars, "CHG", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    int charge = _scanner.readIntFix(3);

                    if (_mol != 0)
                        _mol->setAtomCharge_Silent(atom_idx, charge);
                    else
                    {
                        _qmol->getAtom(atom_idx).removeConstraints(QueryMolecule::ATOM_CHARGE);
                        _qmol->resetAtom(atom_idx,
                                         QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, charge)));
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "RAD", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    int radical = _scanner.readIntFix(3);

                    if (_mol != 0)
                        _mol->setAtomRadical(atom_idx, radical);
                    else
                    {
                        _qmol->getAtom(atom_idx).removeConstraints(QueryMolecule::ATOM_RADICAL);
                        _qmol->resetAtom(atom_idx,
                                         QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));
                    }
                }
                _scanner.skipLine();
            }
            // atom isotope
            else if (strncmp(chars, "ISO", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (--n >= 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    int isotope = _scanner.readIntFix(3);

                    if (_mol != 0)
                        _mol->setAtomIsotope(atom_idx, isotope);
                    else
                    {
                        _qmol->getAtom(atom_idx).removeConstraints(QueryMolecule::ATOM_ISOTOPE);
                        _qmol->resetAtom(atom_idx,
                                         QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, isotope)));
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SUB", 3) == 0)
            {
                if (_qmol == 0)
                    throw Error("substitution counts are allowed only for queries");

                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    int sub_count = _scanner.readIntFix(3);

                    if (sub_count == -1) // no substitution
                        _qmol->resetAtom(atom_idx,
                                         QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, 0)));
                    else if (sub_count == -2)
                    {
                        _qmol->resetAtom(
                            atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN,
                                                                                                                     _qmol->getVertex(atom_idx).degree())));
                    }
                    else if (sub_count > 0)
                        _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(
                                                       _qmol->releaseAtom(atom_idx),
                                                       new QueryMolecule::Atom(
                                                           QueryMolecule::ATOM_SUBSTITUENTS, sub_count,
                                                           (sub_count < MolfileSaver::MAX_SUBSTITUTION_COUNT ? sub_count : QueryMolecule::MAX_ATOM_VALUE))));
                    else
                        throw Error("invalid SUB value: %d", sub_count);
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "RBC", 3) == 0)
            {
                if (_qmol == 0)
                {
                    if (!ignore_noncritical_query_features)
                        throw Error("ring bond count is allowed only for queries");
                }
                else
                {
                    int n = _scanner.readIntFix(3);

                    while (n-- > 0)
                    {
                        _scanner.skip(1);
                        int atom_idx = _scanner.readIntFix(3) - 1;
                        _scanner.skip(1);
                        int rbcount = _scanner.readIntFix(3);

                        if (rbcount == -1) // no ring bonds
                            _qmol->resetAtom(
                                atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 0)));
                        else if (rbcount == -2) // as drawn
                        {
                            int rbonds = 0;
                            const Vertex& vertex = _qmol->getVertex(atom_idx);

                            for (int k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
                                if (_qmol->getEdgeTopology(vertex.neiEdge(k)) == TOPOLOGY_RING)
                                    rbonds++;

                            _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx),
                                                                                new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbonds)));
                        }
                        else if (rbcount > 1)
                            _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(
                                                           _qmol->releaseAtom(atom_idx),
                                                           new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, rbcount, (rbcount < 4 ? rbcount : 100))));
                        else
                            throw Error("ring bond count = %d makes no sense", rbcount);
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "UNS", 3) == 0)
            {
                if (_qmol == 0)
                {
                    if (!ignore_noncritical_query_features)
                        throw Error("unaturated atoms are allowed only for queries");
                }
                else
                {
                    int n = _scanner.readIntFix(3);

                    while (n-- > 0)
                    {
                        _scanner.skip(1);
                        int atom_idx = _scanner.readIntFix(3) - 1;
                        _scanner.skip(1);
                        int unsaturation = _scanner.readIntFix(3);

                        if (unsaturation)
                            _qmol->resetAtom(
                                atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_UNSATURATION, 0)));
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "$3D", 3) == 0)
            {
                if (_qmol == 0)
                {
                    if (!ignore_noncritical_query_features)
                        throw Error("3D features are allowed only for queries");
                }
                else
                {
                    if (n_3d_features == -1)
                    {
                        n_3d_features = _scanner.readIntFix(3);
                        _scanner.skipLine();
                    }
                    else
                    {
                        n_3d_features--;

                        if (n_3d_features < 0)
                            throw Error("3D feature unexpected");

                        _read3dFeature2000();
                    }
                }
            }
            else if (strncmp(chars, "AAL", 3) == 0)
            {
                _scanner.skip(1);
                int site_idx = _scanner.readIntFix(3) - 1;
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    int att_type = _scanner.readIntFix(3);

                    _bmol->setRSiteAttachmentOrder(site_idx, atom_idx, att_type - 1);
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "RGP", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    _bmol->allowRGroupOnRSite(atom_idx, _scanner.readIntFix(3));
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "LOG", 3) == 0)
            {
                // skip something
                _scanner.skip(3);

                _scanner.skip(1);
                int rgroup_idx = _scanner.readIntFix(3);
                _scanner.skip(1);
                int if_then = _scanner.readIntFix(3);
                _scanner.skip(1);
                int rest_h = _scanner.readIntFix(3);
                _scanner.skip(1);

                QS_DEF(Array<char>, occurrence_str);

                RGroup& rgroup = _bmol->rgroups.getRGroup(rgroup_idx);
                rgroup.clear();

                rgroup.if_then = if_then;
                rgroup.rest_h = rest_h;

                _scanner.readLine(occurrence_str, true);

                rgroup.readOccurrence(occurrence_str.ptr());
            }
            else if (strncmp(chars, "APO", 3) == 0)
            {
                int list_length = _scanner.readIntFix(3);

                while (list_length-- > 0)
                {
                    _scanner.skip(1);
                    int atom_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    int att_type = _scanner.readIntFix(3);

                    if (att_type == -1)
                        att_type = 3;

                    for (int att_idx = 0; (1 << att_idx) <= att_type; att_idx++)
                        if (att_type & (1 << att_idx))
                            _bmol->addAttachmentPoint(att_idx + 1, atom_idx);
                }

                _scanner.skipLine();
            }
            else if (strncmp(chars, "SST", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    char type[4] = {0, 0, 0, 0};
                    int sgroup_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    _scanner.readCharsFix(3, type);
                    int idx = _sgroup_mapping[sgroup_idx];
                    SGroup* sgroup = &_bmol->sgroups.getSGroup(idx);
                    if (strcmp(type, "ALT") == 0)
                        sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_ALT;
                    else if (strcmp(type, "RAN") == 0)
                        sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_RAN;
                    else if (strcmp(type, "BLO") == 0)
                        sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_BLO;
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "STY", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    char type[4] = {0, 0, 0, 0};
                    int sgroup_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skip(1);
                    _scanner.readCharsFix(3, type);
                    _sgroup_types.expandFill(sgroup_idx + 1, -1);
                    _sgroup_mapping.expandFill(sgroup_idx + 1, -1);

                    int idx = _bmol->sgroups.addSGroup(type);
                    SGroup* sgroup = &_bmol->sgroups.getSGroup(idx);
                    sgroup->original_group = sgroup_idx + 1;
                    _sgroup_types[sgroup_idx] = sgroup->sgroup_type;
                    _sgroup_mapping[sgroup_idx] = idx;
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SPL", 3) == 0 || strncmp(chars, "SBT", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int sgroup_idx = _scanner.readIntFix(3) - 1;

                    SGroup* sgroup = &_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);

                    _scanner.skip(1);
                    int value = _scanner.readIntFix(3);

                    if (strncmp(chars, "SPL", 3) == 0)
                        sgroup->parent_group = value;
                    else
                        sgroup->brk_style = value;
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SST", 3) == 0)
            {
                int n = _scanner.readIntFix(3);

                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int sgroup_idx = _scanner.readIntFix(3) - 1;

                    SGroup* sgroup = &_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);

                    char subtype[4] = {0, 0, 0, 0};
                    _scanner.readCharsFix(3, subtype);

                    if (strncmp(subtype, "ALT", 3) == 0)
                        sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_ALT;
                    else if (strncmp(subtype, "RAN", 3) == 0)
                        sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_RAN;
                    else if (strncmp(subtype, "BLO", 3) == 0)
                        sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_BLO;
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SAL", 3) == 0 || strncmp(chars, "SBL", 3) == 0 || strncmp(chars, "SDI", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;

                if (_sgroup_mapping[sgroup_idx] >= 0)
                {
                    SGroup* sgroup = &_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);

                    int n = _scanner.readIntFix(3);

                    if (strncmp(chars, "SDI", 3) == 0)
                    {
                        if (n == 4) // should always be 4
                        {
                            Vec2f* brackets = sgroup->brackets.push();

                            _scanner.skipSpace();
                            brackets[0].x = _scanner.readFloat();
                            _scanner.skipSpace();
                            brackets[0].y = _scanner.readFloat();
                            _scanner.skipSpace();
                            brackets[1].x = _scanner.readFloat();
                            _scanner.skipSpace();
                            brackets[1].y = _scanner.readFloat();
                        }
                    }
                    else
                        while (n-- > 0)
                        {
                            _scanner.skip(1);
                            if (strncmp(chars, "SAL", 3) == 0)
                                sgroup->atoms.push(_scanner.readIntFix(3) - 1);
                            else // SBL
                                sgroup->bonds.push(_scanner.readIntFix(3) - 1);
                        }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SDT", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;
                _scanner.skip(1);

                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_DAT)
                {
                    QS_DEF(Array<char>, rest);

                    _scanner.readLine(rest, false);
                    BufferScanner strscan(rest);
                    DataSGroup& sgroup = (DataSGroup&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);

                    // Read field name
                    int k = 30;
                    while (k-- > 0)
                    {
                        if (strscan.isEOF())
                            break;
                        sgroup.name.push(strscan.readChar());
                    }
                    // Remove last spaces because name can have multiple words
                    while (sgroup.name.size() > 0)
                    {
                        if (isspace(sgroup.name.top()))
                            sgroup.name.pop();
                        else
                            break;
                    }

                    sgroup.name.push(0);

                    // Read field type
                    k = 2;
                    while (k-- > 0)
                    {
                        if (strscan.isEOF())
                            break;
                        sgroup.type.push(strscan.readChar());
                    }
                    sgroup.type.push(0);

                    // Read field description
                    k = 20;
                    while (k-- > 0)
                    {
                        if (strscan.isEOF())
                            break;
                        sgroup.description.push(strscan.readChar());
                    }
                    // Remove last spaces because dscription can have multiple words?
                    while (sgroup.description.size() > 0)
                    {
                        if (isspace(sgroup.description.top()))
                            sgroup.description.pop();
                        else
                            break;
                    }
                    sgroup.description.push(0);

                    // Read query code
                    k = 2;
                    while (k-- > 0)
                    {
                        if (strscan.isEOF())
                            break;
                        sgroup.querycode.push(strscan.readChar());
                    }
                    while (sgroup.querycode.size() > 0)
                    {
                        if (isspace(sgroup.querycode.top()))
                            sgroup.querycode.pop();
                        else
                            break;
                    }
                    sgroup.querycode.push(0);

                    // Read query operator
                    k = 20;
                    while (k-- > 0)
                    {
                        if (strscan.isEOF())
                            break;
                        sgroup.queryoper.push(strscan.readChar());
                    }
                    while (sgroup.queryoper.size() > 0)
                    {
                        if (isspace(sgroup.queryoper.top()))
                            sgroup.queryoper.pop();
                        else
                            break;
                    }
                    sgroup.queryoper.push(0);
                }
            }
            else if (strncmp(chars, "SDD", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;
                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_DAT)
                {
                    _scanner.skip(1);
                    DataSGroup& sgroup = (DataSGroup&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);

                    _readSGroupDisplay(_scanner, sgroup);
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SED", 3) == 0 || strncmp(chars, "SCD", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;
                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_DAT)
                {
                    _scanner.skip(1);
                    DataSGroup& sgroup = (DataSGroup&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    int len = sgroup.data.size();
                    _scanner.appendLine(sgroup.data, true);

                    // Remove last spaces because "SED" means end of a paragraph
                    if (strncmp(chars, "SED", 3) == 0)
                    {
                        if (sgroup.data.top() == 0)
                            sgroup.data.pop();
                        while (sgroup.data.size() > len)
                        {
                            if (isspace((unsigned char)sgroup.data.top()))
                                sgroup.data.pop();
                            else
                                break;
                        }
                        // Add new paragraph. Last '\n' will be cleaned at the end
                        sgroup.data.push('\n');
                        if (sgroup.data.top() != 0)
                            sgroup.data.push(0);
                    }
                }
                else
                    _scanner.skipLine();
            }
            else if (strncmp(chars, "SMT", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;
                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SUP)
                {
                    _scanner.skip(1);
                    Superatom& sup = (Superatom&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    _scanner.readQuotedLine(sup.subscript, true);
                }
                else if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_MUL)
                {
                    _scanner.skip(1);
                    MultipleGroup& mg = (MultipleGroup&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    mg.multiplier = _scanner.readInt();
                    _scanner.skipLine();
                }
                else if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SRU)
                {
                    _scanner.skip(1);
                    RepeatingUnit& sru = (RepeatingUnit&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    _scanner.readQuotedLine(sru.subscript, true);
                }
                else
                    _scanner.skipLine();
            }
            else if (strncmp(chars, "SCL", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;
                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SUP)
                {
                    _scanner.skip(1);
                    Superatom& sup = (Superatom&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    _scanner.readLine(sup.sa_class, true);
                }
                else
                    _scanner.skipLine();
            }
            else if (strncmp(chars, "SBV", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;
                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SUP)
                {
                    Superatom& sup = (Superatom&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    Superatom::_BondConnection& bond = sup.bond_connections.push();
                    _scanner.skip(1);
                    bond.bond_idx = _scanner.readIntFix(3) - 1;
                    _scanner.skipSpace();
                    bond.bond_dir.x = _scanner.readFloat();
                    _scanner.skipSpace();
                    bond.bond_dir.y = _scanner.readFloat();
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SDS", 3) == 0)
            {
                _scanner.skip(1);
                char expanded[4] = {0, 0, 0, 0};

                _scanner.readCharsFix(3, expanded);

                if (strncmp(expanded, "EXP", 3) == 0)
                {
                    int n = _scanner.readIntFix(3);

                    while (n-- > 0)
                    {
                        _scanner.skip(1);
                        int sgroup_idx = _scanner.readIntFix(3) - 1;
                        if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SUP)
                        {
                            Superatom& sup = (Superatom&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                            sup.contracted = DisplayOption::Expanded;
                        }
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SPA", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;

                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_MUL)
                {
                    MultipleGroup& mg = (MultipleGroup&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    int n = _scanner.readIntFix(3);
                    while (n-- > 0)
                    {
                        _scanner.skip(1);
                        mg.parent_atoms.push(_scanner.readIntFix(3) - 1);
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SAP", 3) == 0)
            {
                _scanner.skip(1);
                int sgroup_idx = _scanner.readIntFix(3) - 1;

                if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SUP)
                {
                    Superatom& sup = (Superatom&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);
                    int n = _scanner.readIntFix(3);
                    while (n-- > 0)
                    {
                        int idap = sup.attachment_points.add();
                        Superatom::_AttachmentPoint& ap = sup.attachment_points.at(idap);
                        _scanner.skip(1);
                        ap.aidx = _scanner.readIntFix(3) - 1;
                        _scanner.skip(1);
                        ap.lvidx = _scanner.readIntFix(3) - 1;
                        _scanner.skip(1);
                        ap.apid.push(_scanner.readChar());
                        ap.apid.push(_scanner.readChar());
                        ap.apid.push(0);
                    }
                }
                _scanner.skipLine();
            }
            else if (strncmp(chars, "SCN", 3) == 0)
            {
                // The format is the following: M SCNnn8 sss ttt ...
                int n = _scanner.readIntFix(3);

                bool need_skip_line = true;
                while (n-- > 0)
                {
                    _scanner.skip(1);
                    int sgroup_idx = _scanner.readIntFix(3) - 1;

                    char id[4];
                    _scanner.skip(1);
                    _scanner.readCharsFix(3, id);

                    if (_sgroup_types[sgroup_idx] == SGroup::SG_TYPE_SRU)
                    {
                        RepeatingUnit& ru = (RepeatingUnit&)_bmol->sgroups.getSGroup(_sgroup_mapping[sgroup_idx]);

                        if (strncmp(id, "HH", 2) == 0)
                            ru.connectivity = SGroup::HEAD_TO_HEAD;
                        else if (strncmp(id, "HT", 2) == 0)
                            ru.connectivity = SGroup::HEAD_TO_TAIL;
                        else if (strncmp(id, "EU", 2) == 0)
                            ru.connectivity = SGroup::EITHER;
                        else
                        {
                            id[3] = 0;
                            throw Error("Undefined Sgroup connectivity: '%s'", id);
                        }
                    }
                    if (id[2] == '\n')
                    {
                        if (n != 0)
                            throw Error("Unexpected end of M SCN");
                        else
                            // In some molfiles last space is not written
                            need_skip_line = false;
                    }
                }
                if (need_skip_line)
                    _scanner.skipLine();
            }
            else if (strncmp(chars, "MRV", 3) == 0)
            {
                _scanner.readLine(str, false);
                BufferScanner rest(str);

                try
                {
                    rest.skip(1);
                    rest.readCharsFix(3, chars);
                    if (strncmp(chars, "SMA", 3) == 0)
                    {
                        // Marvin's "SMARTS in Molfile" extension
                        if (_qmol == 0)
                        {
                            if (!ignore_noncritical_query_features)
                                throw Error("SMARTS notation allowed only for query molecules");
                        }
                        else
                        {
                            rest.skip(1);
                            int idx = rest.readIntFix(3) - 1;
                            rest.skip(1);

                            QS_DEF(QueryMolecule, smartsmol);
                            SmilesLoader loader(rest);

                            smartsmol.clear();
                            loader.smarts_mode = true;
                            loader.loadQueryMolecule(smartsmol);

                            if (smartsmol.vertexCount() != 1)
                                throw Error("expected 1 atom in SMARTS expression, got %d", smartsmol.vertexCount());

                            _qmol->getAtom(idx).removeConstraints(QueryMolecule::ATOM_NUMBER);
                            _qmol->resetAtom(idx, QueryMolecule::Atom::und(_qmol->releaseAtom(idx), smartsmol.releaseAtom(smartsmol.vertexBegin())));
                        }
                    }
                }
                catch (Scanner::Error&)
                {
                }
            }
            else
                _scanner.skipLine();
        }
        else if (c == 'A')
        {
            QS_DEF(Array<char>, alias);

            // There should be 3 characters to the atom index, but some molfiles
            // has only 2 digits
            _scanner.skipSpace();
            int atom_idx = _scanner.readInt();

            atom_idx--;
            _scanner.skipLine();
            _scanner.readLine(alias, true);
            _preparePseudoAtomLabel(alias);

            if (_atom_types[atom_idx] == _ATOM_ELEMENT)
            {
                _bmol->setAlias(atom_idx, alias.ptr());
            }
            else
            {
                if (_mol != 0)
                    _mol->setPseudoAtom(atom_idx, alias.ptr());
                else
                    _qmol->resetAtom(atom_idx,
                                     QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, alias.ptr())));

                _atom_types[atom_idx] = _ATOM_PSEUDO;
            }
        }
        else if (c == '\n')
            continue;
        else
            _scanner.skipLine();
    }

    // Remove last new lines for data SGroups
    int sgroups_count = _bmol->sgroups.getSGroupCount();
    for (int i = 0; i < sgroups_count; i++)
    {
        SGroup& sgroup = _bmol->sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& dsg = (DataSGroup&)sgroup;
            if (dsg.data.size() > 2 && dsg.data.top(1) == '\n')
            {
                dsg.data.pop();
                dsg.data.top() = 0;
            }
        }
    }

    if (_qmol == 0)
        for (int atom_idx = 0; atom_idx < _atoms_num; atom_idx++)
            if (_atom_types[atom_idx] == _ATOM_A)
                throw Error("'any' atoms are allowed only for queries");

    _fillSGroupsParentIndices();
}

void MolfileLoader::_appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom)
{
    int atom_number = Element::fromString2(atom_label);
    std::unique_ptr<QueryMolecule::Atom> cur_atom;
    if (atom_number != -1)
        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, atom_number);
    else
        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, atom_label);

    if (atom.get() == 0)
        atom.reset(cur_atom.release());
    else
        atom.reset(QueryMolecule::Atom::oder(atom.release(), cur_atom.release()));
}

void MolfileLoader::_convertCharge(int value, int& charge, int& radical)
{
    switch (value)
    {
    case 1:
        charge = 3;
        break;
    case 2:
        charge = 2;
        break;
    case 3:
        charge = 1;
        break;
    case 4:
        radical = 2;
        break;
    case 5:
        charge = -1;
        break;
    case 6:
        charge = -2;
        break;
    case 7:
        charge = -3;
        break;
    }
}

void MolfileLoader::_read3dFeature2000()
{
    // read 3D feature ID (see MDL ctfile documentation)
    int feature_id = _scanner.readIntFix(3);

    _scanner.skipLine();

    Molecule3dConstraints* constraints = &_qmol->spatial_constraints;

    if (constraints->end() == 0)
        constraints->init();

    switch (feature_id)
    {
    case -1: // point defined by 2 points and distance
    {
        std::unique_ptr<Molecule3dConstraints::PointByDistance> constr = std::make_unique<Molecule3dConstraints::PointByDistance>();
        _scanner.skip(6);
        constr->beg_id = _scanner.readIntFix(3) - 1;
        constr->end_id = _scanner.readIntFix(3) - 1;
        constr->distance = _scanner.readFloatFix(10);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -2: // point defined by 2 points and percentage
    {
        std::unique_ptr<Molecule3dConstraints::PointByPercentage> constr = std::make_unique<Molecule3dConstraints::PointByPercentage>();
        _scanner.skip(6);
        constr->beg_id = _scanner.readIntFix(3) - 1;
        constr->end_id = _scanner.readIntFix(3) - 1;
        constr->percentage = _scanner.readFloatFix(10);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -3: // point defined by point, normal line, and distance
    {
        std::unique_ptr<Molecule3dConstraints::PointByNormale> constr = std::make_unique<Molecule3dConstraints::PointByNormale>();
        _scanner.skip(6);
        constr->org_id = _scanner.readIntFix(3) - 1;
        constr->norm_id = _scanner.readIntFix(3) - 1;
        constr->distance = _scanner.readFloatFix(10);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -4: // line defined by 2 or more points (best fit line if more than 2 points)
    {
        std::unique_ptr<Molecule3dConstraints::BestFitLine> constr = std::make_unique<Molecule3dConstraints::BestFitLine>();
        _scanner.skip(6);
        int amount = _scanner.readIntFix(3);
        if (amount < 2)
            throw Error("invalid points amount in M $3D-4 feature");

        constr->max_deviation = _scanner.readFloatFix(10);
        _scanner.skipLine();
        _scanner.skip(6);

        while (amount-- > 0)
            constr->point_ids.push(_scanner.readIntFix(3) - 1);

        _scanner.skipLine();
        constraints->add(constr.release());
        break;
    }
    case -5: // plane defined by 3 or more points (best fit line if more than 3 points)
    {
        std::unique_ptr<Molecule3dConstraints::BestFitPlane> constr = std::make_unique<Molecule3dConstraints::BestFitPlane>();
        _scanner.skip(6);

        int amount = _scanner.readIntFix(3);

        if (amount < 3)
            throw Error("invalid points amount in M $3D-5 feature");

        constr->max_deviation = _scanner.readFloatFix(10);
        _scanner.skipLine();
        _scanner.skip(6);

        while (amount-- > 0)
            constr->point_ids.push(_scanner.readIntFix(3) - 1);

        _scanner.skipLine();
        constraints->add(constr.release());
        break;
    }
    case -6: // plane defined by point and line
    {
        std::unique_ptr<Molecule3dConstraints::PlaneByPoint> constr = std::make_unique<Molecule3dConstraints::PlaneByPoint>();
        _scanner.skip(6);
        constr->point_id = _scanner.readIntFix(3) - 1;
        constr->line_id = _scanner.readIntFix(3) - 1;
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -7: // centroid defined by points
    {
        std::unique_ptr<Molecule3dConstraints::Centroid> constr = std::make_unique<Molecule3dConstraints::Centroid>();
        _scanner.skip(6);

        int amount = _scanner.readIntFix(3);

        if (amount < 1)
            throw Error("invalid amount of points for centroid: %d", amount);

        _scanner.skipLine();
        _scanner.skip(6);

        while (amount-- > 0)
            constr->point_ids.push(_scanner.readIntFix(3) - 1);

        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -8: // normal line defined by point and plane
    {
        std::unique_ptr<Molecule3dConstraints::Normale> constr = std::make_unique<Molecule3dConstraints::Normale>();
        _scanner.skip(6);
        constr->point_id = _scanner.readIntFix(3) - 1;
        constr->plane_id = _scanner.readIntFix(3) - 1;

        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -9: // distance defined by 2 points and range
    {
        std::unique_ptr<Molecule3dConstraints::DistanceByPoints> constr = std::make_unique<Molecule3dConstraints::DistanceByPoints>();
        _scanner.skip(6);
        constr->beg_id = _scanner.readIntFix(3) - 1;
        constr->end_id = _scanner.readIntFix(3) - 1;
        constr->bottom = _scanner.readFloatFix(10);
        constr->top = _scanner.readFloatFix(10);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -10: // distance defined by point, line and range
    {
        std::unique_ptr<Molecule3dConstraints::DistanceByLine> constr = std::make_unique<Molecule3dConstraints::DistanceByLine>();
        _scanner.skip(6);
        constr->point_id = _scanner.readIntFix(3) - 1;
        constr->line_id = _scanner.readIntFix(3) - 1;
        constr->bottom = _scanner.readFloatFix(10);
        constr->top = _scanner.readFloatFix(10);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -11: // distance defined by point, plane and range
    {
        std::unique_ptr<Molecule3dConstraints::DistanceByPlane> constr = std::make_unique<Molecule3dConstraints::DistanceByPlane>();
        _scanner.skip(6);
        constr->point_id = _scanner.readIntFix(3) - 1;
        constr->plane_id = _scanner.readIntFix(3) - 1;
        constr->bottom = _scanner.readFloatFix(10);
        constr->top = _scanner.readFloatFix(10);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -12: // angle defined by 3 points and range
    {
        std::unique_ptr<Molecule3dConstraints::AngleByPoints> constr = std::make_unique<Molecule3dConstraints::AngleByPoints>();
        _scanner.skip(6);
        constr->point1_id = _scanner.readIntFix(3) - 1;
        constr->point2_id = _scanner.readIntFix(3) - 1;
        constr->point3_id = _scanner.readIntFix(3) - 1;
        constr->bottom = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        constr->top = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        _scanner.skipLine();
        constraints->add(constr.release());
        break;
    }
    case -13: // angle defined by 2 lines and range
    {
        std::unique_ptr<Molecule3dConstraints::AngleByLines> constr = std::make_unique<Molecule3dConstraints::AngleByLines>();
        _scanner.skip(6);
        constr->line1_id = _scanner.readIntFix(3) - 1;
        constr->line2_id = _scanner.readIntFix(3) - 1;
        constr->bottom = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        constr->top = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        _scanner.skipLine();
        constraints->add(constr.release());
        break;
    }
    case -14: // angles defined by 2 planes and range
    {
        std::unique_ptr<Molecule3dConstraints::AngleByPlanes> constr = std::make_unique<Molecule3dConstraints::AngleByPlanes>();
        _scanner.skip(6);
        constr->plane1_id = _scanner.readIntFix(3) - 1;
        constr->plane2_id = _scanner.readIntFix(3) - 1;
        constr->bottom = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        constr->top = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        _scanner.skipLine();

        constraints->add(constr.release());
        break;
    }
    case -15: // dihedral angle defined by 4 points
    {
        std::unique_ptr<Molecule3dConstraints::AngleDihedral> constr = std::make_unique<Molecule3dConstraints::AngleDihedral>();
        _scanner.skip(6);
        constr->point1_id = _scanner.readIntFix(3) - 1;
        constr->point2_id = _scanner.readIntFix(3) - 1;
        constr->point3_id = _scanner.readIntFix(3) - 1;
        constr->point4_id = _scanner.readIntFix(3) - 1;
        constr->bottom = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        constr->top = (float)(_scanner.readFloatFix(10) * M_PI / 180);
        _scanner.skipLine();
        constraints->add(constr.release());
        break;
    }
    case -16: // exclusion sphere defines by points and distance
    {
        std::unique_ptr<Molecule3dConstraints::ExclusionSphere> constr = std::make_unique<Molecule3dConstraints::ExclusionSphere>();

        int allowed_atoms_amount;
        Array<int> allowed_atoms;
        _scanner.skip(6);
        constr->center_id = _scanner.readIntFix(3) - 1;
        constr->allow_unconnected = (_scanner.readIntFix(3) != 0);
        allowed_atoms_amount = _scanner.readIntFix(3);
        constr->radius = (float)(_scanner.readFloatFix(10));

        if (allowed_atoms_amount > 0)
        {
            _scanner.skipLine();
            _scanner.skip(6);

            while (allowed_atoms_amount-- > 0)
                constr->allowed_atoms.push(_scanner.readIntFix(3) - 1);
        }

        _scanner.skipLine();
        constraints->add(constr.release());
        break;
    }
    case -17: // fixed atoms
    {
        _scanner.skip(6);
        int amount = _scanner.readIntFix(3);
        _scanner.skipLine();
        _scanner.skip(6);

        while (amount-- > 0)
            _qmol->fixed_atoms.push(_scanner.readIntFix(3) - 1);

        _scanner.skipLine();
        break;
    }
    default:
        throw Error("unknown 3D feature in createFromMolfile: %d", feature_id);
    }
}

int MolfileLoader::_asc_cmp_cb(int& v1, int& v2, void* /*context*/)
{
    return v2 - v1;
}
