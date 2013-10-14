#include "bingo_properties.h"

#include "base_cpp/exception.h"
#include "base_cpp/profiling.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <limits.h>

using namespace bingo;
using namespace indigo;

Properties::Properties ()
{
}

size_t Properties::create (BingoPtr<Properties> &ptr)
{
   ptr.allocate();
   new(ptr.ptr()) Properties();
   return (size_t)ptr;
}

void Properties::load (BingoPtr<Properties> &ptr, size_t offset)
{
   ptr = BingoPtr<Properties>(offset);
}

void Properties::add (const char *prop_name, const char *value)
{
   int prop_id;
   
   for (prop_id = 0; prop_id < _props.size(); prop_id++)
      if (strcmp(_props[prop_id].name.ptr(), prop_name) == 0)
         break;

   if (prop_id == _props.size())
   {
      _PropertyPair &new_pair = _props.push();
      new_pair.name.allocate(strlen(prop_name) + 1);
      strcpy(new_pair.name.ptr(), prop_name);

      new_pair.value.allocate(max_prop_len);
   }

   if (strlen(value) >= max_prop_len)
      throw Exception("BingoProperties: Too long property value");

   strcpy(_props[prop_id].value.ptr(), value);
}

void Properties::add (const char *prop_name, unsigned long value)
{
   std::ostringstream osstr;
   osstr << value;

   add(prop_name, osstr.str().c_str());
}

const char * Properties::getNoThrow (const char *prop_name)
{
   int prop_id;
   
   for (prop_id = 0; prop_id < _props.size(); prop_id++)
      if (strcmp(_props[prop_id].name.ptr(), prop_name) == 0)
         break;

   if (prop_id == _props.size())
      return 0;

   return _props[prop_id].value.ptr();
}

const char * Properties::get (const char *prop_name)
{
   const char *res = getNoThrow(prop_name);

   if (res == 0)
      throw Exception("Unknown property field");

   return res;
}

unsigned long Properties::getULongNoThrow (const char *prop_name)
{
   const char *value = getNoThrow(prop_name);

   if (value == 0)
      return ULONG_MAX;

   unsigned long u_dec;
   std::istringstream isstr(value);
   isstr >> u_dec;

   return u_dec;
}

unsigned long Properties::getULong (const char *prop_name)
{
   unsigned long res = getULongNoThrow(prop_name);

   if (res == ULONG_MAX)
      throw Exception("Unknown property field");

   return res;
}

void Properties::_parseProperty (const std::string &line, std::string &prop_out, std::string &value_out)
{
   int sep = (int)line.find_first_of('=');

   prop_out.assign(line.substr(0, sep));
   value_out.assign(line.substr(sep + 1, std::string::npos));
}
