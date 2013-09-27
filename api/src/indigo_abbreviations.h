/****************************************************************************
 * Copyright (C) 2010-2013 GGA Software Services LLC
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

#ifndef __indigo_arreviations__
#define __indigo_arreviations__

#include <string>
#include <vector>

#include "base_cpp/obj_array.h"

namespace indigo 
{

namespace abbreviations 
{

struct Abbreviation
{
   std::string name, expansion;
   std::vector<std::string> left_aliases, right_aliases, left_aliases2, right_aliases2;

   int connections;
};

class IndigoAbbreviations
{
public:
   IndigoAbbreviations ();

   void clear();
	
   ObjArray<Abbreviation> abbreviations;

private:
   void loadDefault ();
};

IndigoAbbreviations& indigoGetAbbreviationsInstance ();

}}

#endif // __indigo_arreviations__
