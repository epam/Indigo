/****************************************************************************
* Copyright (C) 2009-2016 EPAM Systems
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

#ifndef __tree_h__
#define __tree_h__

#include "obj_array.h"
#include "non_copyable.h"

namespace indigo {

class Tree : public NonCopyable {
public:

   Tree (int label)
   {
      this->label = label;
   }

   explicit Tree () : Tree(-1) {}

   void insert (int label, int parent) {
      Tree* present = _find(parent);
      if (present != nullptr) {
         present->_insert(label);
      } else {
         _insert(parent)._insert(label);
      }
   }

   void insert (int label)
   {
      insert(label, -1);
   }

   ObjArray<Tree> & children () {
      return _children;
   }

   int label;

protected:

   Tree & _insert (int label)
   {
      return _children.push(label);
   }

   Tree * _find (int label)
   {
      if (this->label == label) {
         return this;
      }
      for (auto i = 0; i < _children.size(); i++) {
         Tree & child = _children[i];
         if (child.label == label) {
            return &child;
         }
         Tree * deeper = child._find(label);
         if (deeper != nullptr) {
            return deeper;
         }
      }
      return nullptr;
   }

   ObjArray<Tree> _children;
};

}

#endif