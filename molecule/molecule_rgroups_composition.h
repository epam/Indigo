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

#ifndef __molecule_rgroups_composition__
#define __molecule_rgroups_composition__

#include "molecule/molecule.h"

#include "base_cpp/multimap.h"
#include "base_cpp/array.h"

#include <memory>

using namespace std;

namespace indigo {

class AttachmentIter {
public:
   AttachmentIter(const AttachmentIter &) = delete;
   AttachmentIter() : _limits(nullptr), _end(true), n(-1) {}
   ~AttachmentIter() {}

   AttachmentIter(int n, const Array<int> &limits)
      : _limits(&limits), _end(false), n(n) {
      _fragments.resize(n);
      _fragments.fill(0);
   }

   const Array<int>* operator* () const;
   bool operator!= (const AttachmentIter &other) const;
   AttachmentIter& operator++ ();

   void dump(Array<int> &other) const;
   bool next();

protected:
   const Array<int>* _limits;
   Array<int> _fragments;

   bool _end;
   int n;
};

class Attachments {
public:
   Attachments(const Attachments &) = delete;
   Attachments(int n, const Array<int>& limits)
      : _limits(limits),
      _end(), n(n) {}
   ~Attachments() {}

   unique_ptr<AttachmentIter> begin_ptr() const {
      return unique_ptr<AttachmentIter>(_begin());
   }
   AttachmentIter& begin() {
      AttachmentIter *ptr = _begin();
      _ptrs.add(ptr);
      return *ptr;
   }
   AttachmentIter& end() {
      return _end;
   }

private:
   AttachmentIter* _begin() const {
      return new AttachmentIter(n, _limits);
   }
   const Array<int>& _limits;

   PtrPool<AttachmentIter> _ptrs;
   AttachmentIter _end;

   const int n;
};

class MoleculeRGroupsComposition {
public:
   MoleculeRGroupsComposition(const MoleculeRGroupsComposition &) = delete;
   explicit MoleculeRGroupsComposition(BaseMolecule &mol);
   ~MoleculeRGroupsComposition () {};

   Attachments& attachments() const {
      _init(); return *_ats.get();
   }

   void decorate(const Array<int>     &at, Molecule &out) const;
   void decorate(const AttachmentIter &at, Molecule &out) const;

   unique_ptr<Molecule> decorate(const Array<int>     &at) const;
   unique_ptr<Molecule> decorate(const AttachmentIter &at) const;

   class MoleculeIter {
   public:
      MoleculeIter() = delete;
      MoleculeIter(AttachmentIter& at, const MoleculeRGroupsComposition& parent)
         : _parent(parent), _at(at) {}

      unique_ptr<Molecule> operator* () const {
         return _parent.decorate(_at);
      }
      bool operator!= (const MoleculeIter &other) const {
         return _at != other._at;
      }
      MoleculeIter& operator++ () {
         next(); return *this;
      }

      void dump(Molecule& out) const {
         _parent.decorate(_at, out);
      }
      bool next() const {
         return _at.next();
      }
   protected:
      const MoleculeRGroupsComposition& _parent;
      AttachmentIter& _at;
   };

   MoleculeIter begin() const {
      _init(); return MoleculeIter(_ats->begin(), *this);
   }
   MoleculeIter end() const {
      _init(); return MoleculeIter(_ats->end(),   *this);
   }

   DECL_ERROR;

private:
   inline void _init() const {
      if (_ats.get() == nullptr) {
         _ats.reset(new Attachments(n, _limits));
      }
   }

   BaseMolecule&    _mol;
   MoleculeRGroups& _rgroups;

   Array<int>            _limits;
   Array<int>            _rgroup2size;
   MultiMap<int, int>    _rsite2rgroup;
   RedBlackMap<int, int> _rsite2vertex;

   mutable unique_ptr<Attachments> _ats;

   int n, k;
};

}

#ifdef _WIN32
#endif

#endif