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

#include <cctype>
#include <memory>
#include <regex>
#include <unordered_set>

#include "base_cpp/scanner.h"
#include "graph/cycle_basis.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

IMPL_ERROR(SmilesLoader, "SMILES loader");

SmilesLoader::SmilesLoader(Scanner& scanner) : _scanner(scanner)
{
    ignorable_aam = 0;
    inside_rsmiles = false;
    ignore_closing_bond_direction_mismatch = false;
    ignore_cistrans_errors = false;
    ignore_bad_valence = false;
    ignore_no_chiral_flag = false;
    _mol = 0;
    _qmol = 0;
    _bmol = 0;
    smarts_mode = false;
    _balance = 0;
    _current_compno = 0;
    _inside_smarts_component = false;
}

SmilesLoader::~SmilesLoader()
{
    // clear pool-dependent data in this thread to avoid data races
    _atoms.clear();
}

void SmilesLoader::loadMolecule(Molecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = &mol;
    _qmol = 0;
    _has_atom_coordinates = false;
    _loadMolecule();

    mol.setIgnoreBadValenceFlag(ignore_bad_valence);
}

void SmilesLoader::loadQueryMolecule(QueryMolecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = 0;
    _qmol = &mol;
    _loadMolecule();
}

static char readSgChar(Scanner& scanner)
{
    char c = scanner.readChar();
    constexpr int min_ascii = 0;
    constexpr int max_ascii = 127;
    if (c == '&' && scanner.lookNext() == '#') // Escaped char - &#code;
    {
        long long pos = scanner.tell();
        scanner.skip(1); // Skip '#'
        int code = scanner.tryReadUnsigned();
        if (code >= min_ascii && code <= max_ascii && scanner.lookNext() == ';')
        {
            std::string sgroup_field_sep = ",;:|{}";
            // Decode only ,;:|{}
            if (sgroup_field_sep.find(code) != std::string::npos)
            {
                scanner.skip(1); // skip ';'
                return code;     // return decoded character
            }
        }
        // no decoded char returned - restore position and return '&' as is.
        scanner.seek(pos, SEEK_SET);
    }
    return c;
}

void SmilesLoader::_calcStereocenters()
{
    int i, j;

    for (i = 0; i < _atoms.size(); i++)
    {
        if (_atoms[i].chirality == 0)
            continue;

        if (_bmol->getVertex(i).degree() == 2) // allene stereo center
        {
            int subst[4];
            int subst2[4];
            int left, right;
            bool pure_h[4];

            if (!MoleculeAlleneStereo::possibleCenter(*_bmol, i, left, right, subst, pure_h))
            {
                if (!stereochemistry_options.ignore_errors)
                    throw Error("chirality on atom %d makes no sense", i);
                continue;
            }

            int parity = 3 - _atoms[i].chirality;

            for (j = 0; j < 4; j++)
                if (subst[j] == -1)
                    subst2[j] = -1;
                else
                    subst2[j] = subst[j];

            // Daylight doc says: Hydrogens attached to substituted allene-like atoms
            // are taken to be immediately following that atom
            if (subst2[1] == -1)
                subst2[1] = left;
            if (subst2[3] == -1)
                subst2[3] = right;

            if (subst2[1] < subst2[0])
            {
                std::swap(subst2[1], subst2[0]);
                parity = 3 - parity;
            }

            if (subst2[3] < subst2[2])
            {
                std::swap(subst2[3], subst2[2]);
                parity = 3 - parity;
            }

            // move hydrogens from [0] and [2] to [1] and [3] respectively
            if (pure_h[0])
            {
                if (subst[1] == -1)
                    throw Error("unexpected: subst[1] = -1");
                std::swap(subst[0], subst[1]);
                parity = 3 - parity;
            }
            if (pure_h[2])
            {
                if (subst[3] == -1)
                    throw Error("unexpected: subst[3] = -1");
                std::swap(subst[2], subst[3]);
                parity = 3 - parity;
            }

            _bmol->allene_stereo.add(i, left, right, subst, parity);
        }
        else // ordinary tetrahedral stereo center
        {
            int pyramid[4] = {-1, -1, -1, -1};
            int counter = 0;
            int h_index = -1;

            if (_atoms[i].parent != -1)
                pyramid[counter++] = _atoms[i].parent;

            if (_atoms[i].neighbors.size() == 3)
            {
                h_index = counter;
                pyramid[counter++] = -1;
            }

            for (j = _atoms[i].neighbors.begin(); j != _atoms[i].neighbors.end(); j = _atoms[i].neighbors.next(j))
            {
                int nei = _atoms[i].neighbors.at(j);

                if (counter >= 4)
                {
                    if (!stereochemistry_options.ignore_errors)
                        throw Error("too many bonds for chiral atom %d", i);
                    break;
                }

                if (nei != _atoms[i].parent)
                    pyramid[counter++] = nei;
            }

            if (j != _atoms[i].neighbors.end())
                continue;

            if (counter < 3)
            {
                if (!stereochemistry_options.ignore_errors)
                    throw Error("only %d bonds for chiral atom %d", counter, i);
                continue;
            }

            if (counter == 4)
            {
                j = pyramid[0];
                pyramid[0] = pyramid[1];
                pyramid[1] = pyramid[2];
                pyramid[2] = pyramid[3];
                pyramid[3] = j;

                if (h_index == 0)
                    h_index = 3;
                else if (h_index > 0)
                    h_index--;
            }

            if (h_index >= 0)
            {
                if (counter != 4)
                {
                    if (!stereochemistry_options.ignore_errors)
                        throw Error("implicit hydrogen not allowed with %d neighbor atoms", counter - 1);
                    continue;
                }

                bool parity = true;

                for (j = h_index; j < 3; j++)
                {
                    std::swap(pyramid[j], pyramid[j + 1]);
                    parity = !parity;
                }

                if (!parity)
                    std::swap(pyramid[0], pyramid[1]);
            }

            if (_atoms[i].chirality == 2)
                std::swap(pyramid[0], pyramid[1]);

            if (!_bmol->isPossibleStereocenter(i))
            {
                if (!stereochemistry_options.ignore_errors)
                    throw Error("chirality not possible on atom #%d", i);
                continue;
            }

            _bmol->addStereocenters(i, MoleculeStereocenters::ATOM_ABS, 0, pyramid);
        }
    }
}

void SmilesLoader::_calcCisTrans()
{
    QS_DEF(Array<int>, dirs);
    int i;

    dirs.clear();

    for (i = 0; i < _bonds.size(); i++)
        dirs.push(_bonds[i].dir);

    // there could be bonds added to stereocenters
    for (; i < _bmol->edgeEnd(); i++)
        dirs.push(0);

    _bmol->buildFromSmilesCisTrans(dirs.ptr());
    if (_qmol != 0)
    {
        for (i = 0; i < _bonds.size(); i++)
            if (_bmol->cis_trans.getParity(i) != 0)
                _qmol->setBondStereoCare(i, true);
    }
}

