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

#ifndef __molfile_saver__
#define __molfile_saver__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "molecule/base_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Molecule;
class QueryMolecule;
class Output;

class DLLEXPORT MolfileSaver
{
public:
   enum
   {
      MODE_AUTO = 0, // save to v3000 only if the given molecule has any
                     // v3000-specific features
      MODE_2000,     // force saving to v2000 format
      MODE_3000      // force saving to v3000 format
   };

   MolfileSaver (Output &output);

   void saveBaseMolecule   (BaseMolecule &mol);
   void saveMolecule       (Molecule &mol);
   void saveQueryMolecule  (QueryMolecule &mol);

   void saveCtab3000       (Molecule &mol);
   void saveQueryCtab3000  (QueryMolecule &mol);

   int mode; // one of MODE_***, MODE_AUTO by default
   bool no_chiral; // skip the 'chiral' flag, not regarding of the actual stereochemistry
   bool skip_date; // If true then zero date is written

   // optional parameters for reaction
   const Array<int>* reactionAtomMapping;
   const Array<int>* reactionAtomInversion;
   const Array<int>* reactionAtomExactChange;
   const Array<int>* reactionBondReactingCenter;

   DECL_ERROR;
   
protected:
   void _saveMolecule (BaseMolecule &mol, bool query);

   void _writeHeader (BaseMolecule &mol, Output &output, bool zcoord);
   void _writeCtabHeader (Output &output);
   void _writeAtomLabel (Output &output, int label);
   void _writeMultiString (Output &output, const char *string, int len);
   void _writeCtab (Output &output, BaseMolecule &mol, bool query);
   void _writeOccurrenceRanges (Output &out, const Array<int> &occurrences);
   void _writeRGroup (Output &output, BaseMolecule &mol, int rg_idx);
   void _writeCtabHeader2000 (Output &output, BaseMolecule &mol);
   void _writeCtab2000 (Output &output, BaseMolecule &mol, bool query);
   void _writeRGroupIndices2000 (Output &output, BaseMolecule &mol);
   void _writeAttachmentValues2000 (Output &output, BaseMolecule &fragment);
   void _writeGenericSGroup3000 (BaseMolecule::SGroup &sgroup, int idx, const char *type, Output &output);
   void _writeDataSGroupDisplay (BaseMolecule::DataSGroup &datasgroup, Output &out);
   static bool _checkAttPointOrder (BaseMolecule &mol, int rsite);
   static bool _hasNeighborEitherBond (BaseMolecule &mol, int edge_idx);

   static int _getStereocenterParity (BaseMolecule &mol, int idx);

   bool _getRingBondCountFlagValue (QueryMolecule &qmol, int idx, int &value);
   bool _getSubstitutionCountFlagValue (QueryMolecule &qmol, int idx, int &value);

   Output &_output;
   bool    _v2000;

   TL_CP_DECL(Array<int>, _atom_mapping);
   TL_CP_DECL(Array<int>, _bond_mapping);

   enum
   {
      _SGROUP_TYPE_SUP = 1,
      _SGROUP_TYPE_DAT,
      _SGROUP_TYPE_SRU,
      _SGROUP_TYPE_MUL,
      _SGROUP_TYPE_GEN
   };

private:
   MolfileSaver (const MolfileSaver &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
