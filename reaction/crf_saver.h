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

#ifndef __crf_saver__
#define __crf_saver__

#include "lzw/lzw_encoder.h"
#include "base_cpp/obj.h"

namespace indigo {

class Molecule;
class Reaction;
class LzwDict;

class CrfSaver
{
public:
   // external dictionary, internal encoder
   explicit CrfSaver (LzwDict &dict, Output &output);

   // no dictionary, no encoder
   explicit CrfSaver (Output &output);

   void saveReaction (Reaction &reaction);

   Output *xyz_output;

   bool save_bond_dirs;
   bool save_highlighting;
   bool save_mapping;

   DECL_ERROR;

protected:

   void _init ();

   void _writeReactionInfo (Reaction &reaction);
   void _writeAam (const int *aam, const Array<int> &sequence);
   void _writeMolecule (Molecule &molecule);
   void _writeReactionMolecule (Reaction &reaction, int idx);
   
   Output &_output;
   Obj<LzwEncoder> _encoder;

   const int *_atom_stereo_flags;
   const int *_bond_rc_flags;
   const int *_aam;

private:
   CrfSaver (const CrfSaver &); // no implicit copy
};

}

#endif
