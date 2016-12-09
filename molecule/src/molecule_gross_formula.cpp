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

#include <ctype.h>

#include "base_cpp/scanner.h"
#include "base_cpp/output.h"

#include "molecule/elements.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

using namespace indigo;

int MoleculeGrossFormula::_cmp (_ElemCounter &ec1, _ElemCounter &ec2, void *context)
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
int MoleculeGrossFormula::_cmp_hill_no_carbon (_ElemCounter &ec1, _ElemCounter &ec2, void *context)
{
   if (ec1.counter == 0)
      return 1;
   if (ec2.counter == 0)
      return -1;
   // all elements are compared lexicographically
   return strncmp(Element::toString(ec1.elem), Element::toString(ec2.elem), 3);
}

// comparator implementing the Hill system with carbon:
// C H <other atoms in alphabetical order>
int MoleculeGrossFormula::_cmp_hill (_ElemCounter &ec1, _ElemCounter &ec2, void *context)
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

   // RSites have lowest priority
   if (ec2.elem == ELEM_RSITE)
      return -1;
   if (ec1.elem == ELEM_RSITE)
      return 1;

   return _cmp_hill_no_carbon(ec1, ec2, context);
}

void MoleculeGrossFormula::collect (BaseMolecule &mol, std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > &gross)
{
    if (!mol.isQueryMolecule())
    {
        mol.asMolecule().restoreAromaticHydrogens();
    }
    
    // basic structure and all polymers
    int grossFormulaSize = mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) + 1;
    QS_DEF(ObjArray<Array<int > >, filters);
    filters.clear();
    filters.resize(grossFormulaSize);
    QS_DEF(ObjArray<Array<char> >, indices);
    indices.clear();
    indices.resize(grossFormulaSize);
    
    // first element is for old-style gross formula
    indices[0].appendString(" ", true);
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        filters[0].push(i);
    }
    
    // then polymer sgroups
    for (int i = 1; i < grossFormulaSize; i++)
    {
        RepeatingUnit *ru = (RepeatingUnit *)&mol.sgroups.getSGroup(i - 1, SGroup::SG_TYPE_SRU);
        filters[i].copy(ru->atoms);
        indices[i].copy(ru->subscript.ptr(), ru->subscript.size() - 1); // Remove '0' symbol at the end
        // Filter polymer atoms
        for (int j = 0; j < filters[i].size(); j++)
        {
            int found_idx = filters[0].find(filters[i][j]);
            if (found_idx > -1)
            {
                filters[0].remove(found_idx);
            }
        }
    }
    
    // init ObjArray
    while (gross.first.size() < grossFormulaSize) {
        gross.first.push();
        gross.second.push();
    }
    
    for (int i = 0; i < grossFormulaSize; i++)
    {
        gross.first[i].copy(indices[i]);
        gross.second[i].clear_resize(ELEM_RSITE + 1);
        gross.second[i].zerofill();

        for (int j = 0; j < filters[i].size(); j++)
        {
            if (mol.isPseudoAtom(filters[i][j]) || mol.isTemplateAtom(filters[i][j]))
            {
                continue;
            }
            int number = mol.getAtomNumber(filters[i][j]);

            if (number > 0)
                gross.second[i][number]++;

            if (!mol.isQueryMolecule() && !mol.isRSite(filters[i][j]))
            {
                int implicit_h = mol.asMolecule().getImplicitH(filters[i][j]);

                if (implicit_h >= 0)
                    gross.second[i][ELEM_H] += implicit_h;
            }
        }
    }
}

void MoleculeGrossFormula::toString (const std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > &gross, Array<char> &str, bool add_rsites)
{
    ArrayOutput output(str);

    for (int i = 0; i < gross.second.size(); i++)
    {
        _toString(gross.second[i], output, _cmp, add_rsites);
    }
    output.writeChar(0);
}

void MoleculeGrossFormula::toString_Hill (const std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > &gross, Array<char> &str, bool add_rsites)
{
    ArrayOutput output(str);
    
    // First base molecule
    if (gross.second[0][ELEM_C] == 0)
        _toString(gross.second[0], output, _cmp_hill_no_carbon, add_rsites);
    else
        _toString(gross.second[0], output, _cmp_hill, add_rsites);
    
    // Then polymers repeating units
    for (int i = 1; i < gross.second.size(); i++)
    {
        output.writeChar('(');
        if (gross.second[i][ELEM_C] == 0)
            _toString(gross.second[i], output, _cmp_hill_no_carbon, add_rsites);
        else
            _toString(gross.second[i], output, _cmp_hill, add_rsites);
        output.writeChar(')');
        output.writeArray(gross.first[i]);
    }
    output.writeChar(0);
}


void MoleculeGrossFormula::_toString (const Array<int> &gross, ArrayOutput & output, int (*cmp)(_ElemCounter &, _ElemCounter &, void *), bool add_rsites)
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
   if (add_rsites && gross[ELEM_RSITE] > 0)
    {
        output.writeString(" R#");
        if (gross[ELEM_RSITE] > 1)
        {
            output.printf("%d", gross[ELEM_RSITE]);
        }
    }
}

void MoleculeGrossFormula::fromString (Scanner &scanner, Array<int> &gross)
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

void MoleculeGrossFormula::fromString (const char *str, Array<int> &gross)
{
   BufferScanner scanner(str);

   fromString(scanner, gross);
}

bool MoleculeGrossFormula::leq (const Array<int> &gross1, const Array<int> &gross2)
{
   int i;

   for (i = 1; i < ELEM_MAX; i++)
      if (gross1[i] > gross2[i])
         return false;

   return true;
}

bool MoleculeGrossFormula::geq (const Array<int> &gross1, const Array<int> &gross2)
{
   int i;

   for (i = 1; i < ELEM_MAX; i++)
      if (gross2[i] > 0 && gross1[i] < gross2[i])
         return false;

   return true;
}

bool MoleculeGrossFormula::equal (const Array<int> &gross1, const Array<int> &gross2)
{
   int i;

   for (i = 1; i < ELEM_MAX; i++)
      if (gross2[i] != gross1[i])
         return false;

   return true;
}
