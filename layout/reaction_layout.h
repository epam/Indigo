/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __reaction_layout__

#include "layout/metalayout.h"

namespace indigo {

class Reaction;
class Molecule;
struct Vec2f;

class ReactionLayout {
public:
   explicit ReactionLayout (BaseReaction& r);

   void make ();

   float bond_length;
   float plus_interval_factor;
   float arrow_interval_factor;
   bool preserve_molecule_layout;
   int  max_iterations;

private:
   Metalayout::LayoutItem& _pushMol (Metalayout::LayoutLine& line, int id);
   Metalayout::LayoutItem& _pushSpace (Metalayout::LayoutLine& line, float size);
   BaseMolecule& _getMol (int id);
   void _shiftMol(const Metalayout::LayoutItem& item, const Vec2f& pos);
   void _make ();

   static BaseMolecule& cb_getMol (int id, void* context);
   static void cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

   ReactionLayout (const ReactionLayout& r); // no implicit copy

   BaseReaction& _r;
   Metalayout _ml;
};

}

#endif
