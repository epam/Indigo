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

#ifndef __molecule_layout_h__
#define __molecule_layout_h__

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "layout/molecule_layout_graph.h"
#include "layout/metalayout.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class DLLEXPORT MoleculeLayout
{      
public:
   explicit MoleculeLayout (BaseMolecule &molecule);

   void make ();

   float bond_length;
   bool respect_existing_layout;
   Filter *filter;
   int  max_iterations;

   DECL_ERROR;

protected:
   Metalayout::LayoutItem& _pushMol (Metalayout::LayoutLine& line, BaseMolecule& mol);
   BaseMolecule& _getMol (int id);
   void _make ();
   void _makeLayout ();
   void _updateRepeatingUnits ();
   void _updateMultipleGroups ();

   static BaseMolecule& cb_getMol (int id, void* context);
   static void cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

   void _updateDataSGroups ();

   void _init ();

   Metalayout _ml;
   BaseMolecule          &_molecule;
   AutoPtr<BaseMolecule> _molCollapsed;
   BaseMolecule*         _bm;
   Array<int>            _atomMapping;
   MoleculeLayoutGraph   _layout_graph;
   Array<BaseMolecule*>  _map;
   bool _query;
   bool _hasMulGroups;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
