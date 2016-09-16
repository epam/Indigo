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


   enum
   {
      CIP_DESC_NONE = 0,
      CIP_DESC_UNKNOWN,
      CIP_DESC_s,
      CIP_DESC_r,
      CIP_DESC_S,
      CIP_DESC_R,
      CIP_DESC_E,
      CIP_DESC_Z
   };

   struct CIPContext
   {
      BaseMolecule *mol;
      Array<int>   *cip_desc;
      Array<int>   *used1;
      Array<int>   *used2;
      bool         next_level;
      bool         isotope_check;
      bool         use_stereo;
      bool         use_rule_4;
      int          ref_cip1;
      int          ref_cip2;
      bool         use_rule_5;
      
      inline void clear() {
         mol = 0;
         cip_desc = 0;
         used1 = 0;
         used2 = 0;
         ref_cip1=0;
         ref_cip2=0;
      }
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
   bool add_stereo_desc; // If true then stereo descriptors will be added as DAT S-groups
   bool add_implicit_h;  // If true then MRV_IMPLICIT_H Data S-groups will be added for saving
                         // the number of implicit H for aromatic atoms
                         // (if it is required for correct de-aromatization) (default value is true)

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
   void _writeTGroup (Output &output, BaseMolecule &mol, int tg_idx);
   void _writeCtabHeader2000 (Output &output, BaseMolecule &mol);
   void _writeCtab2000 (Output &output, BaseMolecule &mol, bool query);
   void _checkSGroupIndices (BaseMolecule &mol);
   void _writeRGroupIndices2000 (Output &output, BaseMolecule &mol);
   void _writeAttachmentValues2000 (Output &output, BaseMolecule &fragment);
   void _writeGenericSGroup3000 (SGroup &sgroup, int idx, Output &output);
   void _writeDataSGroupDisplay (DataSGroup &datasgroup, Output &out);
   void _writeFormattedString(Output &output, Array<char> &str, int length);
   static bool _checkAttPointOrder (BaseMolecule &mol, int rsite);
   static bool _hasNeighborEitherBond (BaseMolecule &mol, int edge_idx);

   static int _getStereocenterParity (BaseMolecule &mol, int idx);

   bool _getRingBondCountFlagValue (QueryMolecule &qmol, int idx, int &value);
   bool _getSubstitutionCountFlagValue (QueryMolecule &qmol, int idx, int &value);

   void _updateCIPStereoDescriptors(BaseMolecule &mol);
   void _addCIPStereoDescriptors(BaseMolecule &mol);
   void _addCIPSgroups(BaseMolecule &mol, Array<int> &attom_cip_desc, Array<int> &bond_cip_desc);
   void _calcRSStereoDescriptor (BaseMolecule &mol, BaseMolecule &unfolded_h_mol, int idx,
           Array<int> &atom_cip_desc, Array<int> &stereo_passed, bool use_stereo, Array<int[2]> &equiv_ligands,
           bool &digrap_cip_used);
   void _calcEZStereoDescriptor (BaseMolecule &mol, BaseMolecule &unfolded_h_mol, int idx, Array<int> &bond_cip_desc);
   bool _checkLigandsEquivalence (Array<int> &ligands, Array<int[2]> &equiv_ligands, CIPContext &context);
   static int  _getNumberOfStereoDescritors (Array<int> &atom_cip_desc);
   bool _isPseudoAssymCenter (BaseMolecule &mol, int idx, Array<int> &atom_cip_desc, Array<int> &ligands,
        Array<int[2]> &equiv_ligands);

   int _calcCIPDigraphDescriptor (BaseMolecule &mol, int atom_idx, Array<int> &ligands, Array<int[2]> &equiv_ligands);
   void _addNextLevel (Molecule &source, Molecule &target, int s_idx, int t_idx, Array<int> &used, Array<int> &mapping);
   void _calcStereocenters (Molecule &source, Molecule &mol, Array<int> &mapping);

   static int _cip_rules_cmp (int &i1, int &i2, void *context);

   Output &_output;
   bool    _v2000;

   CP_DECL;
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
