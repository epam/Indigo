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

#ifndef __metalayout_h__
#define __metalayout_h__

#include "base_cpp/reusable_obj_array.h"

class BaseMolecule;
enum ALIGNMENT {ALIGNMENT_LEFT = 0, ALIGNMENT_CENTER, ALIGNMENT_RIGHT};

class Metalayout {
public:
   struct LayoutItem {
      int type;
      int id;
      bool fragment;
      bool over;

      Vec2f min, max;
      Vec2f scaledSize, scaledOffset;
      Vec2f scaleFactor;
   };

   class LayoutLine {
   public:
      DLLEXPORT LayoutLine ();
      DLLEXPORT ~LayoutLine ();
      DLLEXPORT void clear ();

      Array<LayoutItem> items;
      float height;
      float width;
   private:
      LayoutLine (const LayoutLine&);
   };

   DLLEXPORT Metalayout ();
   DLLEXPORT void clear ();
   DLLEXPORT bool isEmpty () const;
   DLLEXPORT void prepare ();
   DLLEXPORT float getAverageBondLength () const;
   DLLEXPORT float getScaleFactor () const;
   DLLEXPORT const Vec2f& getContentSize () const;
   DLLEXPORT void setScaleFactor ();
   DLLEXPORT void process ();
   DLLEXPORT LayoutLine& newLine ();
   DLLEXPORT static void getBoundRect (Vec2f& min, Vec2f& max, BaseMolecule& mol);
   DLLEXPORT void calcContentSize();
   DLLEXPORT void scaleSz();

   void* context;
   void (*cb_process) (LayoutItem& item, const Vec2f& pos, void* context);
   BaseMolecule& (*cb_getMol) (int id, void* context);

   DLLEXPORT static float getTotalMoleculeBondLength (BaseMolecule& mol);
   DLLEXPORT static float getTotalMoleculeClosestDist (BaseMolecule& mol);

   // utility function to use in MoleculeLayout & ReactionLayout
   void adjustMol (BaseMolecule& mol, const Vec2f& min, const Vec2f& pos);

   float horizontalIntervalFactor;
   float verticalIntervalFactor;
   float bondLength;
   
   DEF_ERROR("metalayout");
private:
   Vec2f _contentSize;
   float _avel, _scaleFactor, _offset;

   float _getAverageBondLength();

   ReusableObjArray<LayoutLine> _layout;
};

#endif //__metalayout_h__
