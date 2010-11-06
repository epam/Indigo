/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

ThreadSafeStaticObj<OptionManager> indigo_option_manager;

DLLEXPORT OptionManager & indigoGetOptionManager ()
{
   return indigo_option_manager.ref();
}

OptionManager::OptionManager ()
{
}

int OptionManager::callOptionHandlerInt (const char* name, int value) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_INT);
   return hMapInt.at(name)(value);
}

int OptionManager::callOptionHandlerBool (const char* name, int value) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_BOOL);
   return hMapBool.at(name)(value);
}

int OptionManager::callOptionHandlerFloat (const char* name, float value) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_FLOAT);
   return hMapFloat.at(name)(value);
}

int OptionManager::callOptionHandlerColor (const char* name, float r, float g, float b) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_COLOR);
   return hMapColor.at(name)(r, g, b);
}

int OptionManager::callOptionHandlerXY (const char* name, int x, int y) {
   CHECK_OPT_DEFINED(name);
   CHECK_OPT_TYPE(name, OPTION_XY);
   return hMapXY.at(name)(x, y);
}

bool OptionManager::hasOptionHandler (const char* name) {
   return typeMap.find(name);
}

int OptionManager::callOptionHandler (const char* name, const char* value) {
   if (!typeMap.find(name))
      throw Error("Property \"%s\" not defined", name);
   OPTION_TYPE type = typeMap.at(name);
   int x = 0, y = 0;
   float f = 0, r = 0, g = 0, b = 0;
   switch (type)
   {
   case OPTION_STRING:
      return hMapString.at(name)(value);
   case OPTION_INT:
      if (_parseInt(value, x) < 0)
         throw Error("Cannot recognize \"%s\" as an integer value", value);
      return hMapInt.at(name)(x);
   case OPTION_BOOL:
      if (_parseBool(value, x) < 0)
         throw Error("Cannot recognize \"%s\" as a boolean value", value);
      return hMapBool.at(name)(x);
   case OPTION_FLOAT:
      if (_parseFloat(value, f) < 0)
         throw Error("Cannot recognize \"%s\" as a float value", value);
      return hMapFloat.at(name)(f);
   case OPTION_COLOR:
      if (_parseColor(value, r, g, b) < 0)
         throw Error("Cannot recognize \"%s\" as a color value", value);
      return hMapColor.at(name)(r, g, b);
   case OPTION_XY:
      if (_parseSize(value, x, y) < 0)
         throw Error("Cannot recognize \"%s\" as a pair of integers", value);
      return hMapXY.at(name)(x, y);
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
   if (strcasecmp(str, "true") == 0) {
      val = 1;
      return 1;
   } else if (strcasecmp(str, "false") == 0) {
      val = 0;
      return 1;
   } else
      return _parseInt(str, val);
}

int OptionManager::_parseFloat (const char *str, float& val)
{
   if (sscanf(str, "%f", &val) != 1)
      return -1;
   return 1;
}

int OptionManager::_parseColor (const char *str, float& r, float& g, float& b)
{
   if (sscanf(str, "%f,%f,%f", &r, &g, &b) != 3)
      return -1;
   return 1;
}

int OptionManager::_parseSize (const char *str, int& w, int& h)
{
   if (sscanf(str, "%d,%d", &w, &h) != 2)
      return -1;
   return 1;
}

/*
DEF_SET_OPT_HANDLER(String, optf_string_t, OPTION_STRING, hMapString)
DEF_SET_OPT_HANDLER(Int, optf_int_t, OPTION_INT, hMapInt)
DEF_SET_OPT_HANDLER(Bool, optf_bool_t, OPTION_BOOL, hMapBool)
DEF_SET_OPT_HANDLER(Float, optf_float_t, OPTION_FLOAT, hMapFloat)
DEF_SET_OPT_HANDLER(Color, optf_color_t, OPTION_COLOR, hMapColor)
DEF_SET_OPT_HANDLER(XY, optf_xy_t, OPTION_XY, hMapXY)
*/