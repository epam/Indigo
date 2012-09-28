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

#ifndef __metalayout_h__
#define __metalayout_h__

#include "base_cpp/reusable_obj_array.h"
#include "math/algebra.h"
#include "base_cpp/obj_array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;

class DLLEXPORT Metalayout {
public:
   struct DLLEXPORT LayoutItem {
      LayoutItem () {
         clear();
      }              
      void clear() {
         explicitVerticalOffset = false;
         over = false;
         fragment = false;
      }
      int type;
      int id;
      bool fragment;
      bool over;
      bool explicitVerticalOffset;
      float verticalOffset;

      Vec2f min, max;
      Vec2f scaledSize, scaledOffset;
      Vec2f scaleFactor;
   };

   class DLLEXPORT LayoutLine {
   public:
      LayoutLine ();
      ~LayoutLine ();
      void clear ();

      ObjArray<LayoutItem> items;
      float height;
      float width;
   private:
      LayoutLine (const LayoutLine&);
   };

   Metalayout ();
   void clear ();
   bool isEmpty () const;
   void prepare ();
   float getAverageBondLength () const;
   float getScaleFactor () const;
   const Vec2f& getContentSize () const;
   void setScaleFactor ();
   void process ();
   LayoutLine& newLine ();
   static void getBoundRect (Vec2f& min, Vec2f& max, BaseMolecule& mol);
   void calcContentSize();
   void scaleSz();

   void* context;
   void (*cb_process) (LayoutItem& item, const Vec2f& pos, void* context);
   BaseMolecule& (*cb_getMol) (int id, void* context);

   static float getTotalMoleculeBondLength (BaseMolecule& mol);
   static float getTotalMoleculeClosestDist (BaseMolecule& mol);

   // utility function to use in MoleculeLayout & ReactionLayout
   void adjustMol (BaseMolecule& mol, const Vec2f& min, const Vec2f& pos);

   float horizontalIntervalFactor;
   float verticalIntervalFactor;
   float bondLength;
   
   DECL_ERROR;
private:
   Vec2f _contentSize;
   float _avel, _scaleFactor, _offset;

   float _getAverageBondLength();

   ReusableObjArray<LayoutLine> _layout;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //__metalayout_h__
