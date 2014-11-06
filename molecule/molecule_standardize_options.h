/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __molecule_standardize_options__
#define __molecule_standardize_options__


#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class StandardizeOptions
{
public:
   StandardizeOptions ();

   void reset ();

   // Sets or repairs the stereo on a molecule to a standard form using the coordinates
   // as the guide. Default is false.
   bool standardize_stereo;

   // Sets the charges on a molecule to a standard form. Default is false.
   bool standardize_charges;

   // Translates a molecule so its geometric center lies at the origin. Default is false.
   bool center_molecule;

   // Removes fragments that consist of only a single heavy atom. Default is false.
   bool remove_single_atom_fragments;

};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_standardize_options__
