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

#ifndef __cmf_saver_h__
#define __cmf_saver_h__

#include "base_cpp/obj.h"
#include "lzw/lzw_encoder.h"
#include "math/algebra.h"
#include "molecule/base_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Output;

class DLLEXPORT CmfSaver
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

   bool save_bond_dirs;
   bool save_highlighting;
   bool save_mapping;

   DECL_ERROR;

   struct VecRange
   {
      Vec3f xyz_min, xyz_range;
      bool have_z;
   };
protected:

   void _init ();
   void _encode (byte symbol);

   void _encodeAtom (Molecule &mol, int idx, const int *mapping);
   void _encodeBond (Molecule &mol, int idx, const int *mapping);
   void _encodeCycleNumer (int n);

   void _writeFloatInRange (Output &output, float v, float min, float range);

   struct Mapping
   {
      Array<int> *atom_mapping, *bond_mapping;
   };

   void _encodeString (const Array<char> &str);
   void _encodeUIntArray (const Array<int> &data, const Array<int> &mapping);
   void _encodeUIntArray (const Array<int> &data);
   void _encodeUIntArraySkipNegative (const Array<int> &data);

   void _encodeExtSection (Molecule &mol, const Mapping &mapping);
   void _encodeBaseSGroup (Molecule &mol, BaseMolecule::SGroup &sgroup, const Mapping &mapping);

   //void _encodeSGroups (Molecule &mol, const Mapping &mapping);
   void _writeSGroupsXyz (Molecule &mol, Output &output, const VecRange &range);
   void _writeBaseSGroupXyz (Output &output, BaseMolecule::SGroup &sgroup, const VecRange &range);

   void _writeVec3f (Output &output, const Vec3f &pos, const VecRange &range);
   void _writeVec2f (Output &output, const Vec2f &pos, const VecRange &range);
   void _writeDir2f (Output &output, const Vec2f &dir, const VecRange &range);

   void _updateSGroupsXyzMinMax (Molecule &mol, Vec3f &min, Vec3f &max);
   void _updateBaseSGroupXyzMinMax (BaseMolecule::SGroup &sgroup, Vec3f &min, Vec3f &max);

   TL_CP_DECL(Array<int>, _atom_sequence);

   Output     *_output;
   Obj<LzwEncoder> _encoder_obj;
   LzwEncoder *_ext_encoder;
   Obj<LzwOutput> _encoder_output_obj;

   Molecule *_mol;

private:
   CmfSaver (const CmfSaver &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif /* __cmf_saver_h__ */