void SmilesLoader::_readOtherStuff()
{
    MoleculeCisTrans& cis_trans = _bmol->cis_trans;

    QS_DEF(Array<int>, to_remove);

    std::unordered_set<int> _overtly_defined_abs;

    to_remove.clear();

    while (1)
    {
        char c = _scanner.readChar();

        if (c == '|')
            break;

        if (c == 'w') // 'ANY' stereocenters
        {
            char wmode = 0;
            if (_scanner.lookNext() == 'U')
                wmode = 'U';
            if (_scanner.lookNext() == 'D')
                wmode = 'D';

            if (wmode)
                _scanner.skip(1);

            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'w%c'", wmode);

            while (isdigit(_scanner.lookNext()))
            {
                int atom_idx = _scanner.readUnsigned();
                // handle wiggly bonds
                if (!wmode)
                {
                    // This either bond can mark stereocenter or cis-trans double bond
                    // For example CC=CN |w:1.0|
                    const Vertex& v = _bmol->getVertex(atom_idx);
                    bool found = false;
                    for (int nei : v.neighbors())
                    {
                        int edge_idx = v.neiEdge(nei);
                        if (_bmol->getBondOrder(edge_idx) == BOND_DOUBLE && _bmol->getBondTopology(edge_idx) != TOPOLOGY_RING)
                        {
                            cis_trans.ignore(edge_idx);
                            found = true;
                        }
                    }

                    if (!found)
                    {
                        if (_bmol->isPossibleStereocenter(atom_idx))
                        {
                            // Check if the stereocenter has already been marked as any
                            // For example [H]C1(O)c2ccnn2[C@@H](O)c2ccnn12 |r,w:1.0,1.1|
                            if (!_bmol->stereocenters.exists(atom_idx))
                                _bmol->addStereocenters(atom_idx, MoleculeStereocenters::ATOM_ANY, 0, false);
                        }
                    }
                }

                if (_scanner.lookNext() == '.')
                {
                    _scanner.skip(1);
                    auto bond_idx = _scanner.readUnsigned();
                    if (!_has_directions_on_rings)
                        _has_directions_on_rings = _bmol->getBondTopology(bond_idx) == TOPOLOGY_RING;
                    if (bond_idx < _bmol->edgeCount() && atom_idx < _bmol->vertexCount())
                    {
                        auto& v = _bmol->getEdge(bond_idx);
                        if (v.end == atom_idx)
                            _bmol->swapEdgeEnds(bond_idx);

                        if (v.beg == atom_idx)
                            _bmol->setBondDirection(bond_idx, wmode == 'U' ? BOND_UP : (wmode == 'D' ? BOND_DOWN : BOND_EITHER));
                    }
                }

                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == 'a') // 'ABS' stereocenters
        {
            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'a'");

            while (isdigit(_scanner.lookNext()))
            {
                int idx = _scanner.readUnsigned();

                if (_bmol->stereocenters.exists(idx))
                {
                    _bmol->stereocenters.setType(idx, MoleculeStereocenters::ATOM_ABS, 0);
                }
                else
                {
                    _bmol->addStereocenters(idx, MoleculeStereocenters::ATOM_ABS, 0, false);
                    _bmol->stereocenters.setTetrahydral(idx, false);
                }
                _overtly_defined_abs.insert(idx);

                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == 'o') // 'OR' stereocenters
        {
            int groupno = _scanner.readUnsigned();

            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'o'");

            while (isdigit(_scanner.lookNext()))
            {
                int idx = _scanner.readUnsigned();

                if (_bmol->stereocenters.exists(idx))
                    _bmol->stereocenters.setType(idx, MoleculeStereocenters::ATOM_OR, groupno);
                else
                {
                    _bmol->addStereocenters(idx, MoleculeStereocenters::ATOM_OR, groupno, false);
                    _bmol->stereocenters.setTetrahydral(idx, false);
                }

                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == '&') // 'AND' stereocenters
        {
            int groupno = _scanner.readUnsigned();

            if (_scanner.readChar() != ':')
                throw Error("colon expected after '&'");

            while (isdigit(_scanner.lookNext()))
            {
                int idx = _scanner.readUnsigned();
                if (_bmol->stereocenters.exists(idx))
                    _bmol->stereocenters.setType(idx, MoleculeStereocenters::ATOM_AND, groupno);
                else
                {
                    _bmol->addStereocenters(idx, MoleculeStereocenters::ATOM_AND, groupno, false);
                    _bmol->stereocenters.setTetrahydral(idx, false);
                }
                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == '^') // radicals
        {
            int rad = _scanner.readIntFix(1);
            int radical;

            if (rad == 1)
                radical = RADICAL_DOUBLET;
            else if (rad == 3)
                radical = RADICAL_SINGLET;
            else if (rad == 4)
                radical = RADICAL_TRIPLET;
            else
                throw Error("unsupported radical number: %d", rad);

            if (_scanner.readChar() != ':')
                throw Error("colon expected after radical number");

            while (isdigit(_scanner.lookNext()))
            {
                int idx = _scanner.readUnsigned();

                if (_mol != 0)
                    _mol->setAtomRadical(idx, radical);
                else
                    _qmol->resetAtom(idx, QueryMolecule::Atom::und(_qmol->releaseAtom(idx), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));

                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == '$') // pseudoatoms
        {
            QS_DEF(Array<char>, label);

            for (int i = _bmol->vertexBegin(); i != _bmol->vertexEnd(); i = _bmol->vertexNext(i))
            {
                label.clear();

                while (1)
                {
                    if (_scanner.isEOF())
                        throw Error("end of input while reading $...$ block");
                    c = _scanner.readChar();
                    if (c == ';' || c == '$')
                        break;
                    label.push(c);
                }
                if (c == '$' && i != _bmol->vertexEnd() - 1)
                    throw Error("only %d atoms found in pseudo-atoms $...$ block", i + 1);
                if (c == ';' && i == _bmol->vertexEnd() - 1)
                    throw Error("extra ';' in pseudo-atoms $...$ block");

                if (label.size() > 0)
                {
                    label.push(0);
                    int rnum;

                    if (label.size() > 3 && strncmp(label.ptr(), "_R", 2) == 0 && sscanf(label.ptr() + 2, "%d", &rnum) == 1)
                    {
                        // ChemAxon's Extended SMILES notation for R-sites
                        if (_qmol != 0)
                            _qmol->resetAtom(i, new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
                        _bmol->allowRGroupOnRSite(i, rnum);

                        // check multiple R-sites notation
                        BufferScanner strscan(label.ptr());
                        QS_DEF(Array<char>, word);
                        while (!strscan.isEOF())
                        {
                            strscan.skip(1);
                            strscan.readWord(word, ",;");
                            if (word.size() >= 3 && strncmp(word.ptr(), "_R", 2) == 0 && sscanf(word.ptr() + 2, "%d", &rnum) == 1)
                                _bmol->allowRGroupOnRSite(i, rnum);
                        }
                    }
                    else if (label.size() > 4 && strncmp(label.ptr(), "_AP", 3) == 0 && sscanf(label.ptr() + 3, "%d", &rnum) == 1)
                    {
                        // That is ChemAxon's Extended SMILES notation for attachment
                        // points. We mark the atom for removal and place attachment point
                        // markers on its neighbors.
                        int k;
                        const Vertex& v = _bmol->getVertex(i);

                        for (k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                            _bmol->addAttachmentPoint(rnum, v.neiVertex(k));
                        to_remove.push(i);
                    }
                    else
                    {
                        // That is ChemAxon's Extended SMILES notation for pseudoatoms and
                        // special atoms A,Q,X,M and AH,QH,XH,MH
                        if (label.size() > 3 &&
                            (strncmp(label.ptr() + label.size() - 3, "_p", 2) == 0 || strncmp(label.ptr() + label.size() - 3, "_e", 2) == 0))
                        {
                            label.pop();
                            label.pop();
                            label.pop();
                            label.push(0);
                        }

                        if (_mol != 0)
                        {
                            const auto atomNumber = _mol->getAtomNumber(i);
                            if (ELEM_MIN < atomNumber && atomNumber < ELEM_MAX)
                            {
                                _mol->setAlias(i, label.ptr());
                            }
                            else
                            {
                                _mol->setPseudoAtom(i, label.ptr());
                            }
                        }
                        else
                        {
                            if (label.size() == 2 && label[0] == 'Q')
                            {
                                std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                _qmol->resetAtom(
                                    i, QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                                QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
                            }
                            else if (label.size() == 3 && label[0] == 'Q' && label[1] == 'H')
                            {
                                std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                _qmol->resetAtom(i, QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                            }
                            else if (label.size() == 3 && label[0] == 'A' && label[1] == 'H')
                            {
                                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                                x_atom->type = QueryMolecule::OP_NONE;
                                _qmol->resetAtom(i, x_atom.release());
                            }
                            else if (label.size() == 2 && label[0] == 'X')
                            {
                                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                                x_atom->type = QueryMolecule::OP_OR;
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));

                                std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                _qmol->resetAtom(i, x_atom.release());
                            }
                            else if (label.size() == 3 && label[0] == 'X' && label[1] == 'H')
                            {
                                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                                x_atom->type = QueryMolecule::OP_OR;
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
                                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H));

                                std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                _qmol->resetAtom(i, x_atom.release());
                            }
                            else if (label.size() == 2 && label[0] == 'M')
                            {
                                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                                x_atom->type = QueryMolecule::OP_AND;
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));

                                std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                _qmol->resetAtom(i, x_atom.release());
                            }
                            else if (label.size() == 3 && label[0] == 'M' && label[1] == 'H')
                            {
                                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                                x_atom->type = QueryMolecule::OP_AND;
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
                                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));

                                std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                _qmol->resetAtom(i, x_atom.release());
                            }
                            else
                            {
                                const auto atomNumber = _qmol->getAtomNumber(i);
                                if (ELEM_MIN < atomNumber && atomNumber < ELEM_MAX)
                                {
                                    _qmol->setAlias(i, label.ptr());
                                }
                                else
                                {
                                    std::unique_ptr<QueryMolecule::Atom> atom(_qmol->releaseAtom(i));
                                    atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                                    _qmol->resetAtom(
                                        i, QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, label.ptr())));
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (c == 'c' || c == 't') // CIS and TRANS bonds
        {
            if (_scanner.readChar() != ':')
                throw Error("colon expected after '%c' identifier", c);

            while (isdigit(_scanner.lookNext()))
            {
                int idx = _scanner.readUnsigned();

                bool skip = false;
                if (ignore_cistrans_errors && !MoleculeCisTrans::isGeomStereoBond(*_bmol, _bonds[idx].index, nullptr, false))
                    skip = true;

                if (!skip)
                {
                    _bmol->restoreSubstituents(_bonds[idx].index);
                    const int* subst = _bmol->cis_trans.getSubstituents(_bonds[idx].index);
                    int parity = ((c == 'c') ? MoleculeCisTrans::CIS : MoleculeCisTrans::TRANS);

                    /* CXSmiles doc says:
                       the double bond has the representation a1-a2=a3-a4, where
                       a1 is the smallest atom index of the generated smiles connected to a2
                       a2 is the double bond smaller atom index in the generated smiles
                       a3 is the double bond larger atom index in the generated smiles
                       a4 is the smallest atom index of the generated smiles connected to a3

                     * We need to know if the calculated substituents' indices are not "smallest"
                     * (i.e. they have other substituent with smaller index on the same side).
                     * In that case, we invert the parity.
                     */

                    if (subst[1] != -1 && subst[1] < subst[0])
                        parity = 3 - parity;
                    if (subst[3] != -1 && subst[3] < subst[2])
                        parity = 3 - parity;

                    _bmol->cis_trans.setParity(_bonds[idx].index, parity);
                }
                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == '(') // atom coordinates
        {
            for (int i = _bmol->vertexBegin(); i != _bmol->vertexEnd(); i = _bmol->vertexNext(i))
            {
                float x, y, z = 0;

                x = _scanner.readFloat();
                if (_scanner.readChar() != ',')
                    throw Error("expected comma after X coordinate");

                y = _scanner.readFloat();
                if (_scanner.lookNext() != ';' && _scanner.lookNext() != ')')
                {
                    if (_scanner.readChar() != ',')
                        throw Error("expected comma after Y coordinate");
                    if (_scanner.lookNext() == ';')
                        _scanner.skip(1);
                    else if (_scanner.lookNext() == ')')
                        ;
                    else
                        z = _scanner.readFloat();
                }
                else
                {
                    _scanner.skip(1);
                    if (_scanner.readChar() != ';')
                        throw Error("expected ';' after coordinates");
                }

                _bmol->setAtomXyz(i, x, y, z);
            }
            if (_scanner.readChar() != ')')
                throw Error("expected ')' after coordinates");
            _has_atom_coordinates = true;
        }
        else if (c == 'h') // highlighting (Indigo's own extension)
        {
            c = _scanner.readChar();

            int a = false;

            if (c == 'a')
                a = true;
            else if (c != 'b')
                throw Error("expected 'a' or 'b' after 'h', got '%c'", c);

            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'h%c'", a ? 'a' : 'b');

            while (isdigit(_scanner.lookNext()))
            {
                int idx = _scanner.readUnsigned();

                if (a)
                    _bmol->highlightAtom(idx);
                else
                    _bmol->highlightBond(idx);

                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == 'r')
        {
            if (_scanner.lookNext() == 'b')
            {
                if (_qmol == 0)
                    throw Error("'rb' is allowed only within queries");
                _scanner.skip(1);
                if (_scanner.readChar() != ':')
                    throw Error("colon expected after 'rb' identifier");
                while (isdigit(_scanner.lookNext()))
                {
                    // remove 'x' or 'r' configured ATOM_RING_BONDS
                    int atom_idx = _scanner.readUnsigned();
                    QueryMolecule::Atom& atom = _qmol->getAtom(atom_idx);
                    if (atom.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
                        atom.removeConstraints(QueryMolecule::ATOM_RING_BONDS);

                    if (_scanner.readChar() != ':')
                        throw Error("colon expected after 'rb:n'");
                    if (_scanner.lookNext() == '*')
                    {
                        _scanner.skip(1);
                        int rbonds = 0;
                        const Vertex& vertex = _qmol->getVertex(atom_idx);
                        for (int k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
                            if (_qmol->getEdgeTopology(vertex.neiEdge(k)) == TOPOLOGY_RING)
                                rbonds++;

                        _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx),
                                                                            new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbonds)));
                    }
                    else
                    {
                        int rbcount = _scanner.readUnsigned();
                        if (rbcount)
                        {
                            _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(
                                                           _qmol->releaseAtom(atom_idx),
                                                           new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, rbcount, (rbcount < 4 ? rbcount : 100))));
                        }
                        else
                            _qmol->resetAtom(
                                atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 0)));
                    }
                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else
            {
                // All stereocenters are relative instead of abs
                MoleculeStereocenters& s = _bmol->stereocenters;
                for (int i = s.begin(); i != s.end(); i = s.next(i))
                {
                    int atom = s.getAtomIndex(i);
                    if (s.getType(atom) == MoleculeStereocenters::ATOM_ABS && !ignore_no_chiral_flag &&
                        _overtly_defined_abs.find(atom) == _overtly_defined_abs.end())
                        s.setType(atom, MoleculeStereocenters::ATOM_AND, 1);
                }
            }
        }
        else if ((c == 'S') && (_scanner.lookNext() == 'g'))
        {
            // SGroup block found
            _scanner.skip(1);
            int sg_type = -1;
            // Data S-group - 'SgD:atomic_indexes:field_name:data_value:query_op:unit:tag:(coords)'
            // Optional coordinates in parenthesis if necessary, separated by colon characters.
            // The field values with special characters are escaped.
            // If atomic coordinates are exported (with option c ) (-1) is used in the coordinate field for Data S-group attached to the atoms.
            if (_scanner.lookNext() == 'D')
            {
                _scanner.skip(1);
                sg_type = SGroup::SG_TYPE_DAT;
            }
            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'Sg'");

            // If not a data S-group - get group type after colon
            //
            // 'Sg:type:atomic_indexes:subscript:superscript:head_bond_indexes:tail_bond_indexes:bracket
            //
            // atomic_indexes - Atom indexes separated with commas
            // subscript - Subscript of the S-group. If the subscript equals the keyword of the S-group this field can be empty. Escaped field.
            // superscript - Superscript of the S-group. Only connectivity and flip information is allowed. This field can be empty. Escaped field.
            // *_bond_indexes - The indexes of bonds that share a common bracket in case of ladder-type polymers.
            // head_bond_indexes - Head crossing bond indexes. This field can be empty.
            // tail_bond_indexes - Tail crossing bond indexes. This field can be empty.
            // bracket - bracket orientation, bracket type followed by the coordinates (4 pair, separated with commas). Bracket orientation
            //     can be s or d (single or double), bracket type can be b,c,r,s for braces, chevrons, round and square, respectively.
            //     The brackets are written between parentheses and separated with semicolons.
            if (sg_type == -1)
            {
                char sg = _scanner.lookNext();
                constexpr size_t sg_type_max_len = 3;
                char pchar_sg_type[sg_type_max_len];
                std::string sg_type_str;
                if (sg == 'n')
                {
                    sg_type = SGroup::SG_TYPE_SRU;
                    _scanner.skip(1);
                    if (_scanner.readChar() != ':')
                        throw Error("colon expected after 'Sg:n'");
                }
                else if (sg == 'g')
                {
                    _scanner.readCharsFix(sizeof(pchar_sg_type), pchar_sg_type);
                    sg_type_str = std::string(pchar_sg_type, sizeof(pchar_sg_type));
                    if (sg_type_str == "gen")
                    {
                        if (_scanner.readChar() != ':')
                            throw Error("colon expected after 'Sg:%s'", sg_type_str.c_str());
                        sg_type = SGroup::SG_TYPE_GEN;
                    }
                    else
                        throw Error("unexpected 'Sg' %s", sg_type_str.c_str());
                }
                else
                {
                    throw Error("Unsupported Sg type");
                }
            }

            int idx = _bmol->sgroups.addSGroup(sg_type);
            auto& sgroup = _bmol->sgroups.getSGroup(idx);

            // add brackets
            Vec2f* p = sgroup.brackets.push();
            p[0].set(0, 0);
            p[1].set(0, 0);
            p = sgroup.brackets.push();
            p[0].set(0, 0);
            p[1].set(0, 0);

            while (isdigit(_scanner.lookNext()))
            {
                auto atom_idx = _scanner.readUnsigned();
                sgroup.atoms.push(atom_idx);
                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }

            if (_scanner.lookNext() != ':')
                continue;

            _scanner.skip(1); // skip ':'
            const char* word_delimiter = ":,|";

            if (sg_type == SGroup::SG_TYPE_DAT)
            {
                char c;
                DataSGroup& dsg = static_cast<DataSGroup&>(sgroup);
                // field_name
                _scanner.readWord(dsg.name, word_delimiter);
                if (_scanner.lookNext() != ':') // No more fields
                    continue;
                _scanner.skip(1); // Skip :
                // data_value
                _scanner.readWord(dsg.data, word_delimiter);
                if (_scanner.lookNext() != ':') // No more fields
                    continue;
                _scanner.skip(1); // Skip :
                // query_op
                _scanner.readWord(dsg.queryoper, word_delimiter);
                if (_scanner.lookNext() != ':') // No more fields
                    continue;
                _scanner.skip(1); // Skip :
                // unit
                _scanner.readWord(dsg.description, word_delimiter);
                if (_scanner.lookNext() != ':') // No more fields
                    continue;
                _scanner.skip(1); // Skip :
                // tag
                c = _scanner.lookNext();
                if (c != ':' && c != ',')
                {
                    dsg.tag = c;
                    _scanner.skip(1); // Skip tag
                }
                if (_scanner.lookNext() != ':') // No more fields
                    continue;
                _scanner.skip(1); // Skip :
                // (coords)
                c = _scanner.lookNext();
                if (c != '(') // No more fields
                    continue;
                long long pos = _scanner.tell();
                constexpr char minus1[] = "(-1)";
                constexpr size_t minus1_len = sizeof(minus1) - 1;
                if (_scanner.length() - pos >= minus1_len)
                {
                    // check for (-1)
                    char buf[minus1_len];
                    _scanner.read(minus1_len, buf);
                    if (strncmp(buf, minus1, sizeof(buf)) == 0)
                        continue;
                    _scanner.seek(pos, SEEK_SET);
                }
                _scanner.skip(1); // Skip (
                dsg.display_pos.x = _scanner.readFloat();
                c = _scanner.readChar();
                if (c != ',')
                    throw Error("Data S-group coord error");
                dsg.display_pos.y = _scanner.readFloat();
                c = _scanner.readChar();
                if (c != ')')
                    throw Error("Data S-group coord error");
            }
            else
            {
                QS_DEF(Array<char>, subscript);
                QS_DEF(Array<char>, conn_arr);
                std::string connectivity, flip;
                subscript.clear();
                conn_arr.clear();
                _scanner.readWord(subscript, word_delimiter);
                if (_scanner.lookNext() == ':')
                {
                    _scanner.skip(1);
                    _scanner.readWord(conn_arr, word_delimiter);
                    if (conn_arr.find('#') >= 0)
                    {
                        // Possible encoded symbols. Try to decode
                        BufferScanner word_scan{conn_arr};
                        while (!word_scan.isEOF())
                            connectivity += readSgChar(word_scan);
                    }
                    else
                    {
                        connectivity = conn_arr.ptr();
                    }
                    // If ',' in field - it is both connectivity and flip
                    std::size_t pos = connectivity.find(',');
                    if (pos != std::string::npos)
                    {
                        flip = connectivity.substr(pos + 1);
                        connectivity = connectivity.substr(0, pos);
                    }
                }

                // Set fields for SRU S-Group
                if (sg_type == SGroup::SG_TYPE_SRU)
                {
                    RepeatingUnit& ru = static_cast<RepeatingUnit&>(sgroup);
                    if (subscript.size())
                        ru.subscript.readString(subscript.ptr(), true);
                    if (connectivity == "ht")
                        ru.connectivity = RepeatingUnit::HEAD_TO_TAIL;
                    else if (connectivity == "hh")
                        ru.connectivity = RepeatingUnit::HEAD_TO_HEAD;
                    else if (connectivity == "eu")
                        ru.connectivity = RepeatingUnit::EITHER;
                }

                if (_scanner.lookNext() != ':')
                    continue;
                _scanner.skip(1); // skip :
                // head_bond_indexes - Head crossing bond indexes. This field can be empty.
                while (isdigit(_scanner.lookNext()))
                {
                    auto atom_idx = _scanner.readUnsigned();
                    // no support for now
                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
                if (_scanner.lookNext() != ':')
                    continue;
                _scanner.skip(1); // skip :
                // tail_bond_indexes - Tail crossing bond indexes. This field can be empty.
                while (isdigit(_scanner.lookNext()))
                {
                    auto atom_idx = _scanner.readUnsigned();
                    // no support for now
                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
                if (_scanner.lookNext() != ':')
                    continue;
                _scanner.skip(1); // skip :
                // bracket - bracket orientation, bracket type followed by the coordinates (4 pair, separated with commas).
                if (_scanner.lookNext() != '(')
                    continue;
                _scanner.skip(1); // skip (
                char br_orient = _scanner.readChar();
                c = _scanner.readChar();
                if (c != ',')
                    throw Error("S-group bracket orientation format error");
                char br_type = _scanner.readChar();
                c = _scanner.readChar();
                int count = 0;
                constexpr int bracket_coord_count = 8;
                while (c == ',' && count < bracket_coord_count)
                {
                    float tmp = _scanner.readFloat();
                    c = _scanner.readChar();
                    ++count;
                }
                if (count < bracket_coord_count)
                    throw Error("S-group bracket orientation format error");
                if (c == ',')
                    c = _scanner.readChar();
                if (c != ')')
                    throw Error("S-group bracket orientation format error");
            }
        }
        else if ((c == 'R') && (_scanner.lookNext() == 'G'))
        {
            // RGroup block found
            _scanner.skip(1);

            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'RG'");

            MoleculeRGroups* rgroups = &_bmol->rgroups;
            QS_DEF(Array<char>, label);

            while (1)
            {
                if ((_scanner.lookNext() == '_') || (_scanner.lookNext() == 'L'))
                    label.clear();
                else if (_scanner.lookNext() == '|')
                    break;

                while (1)
                {
                    if (_scanner.isEOF())
                        throw Error("end of input while reading RG block");
                    c = _scanner.readChar();
                    if (c == '=')
                        break;
                    label.push(c);
                }

                if (label.size() > 0)
                {
                    label.push(0);
                    int rnum;

                    if (label.size() > 3 && strncmp(label.ptr(), "_R", 2) == 0 && sscanf(label.ptr() + 2, "%d", &rnum) == 1)
                    {
                        // RGroup description found
                        QS_DEF(Array<char>, rgdesc);
                        RGroup& rgroup = rgroups->getRGroup(rnum);

                        while (1)
                        {
                            if (_scanner.isEOF())
                                throw Error("end of input while reading RG block");

                            if (_scanner.lookNext() == '{')
                            {
                                _scanner.skip(1);
                                _scanner.readWord(rgdesc, "}");
                                _scanner.skip(1);
                            }
                            else if (_scanner.lookNext() == ',')
                            {
                                _scanner.skip(1);
                                continue;
                            }
                            else if ((_scanner.lookNext() == '_') || (_scanner.lookNext() == 'L') || (_scanner.lookNext() == '|'))
                            {
                                break;
                            }
                            else
                            {
                                _scanner.skip(1);
                                continue;
                            }

                            if (rgdesc.size() > 0)
                            {
                                rgdesc.pop();

                                std::unique_ptr<BaseMolecule> fragment(_bmol->neu());
                                BufferScanner rg_scanner(rgdesc);
                                SmilesLoader rg_loader(rg_scanner);

                                if (_bmol->isQueryMolecule())
                                {
                                    rg_loader.loadQueryMolecule(fragment.get()->asQueryMolecule());
                                }
                                else
                                {
                                    rg_loader.loadMolecule(fragment.get()->asMolecule());
                                }

                                rgroup.fragments.add(fragment.release());
                            }
                        }
                    }
                    else if (label.size() > 3 && strncmp(label.ptr(), "LOG", 3) == 0)
                    {
                        // RGroup logic block found
                        while (1)
                        {
                            label.clear();
                            if ((_scanner.lookNext() == '{') || (_scanner.lookNext() == '_'))
                            {
                                if (_scanner.lookNext() == '{')
                                    _scanner.skip(1);

                                while (1)
                                {
                                    if (_scanner.isEOF())
                                        throw Error("end of input while reading LOG block");
                                    c = _scanner.readChar();
                                    if (c == ':')
                                        break;
                                    label.push(c);
                                }
                            }
                            else if (_scanner.lookNext() == '}')
                            {
                                _scanner.skip(1);
                                break;
                            }
                            else
                                break;

                            if (label.size() > 0)
                            {
                                label.push(0);
                                int rnum;

                                if (label.size() > 3 && strncmp(label.ptr(), "_R", 2) == 0 && sscanf(label.ptr() + 2, "%d", &rnum) == 1)
                                {
                                    RGroup& rgroup = rgroups->getRGroup(rnum);

                                    int if_then = 0;
                                    int rest_h = 0;
                                    QS_DEF(Array<char>, occurrence_str);

                                    if (_scanner.lookNext() == '_')
                                    {
                                        label.clear();
                                        while (1)
                                        {
                                            if (_scanner.isEOF())
                                                throw Error("end of input while reading LOG block");
                                            c = _scanner.lookNext();
                                            if (c == ';')
                                                break;
                                            label.push(c);
                                            _scanner.skip(1);
                                        }
                                        label.push(0);

                                        if (label.size() > 3 && strncmp(label.ptr(), "_R", 2) == 0 && sscanf(label.ptr() + 2, "%d", &rnum) == 1)
                                        {
                                            if_then = rnum;
                                        }
                                    }

                                    rgroup.if_then = if_then;

                                    if (_scanner.lookNext() == ';')
                                    {
                                        _scanner.skip(1);
                                        if (_scanner.lookNext() == 'H')
                                        {
                                            rest_h = 1;
                                            _scanner.skip(1);
                                        }
                                    }

                                    rgroup.rest_h = rest_h;

                                    if (_scanner.lookNext() == ';')
                                    {
                                        _scanner.skip(1);
                                        if (_scanner.lookNext() == '.')
                                        {
                                            _scanner.skip(1);
                                            break;
                                        }
                                    }

                                    _scanner.readWord(occurrence_str, ".}");
                                    _readRGroupOccurrenceRanges(occurrence_str.ptr(), rgroup.occurrence);

                                    _scanner.skip(1);
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (c == 'u')
        {
            if (_qmol == 0)
                throw Error("'u' is allowed only within queries");
            if (_scanner.readChar() != ':')
                throw Error("colon expected after 'u' identifier");
            while (isdigit(_scanner.lookNext()))
            {
                int atom_idx = _scanner.readUnsigned();
                _qmol->resetAtom(atom_idx,
                                 QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_UNSATURATION, 0)));
                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
        else if (c == 's')
        {
            if (_qmol == 0)
                throw Error("'s' is allowed only within queries");
            if (_scanner.readChar() != ':')
                throw Error("colon expected after 's' identifier");
            while (isdigit(_scanner.lookNext()))
            {
                int atom_idx = _scanner.readUnsigned();
                if (_scanner.readChar() != ':')
                    throw Error("colon expected after 's:n'");
                int subs = -2;
                if (_scanner.lookNext() == '*')
                {
                    _scanner.skip(1);
                }
                else
                {
                    subs = _scanner.readUnsigned();
                    if (!subs)
                        subs = -1;
                }

                QueryMolecule::Atom& atom = _qmol->getAtom(atom_idx);
                // remove what was set with 'D'
                if (atom.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS))
                    atom.removeConstraints(QueryMolecule::ATOM_SUBSTITUENTS);

                switch (subs)
                {
                case -1:
                    _qmol->resetAtom(atom_idx,
                                     QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, 0)));
                    break;
                case -2:
                    _qmol->resetAtom(atom_idx,
                                     QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN,
                                                                                                                    _qmol->getVertex(atom_idx).degree())));
                    break;
                default:
                    _qmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_qmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS,
                                                                                                                              subs, (subs < 6 ? subs : 100))));
                    break;
                }
                if (_scanner.lookNext() == ',')
                    _scanner.skip(1);
            }
        }
    }

    if (to_remove.size() > 0)
        _bmol->removeAtoms(to_remove);
}

void SmilesLoader::_validateStereoCenters()
{
    for (int i = _bmol->stereocenters.begin(); i < _bmol->stereocenters.end(); i = _bmol->stereocenters.next(i))
    {
        auto atom_idx = _bmol->stereocenters.getAtomIndex(i);
        if (_bmol->isPossibleStereocenter(atom_idx) || _bmol->stereocenters.isAtropisomeric(atom_idx))
            continue;
        if (!stereochemistry_options.ignore_errors)
            throw Error("atom %d is not a stereocenter", atom_idx);
    }
}

void SmilesLoader::loadSMARTS(QueryMolecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = 0;
    _qmol = &mol;
    smarts_mode = true;
    _loadMolecule();
}

void SmilesLoader::_parseMolecule()
{
    _cycles.clear();
    _atom_stack.clear();
    _pending_bonds_pool.clear();

    bool first_atom = true;
    bool inside_polymer = false;

    while (!_scanner.isEOF())
    {
        int next = _scanner.lookNext();

        if ((isspace(next)) || next == '|')
            break;

        _BondDesc* bond = 0;
        bool added_bond = false;

        if (!first_atom)
        {
            while (isdigit(next) || next == '%')
            {
                int number;

                _scanner.skip(1);
                if (next == '%')
                    number = _scanner.readIntFix(2);
                else
                    number = next - '0';

                while (_cycles.size() <= number)
                    _cycles.push().clear();

                // closing some previously numbered atom, like the last '1' in c1ccccc1
                if (_cycles[number].beg >= 0)
                {
                    _bonds.push(_BondDesc{});
                    bond = &_bonds.top();
                    bond->beg = _atom_stack.top();
                    bond->end = _cycles[number].beg;
                    _cycles[number].clear();
                    added_bond = true;

                    if (_qmol != 0)
                        bond->index = _qmol->addBond(bond->beg, bond->end, new QueryMolecule::Bond());

                    _atoms[bond->beg].neighbors.add(bond->end);
                    _atoms[bond->end].closure(number, bond->beg);

                    break;
                }
                // closing some previous pending bond, like the last '1' in C-1=CC=CC=C1'
                else if (_cycles[number].pending_bond >= 0)
                {
                    bond = &_bonds[_cycles[number].pending_bond];
                    bond->end = _atom_stack.top();
                    added_bond = true;
                    _atoms[bond->end].neighbors.add(bond->beg);
                    _atoms[bond->beg].closure(number, bond->end);

                    if (_qmol != 0)
                    {
                        QS_DEF(Array<char>, bond_str);
                        std::unique_ptr<QueryMolecule::Bond> qbond = std::make_unique<QueryMolecule::Bond>();

                        bond_str.readString(_pending_bonds_pool.at(_cycles[number].pending_bond_str), false);
                        _readBond(bond_str, *bond, qbond);
                        bond->index = _qmol->addBond(bond->beg, bond->end, qbond.release());
                    }

                    _cycles[number].clear();

                    break;
                }
                // opening new cycle, like the first '1' in c1ccccc1
                else
                {
                    _cycles[number].beg = _atom_stack.top();
                    _cycles[number].pending_bond = -1;
                    _atoms[_cycles[number].beg].pending(number);
                }
                next = _scanner.lookNext();
            }
            if (added_bond)
                continue;
        }

        if (next == '.')
        {
            _scanner.skip(1);

            if (smarts_mode && _balance == 0)
            {
                _inside_smarts_component = false;
                _atom_stack.clear(); // needed to detect errors like "C.(C)(C)"
            }
            else
            {
                if (_atom_stack.size() < 1)
                    ; // we allow misplaced dots because we are so kind
                else
                    _atom_stack.pop();
            }
            first_atom = true;
            continue;
        }

        if (next == '(')
        {
            _scanner.skip(1);

            if (smarts_mode && first_atom)
            {
                if (_balance > 0)
                    throw Error("hierarchical component-level grouping is not allowed");
                _current_compno++;
                _inside_smarts_component = true;
            }
            else
            {
                if (_atom_stack.size() < 1)
                    throw Error("probably misplaced '('");
                _atom_stack.push(_atom_stack.top());
            }

            _balance++;
            continue;
        }

        if (next == ')')
        {
            _scanner.skip(1);

            if (_balance <= 0)
                throw Error("unexpected ')'");

            _balance--;

            _atom_stack.pop();

            continue;
        }

        if (!first_atom)
        {
            _bonds.push(_BondDesc{});
            bond = &_bonds.top();
            bond->beg = _atom_stack.top();
            added_bond = true;
        }

        std::unique_ptr<QueryMolecule::Bond> qbond;

        if (added_bond)
        {
            QS_DEF(Array<char>, bond_str);

            bond_str.clear();
            while (strchr("-=#:@!;,&~?/\\", next) != NULL)
            {
                bond_str.push(_scanner.readChar());
                next = _scanner.lookNext();
            }

            if (_qmol != 0)
                qbond = std::make_unique<QueryMolecule::Bond>();

            // empty bond designator?
            if (bond_str.size() < 1)
            {
                // In SMARTS mode a missing bond symbol is interpreted as "single or aromatic".
                // (http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html)
                // But if both atoms cannot be aromatic, then empty bond can be just
                // replaced to single.
                // Such case is processed after
            }
            else
                _readBond(bond_str, *bond, qbond);

            // The bond "directions" are already saved in _BondDesc::dir,
            // so we can safely discard them. We are doing that to succeed
            // the later check 'pending bond vs. closing bond'.
            {
                int i;

                for (i = 0; i < bond_str.size(); i++)
                    if (bond_str[i] == '/' || bond_str[i] == '\\')
                    {
                        // Aromatic bonds can be a part of cis-trans configuration
                        // For example in Cn1c2ccccc2c(-c2ccccc2)n/c(=N\O)c1=O or O\N=c1/c(=O)c2ccccc2c(=O)/c/1=N\O
                        if (_atoms[bond->beg].aromatic && bond_str.size() == 1)
                        {
                            // Erase bond type info
                            bond_str[i] = '?';
                            bond->type = -1; // single of aromatic
                        }
                        else
                            bond_str[i] = '-';
                    }
            }

            if (bond_str.size() > 0)
            {
                if (isdigit(next) || next == '%')
                {
                    int number;
                    _scanner.skip(1);

                    if (next == '%')
                        number = _scanner.readIntFix(2);
                    else
                        number = next - '0';

                    // closing some previous numbered atom, like the last '1' in C1C=CC=CC=1
                    if (number >= 0 && number < _cycles.size() && _cycles[number].beg >= 0)
                    {
                        bond->end = _cycles[number].beg;

                        if (_qmol != 0)
                            bond->index = _qmol->addBond(bond->beg, bond->end, qbond.release());

                        _atoms[bond->end].closure(number, bond->beg);
                        _atoms[bond->beg].neighbors.add(bond->end);

                        _cycles[number].clear();
                        continue;
                    }
                    // closing some previous pending cycle bond, like the last '1' in C=1C=CC=CC=1
                    else if (number >= 0 && number < _cycles.size() && _cycles[number].pending_bond >= 0)
                    {
                        int pending_bond_idx = _cycles[number].pending_bond;
                        _BondDesc& pending_bond = _bonds[pending_bond_idx];

                        // transfer direction from closing bond to pending bond
                        if (bond->dir > 0)
                        {
                            if (bond->dir == pending_bond.dir)
                            {
                                if (!ignore_closing_bond_direction_mismatch)
                                    throw Error("cycle %d: closing bond direction does not match pending bond direction", number);
                            }
                            else
                                pending_bond.dir = 3 - bond->dir;
                        }

                        // apart from the direction, check that the closing bond matches the pending bond
                        const char* str = _pending_bonds_pool.at(_cycles[number].pending_bond_str);

                        if (bond_str.size() > 0)
                        {
                            if (!ignore_closing_bond_direction_mismatch)
                            {
                                if ((int)strlen(str) != bond_str.size() || memcmp(str, bond_str.ptr(), strlen(str)) != 0)
                                    throw Error("cycle %d: closing bond description %.*s does not match pending bond description %s", number, bond_str.size(),
                                                bond_str.ptr(), str);
                            }
                        }
                        else
                        {
                            bond_str.readString(str, false);
                            _readBond(bond_str, *bond, qbond);
                        }

                        if (_qmol != 0)
                            pending_bond.index = _qmol->addBond(pending_bond.beg, bond->beg, qbond.release());

                        pending_bond.end = bond->beg;
                        _atoms[pending_bond.end].neighbors.add(pending_bond.beg);
                        _atoms[pending_bond.beg].closure(number, pending_bond.end);

                        // forget the closing bond but move its index here
                        // Bond order should correspons to atoms order
                        // Bond order for the following two molecules should be the same
                        // because later we add cis constraint:
                        //    CCSc1nnc2c(OC=Nc3ccccc-23)n1 |c:9|
                        //    CCSc1nnc-2c(OC=Nc3ccccc-23)n1 |c:9|
                        // Without this shift the 9th bond in the second structure is not double
                        _bonds.top() = pending_bond;
                        _bonds.remove(pending_bond_idx);

                        _cycles[number].clear();
                        continue;
                    }
                    // opening some pending cycle bond, like the first '1' in C=1C=CC=CC=1
                    else
                    {
                        while (_cycles.size() <= number)
                            _cycles.push().clear();
                        _cycles[number].pending_bond = _bonds.size() - 1;
                        _cycles[number].pending_bond_str = _pending_bonds_pool.add(bond_str);
                        _cycles[number].beg = -1; // have it already in the bond
                        _atoms[bond->beg].pending(number);

                        continue;
                    }
                }
            }
        }

        _AtomDesc& atom = _atoms.push(_neipool);

        std::unique_ptr<QueryMolecule::Atom> qatom;

        if (_qmol != 0)
            qatom = std::make_unique<QueryMolecule::Atom>();

        if (added_bond)
            bond->end = _atoms.size() - 1;

        QS_DEF(Array<char>, atom_str);

        atom_str.clear();

        bool brackets = false;

        if (next == '[')
        {
            _scanner.skip(1);
            int cnt = 1;

            while (1)
            {
                if (_scanner.isEOF())
                    throw Error("'[' without a ']'");
                char c = _scanner.readChar();
                if (c == '[')
                    cnt++;
                else if (c == ']')
                {
                    cnt--;
                    if (cnt == 0)
                        break;
                }
                atom_str.push(c);
            }
            brackets = true;
        }
        else if (next == -1)
            throw Error("unexpected end of input");
        else
        {
            _scanner.skip(1);
            atom_str.push(next);
            if (next == 'B' && _scanner.lookNext() == 'r')
                atom_str.push(_scanner.readChar());
            else if (next == 'C' && _scanner.lookNext() == 'l')
                atom_str.push(_scanner.readChar());
        }

        _readAtom(atom_str, brackets, atom, qatom);
        atom.brackets = brackets;

        if (_qmol != 0)
        {
            _qmol->addAtom(qatom.release());

            if (added_bond)
                bond->index = _qmol->addBond(bond->beg, bond->end, qbond.release());
        }

        if (added_bond)
        {
            _atoms[bond->beg].neighbors.add(bond->end);
            _atoms[bond->end].neighbors.add(bond->beg);
            _atoms[bond->end].parent = bond->beg;
            // when going from a polymer atom, make the new atom belong
            // to the same polymer
            if (_atoms[bond->beg].polymer_index >= 0)
            {
                // ... unless it goes from the polymer end
                // and is not in braces
                if (!_atoms[bond->beg].ends_polymer || (_atom_stack.size() >= 2 && _atom_stack.top() == _atom_stack[_atom_stack.size() - 2]))
                    _atoms[bond->end].polymer_index = _atoms[bond->beg].polymer_index;
            }
        }

        if (_inside_smarts_component)
        {
            _qmol->components.expandFill(_atoms.size(), 0);
            _qmol->components[_atoms.size() - 1] = _current_compno;
        }

        if (!first_atom)
            _atom_stack.pop();
        _atom_stack.push(_atoms.size() - 1);
        first_atom = false;

        while (_scanner.lookNext() == '{')
            _handleCurlyBrace(atom, inside_polymer);
        if (inside_polymer)
            atom.polymer_index = _polymer_repetitions.size() - 1;
    }

    if (smarts_mode)
    {
        // SMARTS mode: process only empty bonds, and replace them with single or aromatic
        for (int i = 0; i < _bonds.size(); i++)
        {
            int index = _bonds[i].index;

            QueryMolecule::Bond& qbond = _qmol->getBond(index);

            if (qbond.type != QueryMolecule::OP_NONE || _bonds[i].type == _ANY_BOND)
                continue;

            // A missing bond symbol is interpreted as "single or aromatic".
            // (http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html)
            // But if an atom cannot be aromatic, then bond can be as single
            const Edge& edge = _qmol->getEdge(index);
            QueryMolecule::Atom& q_beg = _qmol->getAtom(edge.beg);
            QueryMolecule::Atom& q_end = _qmol->getAtom(edge.end);

            std::unique_ptr<QueryMolecule::Bond> new_qbond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, BOND_SINGLE);

            bool beg_can_be_aromatic = q_beg.possibleValue(QueryMolecule::ATOM_AROMATICITY, ATOM_AROMATIC);
            bool end_can_be_aromatic = q_end.possibleValue(QueryMolecule::ATOM_AROMATICITY, ATOM_AROMATIC);

            int beg_label = _qmol->getAtomNumber(edge.beg);
            if (beg_label != -1)
                beg_can_be_aromatic &= Element::canBeAromatic(beg_label);

            int end_label = _qmol->getAtomNumber(edge.end);
            if (end_label != -1)
                end_can_be_aromatic &= Element::canBeAromatic(end_label);

            if (beg_can_be_aromatic && end_can_be_aromatic)
            {
                new_qbond.reset(QueryMolecule::Bond::oder(new_qbond.release(), new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
            }

            _qmol->resetBond(index, new_qbond.release());
        }
    }

    int i;

    for (i = 0; i < _cycles.size(); i++)
    {
        if (_cycles[i].beg >= 0)
            throw Error("cycle %d not closed", i);
    }

    if (inside_polymer)
        throw Error("polymer not closed");
}

void SmilesLoader::_handleCurlyBrace(_AtomDesc& atom, bool& inside_polymer)
{
    QS_DEF(Array<char>, curly);
    curly.clear();
    while (1)
    {
        _scanner.skip(1);
        int next = _scanner.lookNext();
        if (next == -1)
            throw Error("unclosed curly brace");
        if (next == '}')
        {
            _scanner.skip(1);
            break;
        }
        curly.push((char)next);
    }
    int repetitions;
    int poly = _parseCurly(curly, repetitions);
    if (poly == _POLYMER_START)
    {
        if (inside_polymer)
            throw Error("nested polymers not allowed");
        inside_polymer = true;
        atom.starts_polymer = true;
        _polymer_repetitions.push(0); // can change it later
    }
    else if (poly == _POLYMER_END)
    {
        if (!inside_polymer)
            throw Error("misplaced polymer ending");
        inside_polymer = false;
        _polymer_repetitions.top() = repetitions;
        atom.polymer_index = _polymer_repetitions.size() - 1;
        atom.ends_polymer = true;
    }
}

void SmilesLoader::_loadParsedMolecule()
{
    int i;

    if (_mol != 0)
    {
        for (i = 0; i < _atoms.size(); i++)
        {
            if (_atoms[i].label == 0)
                throw Error("atom without a label");
            int idx = _mol->addAtom(_atoms[i].label);
            _mol->setAtomCharge(idx, _atoms[i].charge);
            _mol->setAtomIsotope(idx, _atoms[i].isotope);
        }

        for (i = 0; i < _bonds.size(); i++)
        {
            int beg = _bonds[i].beg;
            int end = _bonds[i].end;

            if (end == -1)
                throw Error("probably pending bond %d not closed", i);
            _bonds[i].index = _mol->addBond_Silent(beg, end, _bonds[i].type);
        }
    }

    if (!smarts_mode)
        _markAromaticBonds();

    if (_mol != 0)
    {
        _addExplicitHForStereo();
        _setRadicalsAndHCounts();
    }

    if (smarts_mode)
        // Forbid matching SMARTS atoms to hydrogens
        _forbidHydrogens();

    if (!inside_rsmiles)
    {
        for (i = 0; i < _atoms.size(); i++)
        {
            if (_atoms[i].star_atom && _atoms[i].aam != 0)
            {
                if (_qmol != 0)
                    _qmol->resetAtom(i, new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
                _bmol->allowRGroupOnRSite(i, _atoms[i].aam);
            }
            else if (_atoms[i].label == ELEM_RSITE)
            {
                if (_atoms[i].rsite_num != 0)
                    _bmol->allowRGroupOnRSite(i, _atoms[i].rsite_num);
            }
        }
    }

    for (i = 0; i < _atoms.size(); i++)
    {
        if (_atoms[i].star_atom && _atoms[i].label == ELEM_PSEUDO)
        {
            _mol->setPseudoAtom(i, "A");
        }
    }

    if (_qmol != 0)
        // Replace implicit H with explicit one at required stereocenter
        // or add required number of "any atom" ligands
        _addLigandsForStereo();

    _calcStereocenters();
    _calcCisTrans();

    _scanner.skipSpace();

    if (_scanner.lookNext() == '|')
    {
        _scanner.skip(1);
        _readOtherStuff();
        if (_has_atom_coordinates || _has_directions_on_rings)
        {
            std::vector<int> sensible_bond_directions;
            sensible_bond_directions.resize(_bmol->edgeCount());
            _bmol->buildFromBondsStereocenters(stereochemistry_options, sensible_bond_directions.data());
            _bmol->markBondsStereocenters();
            _bmol->markBondsAlleneStereo();
        }
    }

    // Update attachment orders for rsites
    for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
    {
        if (!_bmol->isRSite(i))
            continue;

        const Vertex& vertex = _bmol->getVertex(i);

        int j, k = 0;
        for (j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
            _bmol->setRSiteAttachmentOrder(i, vertex.neiVertex(j), k++);
    }

    if (!inside_rsmiles)
    {
        _scanner.skipSpace();
        if (!_scanner.isEOF())
            _scanner.readLine(_bmol->name, true);
    }

    _bmol->reaction_atom_mapping.clear_resize(_bmol->vertexCount() + 1);
    _bmol->reaction_atom_mapping.zerofill();
    if (inside_rsmiles)
    {
        for (i = 0; i < _atoms.size(); i++)
            _bmol->reaction_atom_mapping[i] = _atoms[i].aam;
    }
    _bmol->reaction_atom_inversion.clear_resize(_bmol->vertexCount() + 1);
    _bmol->reaction_atom_inversion.zerofill();
    _bmol->reaction_atom_exact_change.clear_resize(_bmol->vertexCount() + 1);
    _bmol->reaction_atom_exact_change.zerofill();
    _bmol->reaction_bond_reacting_center.clear_resize(_bmol->edgeCount() + 1);
    _bmol->reaction_bond_reacting_center.zerofill();

    if (ignorable_aam != 0)
    {
        ignorable_aam->clear_resize(_bmol->vertexCount());
        ignorable_aam->zerofill();
        for (i = 0; i < _atoms.size(); i++)
            ignorable_aam->at(i) = _atoms[i].ignorable_aam ? 1 : 0;
    }

    // handle the polymers (part of the CurlySMILES specification)
    for (i = 0; i < _polymer_repetitions.size(); i++)
        _handlePolymerRepetition(i);
}

void SmilesLoader::_markAromaticBonds()
{
    CycleBasis basis;
    int i;

    basis.create(*_bmol);

    // Mark all 'empty' bonds in "aromatic" rings as aromatic.
    // We use SSSR here because we do not want "empty" bonds to
    // be aromatic when they are contained in some aliphatic (SSSR) ring.
    for (i = 0; i < basis.getCyclesCount(); i++)
    {
        const Array<int>& cycle = basis.getCycle(i);
        int j;
        bool needs_modification = false;

        for (j = 0; j < cycle.size(); j++)
        {
            int idx = cycle[j];
            const Edge& edge = _bmol->getEdge(idx);

            if (!_atoms[edge.beg].aromatic || !_atoms[edge.end].aromatic)
                break;
            if (_bonds[idx].type == BOND_SINGLE || _bonds[idx].type == BOND_DOUBLE || _bonds[idx].type == BOND_TRIPLE)
                break;
            if (_qmol != 0 && !_qmol->possibleBondOrder(_bonds[idx].index, BOND_AROMATIC))
                break;
            if (_bonds[idx].type == -1)
                needs_modification = true;
        }

        if (j != cycle.size())
            continue;

        if (needs_modification)
        {
            for (j = 0; j < cycle.size(); j++)
            {
                int idx = cycle[j];
                if (_bonds[idx].type == -1)
                {
                    _bonds[idx].type = BOND_AROMATIC;
                    int bond_index = _bonds[idx].index;
                    if (_mol != 0)
                        _mol->setBondOrder_Silent(bond_index, BOND_AROMATIC);
                    if (_qmol != 0)
                        _qmol->resetBond(bond_index, QueryMolecule::Bond::und(_qmol->releaseBond(bond_index),
                                                                              new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
                }
            }
        }
    }

    for (i = 0; i < basis.getCyclesCount(); i++)
    {
        const Array<int>& cycle = basis.getCycle(i);
        int j;
        bool needs_modification = false;

        for (j = 0; j < cycle.size(); j++)
        {
            int idx = cycle[j];
            const Edge& edge = _bmol->getEdge(idx);

            if (!_atoms[edge.beg].aromatic || !_atoms[edge.end].aromatic)
            {
                needs_modification = false;
                break;
            }
            if (_bonds[idx].type == BOND_SINGLE || _bonds[idx].type == BOND_DOUBLE || _bonds[idx].type == BOND_TRIPLE)
                continue;
            if (_qmol != 0 && !_qmol->possibleBondOrder(_bonds[idx].index, BOND_AROMATIC))
                continue;
            if (_bonds[idx].type == -1)
                needs_modification = true;
        }

        if (needs_modification)
        {
            for (j = 0; j < cycle.size(); j++)
            {
                int idx = cycle[j];
                const Edge& edge = _bmol->getEdge(idx);
                if ((_bonds[idx].type == -1) && (_atoms[edge.beg].aromatic && _atoms[edge.end].aromatic))
                {
                    _bonds[idx].type = BOND_AROMATIC;
                    int bond_index = _bonds[idx].index;
                    if (_mol != 0)
                        _mol->setBondOrder_Silent(bond_index, BOND_AROMATIC);
                    if (_qmol != 0)
                        _qmol->resetBond(bond_index, QueryMolecule::Bond::und(_qmol->releaseBond(bond_index),
                                                                              new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
                }
            }
        }
    }

    // mark the rest 'empty' bonds as single
    for (i = 0; i < _bonds.size(); i++)
    {
        if (_bonds[i].type == -1)
        {
            int bond_index = _bonds[i].index;
            if (_mol != 0)
                _mol->setBondOrder_Silent(bond_index, BOND_SINGLE);
            if (_qmol != 0)
                _qmol->resetBond(bond_index,
                                 QueryMolecule::Bond::und(_qmol->releaseBond(bond_index), new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE)));
        }
    }
}

void SmilesLoader::_setRadicalsAndHCounts()
{
    int i;

    for (i = 0; i < _atoms.size(); i++)
    {
        int idx = i;

        // The SMILES specification says: Elements in the "organic subset"
        // B, C, N, O, P, S, F, Cl, Br, and I may be written without brackets
        // if the number of attached hydrogens conforms to the lowest normal
        // valence consistent with explicit bonds. We assume that there are
        // no radicals in that case.
        if (!_atoms[i].brackets)
            // We set zero radicals explicitly to properly detect errors like FClF
            // (while F[Cl]F is correct)
            _mol->setAtomRadical(idx, 0);

        if (_atoms[i].hydrogens >= 0)
            _mol->setImplicitH(idx, _atoms[i].hydrogens);
        else if (_atoms[i].brackets)    // no hydrogens in brackets?
            _mol->setImplicitH(idx, 0); // no implicit hydrogens on atom then
        else if (_atoms[i].aromatic && _mol->getAtomAromaticity(i) == ATOM_AROMATIC)
        {
            // Additional check for _mol->getAtomAromaticity(i) is required because
            // a cycle can be non-aromatic while atom letters are small
            if (_atoms[i].label == ELEM_C)
            {
                // here we are basing on the fact that
                // aromatic uncharged carbon always has a double bond
                if (_mol->getVertex(i).degree() < 3)
                    // 2-connected aromatic carbon must have 1 single bond and 1 double bond,
                    // so we have one implicit hydrogen left
                    _mol->setImplicitH(idx, 1);
                else
                    _mol->setImplicitH(idx, 0);
            }
            else
            {
                // Leave the number of hydrogens as unspecified
                // Dearomatization algorithm can find any suitable configuration
            }
        }
    }
}

void SmilesLoader::_forbidHydrogens()
{
    for (int i = 0; i < _atoms.size(); i++)
    {
        // not needed if it is a sure atom or a list without a hydrogen
        if (_qmol->getAtomNumber(i) == -1 && _qmol->possibleAtomNumber(i, ELEM_H))
        {
            // not desired if it is a list with hydrogen
            if (!_qmol->getAtom(i).hasConstraintWithValue(QueryMolecule::ATOM_NUMBER, ELEM_H))
            {
                std::unique_ptr<QueryMolecule::Atom> newatom;
                std::unique_ptr<QueryMolecule::Atom> oldatom(_qmol->releaseAtom(i));

                newatom.reset(
                    QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)), oldatom.release()));

                _qmol->resetAtom(i, newatom.release());
            }
        }
    }
}

void SmilesLoader::_addExplicitHForStereo()
{
    for (int i = 0; i < _atoms.size(); i++)
    {
        if ((_atoms[i].chirality > 0) && (_bmol->getVertex(i).degree() == 2) && (_atoms[i].hydrogens == 1))
        {
            _AtomDesc& atom = _atoms.push(_neipool);
            _BondDesc* bond = &_bonds.push();

            atom.label = ELEM_H;
            int exp_h_idx = _mol->addAtom(atom.label);

            bond->beg = i;
            bond->end = _atoms.size() - 1;
            bond->type = BOND_SINGLE;
            bond->index = _mol->addBond_Silent(bond->beg, bond->end, bond->type);

            _atoms[i].neighbors.add(exp_h_idx);
            _atoms[exp_h_idx].neighbors.add(i);
            _atoms[exp_h_idx].parent = i;

            _atoms[i].hydrogens = 0;
        }
    }
}

void SmilesLoader::_addLigandsForStereo()
{
    bool add_explicit_h = false;
    int num_ligands = 0;

    for (int i = 0; i < _atoms.size(); i++)
    {
        if ((_atoms[i].chirality > 0) && (_bmol->getVertex(i).degree() < 3) && !_isAlleneLike(i))
        {
            if (_atoms[i].hydrogens == 1)
            {
                add_explicit_h = true;
                num_ligands = 3 - _bmol->getVertex(i).degree() - _atoms[i].hydrogens;
            }
            else
                num_ligands = 3 - _bmol->getVertex(i).degree();

            for (int j = 0; j < num_ligands; j++)
            {
                _AtomDesc& atom = _atoms.push(_neipool);
                _BondDesc* bond = &_bonds.push();
                std::unique_ptr<QueryMolecule::Atom> qatom;

                if (add_explicit_h)
                    qatom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
                else
                    qatom.reset(QueryMolecule::Atom::oder(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                          new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));

                std::unique_ptr<QueryMolecule::Bond> qbond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, BOND_SINGLE);

                atom.star_atom = true;
                int any_atom_idx = _qmol->addAtom(qatom.release());

                bond->beg = i;
                bond->end = _atoms.size() - 1;
                bond->type = BOND_SINGLE;
                bond->dir = 0;
                bond->topology = 0;
                bond->index = _qmol->addBond(i, any_atom_idx, qbond.release());

                _atoms[i].neighbors.add(any_atom_idx);
                _atoms[any_atom_idx].neighbors.add(i);
                _atoms[any_atom_idx].parent = i;
            }

            if (_atoms[i].hydrogens == 1)
            {
                _AtomDesc& atom = _atoms.push(_neipool);
                _BondDesc* bond = &_bonds.push();

                std::unique_ptr<QueryMolecule::Atom> qatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, ELEM_H);
                std::unique_ptr<QueryMolecule::Bond> qbond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, BOND_SINGLE);

                atom.label = ELEM_H;
                int exp_h_idx = _qmol->addAtom(qatom.release());

                bond->beg = i;
                bond->end = _atoms.size() - 1;
                bond->type = BOND_SINGLE;
                bond->dir = 0;
                bond->topology = 0;
                bond->index = _qmol->addBond(i, exp_h_idx, qbond.release());

                _atoms[i].neighbors.add(exp_h_idx);
                _atoms[exp_h_idx].neighbors.add(i);
                _atoms[exp_h_idx].parent = i;

                _atoms[i].hydrogens = 0;
                _qmol->getAtom(i).removeConstraints(QueryMolecule::ATOM_TOTAL_H);
            }
        }
    }
}

bool SmilesLoader::_isAlleneLike(int i)
{
    if (_bmol->getVertex(i).degree() == 2)
    {
        int subst[4];
        int left, right;
        bool pure_h[4];

        if (MoleculeAlleneStereo::possibleCenter(*_bmol, i, left, right, subst, pure_h))
            return true;
    }
    return false;
}

void SmilesLoader::_handlePolymerRepetition(int i)
{
    int j, start = -1, end = -1;
    int start_bond = -1, end_bond = -1;
    SGroup* sgroup;

    // no repetitions counter => polymer
    if (_polymer_repetitions[i] == 0)
    {
        int idx = _bmol->sgroups.addSGroup(SGroup::SG_TYPE_SRU);
        sgroup = &_bmol->sgroups.getSGroup(idx);
        RepeatingUnit* ru = (RepeatingUnit*)sgroup;
        ru->connectivity = RepeatingUnit::HEAD_TO_TAIL;
    }
    // repetitions counter present => multiple group
    else
    {
        int idx = _bmol->sgroups.addSGroup(SGroup::SG_TYPE_MUL);
        sgroup = &_bmol->sgroups.getSGroup(idx);
        MultipleGroup* mg = (MultipleGroup*)sgroup;
        mg->multiplier = _polymer_repetitions[i];
    }
    for (j = 0; j < _atoms.size(); j++)
    {
        if (_atoms[j].polymer_index != i)
            continue;
        sgroup->atoms.push(j);
        if (_polymer_repetitions[i] > 0)
            ((MultipleGroup*)sgroup)->parent_atoms.push(j);
        if (_atoms[j].starts_polymer)
            start = j;
        if (_atoms[j].ends_polymer)
            end = j;
    }
    if (start == -1)
        throw Error("internal: polymer start not found");
    if (end == -1)
        throw Error("internal: polymer end not found");
    for (j = 0; j < _bonds.size(); j++)
    {
        if (!_bmol->hasEdge(j))
            // Edge was removed when virtual atoms for
            // attachment points are removed
            continue;

        const Edge& edge = _bmol->getEdge(j);

        if (_atoms[edge.beg].polymer_index != i && _atoms[edge.end].polymer_index != i)
            continue;
        if (_atoms[edge.beg].polymer_index == i && _atoms[edge.end].polymer_index == i)
            sgroup->bonds.push(j);
        else
        {
            // bond going out of the sgroup
            if (start_bond == -1 && (edge.beg == start || edge.end == start))
                start_bond = j;
            else if (end_bond == -1 && (edge.beg == end || edge.end == end))
                end_bond = j;
            else
                throw Error("internal: unknown bond going from sgroup");
        }
    }

    if (end_bond == -1 && start_bond != -1)
    {
        // swap them to make things below easier
        std::swap(start, end);
        std::swap(start_bond, end_bond);
    }

    Vec2f* p = sgroup->brackets.push();
    p[0].set(0, 0);
    p[1].set(0, 0);
    p = sgroup->brackets.push();
    p[0].set(0, 0);
    p[1].set(0, 0);

    if (_polymer_repetitions[i] > 1)
    {
        QS_DEF(Array<int>, mapping);
        std::unique_ptr<BaseMolecule> rep(_bmol->neu());

        rep->makeSubmolecule(*_bmol, sgroup->atoms, &mapping, 0);
        rep->sgroups.clear(SGroup::SG_TYPE_SRU);
        rep->sgroups.clear(SGroup::SG_TYPE_MUL);
        int rep_start = mapping[start];
        int rep_end = mapping[end];

        // already have one instance of the sgroup; add repetitions if they exist
        for (j = 0; j < _polymer_repetitions[i] - 1; j++)
        {
            _bmol->mergeWithMolecule(*rep, &mapping, 0);

            int k;

            for (k = rep->vertexBegin(); k != rep->vertexEnd(); k = rep->vertexNext(k))
                sgroup->atoms.push(mapping[k]);
            for (k = rep->edgeBegin(); k != rep->edgeEnd(); k = rep->edgeNext(k))
            {
                const Edge& edge = rep->getEdge(k);
                sgroup->bonds.push(_bmol->findEdgeIndex(mapping[edge.beg], mapping[edge.end]));
            }

            if (rep_end >= 0 && end_bond >= 0)
            {
                // make new connections from the end of the old fragment
                // to the beginning of the new one, and from the end of the
                // new fragment outwards from the sgroup
                int external = _bmol->getEdge(end_bond).findOtherEnd(end);
                _bmol->removeBond(end_bond);
                if (_mol != 0)
                {
                    _mol->addBond(end, mapping[rep_start], BOND_SINGLE);
                    end_bond = _mol->addBond(mapping[rep_end], external, BOND_SINGLE);
                }
                else
                {
                    _qmol->addBond(end, mapping[rep_start], new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE));
                    end_bond = _qmol->addBond(mapping[rep_end], external, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE));
                }
                end = mapping[rep_end];
            }
        }
    }
    else if (_polymer_repetitions[i] == 0)
    {
        // if the start atom of the polymer does not have an incoming bond...
        if (start_bond == -1)
        {
            if (_mol != 0)
            { // ... add one, with a "star" on the other end.
                int star = _mol->addAtom(ELEM_PSEUDO);
                _mol->setPseudoAtom(star, "*");
                _mol->addBond(start, star, BOND_SINGLE);
            }
            else
            { // if it is a query molecule, add a bond with "any" atom instead
                int any = _qmol->addAtom(new QueryMolecule::Atom());
                _qmol->addBond(start, any, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE));
            }
        }
        // do the same with the end atom
        if (end_bond == -1)
        {
            if (_mol != 0)
            {
                int star = _mol->addAtom(ELEM_PSEUDO);
                _mol->setPseudoAtom(star, "*");
                _mol->addBond(end, star, BOND_SINGLE);
            }
            else
            {
                int any = _qmol->addAtom(new QueryMolecule::Atom());
                _qmol->addBond(end, any, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE));
            }
        }
    }
}

void SmilesLoader::_loadMolecule()
{
    _atoms.clear();
    _bonds.clear();
    _polymer_repetitions.clear();

    _parseMolecule();
    _loadParsedMolecule();
    _validateStereoCenters();
}

void SmilesLoader::_readBond(Array<char>& bond_str, _BondDesc& bond, std::unique_ptr<QueryMolecule::Bond>& qbond)
{
    if (bond_str.find(';') != -1)
    {
        QS_DEF(Array<char>, substring);
        std::unique_ptr<QueryMolecule::Bond> subqbond;
        int i;

        if (_qmol == 0)
            throw Error("';' is allowed only within queries");

        substring.clear();
        for (i = 0; i <= bond_str.size(); i++)
        {
            if (i == bond_str.size() || bond_str[i] == ';')
            {
                subqbond = std::make_unique<QueryMolecule::Bond>();
                _readBond(substring, bond, subqbond);
                qbond.reset(QueryMolecule::Bond::und(qbond.release(), subqbond.release()));
                substring.clear();
            }
            else
                substring.push(bond_str[i]);
        }
        return;
    }
    if (bond_str.find(',') != -1)
    {
        QS_DEF(Array<char>, substring);
        std::unique_ptr<QueryMolecule::Bond> subqbond;
        int i;

        if (_qmol == 0)
            throw Error("',' is allowed only within queries");

        substring.clear();
        for (i = 0; i <= bond_str.size(); i++)
        {
            if (i == bond_str.size() || bond_str[i] == ',')
            {
                subqbond = std::make_unique<QueryMolecule::Bond>();
                _readBond(substring, bond, subqbond);
                if (qbond->type == 0)
                    qbond.reset(subqbond.release());
                else
                    qbond.reset(QueryMolecule::Bond::oder(qbond.release(), subqbond.release()));
                substring.clear();
            }
            else
                substring.push(bond_str[i]);
        }
        return;
    }
    if (bond_str.find('&') != -1)
    {
        QS_DEF(Array<char>, substring);
        std::unique_ptr<QueryMolecule::Bond> subqbond;
        int i;

        if (_qmol == 0)
            throw Error("'&' is allowed only within queries");

        substring.clear();
        for (i = 0; i <= bond_str.size(); i++)
        {
            if (i == bond_str.size() || bond_str[i] == '&')
            {
                subqbond = std::make_unique<QueryMolecule::Bond>();
                _readBond(substring, bond, subqbond);
                qbond.reset(QueryMolecule::Bond::und(qbond.release(), subqbond.release()));
                substring.clear();
            }
            else
                substring.push(bond_str[i]);
        }
        return;
    }
    _readBondSub(bond_str, bond, qbond);
}

void SmilesLoader::_readBondSub(Array<char>& bond_str, _BondDesc& bond, std::unique_ptr<QueryMolecule::Bond>& qbond)
{
    BufferScanner scanner(bond_str);

    bool neg = false;

    while (!scanner.isEOF())
    {
        int next = scanner.lookNext();
        int order = -1;
        int topology = -1;

        if (next == '!')
        {
            scanner.skip(1);
            neg = !neg;
            if (qbond.get() == 0)
                throw Error("'!' is allowed only within queries");
            continue;
        }
        if (next == '-')
        {
            scanner.skip(1);
            order = BOND_SINGLE;
        }
        else if (next == '=')
        {
            scanner.skip(1);
            order = BOND_DOUBLE;
        }
        else if (next == '#')
        {
            scanner.skip(1);
            order = BOND_TRIPLE;
        }
        else if (next == ':')
        {
            scanner.skip(1);
            order = BOND_AROMATIC;
        }
        else if (next == '/')
        {
            scanner.skip(1);
            order = BOND_SINGLE;
            if (bond.dir == 2)
                throw Error("Specificiation of both cis- and trans- bond restriction is not supported yet.");
            bond.dir = 1;
        }
        else if (next == '\\')
        {
            scanner.skip(1);
            order = BOND_SINGLE;
            if (bond.dir == 1)
                throw Error("Specificiation of both cis- and trans- bond restriction is not supported yet.");
            bond.dir = 2;
        }
        else if (next == '~')
        {
            scanner.skip(1);
            order = _ANY_BOND;
            if (qbond.get() == 0)
                throw Error("'~' any bond is allowed only for queries");
        }
        else if (next == '@')
        {
            scanner.skip(1);
            if (qbond.get() == 0)
                throw Error("'@' ring bond is allowed only for queries");
            topology = TOPOLOGY_RING;
        }
        else
            throw Error("Character #%d is unexpected during bond parsing", next);

        std::unique_ptr<QueryMolecule::Bond> subqbond;

        if (order > 0)
        {
            bond.type = order;
            if (qbond.get() != 0)
            {
                if (subqbond.get() == 0)
                    subqbond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, order);
                else
                    subqbond.reset(QueryMolecule::Bond::und(subqbond.release(), new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order)));
            }
        }
        else if (order == _ANY_BOND)
        {
            bond.type = order;
        }

        if (topology > 0)
        {
            if (subqbond.get() == 0)
                subqbond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_TOPOLOGY, topology);
            else
                subqbond.reset(QueryMolecule::Bond::und(subqbond.release(), new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, topology)));
        }

        if (subqbond.get() != 0)
        {
            if (neg)
            {
                subqbond.reset(QueryMolecule::Bond::nicht(subqbond.release()));
                neg = false;
            }
            qbond.reset(QueryMolecule::Bond::und(qbond.release(), subqbond.release()));
        }
    }
}

