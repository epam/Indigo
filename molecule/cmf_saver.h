/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __cmf_saver_h__
#define __cmf_saver_h__

class Molecule;
class Output;

#include "base_cpp/obj.h"
#include "lzw/lzw_encoder.h"

class CmfSaver
{
public:

   // external dictionary, internal encoder
   explicit CmfSaver (LzwDict &dict, Output &output);

   // external dictionary, external encoder
   explicit CmfSaver (LzwEncoder &encoder);

   // no dictionary, no encoder
   explicit CmfSaver (Output &output);

   void saveMolecule (Molecule &mol);
   void saveXyz (Output &output);

   const Array<int> & getAtomSequence ();

   int *atom_flags;
   int *bond_flags;

   bool skip_implicit_h;

   DEF_ERROR("CMF saver");

protected:

   void _init ();
   void _encode (int symbol);

   void _encodeAtom (Molecule &mol, int idx, const int *mapping);
   void _encodeBond (Molecule &mol, int idx, const int *mapping);
   void _encodeCycleNumer (int n);

   void _writeXyz (Molecule &mol);

   TL_CP_DECL(Array<int>, _atom_sequence);

   Output     *_output;
   LzwEncoder *_encoder;
   Obj<LzwEncoder> _encoder_obj;

   Molecule *_mol;

private:
   CmfSaver (const CmfSaver &); // no implicit copy
};

#endif /* __cmf_saver_h__ */

