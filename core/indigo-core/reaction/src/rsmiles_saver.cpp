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

#include "reaction/rsmiles_saver.h"

#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(RSmilesSaver, "reaction SMILES saver");

CP_DEF(RSmilesSaver);

RSmilesSaver::RSmilesSaver(Output& output) : _output(output), CP_INIT, TL_CP_GET(_written_atoms), TL_CP_GET(_written_bonds), TL_CP_GET(_ncomp)
{
    smarts_mode = false;
    chemaxon = true;
}

void RSmilesSaver::saveReaction(Reaction& reaction)
{
    _rxn = &reaction;
    _brxn = &reaction;
    _qrxn = 0;
    _saveReaction();
}

void RSmilesSaver::saveQueryReaction(QueryReaction& reaction)
{
    _qrxn = &reaction;
    _brxn = &reaction;
    _rxn = 0;
    _saveReaction();
}

SmilesSaver& RSmilesSaver::_addMoleculeSaver()
{
    auto saver = std::make_unique<SmilesSaver>(_output);
    saver->write_extra_info = false;
    saver->chemaxon = false;
    saver->separate_rsites = false;
    saver->rsite_indices_as_aam = false;
    saver->smarts_mode = smarts_mode;
    saver->inside_rsmiles = true;
    _smiles_savers.emplace_back(std::move(saver));
    return *_smiles_savers.back();
}

void RSmilesSaver::_writeMolecule(int i)
{
    SmilesSaver& saver = _addMoleculeSaver();

    if (_rxn != 0)
        saver.saveMolecule(_rxn->getMolecule(i));
    else
        saver.saveQueryMolecule(_qrxn->getQueryMolecule(i));

    _ncomp.push(saver.writtenComponents());

    const Array<int>& atoms = saver.writtenAtoms();
    int j;

    for (j = 0; j < atoms.size(); j++)
    {
        _Idx& idx = _written_atoms.push();

        idx.mol = i;
        idx.idx = atoms[j];
    }

    const Array<int>& bonds = saver.writtenBonds();

    for (j = 0; j < bonds.size(); j++)
    {
        _Idx& idx = _written_bonds.push();

        idx.mol = i;
        idx.idx = bonds[j];
    }
}

void RSmilesSaver::_saveReaction()
{
    _written_atoms.clear();
    _written_bonds.clear();
    _ncomp.clear();

    bool dot = false;
    for (auto i : _brxn->reactants)
    {
        if (dot)
            _output.writeChar('.');
        else
            dot = true;

        _writeMolecule(i);
    }

    _output.writeString(">");

    dot = false;
    for (auto i : _brxn->catalysts)
    {
        if (dot)
            _output.writeChar('.');
        else
            dot = true;

        _writeMolecule(i);
    }

    _output.writeString(">");

    dot = false;

    for (auto i : _brxn->intermediates)
    {
        if (dot)
            _output.writeChar('.');
        else
            dot = true;

        _writeMolecule(i);
    }

    for (auto i : _brxn->products)
    {
        if (dot)
            _output.writeChar('.');
        else
            dot = true;

        _writeMolecule(i);
    }

    if (chemaxon && !smarts_mode)
    {
        _comma = false;
        _writeFragmentsInfo();
        _writeRingCisTrans();
        _writeStereogroups();
        _writeRadicals();
        _writePseudoAtoms();
        _writeHighlighting();

        if (_comma)
            _output.writeChar('|');
    }
}

void RSmilesSaver::_writeFragmentsInfo()
{
    int i, j, cnt = 0;

    for (i = 0; i < _ncomp.size(); i++)
    {
        if (_ncomp[i] > 1)
            break;
        cnt += _ncomp[i];
    }

    if (i == _ncomp.size())
        return;

    _startExtension();

    _output.writeString("f:");

    bool comma = false;

    for (; i < _ncomp.size(); i++)
    {
        if (_ncomp[i] > 1)
        {
            if (comma)
                _output.writeChar(',');
            else
                comma = true;
            _output.printf("%d", cnt);
            for (j = 1; j < _ncomp[i]; j++)
                _output.printf(".%d", cnt + j);
        }
        cnt += _ncomp[i];
    }
}

