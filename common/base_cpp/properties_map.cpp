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

#include "base_cpp/properties_map.h"

using namespace indigo;

IMPL_ERROR(PropertiesMap, "properties map");

void PropertiesMap::copy(RedBlackStringObjMap< Array<char> > &other) {
   clear();
   for (int i = other.begin(); i != other.end(); i = other.next(i)) {
      insert(other.key(i), other.value(i).ptr());
   }
}
void PropertiesMap::copy(PropertiesMap& other) {
   clear();
   for(auto p : other.elements()) {
      insert(other.key(p), other.value(p));
   }
}
void PropertiesMap::insert(const char* key, const char* value) {
   int res;
   if (_properties.find(key)) {
      auto& val = _properties.at(key);
      if (value != 0)
         val.readString(value, true);
   } else {
      _propertyNames.add(key);
      int k = _properties.insert(key);
      if (value != 0)
         _properties.value(k).readString(value, true);
   }
}
Array<char>& PropertiesMap::insert(const char* key){
   insert (key, 0); 
   return valueBuf(key);
}
const char* PropertiesMap::key(int i) {
   return _propertyNames.at(i);
}

const char* PropertiesMap::value(int i) {
   auto& buf = valueBuf(_propertyNames.at(i));
   if(buf.size() > 0) {
      return buf.ptr();
   } else {
      return "";
   }
}

Array<char>& PropertiesMap::valueBuf(const char* key) {
   return _properties.at(key);
}

void PropertiesMap::clear() {
   _properties.clear();
   _propertyNames.clear();
}

bool PropertiesMap::contains(const char* key) {
   return _properties.find(key);
}

const char* PropertiesMap::at(const char* key) {
   return _properties.at(key).ptr();
}

void PropertiesMap::remove(const char* key) {
   if(_properties.find(key)) {
      _properties.remove(key);
      int to_remove = -1;
      for (auto i : _propertyNames.elements()) {
         if(strcmp(_propertyNames.at(i), key) == 0) {
            to_remove = i;
            break;
         }
      }
      if(to_remove >= 0) {
         _propertyNames.remove(to_remove);
      } else {
         throw Error("internal error with properties");
      }
   }
}

PropertiesMap::PrAuto PropertiesMap::elements() {
   return PrAuto(*this);
}


PropertiesMap::PrIter::PrIter(PropertiesMap &owner, int idx): _owner(owner), AutoIterator(idx) {
}

PropertiesMap::PrIter & PropertiesMap::PrIter::operator++(){
   _idx = _owner._propertyNames.next(_idx);
   return *this;
}

PropertiesMap::PrIter PropertiesMap::PrAuto::begin(){
   return PropertiesMap::PrIter(_owner, _owner._propertyNames.begin());
}

int PropertiesMap::PrAuto::next(int k) {
   return _owner._propertyNames.next(k);
}
PropertiesMap::PrIter PropertiesMap::PrAuto::end(){
   return PropertiesMap::PrIter(_owner, _owner._propertyNames.end());
}