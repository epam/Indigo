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

#include "reaction/rsmiles_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"
#include "reaction/base_reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(RSmilesLoader, "reaction SMILES loader");

RSmilesLoader::RSmilesLoader(Scanner& scanner) : _scanner(scanner)
{
    ignore_closing_bond_direction_mismatch = false;
    smarts_mode = false;
    ignore_cistrans_errors = false;
    ignore_bad_valence = false;
}

int RSmilesLoader::_selectGroupByPair(int& lead_idx, int& idx, int rcnt, int ccnt, int pcnt) const
{
    if (lead_idx < rcnt)
        return 0;
    lead_idx -= rcnt;
    idx -= rcnt;
    if (lead_idx < ccnt)
        return 1;
    lead_idx -= ccnt;
    idx -= ccnt;
    if (lead_idx < pcnt)
        return 2;
    throw Error("RSmilesLoader::_selectGroup(): Index out of range");
}

int RSmilesLoader::_selectGroup(int& idx, int rcnt, int ccnt, int pcnt) const
{
    int iidx = idx;
    return _selectGroupByPair(iidx, idx, rcnt, ccnt, pcnt);
}

void RSmilesLoader::loadReaction(Reaction& reaction)
{
    _rxn = &reaction;
    _brxn = &reaction;
    _qrxn = 0;
    _loadReaction();
}

void RSmilesLoader::loadQueryReaction(QueryReaction& rxn)
{
    _rxn = 0;
    _brxn = &rxn;
    _qrxn = &rxn;
    _loadReaction();
}