// TODO: this function need refactoring
void RSmilesSaver::_writeStereogroups()
{
    QS_DEF(Array<int>, marked);
    int i, j;

    for (i = 0; i < _written_atoms.size(); i++)
    {
        const _Idx& idx = _written_atoms[i];
        int type = _brxn->getBaseMolecule(idx.mol).stereocenters.getType(idx.idx);

        if (type != 0 && type != MoleculeStereocenters::ATOM_ABS)
            break;
    }

    if (i == _written_atoms.size())
        return;

    marked.clear_resize(_written_atoms.size());
    marked.zerofill();

    int and_group_idx = 1;
    int or_group_idx = 1;

    for (i = 0; i < _written_atoms.size(); i++)
    {
        if (marked[i])
            continue;

        const _Idx& idx = _written_atoms[i];

        int type = _brxn->getBaseMolecule(idx.mol).stereocenters.getType(idx.idx);

        if (type > 0)
        {
            if (_comma)
                _output.writeChar(',');
            else
            {
                _output.writeString(" |");
                _comma = true;
            }
        }

        if (type == MoleculeStereocenters::ATOM_ANY)
        {
            _output.printf("w:%d", i);

            for (j = i + 1; j < _written_atoms.size(); j++)
            {
                const _Idx& jdx = _written_atoms[j];

                if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_ANY)
                {
                    marked[j] = 1;
                    _output.printf(",%d", j);
                }
            }
        }
        else if (type == MoleculeStereocenters::ATOM_ABS)
        {
            _output.printf("a:%d", i);

            for (j = i + 1; j < _written_atoms.size(); j++)
            {
                const _Idx& jdx = _written_atoms[j];

                if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_ABS)
                {
                    marked[j] = 1;
                    _output.printf(",%d", j);
                }
            }
        }
        else if (type == MoleculeStereocenters::ATOM_AND)
        {
            int group = _brxn->getBaseMolecule(idx.mol).stereocenters.getGroup(idx.idx);

            _output.printf("&%d:%d", and_group_idx++, i);
            for (j = i + 1; j < _written_atoms.size(); j++)
            {
                const _Idx& jdx = _written_atoms[j];

                if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_AND &&
                    _brxn->getBaseMolecule(jdx.mol).stereocenters.getGroup(jdx.idx) == group)
                {
                    marked[j] = 1;
                    _output.printf(",%d", j);
                }
            }
        }
        else if (type == MoleculeStereocenters::ATOM_OR)
        {
            int group = _brxn->getBaseMolecule(idx.mol).stereocenters.getGroup(idx.idx);

            _output.printf("o%d:%d", or_group_idx++, i);
            for (j = i + 1; j < _written_atoms.size(); j++)
            {
                const _Idx& jdx = _written_atoms[j];

                if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_OR &&
                    _brxn->getBaseMolecule(jdx.mol).stereocenters.getGroup(jdx.idx) == group)
                {
                    marked[j] = 1;
                    _output.printf(",%d", j);
                }
            }
        }
    }
}

void RSmilesSaver::_writeRadicals()
{
    int atoms_offset = 0;
    int cur_radical = -1;
    for (size_t i = 0; i < _smiles_savers.size(); ++i)
    {
        auto& smiles_saver = _smiles_savers[i];
        smiles_saver->setComma(_comma);
        cur_radical = smiles_saver->writeRadicals(atoms_offset, cur_radical);
        atoms_offset += smiles_saver->writtenAtoms().size();
        _comma = smiles_saver->getComma();
    }
}

void RSmilesSaver::_startExtension()
{
    if (_comma)
        _output.writeChar(',');
    else
    {
        _output.writeString(" |");
        _comma = true;
    }
}

void RSmilesSaver::_writePseudoAtoms()
{
    int atoms_offset = 0;
    for (int i = 0; i < _written_atoms.size(); i++)
    {
        BaseMolecule& mol = _brxn->getBaseMolecule(_written_atoms[i].mol);
        int idx = _written_atoms[i].idx;
        if ((mol.isPseudoAtom(idx) || mol.isAlias(idx)) || (mol.isRSite(idx) && mol.getRSiteBits(idx) > 0))
        {
            _startExtension();
            _output.writeChar('$');
            for (size_t i = 0; i < _smiles_savers.size(); ++i)
            {
                auto& smiles_saver = _smiles_savers[i];
                smiles_saver->writePseudoAtoms(atoms_offset, false);
                atoms_offset += smiles_saver->writtenAtoms().size();
            }
            _output.writeChar('$');
            break;
        }
    }
}

void RSmilesSaver::_writeHighlighting()
{
    for (int j = 0; j < 2; ++j)
    {
        int offset = 0;
        bool is_cont = false;
        for (size_t i = 0; i < _smiles_savers.size(); ++i)
        {
            auto& smiles_saver = _smiles_savers[i];
            smiles_saver->setComma(_comma);
            is_cont = j == 0 ? smiles_saver->writeHighlightedAtoms(offset, is_cont) : smiles_saver->writeHighlightedBonds(offset, is_cont);
            _comma = smiles_saver->getComma();
            offset += j == 0 ? smiles_saver->writtenAtoms().size() : smiles_saver->writtenBonds().size();
        }
    }
}

void RSmilesSaver::_writeRingCisTrans()
{
    int bonds_offset = 0;
    for (size_t i = 0; i < _smiles_savers.size(); ++i)
    {
        auto& smiles_saver = _smiles_savers[i];
        smiles_saver->setComma(_comma);
        smiles_saver->writeRingCisTrans(bonds_offset);
        bonds_offset += smiles_saver->writtenBonds().size();
        _comma = smiles_saver->getComma();
    }
}
