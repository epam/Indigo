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

#ifndef __molecule_mass_h__
#define __molecule_mass_h__

#include "base_cpp/red_black.h"

namespace indigo {

class Molecule;

// Molecular mass calculation
class MoleculeMass
{
    DECL_ERROR;

protected:
    struct _ElemCounter
    {
        int elem;
        double weight;
    };
    
    static int _cmp (_ElemCounter &ec1, _ElemCounter &ec2, void *context);
public:

    MoleculeMass();

   const RedBlackMap<int, double> *relative_atomic_mass_map;

   /* Mass of a molecule calculated using the average mass of each 
    * element weighted for its natural isotopic abundance 
    */
   double molecularWeight (Molecule &mol);

   /* Mass of a molecule containing most likely 
    * isotopic composition for a single random molecule.
    * Notes: in PubChem search engine it is called Exact Mass
    */
   double mostAbundantMass (Molecule &mol);

   /* Mass of a molecule calculated using the mass of 
    * the most abundant isotope of each element.
    * Notes: in Marvin it is called Exact Mass
    */
   double monoisotopicMass (Molecule &mol);

   /* Sum of the mass numbers of all constituent atoms.
    */
   int nominalMass (Molecule &mol);
    
    
    /* Atom weight percentage like "C 77% H 13%"
     */
   void massComposition (Molecule &molecule, Array<char> &str);
};

}

#endif // __molecule_mass_h__
