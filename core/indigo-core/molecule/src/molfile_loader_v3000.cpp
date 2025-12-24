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

void MolfileLoader::_readCtab3000()
{
    QS_DEF(Array<char>, str);

    _scanner.readLine(str, true);
    if (strncmp(str.ptr(), "M  V30 BEGIN CTAB", 17) != 0)
        throw Error("error reading CTAB block header");

    str.clear_resize(14);
    _scanner.read(14, str.ptr());
    if (strncmp(str.ptr(), "M  V30 COUNTS ", 14) != 0)
        throw Error("error reading COUNTS line");

    int i, nsgroups, n3d, chiral_int;

    _scanner.readLine(str, true);
    if (sscanf(str.ptr(), "%d %d %d %d %d", &_atoms_num, &_bonds_num, &nsgroups, &n3d, &chiral_int) < 5)
        throw Error("error parsing COUNTS line");

    _chiral = (chiral_int != 0);

    if (ignore_no_chiral_flag)
        _chiral = true;

    _init();

    bool atom_block_exists = true;
    bool bond_block_exists = true;

    _scanner.readLine(str, true);
    if (strncmp(str.ptr(), "M  V30 BEGIN ATOM", 14) != 0)
    {
        if (_atoms_num > 0)
            throw Error("Error reading ATOM block header");
        atom_block_exists = false;
    }
    else
    {
        for (i = 0; i < _atoms_num; i++)
        {
            _readMultiString(str);
            BufferScanner strscan(str.ptr());

            int& atom_type = _atom_types.push();

            _hcount.push(0);

            atom_type = _ATOM_ELEMENT;

            int isotope = 0;
            int label = 0;
            std::unique_ptr<QueryMolecule::Atom> query_atom;

            strscan.readInt1(); // atom index -- ignored

            QS_DEF(Array<char>, buf);

            strscan.readWord(buf, " [");

            char stopchar = strscan.readChar();
            if (stopchar == '[')
            {
                if (_qmol == 0)
                    throw Error("atom list is allowed only for queries");

                if (buf[0] == 0)
                    atom_type = _ATOM_LIST;
                else if (strcmp(buf.ptr(), "NOT") == 0)
                    atom_type = _ATOM_NOTLIST;
                else
                    throw Error("bad word: %s", buf.ptr());

                bool was_a = false, was_q = false, was_x = false, was_m = false;

                while (1)
                {
                    strscan.readWord(buf, ",]");
                    stopchar = strscan.readChar();

                    if (was_a)
                        throw Error("'A' inside atom list, if present, must be single");
                    if (was_q)
                        throw Error("'Q' inside atom list, if present, must be single");
                    if (was_x)
                        throw Error("'X' inside atom list, if present, must be single");
                    if (was_m)
                        throw Error("'M' inside atom list, if present, must be single");

                    if (buf.size() == 2 && buf[0] == '*')
                    {
                        was_a = true;
                        atom_type = _ATOM_STAR;
                    }
                    else if (buf.size() == 2 && buf[0] == 'A')
                    {
                        was_a = true;
                        atom_type = _ATOM_A;
                    }
                    else if (buf.size() == 3 && buf[0] == 'A' && buf[1] == 'H')
                    {
                        was_a = true;
                        atom_type = _ATOM_AH;
                    }
                    else if (buf.size() == 2 && buf[0] == 'Q')
                    {
                        was_q = true;
                        atom_type = _ATOM_Q;
                    }
                    else if (buf.size() == 3 && buf[0] == 'Q' && buf[1] == 'H')
                    {
                        was_a = true;
                        atom_type = _ATOM_QH;
                    }
                    else if (buf.size() == 2 && buf[0] == 'X')
                    {
                        was_q = true;
                        atom_type = _ATOM_X;
                    }
                    else if (buf.size() == 3 && buf[0] == 'X' && buf[1] == 'H')
                    {
                        was_a = true;
                        atom_type = _ATOM_XH;
                    }
                    else if (buf.size() == 2 && buf[0] == 'M')
                    {
                        was_q = true;
                        atom_type = _ATOM_M;
                    }
                    else if (buf.size() == 3 && buf[0] == 'M' && buf[1] == 'H')
                    {
                        was_a = true;
                        atom_type = _ATOM_MH;
                    }
                    else
                    {
                        _appendQueryAtom(buf.ptr(), query_atom);
                    }

                    if (stopchar == ']')
                        break;
                }
            }
            else
            {
                label = Element::fromString2(buf.ptr());
                long long cur_pos = strscan.tell();
                QS_DEF(ReusableObjArray<Array<char>>, strs);
                strs.clear();
                strs.push().readString("CLASS", false);
                strs.push().readString("SEQID", false);
                auto fw_res = strscan.findWord(strs);
                strscan.seek(cur_pos, SEEK_SET);
                if (fw_res != -1)
                    atom_type = _ATOM_TEMPLATE;
                else if (buf.size() == 2 && buf[0] == 'D')
                {
                    label = ELEM_H;
                    isotope = 2;
                }
                else if (buf.size() == 2 && buf[0] == 'T')
                {
                    label = ELEM_H;
                    isotope = 3;
                }
                else if (buf.size() == 2 && buf[0] == 'Q')
                {
                    if (_qmol == 0)
                        throw Error("'Q' atom is allowed only for queries");

                    atom_type = _ATOM_Q;
                }
                else if (buf.size() == 3 && buf[0] == 'Q' && buf[1] == 'H')
                {
                    if (_qmol == 0)
                        throw Error("'QH' atom is allowed only for queries");

                    atom_type = _ATOM_QH;
                }
                else if (buf.size() == 2 && buf[0] == 'A')
                {
                    if (_qmol == 0)
                        throw Error("'A' atom is allowed only for queries");

                    atom_type = _ATOM_A;
                }
                else if (buf.size() == 3 && buf[0] == 'A' && buf[1] == 'H')
                {
                    if (_qmol == 0)
                        throw Error("'AH' atom is allowed only for queries");

                    atom_type = _ATOM_AH;
                }
                else if (buf.size() == 2 && buf[0] == 'X' && !treat_x_as_pseudoatom)
                {
                    if (_qmol == 0)
                        throw Error("'X' atom is allowed only for queries");

                    atom_type = _ATOM_X;
                }
                else if (buf.size() == 3 && buf[0] == 'X' && buf[1] == 'H' && !treat_x_as_pseudoatom)
                {
                    if (_qmol == 0)
                        throw Error("'XH' atom is allowed only for queries");

                    atom_type = _ATOM_XH;
                }
                else if (buf.size() == 2 && buf[0] == 'M')
                {
                    if (_qmol == 0)
                        throw Error("'M' atom is allowed only for queries");

                    atom_type = _ATOM_M;
                }
                else if (buf.size() == 3 && buf[0] == 'M' && buf[1] == 'H')
                {
                    if (_qmol == 0)
                        throw Error("'MH' atom is allowed only for queries");

                    atom_type = _ATOM_MH;
                }
                else if (buf.size() == 3 && buf[0] == 'R' && buf[1] == '#')
                {
                    atom_type = _ATOM_R;
                    label = ELEM_RSITE;
                }
                else if (label == -1)
                    atom_type = _ATOM_PSEUDO;
            }

            strscan.skipSpace();
            float x = strscan.readFloat();
            strscan.skipSpace();
            float y = strscan.readFloat();
            strscan.skipSpace();
            float z = strscan.readFloat();
            strscan.skipSpace();
            int aamap = strscan.readInt1();

            if (_mol != 0)
            {
                if (atom_type == _ATOM_TEMPLATE)
                {
                    _preparePseudoAtomLabel(buf);
                    _mol->addTemplateAtom(buf.ptr());
                }
                else
                {
                    _mol->addAtom(label);
                    if (atom_type == _ATOM_PSEUDO)
                    {
                        _preparePseudoAtomLabel(buf);
                        _mol->setPseudoAtom(i, buf.ptr());
                    }
                }
            }
            else
            {
                if (atom_type == _ATOM_LIST)
                    _qmol->addAtom(query_atom.release());
                else if (atom_type == _ATOM_NOTLIST)
                    _qmol->addAtom(QueryMolecule::Atom::nicht(query_atom.release()));
                else if (atom_type == _ATOM_ELEMENT)
                    _qmol->addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, label));
                else if (atom_type == _ATOM_PSEUDO)
                    _qmol->addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, buf.ptr()));
                else if (atom_type == _ATOM_TEMPLATE)
                    _qmol->addTemplateAtom(buf.ptr());
                else if (atom_type == _ATOM_A)
                    _qmol->addAtom(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
                else if (atom_type == _ATOM_AH)
                {
                    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();
                    _qmol->addAtom(atom.release());
                }
                else if (atom_type == _ATOM_STAR)
                {
                    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();
                    atom->type = QueryMolecule::ATOM_STAR;
                    _qmol->addAtom(atom.release());
                }
                else if (atom_type == _ATOM_X)
                {
                    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();

                    atom->type = QueryMolecule::OP_OR;
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
                    _qmol->addAtom(atom.release());
                }
                else if (atom_type == _ATOM_XH)
                {
                    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();

                    atom->type = QueryMolecule::OP_OR;
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
                    atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H));
                    _qmol->addAtom(atom.release());
                }
                else if (atom_type == _ATOM_QH)
                    _qmol->addAtom(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                else if (atom_type == _ATOM_Q)
                    _qmol->addAtom(QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                            QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
                else if (atom_type == _ATOM_MH)
                {
                    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();

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

                    _qmol->addAtom(atom.release());
                }
                else if (atom_type == _ATOM_M)
                {
                    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();

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

                    _qmol->addAtom(atom.release());
                }
                else // _ATOM_R
                    _qmol->addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
            }

            // int hcount = 0;
            int irflag = 0;
            int ecflag = 0;
            int radical = 0;

            // read remaining atom properties
            while (true)
            {
                strscan.skipSpace();
                if (strscan.isEOF())
                    break;

                QS_DEF(Array<char>, prop_arr);
                strscan.readWord(prop_arr, "=");

                strscan.skip(1);
                const char* prop = prop_arr.ptr();

                if (strcmp(prop, "CHG") == 0)
                {
                    int charge = strscan.readInt1();

                    if (_mol != 0)
                        _mol->setAtomCharge_Silent(i, charge);
                    else
                    {
                        _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, charge)));
                    }
                }
                else if (strcmp(prop, "RAD") == 0)
                {
                    radical = strscan.readInt1();

                    if (_qmol != 0)
                    {
                        _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));
                    }
                }
                else if (strcmp(prop, "CFG") == 0)
                {
                    strscan.readInt1();
                    // int cfg = strscan.readInt1();

                    // if (cfg == 3)
                    //   _stereocenter_types[idx] = MoleculeStereocenters::ATOM4;
                }
                else if (strcmp(prop, "MASS") == 0)
                {
                    isotope = strscan.readInt1();
                }
                else if (strcmp(prop, "VAL") == 0)
                {
                    int valence = strscan.readInt1();

                    if (valence == -1)
                        valence = 0;

                    _bmol->setExplicitValence(i, valence);
                }
                else if (strcmp(prop, "HCOUNT") == 0)
                {
                    int hcount = strscan.readInt1();

                    if (_qmol == 0)
                    {
                        if (!ignore_noncritical_query_features)
                            throw Error("H count is allowed only for queries");
                    }

                    if (hcount == -1)
                        _hcount[i] = 1;
                    else if (hcount > 0)
                        _hcount[i] = hcount + 1; // to comply to the code in _postLoad()
                    else
                        throw Error("invalid HCOUNT value: %d", hcount);
                }
                else if (strcmp(prop, "STBOX") == 0)
                    _stereo_care_atoms[i] = strscan.readInt1();
                else if (strcmp(prop, "INVRET") == 0)
                    irflag = strscan.readInt1();
                else if (strcmp(prop, "EXACHG") == 0)
                    ecflag = strscan.readInt1();
                else if (strcmp(prop, "SUBST") == 0)
                {
                    if (_qmol == 0)
                        throw Error("substitution count is allowed only for queries");

                    int subst = strscan.readInt1();

                    if (subst != 0)
                    {
                        if (subst == -1)
                            _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, 0)));
                        else if (subst == -2)
                        {
                            _qmol->resetAtom(
                                i, QueryMolecule::Atom::und(_qmol->releaseAtom(i),
                                                            new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN, _qmol->getVertex(i).degree())));
                        }
                        else if (subst > 0)
                            _qmol->resetAtom(
                                i, QueryMolecule::Atom::und(
                                       _qmol->releaseAtom(i),
                                       new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, subst,
                                                               (subst < MolfileSaver::MAX_SUBSTITUTION_COUNT ? subst : QueryMolecule::MAX_ATOM_VALUE))));
                        else
                            throw Error("invalid SUBST value: %d", subst);
                    }
                }
                else if (strcmp(prop, "UNSAT") == 0)
                {
                    if (_qmol == 0)
                    {
                        if (!ignore_noncritical_query_features)
                            throw Error("unsaturation flag is allowed only for queries");
                    }
                    else
                    {
                        bool unsat = (strscan.readInt1() > 0);

                        if (unsat)
                            _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_UNSATURATION, 0)));
                    }
                }
                else if (strcmp(prop, "RBCNT") == 0)
                {
                    if (_qmol == 0)
                    {
                        if (!ignore_noncritical_query_features)
                            throw Error("ring bond count is allowed only for queries");
                    }
                    else
                    {
                        int rb = strscan.readInt1();
                        if (rb != 0)
                        {
                            if (rb == -1)
                                rb = 0;
                            else if (rb == -2)
                            {
                                int rbonds = 0;
                                const Vertex& vertex = _qmol->getVertex(i);

                                for (int k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
                                    if (_qmol->getEdgeTopology(vertex.neiEdge(k)) == TOPOLOGY_RING)
                                        rbonds++;

                                _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i),
                                                                             new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbonds)));
                            }
                            else if (rb > 1)
                                _qmol->resetAtom(
                                    i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, rb,
                                                                                                               (rb < 4 ? rb : QueryMolecule::MAX_ATOM_VALUE))));
                            else
                                throw Error("invalid RBCNT value: %d", rb);
                        }
                    }
                }
                else if (strcmp(prop, "RGROUPS") == 0)
                {
                    int n_rg;

                    strscan.skip(1); // skip '('
                    n_rg = strscan.readInt1();
                    while (n_rg-- > 0)
                        _bmol->allowRGroupOnRSite(i, strscan.readInt1());
                }
                else if (strcmp(prop, "ATTCHPT") == 0)
                {
                    int att_type = strscan.readInt1();

                    if (att_type == -1)
                        att_type = 3;

                    for (int att_idx = 0; (1 << att_idx) <= att_type; att_idx++)
                        if (att_type & (1 << att_idx))
                            _bmol->addAttachmentPoint(att_idx + 1, i);
                }
                else if (strcmp(prop, "ATTCHORD") == 0)
                {
                    int n_items, nei_idx, att_type;
                    QS_DEF(Array<char>, att_id);

                    strscan.skip(1); // skip '('
                    n_items = strscan.readInt1() / 2;
                    while (n_items-- > 0)
                    {
                        nei_idx = strscan.readInt1();
                        if (atom_type == _ATOM_R)
                        {
                            att_type = strscan.readInt1();
                            _bmol->setRSiteAttachmentOrder(i, nei_idx - 1, att_type - 1);
                        }
                        else
                        {
                            strscan.readWord(att_id, " )");
                            att_id.push(0);
                            _bmol->setTemplateAtomAttachmentOrder(i, nei_idx - 1, att_id.ptr());
                            strscan.skip(1); // skip stop character
                        }
                    }
                }
                else if (strcmp(prop, "CLASS") == 0)
                {
                    QS_DEF(Array<char>, temp_class);
                    strscan.readWord(temp_class, 0);
                    temp_class.push(0);
                    _bmol->setTemplateAtomClass(i, temp_class.ptr());
                }
                else if (strcmp(prop, "SEQID") == 0)
                {
                    int seq_id = strscan.readInt1();
                    _bmol->setTemplateAtomSeqid(i, seq_id);
                }
                else if (strcmp(prop, "SEQNAME") == 0)
                {
                    QS_DEF(Array<char>, seq_name);
                    strscan.readWord(seq_name, 0);
                    seq_name.push(0);
                }
                else
                {
                    throw Error("unsupported property of CTAB3000: %s", prop);
                }
            }

            if (isotope != 0)
            {
                if (_mol != 0)
                    _mol->setAtomIsotope(i, isotope);
                else
                    _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, isotope)));
            }

            if (_mol != 0)
                _mol->setAtomRadical(i, radical);

            _bmol->reaction_atom_inversion[i] = irflag;
            _bmol->reaction_atom_exact_change[i] = ecflag;
            _bmol->reaction_atom_mapping[i] = aamap;

            _bmol->setAtomXyz(i, x, y, z);
        }
        _scanner.readLine(str, true);
        if (strncmp(str.ptr(), "M  V30 END ATOM", 15) != 0)
            throw Error("Error reading ATOM block footer");
    }

    if (atom_block_exists)
        _scanner.readLine(str, true);
    if (strncmp(str.ptr(), "M  V30 BEGIN BOND", 17) != 0)
    {
        if (_bonds_num > 0)
            throw Error("Error reading BOND block header");
        bond_block_exists = false;
    }
    else
    {
        for (i = 0; i < _bonds_num; i++)
        {
            int reacting_center = 0;

            _readMultiString(str);
            BufferScanner strscan(str.ptr());

            strscan.readInt1(); // bond index -- ignored

            int order = strscan.readInt1();
            int beg = strscan.readInt1() - 1;
            int end = strscan.readInt1() - 1;

            if (_mol != 0)
            {
                if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC || order == _BOND_COORDINATION ||
                    order == _BOND_HYDROGEN)
                    _mol->addBond_Silent(beg, end, order);
                else if (order == _BOND_COORDINATION || order == _BOND_HYDROGEN)
                    _mol->addBond_Silent(beg, end, BOND_ZERO);
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
                _qmol->addBond(beg, end, QueryMolecule::createQueryMoleculeBond(order, 0, 0));
            }

            while (true)
            {
                strscan.skipSpace();
                if (strscan.isEOF())
                    break;

                QS_DEF(Array<char>, prop);

                strscan.readWord(prop, "=");
                strscan.skip(1);

                int n;

                if (strcmp(prop.ptr(), "CFG") == 0)
                {
                    n = strscan.readInt1();

                    if (n == 1)
                        _bmol->setBondDirection(i, BOND_UP);
                    else if (n == 3)
                        _bmol->setBondDirection(i, BOND_DOWN);
                    else if (n == 2)
                    {
                        int bond_order = _bmol->getBondOrder(i);
                        if (bond_order == BOND_SINGLE)
                            _bmol->setBondDirection(i, BOND_EITHER);
                        else if (bond_order == BOND_DOUBLE)
                            _ignore_cistrans[i] = 1;
                        else
                            throw Error("unknown bond CFG=%d for the bond order %d", n, bond_order);
                    }
                    else
                        throw Error("unknown bond CFG=%d", n);
                }
                else if (strcmp(prop.ptr(), "STBOX") == 0)
                {
                    if (_qmol == 0)
                        if (!ignore_noncritical_query_features)
                            throw Error("stereo care box is allowed only for queries");

                    if ((strscan.readInt1() != 0))
                        _stereo_care_bonds[i] = 1;
                }
                else if (strcmp(prop.ptr(), "TOPO") == 0)
                {
                    if (_qmol == 0)
                    {
                        if (!ignore_noncritical_query_features)
                            throw Error("bond topology setting is allowed only for queries");
                    }
                    else
                    {
                        int topo = strscan.readInt1();

                        _qmol->resetBond(
                            i, QueryMolecule::Bond::und(_qmol->releaseBond(i),
                                                        new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, topo == 1 ? TOPOLOGY_RING : TOPOLOGY_CHAIN)));
                    }
                }
                else if (strcmp(prop.ptr(), "RXCTR") == 0)
                    reacting_center = strscan.readInt1();
                else if (strcmp(prop.ptr(), "ENDPTS") == 0)
                {
                    strscan.skip(1); // (
                    n = strscan.readInt1();
                    while (n-- > 0)
                    {
                        strscan.readInt();
                        strscan.skipSpace();
                    }
                    strscan.skip(1); // )
                }
                else if (strcmp(prop.ptr(), "ATTACH") == 0)
                {
                    while (!strscan.isEOF())
                    {
                        char c = strscan.readChar();
                        if (c == ' ')
                            break;
                    }
                }
                else if (strcmp(prop.ptr(), "DISP") == 0)
                {
                    while (!strscan.isEOF())
                    {
                        char c = strscan.readChar();
                        if (c == ' ')
                            break;
                    }
                }
                else
                {
                    throw Error("unsupported property of CTAB3000 (in BOND block): %s", prop.ptr());
                }
            }
            _bmol->reaction_bond_reacting_center[i] = reacting_center;
        }

        _scanner.readLine(str, true);
        if (strncmp(str.ptr(), "M  V30 END BOND", 15) != 0)
            throw Error("Error reading BOND block footer");

        _scanner.readLine(str, true);
    }

    // Read collections and sgroups
    // There is no predefined order: sgroups may appear before collection
    bool collection_parsed = false, sgroups_parsed = false;
    while (strncmp(str.ptr(), "M  V30 END CTAB", 15) != 0)
    {
        if (strncmp(str.ptr(), "M  V30 BEGIN COLLECTION", 23) == 0)
        {
            if (collection_parsed)
                throw Error("COLLECTION block has already been parsed");
            _readCollectionBlock3000();
            collection_parsed = true;
        }
        else if (strncmp(str.ptr(), "M  V30 BEGIN SGROUP", 19) == 0)
        {
            if (sgroups_parsed)
                throw Error("SGROUP block has already been parsed");
            _readSGroupsBlock3000();
            sgroups_parsed = true;
        }
        else if (strncmp(str.ptr(), "M  V30 LINKNODE", 15) == 0)
            throw Error("link nodes are not supported yet (%s)", str.ptr());
        else
            throw Error("error reading CTAB block footer: %s", str.ptr());

        _scanner.readLine(str, true);
    }
}

