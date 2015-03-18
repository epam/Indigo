/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __molecule_sgroups__
#define __molecule_sgroups__

#include "base_cpp/array.h"
#include "base_cpp/obj_poll.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;

class DLLEXPORT Sgroup
{
public:   
   explicit Sgroup ();
   virtual ~Sgroup ();

protected:
   int    sgroup_type;
   int    original_group;
   int    parent_group; // parent group number; represented with SPL in Molfile format 

   Array<int> atoms; // represented with SAL in Molfile format
   Array<int> bonds; // represented with SBL in Molfile format

   int    brk_style; // represented with SBT in Molfile format
   Array<Vec2f[2]> brackets; // represented with SDI in Molfile format

   explicit Sgroup (Sgroup &other);
};

class DLLEXPORT MoleculeSgroups
{
public:

   MoleculeSgroups ();
   ~MoleculeSgroups ();

   DECL_ERROR;

   Sgroup &getSgroup  (int idx);
   int getSgroupCount () const;

   void clear ();

protected:
   ObjPool<Sgroup> _sgroups;
};


}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
