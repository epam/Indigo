/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __properties_map_h__
#define __properties_map_h__

#include "base_cpp/auto_iter.h"
#include "base_cpp/red_black.h"
#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/array.h"

namespace indigo
{

class DLLEXPORT PropertiesMap {
public:
   DECL_ERROR;
   
   explicit PropertiesMap(){}
   ~PropertiesMap(){}
//   inline RedBlackStringObjMap< Array<char> >& getProperties() {
//      return _properties;
//   }
   void copy(RedBlackStringObjMap< Array<char> > &properties);
   void copy(PropertiesMap&);
   void insert(const char* key, const char* value);
   Array<char>& insert(const char* key);
   
   const char* key(int);
   const char* value(int);
   Array<char>& valueBuf(const char* key);
   
   bool contains(const char* key);
   const char* at(const char* key);
   void remove(const char* key);
   void clear();

   class PrIter : public AutoIterator {
   public:
      PrIter(PropertiesMap &owner, int idx);
      PrIter & operator++();

   private:
      PropertiesMap &_owner;
   };

   class PrAuto {
   public:
      PrAuto(PropertiesMap &owner) : _owner(owner) {
      }
      PrIter begin();
      int next(int);
      PrIter end();
   private:
      PropertiesMap &_owner;
   };

   
   PrAuto elements();

private:
   PropertiesMap(const PropertiesMap&);
   RedBlackStringObjMap< Array<char> > _properties;
   ObjArray< Array<char> > _propertyNames;
};



}

#endif // __auto_iter_h__
