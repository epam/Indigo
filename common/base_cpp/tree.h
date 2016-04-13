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

namespace indigo {

class Tree {
public:

   Tree (int label)
   {
      this->label = label;
   }

   explicit Tree () : Tree(-1) {}

   Tree & insert (int label)
   {
      //TODO: check for presence of label and correct value
      return _children.push(label);
   }

   Tree & provide (int label)
   {
      Tree* tree = find(label);
      if (tree != nullptr) {
         return *tree;
      }
      return insert(label);
   }

   void insert(int label, int parent) {
      Tree& tree = provide(parent);
      tree.insert(label);
   }

   ObjArray<Tree> & children() {
      return _children;
   }

   int label;

protected:

   Tree * find(int label) {
      if (this->label == label) {
         return this;
      }
      for (auto i = 0; i < _children.size(); i++) {
         Tree & child = _children[i];
         if (child.label == label) {
            return &child;
         }
         Tree * deeper = child.find(label);
         if (deeper != nullptr) {
            return deeper;
         }
      }
      return nullptr;
   }

   ObjArray<Tree> _children;

private:
   Tree (const Tree &); //no implicit copy
};

}

#endif