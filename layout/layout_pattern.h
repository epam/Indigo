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

#ifndef __layout_pattern_h__
#define __layout_pattern_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "math/algebra.h"
#include "graph/graph.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo
{

struct PatternAtom
{
   explicit PatternAtom (Vec2f pos_) : pos(pos_) {}
   Vec2f pos;
};

struct PatternBond
{
   explicit PatternBond (int type_) : type(type_), parity(0) {}

   int type;        // see BOND_***
   int parity;
};

class DLLEXPORT PatternLayout : public Graph
{      
public:
   explicit PatternLayout ();
   virtual ~PatternLayout ();

   int addBond (int atom_beg, int atom_end, int type);
   int addAtom (float x, float y);
   int addOutlinePoint (float x, float y);
   bool isFixed () const { return _fixed; }
   void fix () { _fixed = true; }
   void setName (const char *name) { _name.readString(name, true); }
   const char * getName () const { return _name.ptr(); }

   void calcMorganCode ();
   long morganCode () const { return _morgan_code; }

   const PatternAtom  & getAtom       (int idx) const;
   const PatternBond  & getBond       (int idx) const;
   const Array<Vec2f> & getOutline    () const { return _outline; }

   DECL_ERROR;

protected:

   Array<PatternAtom> _atoms;
   Array<PatternBond> _bonds;

   Array<Vec2f> _outline;

   Array<char> _name;

   long _morgan_code;
   bool _fixed;

   // no implicit copy
   PatternLayout (const PatternLayout &);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
