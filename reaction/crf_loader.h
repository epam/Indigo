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

#ifndef __crf_loader__
#define __crf_loader__

#include "lzw/lzw_decoder.h"
#include "base_cpp/obj.h"
#include "crf_saver.h"

namespace indigo {

class Reaction;

class CrfLoader
{
public:
   // external dictionary, internal encoder
   explicit CrfLoader (LzwDict &dict, Scanner &scanner);

   // no dictionary, no encoder
   explicit CrfLoader (Scanner &scanner);

   void loadReaction (Reaction &reaction);

   Scanner *xyz_scanner;
   int version; // By default the latest version 2 is used

   DECL_ERROR;
protected:

   void _init ();

   void _loadMolecule (Molecule &molecule);
   void _loadReactionMolecule (Reaction &reaction, int index, bool have_aam);

   Scanner &_scanner;

   Obj<LzwDecoder> _decoder;
   LzwDict        *_dict;

   Array<int> *_bond_rc_flags;
   Array<int> *_atom_stereo_flags;
   Array<int> *_aam;

private:
   CrfLoader (const CrfLoader &); // no implicit copy
};

}

#endif
