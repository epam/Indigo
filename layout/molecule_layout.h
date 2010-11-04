/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __molecule_layout_h__
#define __molecule_layout_h__

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "layout/molecule_layout_graph.h"
#include "layout/metalayout.h"

class MoleculeLayout
{      
public:
   explicit DLLEXPORT MoleculeLayout (BaseMolecule &molecule);

   DLLEXPORT void make ();

   float bond_length;
   bool respect_existing_layout;
   Filter *filter;
   int  max_iterations;

   DEF_ERROR("molecule_layout");

protected:
   Metalayout::LayoutItem& _pushMol (Metalayout::LayoutLine& line, BaseMolecule& mol);
   BaseMolecule& _getMol (int id);
   void _make ();

   static BaseMolecule& cb_getMol (int id, void* context);
   static void cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

   void _init ();

   Metalayout _ml;
   BaseMolecule         &_molecule;
   MoleculeLayoutGraph  _layout_graph;
   Array<BaseMolecule*> _map;
   bool _query;
};

#endif
