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

#ifndef __molecule_mass_h__
#define __molecule_mass_h__

#include "base_cpp/red_black.h"

namespace indigo {

class Molecule;

// Molecular mass calculation
class MoleculeMass
{
public:
   MoleculeMass();

   const RedBlackMap<int, float> *relative_atomic_mass_map;

   /* Mass of a molecule calculated using the average mass of each 
    * element weighted for its natural isotopic abundance 
    */
   float molecularWeight (Molecule &mol);

   /* Mass of a molecule containing most likely 
    * isotopic composition for a single random molecule.
    * Notes: in PubChem search engine it is called Exact Mass
    */
   float mostAbundantMass (Molecule &mol);

   /* Mass of a molecule calculated using the mass of 
    * the most abundant isotope of each element.
    * Notes: in Marvin it is called Exact Mass
    */
   float monoisotopicMass (Molecule &mol);

   /* Sum of the mass numbers of all constituent atoms.
    */
   int nominalMass (Molecule &mol);
};

}

#endif // __molecule_mass_h__
