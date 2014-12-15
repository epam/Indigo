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
   class PropertyData {
   public:
      Array<char> propertyName;
      Array<char> propertyValue;
      PropertyData(){};
   private:
      PropertyData(PropertyData&);
   };

   bool enabled;
   Array<char> fonttable;
   Array<char> colortable;
   Array<char> propertyNameCaption;
   Array<char> propertyValueCaption;
   ObjArray<PropertyData> property_data;

   RenderCdxmlContext() :enabled(false){};

private:
   RenderCdxmlContext(RenderCdxmlContext&);
};

class RenderParamCdxmlInterface {
public:
   static void render(RenderParams& params);
};

}

#endif // __render_cdxml_h__
