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

namespace indigo {

class MoleculeRGroupsComposition : NonCopyable {
public:
   explicit MoleculeRGroupsComposition(BaseMolecule &mol);
   ~MoleculeRGroupsComposition () {};

   //State of search abstracted from chemistry details
   class AttachmentIter : NonCopyable {
   public:
      //Default constructor is used to indicate final state
      AttachmentIter() : _limits(nullptr), _end(true), _size(-1) {}
      ~AttachmentIter() {}

      //Constructs (0,...,0) state, limits indicates max number on every site
      AttachmentIter(int size, const Array<int> &limits)
         : _limits(&limits), _end(false), _size(size) {
         _fragments.resize(size);
         _fragments.fill(0);
      }

      //Operators *, != and ++ are used for range-loop iteration
      const Array<int>* operator* () const;
      bool operator!= (const AttachmentIter &other) const;
      AttachmentIter& operator++ ();

      //Output numbers assigned to sites
      void dump(Array<int> &other) const;

      //Move to next state, returns true if the iterator has next element
      bool next();

   protected:
      const Array<int>* _limits;
      Array<int> _fragments;

      bool _end;
      int _size;
   };

   //Collection of search states
   class Attachments : NonCopyable {
   public:
      Attachments(int size, const Array<int>& limits)
         : _limits(limits),
         _end(), _size(size) {}
      ~Attachments() {}

      std::unique_ptr<AttachmentIter> begin_ptr() const {
         return std::unique_ptr<AttachmentIter>(_begin());
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
         return new AttachmentIter(_size, _limits);
      }
      const Array<int>& _limits;

      PtrPool<AttachmentIter> _ptrs;
      AttachmentIter _end;

      const int _size;
   };

   //Searchs all possible assignments of rgroup numbers to sites
   Attachments& attachments() const {
      _init(); return *_ats.get();
   }

   //Assembles result molecule from abstract assigment of numbers to sites
   void decorate(const Array<int>     &at, Molecule &out) const;
   void decorate(const AttachmentIter &at, Molecule &out) const;

   std::unique_ptr<Molecule> decorate(const Array<int>     &at) const;
   std::unique_ptr<Molecule> decorate(const AttachmentIter &at) const;

   //Iterator for result molecules, essentially wrapper of AttachmentIter
   class MoleculeIter {
   public:
      MoleculeIter() = delete;
      MoleculeIter(AttachmentIter& at, const MoleculeRGroupsComposition& parent)
         : _parent(parent), _at(at) {}

      //Operators *, != and ++ are used for range-loop iteration
      std::unique_ptr<Molecule> operator* () const {
         return _parent.decorate(_at);
      }
      bool operator!= (const MoleculeIter &other) const {
         return _at != other._at;
      }
      MoleculeIter& operator++ () {
         next(); return *this;
      }

      //Assemble resulting molecule from attachment
      void dump(Molecule& out) const {
         _parent.decorate(_at, out);
      }

      //Shift to next molecule, returns true if there is next molecule after
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
      _init(); return MoleculeIter(_ats->end(), *this);
   }

   DECL_ERROR;

private:
   inline void _init() const {
      if (_ats.get() == nullptr) {
         _ats.reset(new Attachments(_rsites_count, _limits));
      }
   }

   BaseMolecule&    _mol;
   MoleculeRGroups& _rgroups;

   Array<int>            _limits;
   Array<int>            _rgroup2size;
   MultiMap<int, int>    _rsite2rgroup;
   RedBlackMap<int, int> _rsite2vertex;

   mutable std::unique_ptr<Attachments> _ats;

   int _rsites_count, _rgroups_count;
};

}

#ifdef _WIN32
#endif

#endif