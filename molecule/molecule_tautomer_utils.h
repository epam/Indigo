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

#ifndef __molecule_tautomer_utils__
#define __molecule_tautomer_utils__

namespace indigo {

class Graph;
class Molecule;

class MoleculeTautomerUtils
{
public:

   static void countHReplacements (BaseMolecule &g, Array<int> &h_rep_count);

   static void highlightChains (BaseMolecule &g1, BaseMolecule &g2,
      const Array<int> &chains_2, const int *core_2);

private:

   static bool _isRepMetal (int elem);
};

}

#endif