void MolfileLoader::_readSGroupsBlock3000()
{
    QS_DEF(Array<char>, str);

    while (1)
    {
        _readMultiString(str);

        if (strncmp(str.ptr(), "END SGROUP", 10) == 0)
            break;
        if (STRCMP(str.ptr(), "M  V30 DEFAULT") == 0)
            continue;
        _readSGroup3000(str.ptr());
    }

    _fillSGroupsParentIndices();
}

void MolfileLoader::_fillSGroupsParentIndices()
{
    MoleculeSGroups& sgroups = _bmol->sgroups;

    std::multimap<int, int> indices;
    // original index can be arbitrary, sometimes key is used multiple times

    for (auto i = sgroups.begin(); i != sgroups.end(); i++)
    {
        SGroup& sgroup = sgroups.getSGroup(i);
        indices.emplace(sgroup.original_group, i);
    }

    // TODO: replace parent_group with parent_idx
    for (auto i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
    {
        SGroup& sgroup = sgroups.getSGroup(i);
        if (indices.count(sgroup.parent_group) == 1)
        {
            const auto it = indices.find(sgroup.parent_group);
            // TODO: check fix
            auto parent_idx = it->second;
            SGroup& parent_sgroup = sgroups.getSGroup(parent_idx);
            if (&sgroup != &parent_sgroup)
            {
                sgroup.parent_idx = it->second;
            }
            else
            {
                sgroup.parent_idx = -1;
            }
        }
        else
        {
            sgroup.parent_idx = -1;
        }
    }
}

void MolfileLoader::_readCollectionBlock3000()
{
    QS_DEF(Array<char>, str);

    while (1)
    {
        _readMultiString(str);

        if (strncmp(str.ptr(), "END COLLECTION", 14) == 0)
            break;

        BufferScanner strscan(str.ptr());
        char coll[14];

        strscan.readCharsFix(13, coll);
        coll[13] = 0;

        int stereo_type = 0;
        int stereo_group = 0;
        int n = 0;

        if (strcmp(coll, "MDLV30/STERAC") == 0)
            stereo_type = MoleculeStereocenters::ATOM_AND;
        else if (strcmp(coll, "MDLV30/STEREL") == 0)
            stereo_type = MoleculeStereocenters::ATOM_OR;
        else if (strcmp(coll, "MDLV30/STEABS") == 0)
            stereo_type = MoleculeStereocenters::ATOM_ABS;
        else if (strcmp(coll, "MDLV30/HILITE") == 0)
        {
            QS_DEF(Array<char>, what);

            strscan.skipSpace();
            strscan.readWord(what, " =");

            if (strcmp(what.ptr(), "ATOMS") == 0)
            {
                strscan.skip(2); // =(
                n = strscan.readInt1();
                while (n-- > 0)
                    _bmol->highlightAtom(strscan.readInt1() - 1);
            }
            else if (strcmp(what.ptr(), "BONDS") == 0)
            {
                strscan.skip(2); // =(
                n = strscan.readInt1();
                while (n-- > 0)
                    _bmol->highlightBond(strscan.readInt1() - 1);
            }
            else
                throw Error("unknown highlighted object: %s", what.ptr());

            continue;
        }
        else
        {
            _bmol->custom_collections.add(str);
            continue;
        }

        if (stereo_type == MoleculeStereocenters::ATOM_OR || stereo_type == MoleculeStereocenters::ATOM_AND)
            stereo_group = strscan.readInt1();
        else
            strscan.skip(1);

        strscan.skip(7); // ATOMS=(
        n = strscan.readInt1();
        while (n-- > 0)
        {
            int atom_idx = strscan.readInt1() - 1;

            _stereocenter_types[atom_idx] = stereo_type;
            _stereocenter_groups[atom_idx] = stereo_group;
        }
    }
}

void MolfileLoader::_preparePseudoAtomLabel(Array<char>& pseudo)
{
    // if the string is quoted, unquote it
    if (pseudo.size() > 2 && pseudo[0] == '\'' && pseudo[pseudo.size() - 2] == '\'')
    {
        pseudo.remove(pseudo.size() - 2);
        pseudo.remove(0);
    }

    if (pseudo.size() <= 1)
        throw Error("empty pseudo-atom");
}

void MolfileLoader::_readMultiString(Array<char>& str)
{
    QS_DEF(Array<char>, tmp);

    str.clear();
    tmp.clear_resize(7);

    while (1)
    {
        bool to_next = false;

        _scanner.read(7, tmp.ptr());
        if (strncmp(tmp.ptr(), "M  V30 ", 7) != 0)
            throw Error("error reading multi-string in CTAB v3000");

        _scanner.readLine(tmp, true);

        if (tmp[tmp.size() - 2] == '-')
        {
            tmp[tmp.size() - 2] = 0;
            tmp.pop();
            to_next = true;
        }
        str.appendString(tmp.ptr(), true);
        if (!to_next)
            break;
    }
}

void MolfileLoader::_readStringInQuotes(Scanner& scanner, Array<char>* str)
{
    char first = scanner.readChar();
    if (first == ' ')
        return;

    // Check if data is already present, then append to it using new line
    if (str && str->size() > 0)
    {
        if (str->top() == 0)
            str->pop();
        str->push('\n');
    }

    // If label is in quotes then read till the end quote
    bool in_quotes = (first == '"');
    if (!in_quotes && str)
        str->push(first);

    while (!scanner.isEOF())
    {
        char c = scanner.readChar();
        if (in_quotes)
        {
            // Break if other quote is reached
            if (c == '"')
                break;
        }
        else
        {
            // Break if space is reached
            if (isspace(c))
                break;
        }
        if (str)
            str->push(c);
    }
    if (str)
        str->push(0);
}

void MolfileLoader::_readRGroups3000()
{
    QS_DEF(Array<char>, str);

    MoleculeRGroups* rgroups = &_bmol->rgroups;

    while (!_scanner.isEOF())
    {
        long long next_block_pos = _scanner.tell();

        _scanner.readLine(str, true);

        if (strncmp(str.ptr(), "M  V30 BEGIN RGROUP", 19) == 0)
        {
            _rgfile = true;

            int rg_idx;

            if (sscanf(str.ptr(), "M  V30 BEGIN RGROUP %d", &rg_idx) != 1)
                throw Error("can not read rgroup index");

            RGroup& rgroup = rgroups->getRGroup(rg_idx);

            _readMultiString(str);

            BufferScanner strscan(str.ptr());

            if (strncmp(str.ptr(), "RLOGIC", 6) != 0)
                throw Error("Error reading RGROUP block");

            strscan.skip(7);
            rgroup.if_then = strscan.readInt1();
            rgroup.rest_h = strscan.readInt1();

            if (!strscan.isEOF())
            {
                QS_DEF(Array<char>, occ);

                strscan.readLine(occ, true);
                rgroup.occurrence.clear();
                rgroup.readOccurrence(occ.ptr());
            }

            while (!_scanner.isEOF())
            {
                long long pos = _scanner.tell();

                _scanner.readLine(str, true);
                if (strcmp(str.ptr(), "M  V30 BEGIN CTAB") == 0)
                {
                    _scanner.seek(pos, SEEK_SET);
                    std::unique_ptr<BaseMolecule> fragment(_bmol->neu());

                    MolfileLoader loader(_scanner);
                    loader._bmol = fragment.get();
                    if (_bmol->isQueryMolecule())
                    {
                        loader._qmol = &fragment.get()->asQueryMolecule();
                        loader._mol = 0;
                    }
                    else
                    {
                        loader._qmol = 0;
                        loader._mol = &fragment.get()->asMolecule();
                    }
                    loader._readCtab3000();
                    loader._postLoad();
                    rgroup.fragments.add(fragment.release());
                }
                else if (strcmp(str.ptr(), "M  V30 END RGROUP") == 0)
                    break;
                else
                    throw Error("unexpected string in rgroup: %s", str.ptr());
            }
        }
        else if ((strncmp(str.ptr(), "M  END", 6) == 0) || (strncmp(str.ptr(), "M  V30 BEGIN TEMPLATE", 21) == 0))
        {
            _scanner.seek(next_block_pos, SEEK_SET);
            break;
        }
        else
            throw Error("unexpected string in rgroup: %s", str.ptr());
    }
}

void MolfileLoader::_readSGroup3000(const char* str)
{
    BufferScanner scanner(str);
    QS_DEF(Array<char>, type);
    QS_DEF(Array<char>, entity);
    entity.clear();
    type.clear();

    MoleculeSGroups* sgroups = &_bmol->sgroups;

    scanner.skipSpace();
    int sgroup_idx = scanner.readInt();
    scanner.skipSpace();
    scanner.readWord(type, 0);
    type.push(0);
    scanner.skipSpace();
    scanner.readInt();
    scanner.skipSpace();

    int idx = sgroups->addSGroup(type.ptr());
    SGroup* sgroup = &sgroups->getSGroup(idx);
    sgroup->original_group = sgroup_idx;

    DataSGroup* dsg = 0;
    Superatom* sup = 0;
    RepeatingUnit* sru = 0;
    if (sgroup->sgroup_type == SGroup::SG_TYPE_DAT)
        dsg = (DataSGroup*)sgroup;
    else if (sgroup->sgroup_type == SGroup::SG_TYPE_SUP)
        sup = (Superatom*)sgroup;
    else if (sgroup->sgroup_type == SGroup::SG_TYPE_SRU)
        sru = (RepeatingUnit*)sgroup;

    int n;

    while (!scanner.isEOF())
    {
        scanner.readWord(entity, "=");
        if (scanner.isEOF())
            return;      // should not actually happen
        scanner.skip(1); // =
        entity.push(0);
        if (strcmp(entity.ptr(), "ATOMS") == 0)
        {
            scanner.skip(1); // (
            n = scanner.readInt1();
            while (n-- > 0)
            {
                sgroup->atoms.push(scanner.readInt() - 1);
                scanner.skipSpace();
            }
            scanner.skip(1); // )
        }
        else if ((strcmp(entity.ptr(), "XBONDS") == 0) || (strcmp(entity.ptr(), "CBONDS") == 0))
        {
            scanner.skip(1); // (
            n = scanner.readInt1();
            while (n-- > 0)
            {
                sgroup->bonds.push(scanner.readInt() - 1);
                scanner.skipSpace();
            }
            scanner.skip(1); // )
        }
        else if (strcmp(entity.ptr(), "PATOMS") == 0)
        {
            scanner.skip(1); // (
            n = scanner.readInt1();
            while (n-- > 0)
            {
                idx = scanner.readInt() - 1;

                if (sgroup->sgroup_type == SGroup::SG_TYPE_MUL)
                    ((MultipleGroup*)sgroup)->parent_atoms.push(idx);

                scanner.skipSpace();
            }
            scanner.skip(1); // )
        }
        else if (strcmp(entity.ptr(), "SUBTYPE") == 0)
        {
            QS_DEF(Array<char>, subtype);
            subtype.clear();
            scanner.readWord(subtype, 0);
            if (strcmp(subtype.ptr(), "ALT") == 0)
                sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_ALT;
            else if (strcmp(subtype.ptr(), "RAN") == 0)
                sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_RAN;
            else if (strcmp(subtype.ptr(), "BLO") == 0)
                sgroup->sgroup_subtype = SGroup::SG_SUBTYPE_BLO;
        }
        else if (strcmp(entity.ptr(), "MULT") == 0)
        {
            int mult = scanner.readInt();
            if (sgroup->sgroup_type == SGroup::SG_TYPE_MUL)
                ((MultipleGroup*)sgroup)->multiplier = mult;
        }
        else if (strcmp(entity.ptr(), "PARENT") == 0)
        {
            int parent = scanner.readInt();
            sgroup->parent_group = parent;
        }
        else if (strcmp(entity.ptr(), "BRKTYP") == 0)
        {
            QS_DEF(Array<char>, style);
            style.clear();
            scanner.readWord(style, 0);
            if (strcmp(style.ptr(), "BRACKET") == 0)
                sgroup->brk_style = _BRKTYP_SQUARE;
            if (strcmp(style.ptr(), "PAREN") == 0)
                sgroup->brk_style = _BRKTYP_ROUND;
        }
        else if (strcmp(entity.ptr(), "BRKXYZ") == 0)
        {
            scanner.skip(1); // (
            n = scanner.readInt1();
            if (n != 9)
                throw Error("BRKXYZ number is %d (must be 9)", n);

            scanner.skipSpace();
            float x1 = scanner.readFloat();
            scanner.skipSpace();
            float y1 = scanner.readFloat();
            scanner.skipSpace();
            scanner.readFloat();
            scanner.skipSpace(); // skip z
            float x2 = scanner.readFloat();
            scanner.skipSpace();
            float y2 = scanner.readFloat();
            scanner.skipSpace();
            scanner.readFloat();
            scanner.skipSpace(); // skip z
            // skip 3-rd point
            scanner.readFloat();
            scanner.skipSpace();
            scanner.readFloat();
            scanner.skipSpace();
            scanner.readFloat();
            scanner.skipSpace();
            Vec2f* brackets = sgroup->brackets.push();
            brackets[0].set(x1, y1);
            brackets[1].set(x2, y2);
            scanner.skip(1); // )
        }
        else if (strcmp(entity.ptr(), "CONNECT") == 0)
        {
            char c1 = scanner.readChar();
            char c2 = scanner.readChar();

            if (sgroup->sgroup_type == SGroup::SG_TYPE_SRU)
            {
                if (c1 == 'H' && c2 == 'T')
                    ((RepeatingUnit*)sgroup)->connectivity = SGroup::HEAD_TO_TAIL;
                else if (c1 == 'H' && c2 == 'H')
                    ((RepeatingUnit*)sgroup)->connectivity = SGroup::HEAD_TO_HEAD;
            }
        }
        else if (strcmp(entity.ptr(), "FIELDNAME") == 0)
        {
            _readStringInQuotes(scanner, dsg ? &dsg->name : NULL);
        }
        else if (strcmp(entity.ptr(), "FIELDINFO") == 0)
        {
            _readStringInQuotes(scanner, dsg ? &dsg->description : NULL);
        }
        else if (strcmp(entity.ptr(), "FIELDDISP") == 0)
        {
            QS_DEF(Array<char>, substr);
            substr.clear();
            _readStringInQuotes(scanner, &substr);
            if (substr.size() > 0)
                substr.pop(); // remove trailing 0
            if (dsg != 0)
            {
                BufferScanner subscan(substr);
                _readSGroupDisplay(subscan, *dsg);
            }
        }
        else if (strcmp(entity.ptr(), "FIELDDATA") == 0)
        {
            _readStringInQuotes(scanner, &dsg->data);
        }
        else if (strcmp(entity.ptr(), "QUERYTYPE") == 0)
        {
            _readStringInQuotes(scanner, dsg ? &dsg->querycode : NULL);
        }
        else if (strcmp(entity.ptr(), "QUERYOP") == 0)
        {
            _readStringInQuotes(scanner, dsg ? &dsg->queryoper : NULL);
        }
        else if (strcmp(entity.ptr(), "LABEL") == 0)
        {
            bool has_quote = false;
            while (!scanner.isEOF())
            {
                char c = scanner.readChar();
                if (c == '"')
                {
                    if (has_quote)
                        break;
                    else
                        has_quote = true;
                    continue;
                }
                if (c == ' ' && !has_quote)
                    break;
                if (sup != 0)
                    sup->subscript.push(c);
                if (sru != 0)
                    sru->subscript.push(c);
            }
            if (sup != 0)
                sup->subscript.push(0);
            if (sru != 0)
                sru->subscript.push(0);
        }
        else if (strcmp(entity.ptr(), "CLASS") == 0)
        {
            while (!scanner.isEOF())
            {
                char c = scanner.readChar();
                if (c == ' ')
                    break;
                if (sup != 0)
                    sup->sa_class.push(c);
            }
            if (sup != 0)
                sup->sa_class.push(0);
        }
        else if (strcmp(entity.ptr(), "SEQID") == 0)
        {
            int seqid = scanner.readInt();
            if ((sup != 0) && seqid > 0)
                sup->seqid = seqid;
        }
        else if (strcmp(entity.ptr(), "NATREPLACE") == 0)
        {
            while (!scanner.isEOF())
            {
                char c = scanner.readChar();
                if (c == ' ')
                    break;
                if (sup != 0)
                    sup->sa_natreplace.push(c);
                if (dsg != 0)
                    dsg->sa_natreplace.push(c);
            }
            if (sup != 0)
                sup->sa_natreplace.push(0);
            if (dsg != 0)
                dsg->sa_natreplace.push(0);
        }
        else if (strcmp(entity.ptr(), "ESTATE") == 0)
        {
            while (!scanner.isEOF())
            {
                char c = scanner.readChar();
                if (c == ' ')
                    break;
                if (c == 'E')
                {
                    if (sup != 0)
                        sup->contracted = DisplayOption::Expanded;
                }
                else
                {
                    if (sup != 0)
                        sup->contracted = DisplayOption::Undefined;
                }
            }
        }
        else if (strcmp(entity.ptr(), "CSTATE") == 0)
        {
            if (sup != 0)
            {
                scanner.skip(1); // (
                n = scanner.readInt1();
                if (n != 4)
                    throw Error("CSTATE number is %d (must be 4)", n);
                scanner.skipSpace();
                Superatom::_BondConnection& bond = sup->bond_connections.push();
                idx = scanner.readInt() - 1;
                bond.bond_idx = idx;
                scanner.skipSpace();
                bond.bond_dir.x = scanner.readFloat();
                scanner.skipSpace();
                bond.bond_dir.y = scanner.readFloat();
                scanner.skipSpace();

                scanner.skipUntil(")"); // Skip z coordinate
                scanner.skip(1);        // )
            }
            else // skip for all other sgroups
            {
                scanner.skipUntil(")");
                scanner.skip(1);
            }
        }
        else if (strcmp(entity.ptr(), "SAP") == 0)
        {
            if (sup != 0)
            {
                scanner.skip(1); // (
                n = scanner.readInt1();
                if (n != 3)
                    throw Error("SAP number is %d (must be 3)", n);
                scanner.skipSpace();
                idx = scanner.readInt() - 1;
                int idap = sup->attachment_points.add();
                Superatom::_AttachmentPoint& ap = sup->attachment_points.at(idap);
                ap.aidx = idx;
                scanner.skipSpace();
                ap.lvidx = scanner.readInt() - 1;
                scanner.skip(1);

                ap.apid.clear();

                while (!scanner.isEOF())
                {
                    char c = scanner.readChar();
                    if (c == ')')
                        break;
                    ap.apid.push(c);
                }
                ap.apid.push(0);
            }
            else // skip for all other sgroups
            {
                scanner.skipUntil(")");
                scanner.skip(1);
            }
        }
        else
        {
            if (scanner.lookNext() == '(')
            {
                scanner.skip(1);
                n = scanner.readInt1();
                while (n-- > 0)
                {
                    scanner.readFloat();
                    scanner.skipSpace();
                }
                scanner.skip(1); // )
            }
            else
            {
                // Unknown property that can have value in quotes: PROP="     "
                _readStringInQuotes(scanner, NULL);
            }
        }
        scanner.skipSpace();
    }
}

void MolfileLoader::_readTGroups3000()
{
    QS_DEF(Array<char>, str);

    MoleculeTGroups* tgroups = &_bmol->tgroups;

    while (!_scanner.isEOF())
    {
        long long next_block_pos = _scanner.tell();

        _scanner.readLine(str, true);

        if (strncmp(str.ptr(), "M  V30 BEGIN TEMPLATE", 21) == 0)
        {
            while (!_scanner.isEOF())
            {
                int tg_idx = 0;

                _readMultiString(str);

                if (strcmp(str.ptr(), "END TEMPLATE") == 0)
                    break;

                BufferScanner strscan(str.ptr());

                if (strncmp(str.ptr(), "TEMPLATE", 8) == 0)
                {
                    strscan.skip(8);
                    tg_idx = strscan.readInt1();
                }
                if (tg_idx == 0)
                    throw Error("can not read template index");

                int idx = tgroups->addTGroup();
                TGroup& tgroup = tgroups->getTGroup(idx);
                tgroup.tgroup_id = tg_idx;
                _max_template_id = std::max(_max_template_id, tg_idx);

                QS_DEF(Array<char>, word);
                strscan.skipSpace();
                strscan.readWord(word, " /");
                char stop_char = strscan.readChar();
                if (stop_char == '/')
                {
                    tgroup.tgroup_class.copy(word);
                    strscan.readWord(word, " /");
                    tgroup.tgroup_name.copy(word);

                    if (!strscan.isEOF())
                    {
                        stop_char = strscan.readChar();
                        if (stop_char == '/' && !strscan.isEOF())
                        {
                            strscan.readWord(word, " /");
                            tgroup.tgroup_alias.copy(word);
                            if (!strscan.isEOF()) // Skip stop char
                                strscan.skip(1);
                        }
                    }
                }
                else
                {
                    tgroup.tgroup_name.copy(word);
                }

                while (!strscan.isEOF())
                {
                    strscan.skipSpace();
                    strscan.readWord(word, "=");
                    strscan.skip(1); // =
                    word.push(0);

                    if (strcmp(word.ptr(), "COMMENT") == 0)
                    {
                        _readStringInQuotes(strscan, &tgroup.tgroup_comment);
                    }
                    else if (strcmp(word.ptr(), "NATREPLACE") == 0)
                    {
                        _readStringInQuotes(strscan, &tgroup.tgroup_natreplace);
                    }
                    else

                        if (strcmp(word.ptr(), "FULLNAME") == 0)
                    {
                        _readStringInQuotes(strscan, &tgroup.tgroup_full_name);
                    }
                }

                long long pos = _scanner.tell();
                _scanner.readLine(str, true);
                if (strcmp(str.ptr(), "M  V30 BEGIN CTAB") == 0)
                {
                    _scanner.seek(pos, SEEK_SET);
                    tgroup.fragment.reset(_bmol->neu());
                    MolfileLoader loader(_scanner);
                    loader._disable_sgroups_conversion = true;
                    loader.copyProperties(*this);
                    loader._bmol = tgroup.fragment.get();
                    if (_bmol->isQueryMolecule())
                    {
                        loader._qmol = &loader._bmol->asQueryMolecule();
                        loader._mol = 0;
                    }
                    else
                    {
                        loader._qmol = 0;
                        loader._mol = &loader._bmol->asMolecule();
                    }
                    loader._readCtab3000();
                    loader._postLoad();
                    if (_monomer_library != nullptr && tgroup.tgroup_class.size() > 0)
                    {
                        auto& class_map = MonomerTemplates::getStrToMonomerType();
                        auto class_it = class_map.find(tgroup.tgroup_class.ptr());
                        if (class_it != class_map.end())
                        {
                            // Search monomer library for monomer with same class and alias
                            std::string id = _monomer_library->getMonomerTemplateIdByAlias(class_it->second, tgroup.tgroup_name.ptr());
                            if (id.size() == 0 && tgroup.tgroup_alias.size() > 0)
                                id = _monomer_library->getMonomerTemplateIdByAlias(class_it->second, tgroup.tgroup_alias.ptr());
                            if (id.size() > 0)
                            {
                                // template with same class and alias found
                                // check inchi keys of structures
                                try
                                {
                                    const auto& templ = _monomer_library->getMonomerTemplateById(id);
                                    std::string templ_inchi_str;
                                    StringOutput templ_inchi_output(templ_inchi_str);
                                    MoleculeInChI templ_inchi(templ_inchi_output);
                                    auto templ_tgroup = templ.getTGroup();
                                    templ_inchi.outputInChI(templ_tgroup->fragment->asMolecule());
                                    std::string tg_inchi_str;
                                    StringOutput tg_inchi_output(tg_inchi_str);
                                    MoleculeInChI tg_inchi(tg_inchi_output);
                                    tg_inchi.outputInChI(tgroup.fragment->asMolecule());
                                    if (templ_inchi_str == tg_inchi_str)
                                    {
                                        std::string tgroup_name = tgroup.tgroup_name.ptr();
                                        auto tgroup_id = tgroup.tgroup_id;
                                        tgroup.copy_without_fragment(*templ_tgroup);
                                        // restore tgroup_name. we can't replace it with tgroup_name from library without updating template atoms'names.
                                        tgroup.tgroup_name.readString(tgroup_name.c_str(), true);
                                        tgroup.tgroup_id = tgroup_id;
                                        tgroup.tgroup_text_id.readString(id.c_str(), true);
                                    }
                                }
                                catch (...)
                                {
                                    // just do not copy on error
                                }
                            }
                        }
                    }
                }
                else
                    throw Error("unexpected string in template: %s", str.ptr());
            }
        }
        else
        {
            _scanner.seek(next_block_pos, SEEK_SET);
            break;
        }
    }
}

void MolfileLoader::_readSGroupDisplay(Scanner& scanner, DataSGroup& dsg)
{
    try
    {
        int constexpr MIN_SDD_SIZE = 36;
        bool well_formatted = scanner.length() >= MIN_SDD_SIZE;
        dsg.display_pos.x = scanner.readFloatFix(10);
        dsg.display_pos.y = scanner.readFloatFix(10);
        int ch = ' ';
        if (well_formatted)
        {
            scanner.skip(4);
            ch = scanner.readChar();
        }
        else
        {
            for (int i = 0; i < 5 && ch == ' '; i++)
                ch = scanner.readChar();
        }
        if (ch == 'A') // means "attached"
            dsg.detached = false;
        else if (ch == 'D')
            dsg.detached = true;
        else
            throw Error("Expected 'A' or 'D' but got '%c'.", ch);
        ch = scanner.readChar();
        if (ch == 'R')
            dsg.relative = true;
        else if (ch != 'A')
            throw Error("Expected 'A' or 'R' but got '%c'.", ch);
        ch = scanner.readChar();
        if (ch == 'U')
            dsg.display_units = true;
        else if (ch != ' ')
            throw Error("Expected 'U' or ' ' but got '%c'.", ch);

        if (well_formatted)
        {
            scanner.skip(3);
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                ch = scanner.lookNext();
                if (ch != ' ')
                    break;
                scanner.skip(1);
            }
        }

        long long cur = scanner.tell();

        char chars[4] = {0, 0, 0, 0};
        scanner.readCharsFix(3, chars);
        if (strncmp(chars, "ALL", 3) == 0)
            dsg.num_chars = 0;
        else
        {
            scanner.seek(cur, SEEK_CUR);
            dsg.num_chars = scanner.readInt1();
        }

        if (well_formatted)
        {
            scanner.skip(7);
            dsg.tag = scanner.readChar();
        }
        else
        {
            ch = ' ';
            // read kkk: Number of lines to display (unused, always 1)
            for (int i = 0; i < 3 && ch == ' '; i++)
                ch = scanner.readChar();
            ch = ' ';
            // read tag
            for (int i = 0; i < 5 && ch == ' '; i++)
                ch = scanner.readChar();
            if (ch != ' ')
                dsg.tag = ch;
        }

        if (scanner.lookNext() == '\n' || scanner.lookNext() == '\r')
            return;

        cur = scanner.tell();
        scanner.seek(0LL, SEEK_END);
        long long end = scanner.tell();
        scanner.seek(cur, SEEK_SET);

        if (end - cur + 1 > 2)
        {
            for (auto i = 0; i < 2; i++)
            {
                scanner.skip(1);
                if (scanner.lookNext() == '\n' || scanner.lookNext() == '\r')
                    return;
            }
            int c = scanner.readChar();
            if (c >= '1' && c <= '9')
                dsg.dasp_pos = c - '0';
        }
    }
    catch (Scanner::Error)
    {
        // Ignore scanner error - just use default values.
    }
}

