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

#include "base_cpp/array.h"
#include "molecule/base_molecule.h"

#include "molecule/molecule_rgroups_composition.h"

using namespace indigo;

IMPL_ERROR(MoleculeRGroupsComposition, "molecule rgroups composition");

MoleculeRGroupsComposition::MoleculeRGroupsComposition(BaseMolecule& mol)
    : _mol(mol), _rgroups(_mol.rgroups), _rsites_count(_mol.countRSites()), _rgroups_count(_rgroups.getRGroupCount())
{
    _limits.resize(_rsites_count);
    _rgroup2size.resize(_rgroups_count + 1);

    int rsite = 0;
    for (auto vertex : _mol.vertices())
    {
        if (!_mol.isRSite(vertex))
        {
            continue;
        }
        _rsite2vertex.insert(rsite, vertex);

        ArrayInt rgroups;
        _mol.getAllowedRGroups(vertex, rgroups);
        _rsite2rgroup.insert(vertex, rgroups);

        int total = 0;
        for (int i = 0; i < rgroups.size(); i++)
        {
            int rgroup = rgroups[i];
            int size = _rgroups.getRGroup(rgroup).fragments.size();
            _rgroup2size[rgroup] = size;
            total += size;
        }
        _limits[rsite] = total - 1;
        rsite++;
    }
}

std::unique_ptr<Molecule> MoleculeRGroupsComposition::decorate(const AttachmentIter& at) const
{
    ArrayInt fs;
    at.dump(fs);
    return decorate(fs);
}

std::unique_ptr<Molecule> MoleculeRGroupsComposition::decorate(const ArrayInt& at) const
{
    std::unique_ptr<Molecule> result(new Molecule());
    decorate(at, *result.get());
    return result;
}

void MoleculeRGroupsComposition::decorate(const AttachmentIter& at, Molecule& mol) const
{
    ArrayInt fs;
    at.dump(fs);
    decorate(fs, mol);
}

void MoleculeRGroupsComposition::decorate(const ArrayInt& fs, Molecule& mol) const
{
    mol.clone(_mol, nullptr, nullptr);

    for (int i = 0; i < fs.size(); i++)
    {
        BaseMolecule& fragment = _fragment(i, fs[i]);

        int rsite = _rsite2vertex.at(i);
        int apcount = fragment.attachmentPointCount();
        int apoint = fragment.getAttachmentPoint(apcount, 0);

        ArrayInt map;
        mol.mergeWithMolecule(fragment, &map);

        int atom = mol.getAtomNumber(map[apoint]);
        if (mol.mergeAtoms(rsite, map[apoint]) == rsite)
        {
            mol.resetAtom(rsite, atom);
        }
    }

    mol.removeAttachmentPoints();
    mol.rgroups.clear();
}

using MoleculeIter = MoleculeRGroupsComposition::MoleculeIter;

#define EMPTY_RGROUPS std::unique_ptr<MoleculeRGroups>(new MoleculeRGroups())
#define MAKE_RGROUPS(T) std::unique_ptr<T>(new T(*this))

std::unique_ptr<MoleculeRGroups> MoleculeIter::modifyRGroups(const char* options) const
{
    if (!strcmp(options, OPTION(ERASE)) || !strcmp(options, ""))
    {
        return EMPTY_RGROUPS;
    }
    if (!strcmp(options, OPTION(LEAVE)))
    {
        return MAKE_RGROUPS(SourceRGroups);
    }
    if (!strcmp(options, OPTION(ORDER)))
    {
        return MAKE_RGROUPS(OrderedRGroups);
    }
    return EMPTY_RGROUPS;
}

MoleculeIter::SourceRGroups::SourceRGroups(const MoleculeIter& m)
{
    ArrayInt fs;
    m._at.dump(fs);
    MultiMap<int, int> rgroup2fragment;
    RedBlackMap<Fragment, int> fragment2count;
    for (auto i = 0; i < fs.size(); i++)
    {
        auto x = m._parent._fragment_coordinates(i, fs[i]);
        if (rgroup2fragment.find(x.rgroup, x.fragment))
        {
            int count = fragment2count.at(x);
            fragment2count.remove(x);
            fragment2count.insert(x, count + 1);
        }
        else
        {
            rgroup2fragment.insert(x.rgroup, x.fragment);
            fragment2count.insert(x, 1);
        }
    }

    const RedBlackSet<int>& rgroups = rgroup2fragment.keys();
    for (auto i = rgroups.begin(); i != rgroups.end(); i = rgroups.next(i))
    {
        auto r = rgroups.key(i);
        RGroup& rgroup = _rgroups.push();
        RGroup& source = m._parent._rgroups.getRGroup(r);

        const RedBlackSet<int>& fs_r = rgroup2fragment[r];
        for (auto j = fs_r.begin(); j != fs_r.end(); j = fs_r.next(j))
        {
            auto f = fs_r.key(j);
            for (auto k = 0; k < fragment2count.at({r, f}); k++)
            {
                int fr_idx = rgroup.fragments.add(new Molecule());
                BaseMolecule* fragment = rgroup.fragments.at(fr_idx);
                fragment->clone(*source.fragments[f], nullptr, nullptr);
                fragment->removeAttachmentPoints();
            }
        }
    }
}

MoleculeIter::OrderedRGroups::OrderedRGroups(const MoleculeIter& m)
{
    ArrayInt fs;
    m._at.dump(fs);
    for (auto i = 0; i < fs.size(); i++)
    {
        RGroup& rgroup = _rgroups.push();
        int fr_idx = rgroup.fragments.add(new Molecule());
        BaseMolecule* fragment = rgroup.fragments.at(fr_idx);
        fragment->clone(m._parent._fragment(i, fs[i]), nullptr, nullptr);
        fragment->removeAttachmentPoints();
    }
}

using AttachmentIter = MoleculeRGroupsComposition::AttachmentIter;

bool AttachmentIter::operator!=(const AttachmentIter& other) const
{
    if (_end && other._end)
    {
        return false;
    }
    if (_end != other._end)
    {
        return true;
    }

    for (auto i = 0; i < _fragments.size(); i++)
    {
        if (_fragments[i] != other._fragments[i])
        {
            return true;
        }
    }
    return false;
}

AttachmentIter& AttachmentIter::operator++()
{
    next();
    return *this;
}

// todo: gray codes? every digit has its own limit
bool AttachmentIter::next()
{
    for (int i = 0; i < _size; i++)
    {
        if (_fragments[i] < _limits->at(i))
        {
            _fragments[i]++;
            for (int j = 0; j < i; j++)
            {
                _fragments[j] = 0;
            }
            return true;
        }
    }
    _end = true;
    return false;
}

const ArrayInt* AttachmentIter::operator*() const
{
    return &_fragments;
}

void AttachmentIter::dump(ArrayInt& other) const
{
    other.copy(_fragments);
}