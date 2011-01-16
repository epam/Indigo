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

#ifndef __cmf_loader_h__
#define __cmf_loader_h__

#include "base_cpp/bitinworker.h"
#include "lzw/lzw_dictionary.h"
#include "lzw/lzw_decoder.h"
#include "base_cpp/obj.h"

namespace indigo {

class Molecule;
class Scanner;
struct Vec3f;

class CmfLoader
{
public:

   // external dictionary, internal decoder
   explicit CmfLoader (LzwDict &dict, Scanner &scanner);

   // external dictionary, external decoder
   explicit CmfLoader (LzwDecoder &decoder);

   // no dictionary, no decoder
   explicit CmfLoader (Scanner &scanner);
      
   void loadMolecule (Molecule &mol);
   void loadXyz (Scanner &scanner);

   bool skip_cistrans;
   bool skip_stereocenters;
   bool skip_valence;

   Array<int> *atom_flags;
   Array<int> *bond_flags;

   DEF_ERROR("CMF loader");
protected:

   struct _AtomDesc
   {
      int label;
      int pseudo_atom_idx; // refers to _pseudo_labels
      int isotope;
      int charge;
      int hydrogens;
      int valence;
      int radical;
      
      int  stereo_type;
      int  stereo_group;
      bool stereo_invert_pyramid;

      int  flags;
      
      bool rsite;
      int  rsite_bits;
   };

   struct _BondDesc
   {
      int  beg;
      int  end;
      int  type;
      int  cis_trans;
      bool in_ring;

      int  flags;
   };

   void _init ();
   
   bool _getNextCode (int &code);

   void _readBond (int &code, _BondDesc &bond);
   bool _readAtom (int &code, _AtomDesc &atom);
   bool _readCycleNumber (int &code, int &n);

   Scanner *_scanner;
   
   Obj<LzwDecoder> _decoder_obj;
   LzwDecoder     *_decoder;

   TL_CP_DECL(Array<_AtomDesc>, _atoms);
   TL_CP_DECL(Array<_BondDesc>, _bonds);
   TL_CP_DECL(StringPool,       _pseudo_labels);
   Molecule *_mol;

private:
   CmfLoader (const CmfLoader &); // no implicit copy
};

}

#endif /* __cmf_loader_h__ */
