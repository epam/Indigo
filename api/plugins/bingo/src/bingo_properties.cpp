#include "bingo_properties.h"

#include "base_cpp\exception.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace bingo;
using namespace indigo;

Properties::Properties()
{
}

void Properties::create( const char *filename )
{
   _filename = filename;
   std::ofstream property_file(filename);
   _props.clear();
}

void Properties::load( const char *filename )
{
   _filename = filename;
   
   std::string line;
   std::ifstream property_file(filename);
   
   if (property_file.is_open())
   {
      while (property_file.good())
      {
         std::getline(property_file, line);
         _parseProperty(line);
      }

      property_file.close();
   }
}

void Properties::add( const char *prop_name, const char *value )
{
   if (_filename.empty())
      throw Exception("Property file's name wasn't initialized");

   _props.insert(_PropertyPair(prop_name, value));

   std::ofstream property_file;
   property_file.open(_filename.c_str(), std::ios::out | std::ios::app);
   property_file << prop_name << '=' << value << std::endl;
}

const char * Properties::get( const char *prop_name )
{
   return _props[prop_name].c_str();
}

void Properties::_parseProperty( const std::string &line )
{
   int sep = (int)line.find_first_of('=');
   
   _props.insert(_PropertyPair(line.substr(0, sep), line.substr(sep + 1, std::string::npos)));
}