bool SmilesLoader::_readAtomLogic(Array<char>& atom_str, bool first_in_brackets, _AtomDesc& atom, std::unique_ptr<QueryMolecule::Atom>& qatom)
{
    QS_DEF(Array<char>, atom_str_copy);
    if (atom_str.size() < 1)
        throw Error("empty atom?");

    atom_str_copy.copy(atom_str);
    int i, k;

    while ((k = atom_str_copy.find('$')) != -1)
    {
        // fill the "$(...) part of atom_str_copy with '^'"
        int cnt = 1;
        atom_str_copy[k] = '^';
        for (i = k + 2; i < atom_str_copy.size(); i++)
        {
            if (atom_str_copy[i] == '(')
                cnt++;
            else if (atom_str_copy[i] == ')')
                cnt--;

            if (cnt == 0)
                break;

            atom_str_copy[i] = '^';
        }
    }

    if (atom_str_copy.find(';') != -1)
    {
        QS_DEF(Array<char>, substring);
        std::unique_ptr<QueryMolecule::Atom> subqatom;
        int i, k = 0;

        if (qatom.get() == 0)
            throw Error("';' is allowed only for query molecules");

        substring.clear();
        for (i = 0; i <= atom_str_copy.size(); i++)
        {
            if (i == atom_str.size() || atom_str_copy[i] == ';')
            {
                subqatom = std::make_unique<QueryMolecule::Atom>();
                _readAtom(substring, first_in_brackets && (k == 0), atom, subqatom);
                qatom.reset(QueryMolecule::Atom::und(qatom.release(), subqatom.release()));
                substring.clear();
                k++;
            }
            else
                substring.push(atom_str[i]);
        }
        return false;
    }

    if (atom_str_copy.find(',') != -1)
    {
        QS_DEF(Array<char>, substring);
        std::unique_ptr<QueryMolecule::Atom> subqatom;
        int i, k = 0;

        if (qatom.get() == 0)
            throw Error("',' is allowed only for query molecules");

        substring.clear();
        for (i = 0; i <= atom_str.size(); i++)
        {
            if (i == atom_str.size() || atom_str_copy[i] == ',')
            {
                subqatom = std::make_unique<QueryMolecule::Atom>();
                _readAtom(substring, first_in_brackets && (k == 0), atom, subqatom);
                if (qatom->type == 0)
                    qatom.reset(subqatom.release());
                else
                    qatom.reset(QueryMolecule::Atom::oder(qatom.release(), subqatom.release()));
                substring.clear();
                k++;
            }
            else
                substring.push(atom_str[i]);
        }
        return false;
    }

    if (atom_str_copy.find('&') != -1)
    {
        QS_DEF(Array<char>, substring);
        std::unique_ptr<QueryMolecule::Atom> subqatom;
        int i, k = 0;

        if (qatom.get() == 0)
            throw Error("'&' is allowed only for query molecules");

        substring.clear();
        for (i = 0; i <= atom_str.size(); i++)
        {
            if (i == atom_str.size() || atom_str_copy[i] == '&')
            {
                subqatom = std::make_unique<QueryMolecule::Atom>();
                _readAtom(substring, first_in_brackets && (k == 0), atom, subqatom);
                qatom.reset(QueryMolecule::Atom::und(qatom.release(), subqatom.release()));
                substring.clear();
                k++;
            }
            else
                substring.push(atom_str[i]);
        }
        return false;
    }
    return true;
}

