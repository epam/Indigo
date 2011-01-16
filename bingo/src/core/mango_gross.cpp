/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "core/bingo_context.h"
#include "core/mango_index.h"
#include "core/mango_matchers.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/gross_formula.h"
#include "molecule/elements.h"

MangoGross::MangoGross (BingoContext &context) :
_context(context)
{
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
}

void MangoGross::parseQuery (const Array<char> &query)
{
   BufferScanner scanner(query);

   parseQuery(scanner);
}

void MangoGross::parseQuery (const char *query)
{
   BufferScanner scanner(query);

   parseQuery(scanner);
}

void MangoGross::parseQuery (Scanner &scanner)
{
   scanner.skipSpace();

   char c = scanner.readChar();

   if (c == '>')
   {
      if (scanner.readChar() != '=')
         throw Error("expecting '=' after '>'");
      _sign = 1;
   }
   else if (c == '<')
   {
      if (scanner.readChar() != '=')
         throw Error("expecting '=' after '<'");
      _sign = -1;
   }
   else if (c == '=')
      _sign = 0;
   else
      throw Error("expected one of '<= >= =', got %c", c);

   GrossFormula::fromString(scanner, _query_gross);

   ArrayOutput out(_conditions);

   bool first = true;

   if (_sign == 0)
   {
      QS_DEF(Array<char>, query_gross_str);

      GrossFormula::toString(_query_gross, query_gross_str);

      out.printf("gross = '%s'", query_gross_str.ptr());
   }
   else for (int i = 0; i < NELEM(MangoIndex::counted_elements); i++)
   {
      int elem = MangoIndex::counted_elements[i];

      if (_query_gross[elem] <= 0 && _sign == 1)
         continue;

      if (!first)
         out.printf(" AND ");

      first = false;

      if (_sign == 1)
         out.printf("cnt_%s >= %d", Element::toString(elem), _query_gross[elem]);
      else // _sign == -1
         out.printf("cnt_%s <= %d", Element::toString(elem), _query_gross[elem]);
   }
   out.writeChar(0);
}

const char * MangoGross::getConditions ()
{
   return _conditions.ptr();
}

bool MangoGross::checkGross (const Array<int> &target_gross)
{
   if (_sign == 1)
      return GrossFormula::geq(target_gross, _query_gross);
   else if (_sign == -1)
      return GrossFormula::leq(target_gross, _query_gross);
   // _sign == 0
   return GrossFormula::equal(target_gross, _query_gross);
}

bool MangoGross::checkGross (const char *target_gross_str)
{
   QS_DEF(Array<int>, target_gross);

   GrossFormula::fromString(target_gross_str, target_gross);

   return checkGross(target_gross);
}

bool MangoGross::checkMolecule (const Array<char> &target_buf)
{
   BufferScanner scanner(target_buf);

   return checkMolecule(scanner);
}

bool MangoGross::checkMolecule (Scanner &scanner)
{
   QS_DEF(Molecule, target);
   QS_DEF(Array<int>, target_gross);

   MoleculeAutoLoader loader(scanner);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
   loader.skip_3d_chirality = true;
   loader.loadMolecule(target);

   GrossFormula::collect(target, target_gross);

   return checkGross(target_gross);
}
