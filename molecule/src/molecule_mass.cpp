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

#include "molecule/molecule_mass.h"

#include "molecule/molecule.h"
#include "molecule/elements.h"

using namespace indigo;

MoleculeMass::MoleculeMass()
{
   relative_atomic_mass_map = NULL;
}

float MoleculeMass::molecularWeight (Molecule &mol)
{
   double molmass = 0;
   int impl_h = 0;
   int elements_count[ELEM_MAX] = {0};

   for (int v = mol.vertexBegin(); 
            v != mol.vertexEnd(); 
            v = mol.vertexNext(v))
   {
      if (mol.isPseudoAtom(v) || mol.isRSite(v))
      {
         continue;
      }

      int number = mol.getAtomNumber(v);
      int isotope = mol.getAtomIsotope(v);
      

      if (isotope == 0)
      {
         float *value = 0;
         if (relative_atomic_mass_map != NULL)
         {
            value = relative_atomic_mass_map->at2(number);
         }

         if (value == 0)
         {
            elements_count[number]++;
         }
         else
         {
            molmass += *value;
         }
      }
      else
      {
         molmass += Element::getRelativeIsotopicMass(number, isotope);
      }

      // Add hydrogens
      impl_h += mol.getImplicitH(v);
      
   } 

   for (int i = ELEM_MIN; i < ELEM_MAX; i++) 
   {
      if (elements_count[i])
      {
         molmass += Element::getStandardAtomicWeight(i) * (double)elements_count[i];
      }
   }

   molmass += Element::getStandardAtomicWeight(ELEM_H) * impl_h;

   return (float)molmass;
}

float MoleculeMass::mostAbundantMass (Molecule &mol)
{
   double molmass = 0;

   // Count elements without explicit isotope marks
   int elements_counts[ELEM_MAX] = {0};

   for (int v = mol.vertexBegin(); 
            v != mol.vertexEnd(); 
            v = mol.vertexNext(v))
   {
      if (mol.isPseudoAtom(v))
         continue;

      int number = mol.getAtomNumber(v);
      int isotope = mol.getAtomIsotope(v);
      int impl_h = mol.getImplicitH(v);

      if (isotope == 0)
         elements_counts[number]++;
      else
         molmass += Element::getRelativeIsotopicMass(number, isotope);

      // Add hydrogens
      elements_counts[ELEM_H] += impl_h;
   } 

   // Compute mass of the most abunant composition
   for (int i = ELEM_MIN; i < ELEM_MAX; i++)
   {
      int count = elements_counts[i];
      if (count == 0)
         continue;

      int count_left = count;
      int min_iso, max_iso;
      Element::getMinMaxIsotopeIndex(i, min_iso, max_iso);
      for (int j = min_iso; j <= max_iso; j++)
      {
         float composition;
         if (!Element::getIsotopicComposition(i, j, composition))
            continue;

         int such_isotope_count = (int)(composition * count / 100 + 0.5f);

         molmass += Element::getRelativeIsotopicMass(i, j) * such_isotope_count;
         count_left -= such_isotope_count;
      }

      if (count_left != 0)
      {
         // Corrections in case of rounding errors
         int default_iso = Element::getDefaultIsotope(i);
         molmass += Element::getRelativeIsotopicMass(i, default_iso) * count_left;
      }
   }

   return (float)molmass;
}

float MoleculeMass::monoisotopicMass (Molecule &mol)
{
   double molmass = 0;

   for (int v = mol.vertexBegin(); 
            v != mol.vertexEnd(); 
            v = mol.vertexNext(v))
   {
      if (mol.isPseudoAtom(v))
         continue;

      int number = mol.getAtomNumber(v);
      int isotope = mol.getAtomIsotope(v);
      int impl_h = mol.getImplicitH(v);

      if (isotope == 0)
         isotope = Element::getDefaultIsotope(number);

      molmass += Element::getRelativeIsotopicMass(number, isotope);

      // Add hydrogens
      molmass += Element::getRelativeIsotopicMass(ELEM_H, 1) * impl_h;
   } 

   return (float)molmass;
}

int MoleculeMass::nominalMass (Molecule &mol)
{
   int molmass = 0;

   for (int v = mol.vertexBegin(); 
            v != mol.vertexEnd(); 
            v = mol.vertexNext(v))
   {
      if (mol.isPseudoAtom(v))
         continue;  

      int number = mol.getAtomNumber(v);
      int isotope = mol.getAtomIsotope(v);
      int impl_h = mol.getImplicitH(v);

      if (isotope == 0)
         molmass += Element::getDefaultIsotope(number);
      else
         molmass += isotope;

      // Add hydrogens
      molmass += impl_h;
   } 

   return molmass;
}