void SmilesLoader::_readAtom(Array<char>& atom_str, bool first_in_brackets, _AtomDesc& atom, std::unique_ptr<QueryMolecule::Atom>& qatom)
{
    if (!_readAtomLogic(atom_str, first_in_brackets, atom, qatom))
        return;

    BufferScanner scanner(atom_str);

    bool element_assigned = false;
    bool neg = false;
    while (!scanner.isEOF())
    {
        bool isotope_set = false;
        int element = -1;
        int aromatic = 0;
        int next = scanner.lookNext();
        std::unique_ptr<QueryMolecule::Atom> subatom;

        if (next == '!')
        {
            if (qatom.get() == 0)
                throw Error("'!' is allowed only within queries");

            scanner.skip(1);
            neg = !neg;
            first_in_brackets = false;
            continue;
        }
        else if (next == '$')
        {
            scanner.skip(1);
            if (scanner.readChar() != '(')
                throw Error("'$' must be followed by '('");

            QS_DEF(Array<char>, subexp);

            subexp.clear();
            int cnt = 1;

            while (1)
            {
                char c = scanner.readChar();
                if (c == '(')
                    cnt++;
                else if (c == ')')
                {
                    cnt--;
                    if (cnt == 0)
                        break;
                }
                subexp.push(c);
            }

            BufferScanner subscanner(subexp);
            std::unique_ptr<SmilesLoader> subloader = std::make_unique<SmilesLoader>(subscanner);
            std::unique_ptr<QueryMolecule> fragment = std::make_unique<QueryMolecule>();

            subloader->loadSMARTS(*fragment);
            fragment->fragment_smarts.copy(subexp);
            fragment->fragment_smarts.push(0);

            if (subatom.get() == 0)
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_FRAGMENT, fragment.release());
            else
                subatom.reset(QueryMolecule::Atom::und(subatom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_FRAGMENT, fragment.release())));
        }
        else if (isdigit(next))
        {
            int isotope = scanner.readUnsigned();

            if (qatom.get() != 0)
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_ISOTOPE, isotope);
            else
                atom.isotope = isotope;
            isotope_set = true;
        }
        else if (next == 'H')
        {
            scanner.skip(1);

            // Now comes the trouble with the 'H' symbol.
            // As the manual says
            // (see http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html):
            //    [H] means hydrogen atom.
            //    [*H2] means any atom with exactly two hydrogens attached.
            // Yet in the combined expressions like [n;H1] 'H' means the hydrogen
            // count, not the element. To distinguish these things, we use
            // the 'first in brackets' flag, which is true only for the very
            // first sub-expression in the brackets.
            // Also, the following elements begin with H: He, Hs, Hf, Ho, Hg
            if (strchr("esfog", scanner.lookNext()) == NULL)
            {
                if (first_in_brackets)
                    element = ELEM_H;
                else
                {
                    atom.hydrogens = 1;
                    if (isdigit(scanner.lookNext()))
                        atom.hydrogens = scanner.readUnsigned();
                    if (qatom.get() != 0)
                        subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_TOTAL_H, atom.hydrogens);
                }
            }
            else
                element = Element::fromTwoChars('H', scanner.readChar());
        }
        // The 'A' symbol is weird too. It can be the 'aliphatic' atomic primitive,
        // and can also be Al, Ar, As, Ag, Au, At, Ac, or Am.
        else if (next == 'A')
        {
            scanner.skip(1);

            if (strchr("lrsgutcm", scanner.lookNext()) == NULL)
            {
                if (qatom.get() == 0)
                    throw Error("'A' specifier is allowed only for query molecules");

                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_AROMATICITY, ATOM_ALIPHATIC);
            }
            else
                element = Element::fromTwoChars('A', scanner.readChar());
        }
        // Similarly, 'R' can start Rb, Ru, Rh, Re, Rn, Ra, Rf, Rg
        else if (next == 'R')
        {
            scanner.skip(1);

            if (strchr("buhenafg", scanner.lookNext()) == NULL)
            {
                if (qatom.get() != 0)
                {
                    if (isdigit(scanner.lookNext()))
                    {
                        int rc = scanner.readUnsigned();

                        if (rc == 0)
                            subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RING_BONDS, 0);
                        else
                            subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_SSSR_RINGS, rc);
                    }
                    else
                        subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RING_BONDS, 1, 100);
                }
                else
                {
                    // Check possible Biovia Draw R-sites notaion
                    if (isdigit(scanner.lookNext()))
                    {
                        int rc = scanner.readUnsigned();
                        atom.label = ELEM_RSITE;
                        atom.rsite_num = rc;
                    }
                }
            }
            else
                element = Element::fromTwoChars('R', scanner.readChar());
        }
        // Yet 'D' can start Db, Ds, Dy
        else if (next == 'D')
        {
            scanner.skip(1);

            if (strchr("bsy", scanner.lookNext()) == NULL)
            {
                if (qatom.get() == 0)
                    throw Error("'D' specifier is allowed only for query molecules");

                int degree = 1;

                if (isdigit(scanner.lookNext()))
                    degree = scanner.readUnsigned();

                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_SUBSTITUENTS, degree);
            }
            else
                element = Element::fromTwoChars('D', scanner.readChar());
        }
        // ... and 'X' can start Xe
        else if (next == 'X')
        {
            scanner.skip(1);

            if (scanner.lookNext() != 'e')
            {
                if (qatom.get() == 0)
                    throw Error("'X' specifier is allowed only for query molecules");

                int conn = 1;

                if (isdigit(scanner.lookNext()))
                    conn = scanner.readUnsigned();

                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_CONNECTIVITY, conn);
            }
            else
                element = Element::fromTwoChars('X', scanner.readChar());
        }
        else if (next == '*')
        {
            atom.star_atom = true;
            scanner.skip(1);
            if (first_in_brackets && atom_str.size() < 2 && !smarts_mode)
            {
                atom.label = ELEM_RSITE;
            }
            else if (first_in_brackets && scanner.lookNext() == ':' && !inside_rsmiles)
            {
                atom.label = ELEM_RSITE;
            }
            else
            {
                if (qatom.get() == 0)
                    atom.label = ELEM_PSEUDO;
                else
                {
                    subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, ELEM_H);
                    subatom.reset(QueryMolecule::Atom::nicht(subatom.release()));
                }
            }
        }
        else if (next == '#')
        {
            scanner.skip(1);
            if (scanner.lookNext() == 'G')
            {
                scanner.skip(1);
                auto group = scanner.readUnsigned();
                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();
                x_atom->type = QueryMolecule::OP_OR;
                switch (group)
                {
                case 1: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Li));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Na));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_K));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rb));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cs));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Fr));
                    break;
                }
                case 2: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Be));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Mg));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ca));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Sr));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ba));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ra));
                    break;
                }
                case 3: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_B));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Al));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ga));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_In));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ti));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Nh));
                    break;
                }
                case 4: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Si));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ge));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Sn));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Pb));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Fl));
                    break;
                }
                case 5: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_As));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Sb));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Bi));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Mc));
                    break;
                }
                case 6: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Te));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Pa));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Lv));
                    break;
                }
                case 7: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ts));
                    break;
                }
                case 8: {
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ra));
                    x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Og));
                    break;
                }
                default:
                    throw Error("Unknown group %d", group);
                }
                if (neg)
                {
                    x_atom.reset(QueryMolecule::Atom::nicht(x_atom.release()));
                }
                qatom.reset(x_atom.release());
            }
            else if (scanner.lookNext() == 'X')
            {
                scanner.skip(1);
                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();
                x_atom->type = QueryMolecule::OP_AND;
                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
                x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                if (neg)
                {
                    x_atom.reset(QueryMolecule::Atom::nicht(x_atom.release()));
                }
                qatom.reset(x_atom.release());
            }
            else if (scanner.lookNext() == 'N')
            {
                scanner.skip(1);
                std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();
                x_atom->type = QueryMolecule::OP_OR;
                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O));
                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N));
                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                if (neg)
                {
                    x_atom.reset(QueryMolecule::Atom::nicht(x_atom.release()));
                }
                qatom.reset(x_atom.release());
            }
            else
            {
                element = scanner.readUnsigned();
                if (qatom.get() == 0)
                    throw Error("'#%d' atom representation allowed only for query molecules", element);
            }
        }
        // Now we check that we have here an element from the periodic table.
        // We assume that this must be an alphabetic character and also
        // something not from the alphabetic SMARTS 'atomic primitives'
        // (see http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html).
        else if (isalpha(next) && strchr("hrvxastiqw", next) == NULL)
        {
            scanner.skip(1);

            if (next == 'b')
            {
                element = ELEM_B;
                aromatic = ATOM_AROMATIC;
            }
            else if (next == 'c')
            {
                element = ELEM_C;
                aromatic = ATOM_AROMATIC;
            }
            else if (next == 'n')
            {
                element = ELEM_N;
                aromatic = ATOM_AROMATIC;
            }
            else if (next == 'o')
            {
                element = ELEM_O;
                aromatic = ATOM_AROMATIC;
            }
            else if (next == 'p')
            {
                element = ELEM_P;
                aromatic = ATOM_AROMATIC;
            }
            else if (islower(next))
                throw Error("unrecognized lowercase symbol: %c", next);

            // Now we are sure that 'next' is a capital letter

            // Check if we have a lowercase letter right after...
            else if (isalpha(scanner.lookNext()) && islower(scanner.lookNext()) &&
                     // If a lowercase letter is following the uppercase letter,
                     // we should consider reading them as a single element.
                     // They can possibly not form an element: for example,
                     // [Nr] is formally a nitrogen in a ring (although nobody would
                     // write it that way: [N;r] is much more clear).
                     (Element::fromTwoChars2(next, scanner.lookNext())) > 0 && (Element::fromTwoChars2(next, scanner.lookNext()) != ELEM_Cn))
            {
                element = Element::fromTwoChars2(next, scanner.lookNext());
                scanner.skip(1);
                if (smarts_mode)
                    if (element == ELEM_As || element == ELEM_Se)
                        aromatic = ATOM_ALIPHATIC;
            }
            else if ((next == 'C' && scanner.lookNext() == 'n') && first_in_brackets)
            {
                scanner.skip(1);
                element = ELEM_Cn;
            }
            else
            {
                // It is a single-char uppercase element identifier then
                element = Element::fromChar(next);

                if (smarts_mode)
                    if (element == ELEM_B || element == ELEM_C || element == ELEM_N || element == ELEM_O || element == ELEM_P || element == ELEM_S)
                        aromatic = ATOM_ALIPHATIC;
            }
        }
        else if (next == '@')
        {
            atom.chirality = 1;
            scanner.skip(1);
            if (scanner.lookNext() == '@')
            {
                atom.chirality = 2;
                scanner.skip(1);
            }
            else
            {
                std::string current((const char*)scanner.curptr(), scanner.length() - scanner.tell());
                std::smatch match;
                if (std::regex_search(current, match, std::regex("^(TH|AL)([1-2])")))
                {
                    atom.chirality = std::stoi(match[2]);
                    scanner.skip(3);
                }
                else if (std::regex_search(current, match, std::regex("^SP[1-3]")))
                {
                    // this type of chirality not supported. just skip it.
                    scanner.skip(3);
                }
                else if (std::regex_search(current, match, std::regex(R"((TB([1-9]|1[0-9]|20)|OH([1-9]|1\d|2\d|30))(?!\d))")))
                {
                    const int TB_GROUP = 2;
                    const int OH_GROUP = 3;
                    int value = std::stoi(match.str(TB_GROUP).empty() ? match.str(OH_GROUP) : match.str(TB_GROUP));
                    if (value <= 2)
                        atom.chirality = value;
                    scanner.skip(3);
                    if (value >= 10)
                        scanner.skip(1);
                }
            }
        }
        else if (next == '+' || next == '-')
        {
            char c = scanner.readChar();
            if (c == '+')
                atom.charge = 1;
            else
                atom.charge = -1;

            if (isdigit(scanner.lookNext()))
                atom.charge *= scanner.readUnsigned();
            else
                while (scanner.lookNext() == c)
                {
                    scanner.skip(1);
                    if (c == '+')
                        atom.charge++;
                    else
                        atom.charge--;
                }

            if (qatom.get() != 0)

                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_CHARGE, atom.charge);
        }
        else if (next == 'a') // can be [as] or SMARTS aromaticity flag
        {
            scanner.skip(1);

            if (scanner.lookNext() == 's')
            {
                scanner.skip(1);

                element = ELEM_As;
                aromatic = ATOM_AROMATIC;
            }
            else
            {
                if (qatom.get() == 0)
                    throw Error("'a' specifier is allowed only for query molecules");

                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_AROMATICITY, ATOM_AROMATIC);
            }
        }
        else if (next == 's') // can be [s], [se] or [si]
        {
            scanner.skip(1);
            if (scanner.lookNext() == 'e')
            {
                scanner.skip(1);
                element = ELEM_Se;
                aromatic = ATOM_AROMATIC;
            }
            else if (scanner.lookNext() == 'i')
            {
                // Aromatic Si cannot occure in SMILES by specification, but
                // Cactvs produces it
                scanner.skip(1);
                element = ELEM_Si;
                aromatic = ATOM_AROMATIC;
            }
            else
            {
                element = ELEM_S;
                aromatic = ATOM_AROMATIC;
            }
        }
        else if (next == 't') // [te]
        {
            // Aromatic Te cannot occure in SMILES by specification, but
            // RDKit produces it within extended SMILES
            scanner.skip(1);
            if (scanner.lookNext() == 'e')
            {
                scanner.skip(1);
                element = ELEM_Te;
                aromatic = ATOM_AROMATIC;
            }
            else
                throw Error("invalid character within atom description: '%c'", next);
        }
        else if (next == 'h')
        {
            scanner.skip(1);

            if (qatom.get() == 0)
                throw Error("'h' specifier is allowed only for query molecules");

            if (isdigit(scanner.lookNext()))
            {
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_IMPLICIT_H, scanner.readUnsigned());
            }
            else
            {
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_IMPLICIT_H, 1, 100);
            }
        }
        else if (next == 'r')
        {
            scanner.skip(1);
            if (qatom.get() == 0)
                throw Error("'r' specifier is allowed only for query molecules");

            if (isdigit(scanner.lookNext()))
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_SMALLEST_RING_SIZE, scanner.readUnsigned());
            else
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RING_BONDS, 1, 100);
        }
        else if (next == 'v')
        {
            scanner.skip(1);
            if (qatom.get() == 0)
                throw Error("'v' specifier is allowed only for query molecules");

            int val = 1;

            if (isdigit(scanner.lookNext()))
                val = scanner.readUnsigned();

            subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_TOTAL_BOND_ORDER, val);
        }
        else if (next == 'w')
        {
            scanner.skip(1);
            if (qatom.get() == 0)
                throw Error("'v' specifier is allowed only for query molecules");

            int val = 1;

            if (isdigit(scanner.lookNext()))
                val = scanner.readUnsigned();

            subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_SUBSTITUENTS, val);
        }
        else if (next == 'x' || next == 'q')
        {
            scanner.skip(1);
            if (qatom.get() == 0)
                throw Error("'x' specifier is allowed only for query molecules");

            if (isdigit(scanner.lookNext()))
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RING_BONDS, scanner.readUnsigned());
            else
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RING_BONDS, 1, 100);
        }
        else if (next == ':')
        {
            scanner.skip(1);
            if (scanner.lookNext() == '?')
            {
                if (_qmol == 0)
                    throw Error("ignorable AAM numbers are allowed only for queries");
                atom.ignorable_aam = true;
                scanner.skip(1);
            }
            atom.aam = scanner.readUnsigned();
        }
        else if (next == 'i')
        {
            scanner.skip(1);
            subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PI_BONDED, 1);
        }
        else
            throw Error("invalid character within atom description: '%c'", next);

        if (element > 0)
        {
            if (qatom.get() != 0)
                subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, element);
            else
            {
                if (element_assigned)
                    throw Error("two element labels for one atom");
                atom.label = element;
            }
            element_assigned = true;
        }

        if (aromatic != 0)
        {
            if (aromatic == ATOM_AROMATIC)
                atom.aromatic = true;

            if (qatom.get() != 0)
            {
                if (subatom.get() == 0)
                    subatom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_AROMATICITY, aromatic);
                else
                    subatom.reset(QueryMolecule::Atom::und(subatom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, aromatic)));
            }
        }

        if (subatom.get() != 0)
        {
            if (neg)
            {
                subatom.reset(QueryMolecule::Atom::nicht(subatom.release()));
                neg = false;
            }
            if (qatom != nullptr)
            {
                qatom.reset(QueryMolecule::Atom::und(qatom.release(), subatom.release()));
            }
            else
            {
                qatom = std::move(subatom);
            }
        }

        // we check for isotope_set here to treat [2H] as deuterium atom,
        // not like something with isotope number 2 and h-count 1
        if (!isotope_set)
            first_in_brackets = false;
    }
}

