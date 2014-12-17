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

#ifndef __render_cdxml_h__
#define __render_cdxml_h__
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"

namespace indigo {

class RenderParams;
class Molecule;
struct Vec2f;

class RenderCdxmlContext {
public:
   enum {
      ALIGNMENT_LEFT,
      ALIGNMENT_RIGHT
   };
   class PropertyData {
   public:
      Array<char> propertyName;
      Array<char> propertyValue;
      PropertyData(){};
   private:
      PropertyData(PropertyData&);
   };

   bool enabled;
   int keyAlignment;
   float propertyFontSize;
   Array<char> titleFont;
   Array<char> titleFace;
   Array<char> fonttable;
   Array<char> colortable;
   Array<char> propertyNameCaption;
   Array<char> propertyValueCaption;
   ObjArray<PropertyData> property_data;

   void clear() {
      enabled = false;
      keyAlignment = ALIGNMENT_LEFT;
      propertyFontSize = 12.0f;
      titleFont.clear();
      titleFace.clear();
      fonttable.clear();
      colortable.clear();
      propertyNameCaption.clear();
      propertyValueCaption.clear();
      property_data.clear();
   }

   RenderCdxmlContext() :enabled(false){
      clear();
   };

private:
   RenderCdxmlContext(RenderCdxmlContext&);
};

class RenderParamCdxmlInterface {
public:
   static void render(RenderParams& params);
};

}

#endif // __render_cdxml_h__
