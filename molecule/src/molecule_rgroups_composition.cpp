/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "base_cpp/array.h"
#include "molecule/base_molecule.h"

#include "molecule/molecule_rgroups_composition.h"

using namespace indigo;

IMPL_ERROR(MoleculeRGroupsComposition, "molecule rgroups composition");

MoleculeRGroupsComposition::MoleculeRGroupsComposition(BaseMolecule &mol)
: _mol(mol), _rgroups(_mol.rgroups),
  _rsites_count(_mol.countRSites()),
  _rgroups_count(_rgroups.getRGroupCount())
{
   _limits.resize(_rsites_count);
   _rgroup2size.resize(_rgroups_count+1);

   int rsite = 0;
   for (auto vertex : _mol.vertices()) {
      if (!_mol.isRSite(vertex)) { continue; }
      _rsite2vertex.insert(rsite, vertex);

      Array<int> rgroups;
      _mol.getAllowedRGroups(vertex, rgroups);
      _rsite2rgroup.insert(vertex, rgroups);

      int total = 0;
      for (int i = 0; i < rgroups.size(); i++) {
         int rgroup = rgroups[i];
         int size   = _rgroups.getRGroup(rgroup).fragments.size();
         _rgroup2size[rgroup] = size;
         total += size;
      }
      _limits[rsite] = total-1;
      rsite++;
   }
}

std::unique_ptr<Molecule> MoleculeRGroupsComposition::decorate(const AttachmentIter &at) const {
   Array<int> fs;
   at.dump(fs);
   return decorate(fs);
}

std::unique_ptr<Molecule> MoleculeRGroupsComposition::decorate(const Array<int> &at) const {
   std::unique_ptr<Molecule> result(new Molecule());
   decorate(at, *result.get());
   return result;
}

void MoleculeRGroupsComposition::decorate(const AttachmentIter &at, Molecule &mol) const {
   Array<int> fs;
   at.dump(fs);
   decorate(fs, mol);
}

void MoleculeRGroupsComposition::decorate(const Array<int> &fs, Molecule &mol) const {
   mol.clone(_mol, nullptr, nullptr);

   for (int i = 0; i < fs.size(); i++) {
      BaseMolecule &fragment = _fragment(i, fs[i]);

      int rsite = _rsite2vertex.at(i);
      int apcount = fragment.attachmentPointCount();
      int apoint = fragment.getAttachmentPoint(apcount, 0);

      Array<int> map;
      mol.mergeWithMolecule(fragment, &map);

      int atom = mol.getAtomNumber(map[apoint]);
      if (mol.mergeAtoms(rsite, map[apoint]) == rsite) {
         mol.resetAtom(rsite, atom);
      }
   }

   mol.removeAttachmentPoints();
   mol.rgroups.clear();
}

using MoleculeIter = MoleculeRGroupsComposition::MoleculeIter;

#define EMPTY_RGROUPS   std::unique_ptr<MoleculeRGroups>(new MoleculeRGroups())
#define MAKE_RGROUPS(T) std::unique_ptr<T>(new T(*this))

std::unique_ptr<MoleculeRGroups> MoleculeIter::modifyRGroups(const char *options) const {
   if (!strcmp(options, OPTION(ERASE)) || !strcmp(options, "")) {
      return EMPTY_RGROUPS;
   }
   if (!strcmp(options, OPTION(LEAVE))) {
      return MAKE_RGROUPS(SourceRGroups);
   }
   if (!strcmp(options, OPTION(ORDER))) {
      return MAKE_RGROUPS(OrderedRGroups);
   }
   return EMPTY_RGROUPS;
}

MoleculeIter::SourceRGroups::SourceRGroups(const MoleculeIter &m) {
   Array<int> fs;
   m._at.dump(fs);
   MultiMap<int, int> rgroup2fragment;
   for (auto i = 0; i < fs.size(); i++) {
      auto x = m._parent._fragment_coordinates(i, fs[i]);
      rgroup2fragment.insert(x.rgroup, x.fragment);
   }

   const RedBlackSet<int> &rgroups = rgroup2fragment.keys();
   for (auto i = rgroups.begin(); i != rgroups.end(); i = rgroups.next(i)) {
      auto r = rgroups.key(i);
      RGroup &rgroup = _rgroups.push();
      RGroup &source = m._parent._rgroups.getRGroup(r);

      const RedBlackSet<int> &fs_r = rgroup2fragment[r];
      for (auto j = fs_r.begin(); j != fs_r.end(); j = fs_r.next(j)) {
         Molecule *fragment = new Molecule();
         fragment->clone(*source.fragments[fs_r.key(j)], nullptr, nullptr);
         fragment->removeAttachmentPoints();
         rgroup.fragments.add(fragment);
      }
   }
}

MoleculeIter::OrderedRGroups::OrderedRGroups(const MoleculeIter &m) {
   Array<int> fs;
   m._at.dump(fs);
   for (auto i = 0; i < fs.size(); i++) {
      RGroup &rgroup = _rgroups.push();
      Molecule *fragment = new Molecule();
      fragment->clone(m._parent._fragment(i, fs[i]), nullptr, nullptr);
      fragment->removeAttachmentPoints();
      rgroup.fragments.add(fragment);
   }
}

using AttachmentIter = MoleculeRGroupsComposition::AttachmentIter;

bool AttachmentIter::operator!= (const AttachmentIter &other) const {
   if (_end && other._end) { return false; }
   if (_end != other._end) { return true; }

   for (auto i = 0; i < _fragments.size(); i++) {
      if (_fragments[i] != other._fragments[i]) {
         return true;
      }
   }
   return false;
}

AttachmentIter& AttachmentIter::operator++() {
   next();
   return *this;
}

//todo: gray codes? every digit has its own limit
bool AttachmentIter::next() {
   for (int i = 0; i < _size; i++) {
      if (_fragments[i] < _limits->at(i)) {
         _fragments[i]++;
         for (int j = 0; j < i; j++) {
            _fragments[j] = 0;
         }
         return true;
      }
   }
   _end = true;
   return false;
}

const Array<int>* AttachmentIter::operator*() const {
   return &_fragments;
}

void AttachmentIter::dump(Array<int> &other) const {
   other.copy(_fragments);
}