int SmilesLoader::_parseCurly(Array<char>& curly, int& repetitions)
{
    if (curly.size() == 1 && curly[0] == '-')
        return _POLYMER_START;

    if (curly.size() >= 2 && curly[0] == '+')
    {
        if (curly[1] == 'r')
            throw Error("ring repeating units not supported");
        if (curly[1] == 'n')
        {
            repetitions = 0;
            BufferScanner scanner(curly.ptr() + 2, curly.size() - 2);
            if (scanner.lookNext() == 'n')
            {
                scanner.skip(2);
                repetitions = scanner.readInt();
            }
            return _POLYMER_END;
        }
    }
    return 0;
}

void SmilesLoader::_readRGroupOccurrenceRanges(const char* str, Array<int>& ranges)
{
    int beg = -1, end = -1;
    int add_beg = 0, add_end = 0;

    while (*str != 0)
    {
        if (*str == '>')
        {
            end = 0xFFFF;
            add_beg = 1;
        }
        else if (*str == '<')
        {
            beg = 0;
            add_end = -1;
        }
        else if (isdigit(*str))
        {
            sscanf(str, "%d", beg == -1 ? &beg : &end);
            while (isdigit(*str))
                str++;
            continue;
        }
        else if (*str == ',')
        {
            if (end == -1)
                end = beg;
            else
                beg += add_beg, end += add_end;
            ranges.push((beg << 16) | end);
            beg = end = -1;
            add_beg = add_end = 0;
        }
        str++;
    }

    if (beg == -1 && end == -1)
        return;

    if (end == -1)
        end = beg;
    else
        beg += add_beg, end += add_end;
    ranges.push((beg << 16) | end);
}

