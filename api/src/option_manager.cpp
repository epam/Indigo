/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#include "option_manager.h"
#include "base_cpp/scanner.h"

ThreadSafeStaticObj<OptionManager> indigo_option_manager;

DLLEXPORT OptionManager & indigoGetOptionManager ()
{
   return indigo_option_manager.ref();
}

IMPL_ERROR(OptionManager, "option manager");

OptionManager::OptionManager ()
{
}

void OptionManager::callOptionHandlerInt (const char* name, int value) {
   CHECK_OPT_DEFINED(name);

   if (typeMap.at(name) == OPTION_BOOL && (value == 0 || value == 1))
   {
      hMapBool.at(name)(value);
      return;
   }

   CHECK_OPT_TYPE(name, OPTION_INT);
   hMapInt.at(name)(value);
}

void OptionManager::callOptionHandlerBool (const char* name, int value) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_BOOL);
   hMapBool.at(name)(value);
}

void OptionManager::callOptionHandlerFloat (const char* name, float value) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_FLOAT);
   hMapFloat.at(name)(value);
}

void OptionManager::callOptionHandlerColor (const char* name, float r, float g, float b) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_COLOR);
   hMapColor.at(name)(r, g, b);
}

void OptionManager::callOptionHandlerXY (const char* name, int x, int y) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_XY);
   hMapXY.at(name)(x, y);
}

bool OptionManager::hasOptionHandler (const char* name) {
   return typeMap.find(name);
}

void OptionManager::callOptionHandler (const char* name, const char* value) {
   if (!typeMap.find(name))
      throw Error("Property \"%s\" not defined", name);
   OPTION_TYPE type = typeMap.at(name);
   int x = 0, y = 0;
   float f = 0, r = 0, g = 0, b = 0;
   switch (type)
   {
   case OPTION_STRING:
      hMapString.at(name)(value);
      break;
   case OPTION_INT:
      if (_parseInt(value, x) < 0)
         throw Error("Cannot recognize \"%s\" as an integer value", value);
      hMapInt.at(name)(x);
      break;
   case OPTION_BOOL:
      if (_parseBool(value, x) < 0)
         throw Error("Cannot recognize \"%s\" as a boolean value", value);
      hMapBool.at(name)(x);
      break;
   case OPTION_FLOAT:
      if (_parseFloat(value, f) < 0)
         throw Error("Cannot recognize \"%s\" as a float value", value);
      hMapFloat.at(name)(f);
      break;
   case OPTION_COLOR:
      if (_parseColor(value, r, g, b) < 0)
         throw Error("Cannot recognize \"%s\" as a color value", value);
      hMapColor.at(name)(r, g, b);
      break;
   case OPTION_XY:
      if (_parseSize(value, x, y) < 0)
         throw Error("Cannot recognize \"%s\" as a pair of integers", value);
      hMapXY.at(name)(x, y);
      break;
   default:
      throw Error("Option type not supported");
   }
}

int OptionManager::nOptions () const {
   return typeMap.size();
}

int OptionManager::_parseInt (const char *str, int &val)
{
   if (sscanf(str, "%d", &val) != 1)
      return -1;
   return 1;
}

int OptionManager::_parseBool (const char *str, int &val)
{
   if (strcasecmp(str, "true") == 0 || strcasecmp(str, "on") == 0 ||
       strcasecmp(str, "yes") == 0) {
      val = 1;
      return 1;
   } else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "off") == 0 ||
              strcasecmp(str, "no") == 0) {
      val = 0;
      return 1;
   } else
      return _parseInt(str, val);
}

int OptionManager::_parseFloat (const char *str, float& val)
{
   BufferScanner scanner(str);
   if (!scanner.tryReadFloat(val))
      return -1;
   return 1;
}

int OptionManager::_parseColor (const char *str, float& r, float& g, float& b)
{
   BufferScanner scanner(str);
   if (!scanner.tryReadFloat(r))
      return -1;
   scanner.skipSpace();
   if (scanner.isEOF())
      return -1;
   if (scanner.readChar() != ',')
      return -1;
   scanner.skipSpace();
   if (!scanner.tryReadFloat(g))
      return -1;
   scanner.skipSpace();
   if (scanner.isEOF())
      return -1;
   if (scanner.readChar() != ',')
      return -1;
   scanner.skipSpace();
   if (!scanner.tryReadFloat(b))
      return -1;
   return 1;
}

int OptionManager::_parseSize (const char *str, int& w, int& h)
{
   if (sscanf(str, "%d,%d", &w, &h) != 2)
      return -1;
   return 1;
}
