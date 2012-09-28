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

#include <ctype.h>

#include "base_cpp/scanner.h"
#include "base_cpp/output.h"

#include "molecule/elements.h"
#include "molecule/base_molecule.h"
#include "molecule/gross_formula.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

using namespace indigo;

int GrossFormula::_cmp (_ElemCounter &ec1, _ElemCounter &ec2, void *context)
{
   if (ec1.counter == 0)
      return 1;
   if (ec2.counter == 0)
      return -1;

   if (ec2.elem == ELEM_H) // move hydrogen to the end
      return -1;
   if (ec1.elem == ELEM_H)
      return 1;

   return ec1.elem - ec2.elem;
}

// comparator implementing the Hill system without carbon:
// <all atoms in alphabetical order>
int GrossFormula::_cmp_hill_no_carbon (_ElemCounter &ec1, _ElemCounter &ec2, void *context)
{
   if (ec1.counter == 0)
      return 1;
   if (ec2.counter == 0)
      return -1;
   // all elements are compared lexicographically
   return strcmp(Element::toString(ec1.elem), Element::toString(ec2.elem));
}

// comparator implementing the Hill system with carbon:
// C H <other atoms in alphabetical order>
int GrossFormula::_cmp_hill (_ElemCounter &ec1, _ElemCounter &ec2, void *context)
{
   if (ec1.counter == 0)
      return 1;
   if (ec2.counter == 0)
      return -1;

   // carbon has the highest priority
   if (ec2.elem == ELEM_C)
      return 1;
   if (ec1.elem == ELEM_C)
      return -1;

   // hydrogen has the highest priority after carbon
   if (ec2.elem == ELEM_H)
      return 1;
   if (ec1.elem == ELEM_H)
      return -1;
   return _cmp_hill_no_carbon(ec1, ec2, context);
}

void GrossFormula::collect (BaseMolecule &mol, Array<int> &gross)
{
   int i;

   gross.clear_resize(ELEM_MAX);
   gross.zerofill();

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
   {
      if (mol.isPseudoAtom(i) || mol.isRSite(i))
         continue;
      int number = mol.getAtomNumber(i);

      if (number > 0)
         gross[number]++;

      if (!mol.isQueryMolecule())
      {
         int implicit_h = mol.asMolecule().getImplicitH(i);

         if (implicit_h >= 0)
            gross[ELEM_H] += implicit_h;
      }
   }
}

void GrossFormula::toString (const Array<int> &gross, Array<char> &str)
{
   _toString(gross, str, _cmp);
}

void GrossFormula::toString_Hill (const Array<int> &gross, Array<char> &str)
{
   if (gross[ELEM_C] == 0)
      _toString(gross, str, _cmp_hill_no_carbon);
   else
      _toString(gross, str, _cmp_hill);
}


void GrossFormula::_toString (const Array<int> &gross, Array<char> &str,
                              int (*cmp)(_ElemCounter &, _ElemCounter &, void *))
{
   QS_DEF(Array<_ElemCounter>, counters);
   int i;

   counters.clear();

   for (i = 1; i < ELEM_MAX; i++)
   {
      _ElemCounter &ec = counters.push();
 
      ec.elem = i;
      ec.counter = gross[i];
   }

   counters.qsort(cmp, 0);

   ArrayOutput output(str);

   bool first_written = false;

   for (i = 0; i < counters.size(); i++)
   {
      if (counters[i].counter < 1)
         break; 

      if (first_written)
         output.printf(" ");

      output.printf(Element::toString(counters[i].elem));
      if (counters[i].counter > 1)
         output.printf("%d", counters[i].counter);
      first_written = true;
   }
   output.writeChar(0);
}

void GrossFormula::fromString (Scanner &scanner, Array<int> &gross)
{
   gross.clear_resize(ELEM_MAX);
   gross.zerofill();

   scanner.skipSpace();
   while (!scanner.isEOF())
   {
      int elem = Element::read(scanner);
      scanner.skipSpace();
      int counter = 1;
      
      if (isdigit(scanner.lookNext()))
      {
         counter = scanner.readUnsigned();
         scanner.skipSpace();
      }

      gross[elem] += counter;
   }
}

void GrossFormula::fromString (const char *str, Array<int> &gross)
{
   BufferScanner scanner(str);

   fromString(scanner, gross);
}

bool GrossFormula::leq (const Array<int> &gross1, const Array<int> &gross2)
{
   int i;

   for (i = 1; i < ELEM_MAX; i++)
      if (gross1[i] > gross2[i])
         return false;

   return true;
}

bool GrossFormula::geq (const Array<int> &gross1, const Array<int> &gross2)
{
   int i;

   for (i = 1; i < ELEM_MAX; i++)
      if (gross2[i] > 0 && gross1[i] < gross2[i])
         return false;

   return true;
}

bool GrossFormula::equal (const Array<int> &gross1, const Array<int> &gross2)
{
   int i;

   for (i = 1; i < ELEM_MAX; i++)
      if (gross2[i] != gross1[i])
         return false;

   return true;
}