SmilesLoader::_AtomDesc::_AtomDesc(Pool<List<int>::Elem>& neipool) : neighbors(neipool)
{
    label = 0;
    isotope = 0;
    charge = 0;
    hydrogens = -1;
    chirality = 0;
    aromatic = 0;
    aam = 0;
    ignorable_aam = false;
    brackets = false;
    star_atom = false;
    ends_polymer = false;
    starts_polymer = false;
    polymer_index = -1;

    parent = -1;
    rsite_num = 0;
}

SmilesLoader::_AtomDesc::~_AtomDesc()
{
}

void SmilesLoader::_AtomDesc::pending(int cycle)
{
    if (cycle < 1)
        throw Error("cycle number %d is not allowed", cycle);
    neighbors.add(-cycle);
}

void SmilesLoader::_AtomDesc::closure(int cycle, int end)
{
    int i;

    if (cycle < 1)
        throw Error("cycle number %d is not allowed", cycle);

    for (i = neighbors.begin(); i != neighbors.end(); i = neighbors.next(i))
    {
        if (neighbors.at(i) == -cycle)
        {
            neighbors.at(i) = end;
            break;
        }
    }
}

SmilesLoader::_BondDesc::_BondDesc() : beg(-1), end(-1), type(-1), dir(0), topology(0), index(-1)
{
}
