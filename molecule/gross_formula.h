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

#ifndef __gross_formula__
#define __gross_formula__

namespace indigo {

#include "base_cpp/array.h"

class BaseMolecule;

class GrossFormula
{
public:
   static void collect (BaseMolecule &molecule, Array<int> &gross);
   static void toString (const Array<int> &gross, Array<char> &str);
   static void toString_Hill (const Array<int> &gross, Array<char> &str);
   static void fromString (const char *str, Array<int> &gross);
   static void fromString (Scanner &scanner, Array<int> &gross);

   static bool leq  (const Array<int> &gross1, const Array<int> &gross2);
   static bool geq  (const Array<int> &gross1, const Array<int> &gross2);
   static bool equal (const Array<int> &gross1, const Array<int> &gross2);

protected:
   struct _ElemCounter
   {
      int elem;
      int counter;
   };

   static void _toString (const Array<int> &gross, Array<char> &str,
                          int (*cmp)(_ElemCounter &, _ElemCounter &, void *));
   static int _cmp      (_ElemCounter &ec1, _ElemCounter &ec2, void *context);
   static int _cmp_hill (_ElemCounter &ec1, _ElemCounter &ec2, void *context);
   static int _cmp_hill_no_carbon (_ElemCounter &ec1, _ElemCounter &ec2, void *context);
};

}

#endif