void RSmilesLoader::_loadReaction()
{
    _brxn->clear();

    int i;

    std::unique_ptr<BaseMolecule> mols[3];
    std::unique_ptr<BaseMolecule>& rcnt = mols[0];
    std::unique_ptr<BaseMolecule>& ctlt = mols[1];
    std::unique_ptr<BaseMolecule>& prod = mols[2];

    QS_DEF(Array<int>, rcnt_aam);
    QS_DEF(Array<int>, ctlt_aam);
    QS_DEF(Array<int>, prod_aam);
    QS_DEF(Array<int>, rcnt_aam_ignorable);
    QS_DEF(Array<int>, prod_aam_ignorable);
    QS_DEF(Array<char>, buf);
    Array<int>* aams[] = {&rcnt_aam, &ctlt_aam, &prod_aam};
    Array<int>* ignorable_aams[] = {&rcnt_aam_ignorable, 0, &prod_aam_ignorable};

    // read the reactants
    buf.clear();
    while (1)
    {
        char c = _scanner.readChar();

        if (c == '>')
            break;
        buf.push(c);
    }

    BufferScanner r_scanner(buf);
    SmilesLoader r_loader(r_scanner);

    r_loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    r_loader.inside_rsmiles = true;
    r_loader.ignorable_aam = &rcnt_aam_ignorable;
    r_loader.smarts_mode = smarts_mode;
    r_loader.ignore_cistrans_errors = ignore_cistrans_errors;
    r_loader.stereochemistry_options = stereochemistry_options;
    r_loader.ignore_bad_valence = ignore_bad_valence;

    if (_rxn != 0)
    {
        rcnt = std::make_unique<Molecule>();
        r_loader.loadMolecule(static_cast<Molecule&>(*rcnt));
    }
    else
    {
        rcnt = std::make_unique<QueryMolecule>();
        r_loader.loadQueryMolecule(static_cast<QueryMolecule&>(*rcnt));
    }
    rcnt_aam.copy(rcnt->reaction_atom_mapping);

    // read the catalysts (agents)
    buf.clear();
    while (1)
    {
        char c = _scanner.readChar();

        if (c == '>')
            break;
        buf.push(c);
    }

    if (_rxn != 0)
        ctlt = std::make_unique<Molecule>();
    else
        ctlt = std::make_unique<QueryMolecule>();

    BufferScanner c_scanner(buf);
    SmilesLoader c_loader(c_scanner);

    c_loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    c_loader.inside_rsmiles = true;
    c_loader.smarts_mode = smarts_mode;
    c_loader.ignore_cistrans_errors = ignore_cistrans_errors;
    c_loader.stereochemistry_options = stereochemistry_options;
    c_loader.ignore_bad_valence = ignore_bad_valence;

    if (_rxn != 0)
    {
        ctlt = std::make_unique<Molecule>();
        c_loader.loadMolecule(static_cast<Molecule&>(*ctlt));
    }
    else
    {
        ctlt = std::make_unique<QueryMolecule>();
        c_loader.loadQueryMolecule(static_cast<QueryMolecule&>(*ctlt));
    }
    ctlt_aam.copy(ctlt->reaction_atom_mapping);

    bool vbar = false;

    // read the products
    buf.clear();
    while (!_scanner.isEOF())
    {
        char c = _scanner.readChar();

        if (c == '|')
        {
            vbar = true;
            break;
        }
        buf.push(c);
    }

    BufferScanner p_scanner(buf);
    SmilesLoader p_loader(p_scanner);

    p_loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    p_loader.inside_rsmiles = true;
    p_loader.ignorable_aam = &prod_aam_ignorable;
    p_loader.smarts_mode = smarts_mode;
    p_loader.ignore_cistrans_errors = ignore_cistrans_errors;
    p_loader.stereochemistry_options = stereochemistry_options;
    p_loader.ignore_bad_valence = ignore_bad_valence;

    if (_rxn != 0)
    {
        prod = std::make_unique<Molecule>();
        p_loader.loadMolecule(static_cast<Molecule&>(*prod));
    }
    else
    {
        prod = std::make_unique<QueryMolecule>();
        p_loader.loadQueryMolecule(static_cast<QueryMolecule&>(*prod));
    }
    prod_aam.copy(prod->reaction_atom_mapping);

    QS_DEF(Array<int>, r_fragments);
    QS_DEF(Array<int>, c_fragments);
    QS_DEF(Array<int>, p_fragments);
    Array<int>* fragments[] = {&r_fragments, &c_fragments, &p_fragments};

    r_fragments.clear_resize(rcnt->countComponents());
    c_fragments.clear_resize(ctlt->countComponents());
    p_fragments.clear_resize(prod->countComponents());

    for (i = 0; i < r_fragments.size(); i++)
        r_fragments[i] = i;

    for (i = 0; i < c_fragments.size(); i++)
        c_fragments[i] = i;

    for (i = 0; i < p_fragments.size(); i++)
        p_fragments[i] = i;

    bool have_highlighting = false;

    QS_DEF(Array<int>, hl_atoms);
    QS_DEF(Array<int>, hl_bonds);

    hl_atoms.clear_resize(rcnt->vertexCount() + ctlt->vertexCount() + prod->vertexCount());
    hl_bonds.clear_resize(rcnt->edgeCount() + ctlt->edgeCount() + prod->edgeCount());
    hl_atoms.zerofill();
    hl_bonds.zerofill();

    if (vbar)
    {
        BaseMolecule* stereo[] = {rcnt.get(), ctlt.get(), prod.get()};

        while (1)
        {
            char c = _scanner.readChar();

            if (c == '|')
                break;

            if (c == 'w')
            {
                if (_scanner.readChar() != ':')
                    throw Error("colon expected after 'w'");

                while (isdigit(_scanner.lookNext()))
                {
                    int idx = _scanner.readUnsigned();

                    int group = _selectGroup(idx, rcnt->vertexCount(), ctlt->vertexCount(), prod->vertexCount());
                    stereo[group]->addStereocenters(idx, MoleculeStereocenters::ATOM_ANY, 0, false);

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else if (c == 'a')
            {
                if (_scanner.readChar() != ':')
                    throw Error("colon expected after 'a'");

                while (isdigit(_scanner.lookNext()))
                {
                    int idx = _scanner.readUnsigned();

                    int group = _selectGroup(idx, rcnt->vertexCount(), ctlt->vertexCount(), prod->vertexCount());
                    stereo[group]->stereocenters.setType(idx, MoleculeStereocenters::ATOM_ABS, 0);

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else if (c == 'o')
            {
                int groupno = _scanner.readUnsigned();

                if (_scanner.readChar() != ':')
                    throw Error("colon expected after 'o'");

                while (isdigit(_scanner.lookNext()))
                {
                    int idx = _scanner.readUnsigned();

                    int group = _selectGroup(idx, rcnt->vertexCount(), ctlt->vertexCount(), prod->vertexCount());
                    stereo[group]->stereocenters.setType(idx, MoleculeStereocenters::ATOM_OR, groupno);

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else if (c == '&')
            {
                int groupno = _scanner.readUnsigned();

                if (_scanner.readChar() != ':')
                    throw Error("colon expected after '&'");

                while (isdigit(_scanner.lookNext()))
                {
                    int idx = _scanner.readUnsigned();

                    int group = _selectGroup(idx, rcnt->vertexCount(), ctlt->vertexCount(), prod->vertexCount());
                    stereo[group]->stereocenters.setType(idx, MoleculeStereocenters::ATOM_AND, groupno);

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else if (c == '^')
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

                    int group = _selectGroup(idx, rcnt->vertexCount(), ctlt->vertexCount(), prod->vertexCount());

                    if (_rxn != 0)
                        (dynamic_cast<Molecule&>(*mols[group])).setAtomRadical(idx, radical);
                    else
                    {
                        QueryMolecule& qmol = dynamic_cast<QueryMolecule&>(*mols[group]);
                        qmol.resetAtom(idx, (QueryMolecule::Atom*)QueryMolecule::Atom::und(qmol.releaseAtom(idx),
                                                                                           new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));
                    }

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else if (c == 'f')
            {
                if (_scanner.readChar() != ':')
                    throw Error("colon expected after 'f'");

                while (isdigit(_scanner.lookNext()))
                {
                    int idx = _scanner.readUnsigned();

                    while (_scanner.lookNext() == '.')
                    {
                        _scanner.skip(1);

                        int idx1 = idx;
                        int index_in_group = _scanner.readUnsigned();
                        int group = _selectGroupByPair(idx1, index_in_group, r_fragments.size(), c_fragments.size(), p_fragments.size());
                        (*fragments[group])[index_in_group] = idx1;
                    }

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
            else if (c == '$')
            {
                int k = rcnt->vertexCount() + ctlt->vertexCount() + prod->vertexCount();
                QS_DEF(Array<char>, label);

                for (i = 0; i < k; i++)
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
                    if (c == '$' && i != k - 1)
                        throw Error("only %d atoms found in pseudo-atoms $...$ block", i + 1);
                    if (label.size() > 0)
                    {
                        label.push(0);

                        int idx = i;
                        int group = _selectGroup(idx, rcnt->vertexCount(), ctlt->vertexCount(), prod->vertexCount());
                        int rnum;

                        if (label.size() > 3 && label[0] == '_' && label[1] == 'R' && sscanf(label.ptr() + 2, "%d", &rnum) == 1)
                        {
                            // ChemAxon's Extended SMILES notation for R-sites
                            if (_qrxn != 0)
                            {
                                QueryMolecule& qmol = dynamic_cast<QueryMolecule&>(*mols[group]);
                                qmol.resetAtom(idx, new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
                            }
                            mols[group]->allowRGroupOnRSite(idx, rnum);
                        }
                        else
                        {
                            if (_rxn != 0)
                                (dynamic_cast<Molecule&>(*mols[group])).setPseudoAtom(idx, label.ptr());
                            else
                            {
                                QueryMolecule& qmol = dynamic_cast<QueryMolecule&>(*mols[group]);
                                qmol.resetAtom(idx, (QueryMolecule::Atom*)QueryMolecule::Atom::und(
                                                        qmol.releaseAtom(idx), new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, label.ptr())));
                            }
                        }
                    }
                }
            }
            else if (c == 'h')
            {
                have_highlighting = true;

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
                        hl_atoms[idx] = 1;
                    else
                        hl_bonds[idx] = 1;

                    if (_scanner.lookNext() == ',')
                        _scanner.skip(1);
                }
            }
        }
    }

    // Read name
    Scanner* scanner_for_name;
    if (vbar)
        scanner_for_name = &_scanner;
    else
        scanner_for_name = &p_scanner;

    scanner_for_name->skipSpace();
    if (!scanner_for_name->isEOF())
        scanner_for_name->readLine(_brxn->name, true);

    std::unique_ptr<BaseMolecule> mol;
    QS_DEF(Array<int>, aam);
    QS_DEF(Array<int>, ignorable_aam);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, hl_atoms_frag);
    QS_DEF(Array<int>, hl_bonds_frag);

    if (_rxn != 0)
        mol = std::make_unique<Molecule>();
    else
        mol = std::make_unique<QueryMolecule>();

    for (int v = 0; v < 3; ++v)
    {
        for (i = 0; i < fragments[v]->size(); i++)
        {
            int j, k;

            if ((*fragments[v])[i] == -1)
                continue;

            mol->clear();
            aam.clear();
            ignorable_aam.clear();
            hl_atoms_frag.clear();
            hl_bonds_frag.clear();

            for (j = i; j < fragments[v]->size(); j++)
            {
                std::unique_ptr<BaseMolecule> fragment;

                if (_rxn != 0)
                    fragment = std::make_unique<Molecule>();
                else
                    fragment = std::make_unique<QueryMolecule>();

                if ((*fragments[v])[j] == i)
                {
                    (*fragments[v])[j] = -1;
                    Filter filt(mols[v]->getDecomposition().ptr(), Filter::EQ, j);
                    fragment->makeSubmolecule(*mols[v], filt, &mapping, 0);

                    mol->mergeWithMolecule(*fragment, 0);

                    for (k = 0; k < fragment->vertexCount(); k++)
                    {
                        aam.push((*aams[v])[mapping[k]]);
                        if (ignorable_aams[v] != 0)
                            ignorable_aam.push((*ignorable_aams[v])[mapping[k]]);

                        int idx = mapping[k];

                        for (int w = 0; w < v; w++)
                            idx += mols[w]->vertexCount();

                        hl_atoms_frag.push(hl_atoms[idx]);
                    }

                    for (k = 0; k < fragment->edgeCount(); k++)
                    {
                        const Edge& edge = fragment->getEdge(k);

                        int idx = mols[v]->findEdgeIndex(mapping[edge.beg], mapping[edge.end]);

                        if (idx < 0)
                            throw Error("internal: can not find edge");

                        for (int w = 0; w < v; w++)
                            idx += mols[w]->edgeCount();

                        hl_bonds_frag.push(hl_bonds[idx]);
                    }
                }
            }

            int idx;
            if (v == 0)
                idx = _brxn->addReactantCopy(*mol, 0, 0);
            else if (v == 1)
                idx = _brxn->addCatalystCopy(*mol, 0, 0);
            else if (v == 2)
                idx = _brxn->addProductCopy(*mol, 0, 0);

            _brxn->getAAMArray(idx).copy(aam);
            if (_qrxn != 0)
                _qrxn->getIgnorableAAMArray(idx).copy(ignorable_aam);

            if (have_highlighting)
            {
                Filter vfilter(hl_atoms_frag.ptr(), Filter::NEQ, 0);
                Filter efilter(hl_bonds_frag.ptr(), Filter::NEQ, 0);

                _brxn->getBaseMolecule(idx).highlightAtoms(vfilter);
                _brxn->getBaseMolecule(idx).highlightBonds(efilter);
            }
        }
    }
}