void MolfileLoader::_init()
{
    _hcount.clear();
    _atom_types.clear();
    _sgroup_types.clear();
    _sgroup_mapping.clear();
    _stereo_care_atoms.clear_resize(_atoms_num);
    _stereo_care_atoms.zerofill();
    _stereo_care_bonds.clear_resize(_bonds_num);
    _stereo_care_bonds.zerofill();
    _stereocenter_types.clear_resize(_atoms_num);
    _stereocenter_types.zerofill();
    _stereocenter_groups.clear_resize(_atoms_num);
    _stereocenter_groups.zerofill();
    _sensible_bond_directions.clear_resize(_bonds_num);
    _sensible_bond_directions.zerofill();
    _ignore_cistrans.clear_resize(_bonds_num);
    _ignore_cistrans.zerofill();

    _stereo_care_bonds.clear_resize(_bonds_num);
    _stereo_care_bonds.zerofill();

    _bmol->reaction_atom_mapping.clear_resize(_atoms_num);
    _bmol->reaction_atom_mapping.zerofill();
    _bmol->reaction_atom_inversion.clear_resize(_atoms_num);
    _bmol->reaction_atom_inversion.zerofill();
    _bmol->reaction_atom_exact_change.clear_resize(_atoms_num);
    _bmol->reaction_atom_exact_change.zerofill();
    _bmol->reaction_bond_reacting_center.clear_resize(_bonds_num);
    _bmol->reaction_bond_reacting_center.zerofill();
}
