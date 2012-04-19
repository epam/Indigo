/****************************************************************************
 * Copyright (C) 2010-2012 GGA Software Services LLC
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

#include "indigo-inchi.h"
                      
#include "molecule/elements.h"
#include "molecule/molecule.h"

#include "indigo_internal.h"
#include "indigo_molecule.h"
#include "option_manager.h"

#include "inchi_api.h"
#include "mode.h"

#include <algorithm>

using namespace indigo;

CEXPORT const char* indigoInchiVersion ()
{
   return INCHI_NAME " version " INCHI_VERSION TARGET_ID_STRING;
}

//
// IndigoInchi
//

class IndigoInchi
{
public:
   IndigoInchi ();

   void clear();

   // Input parameters
   Array<char> options;

   // Output additional results
   Array<char> warning, log, auxInfo;

   void loadMoleculeFromInchi (const char *inchi, Molecule &mol);
   void parseInchiOutput (const inchi_OutputStruct &inchi_output, Molecule &mol);

   void generateInchiInput (Molecule &mol, inchi_Input &input, 
      Array<inchi_Atom> &atoms, Array<inchi_Stereo0D> &stereo);
   void saveMoleculeIntoInchi (Molecule &mol, Array<char> &inchi);

   void neutralizeV5Nitrogen (Molecule &mol);

   static inchi_BondType getInchiBondType (int bond_order);
};

IndigoInchi::IndigoInchi ()
{
   clear();
}

void IndigoInchi::clear()
{
   options.clear();
   options.push(0);

   warning.clear();
   log.clear();
   auxInfo.clear();
}

void IndigoInchi::loadMoleculeFromInchi (const char *inchi_string, Molecule &mol)
{
   inchi_InputINCHI inchi_input;
   inchi_input.szInChI = (char *)inchi_string;
   inchi_input.szOptions = (char *)options.ptr();

   inchi_OutputStruct inchi_output;

   try 
   {
      int retcode = GetStructFromINCHI(&inchi_input, &inchi_output);

      if (inchi_output.szMessage)
         warning.readString(inchi_output.szMessage, true);
      if (inchi_output.szLog)
         log.readString(inchi_output.szLog, true);

      if (retcode != inchi_Ret_OKAY && retcode != inchi_Ret_WARNING)
         throw IndigoError("Indigo-InChI: InChI loading failed: %s. Code: %d.", 
            inchi_output.szMessage, retcode);

      parseInchiOutput(inchi_output, mol);
   }
   catch (...)
   {
      FreeStructFromINCHI(&inchi_output);
      throw;
   }
}

void IndigoInchi::neutralizeV5Nitrogen (Molecule &mol)
{
   // Initial structure C[C@H](O)[C@H](COC)/N=[N+](\[O-])/C=CCCCCCC
   // is loaded via InChI as CCCCCCC=CN(=O)=N[C@@H](COC)[C@H](C)O
   // and we cannot restore cis-trans configuration for O=N=N-C bond
   for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
      if (mol.isNitrogenV5(v))
      {
         const Vertex &vertex = mol.getVertex(v);
         for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
         {
            int nei_edge = vertex.neiEdge(nei);
            if (mol.getBondOrder(nei_edge) != BOND_DOUBLE)
               continue;

            int nei_idx = vertex.neiVertex(nei);
            int number = mol.getAtomNumber(nei_idx);
            int charge = mol.getAtomCharge(nei_idx);
            int radical = mol.getAtomRadical(nei_idx);
            if ((number == ELEM_O || number == ELEM_S) && charge == 0 && radical == 0)
            {
               mol.setAtomCharge(v, 1);
               mol.setAtomCharge(nei_idx, -1);
               mol.setBondOrder(nei_edge, BOND_SINGLE);
               break;
            }
         }
      }
}

void IndigoInchi::parseInchiOutput (const inchi_OutputStruct &inchi_output, Molecule &mol)
{
   mol.clear();

   Array<int> atom_indices;
   atom_indices.clear();
   
   // Add atoms
   for (AT_NUM i = 0; i < inchi_output.num_atoms; i ++)
   {
      const inchi_Atom &inchi_atom = inchi_output.atom[i];

      int idx = mol.addAtom(Element::fromString(inchi_atom.elname));
      atom_indices.push(idx);
   }

   // Add bonds
   for (AT_NUM i = 0; i < inchi_output.num_atoms; i ++)
   {
      const inchi_Atom &inchi_atom = inchi_output.atom[i];
      for (AT_NUM bi = 0; bi < inchi_atom.num_bonds; bi++)
      {
         AT_NUM nei = inchi_atom.neighbor[bi];
         if (i > nei)
            // Add bond only once
            continue;
         int bond_order = inchi_atom.bond_type[bi];
         if (bond_order == INCHI_BOND_TYPE_NONE)
            throw Molecule::Error("Indigo-InChI: NONE-typed bonds are not supported");
         if (bond_order >= INCHI_BOND_TYPE_ALTERN)
            throw Molecule::Error("Indigo-InChI: ALTERN-typed bonds are not supported");
         int bond = mol.addBond(atom_indices[i], atom_indices[nei], bond_order);
      }
   }

   // Add Hydrogen isotope atoms at the end to preserver 
   // the same atom ordering
   for (AT_NUM i = 0; i < inchi_output.num_atoms; i ++)
   {
      const inchi_Atom &inchi_atom = inchi_output.atom[i];

      int root_atom = atom_indices[i];
      for (int iso = 1; iso <= NUM_H_ISOTOPES; iso++)
      {
         int count = inchi_atom.num_iso_H[iso];
         while (count-- > 0)
         {
            int h = mol.addAtom(ELEM_H);
            mol.setAtomIsotope(h, iso);
            mol.addBond(root_atom, h, BOND_SINGLE);
         }
      }
   }

   // Set atom charges, radicals and etc.
   for (int i = 0; i < inchi_output.num_atoms; i++)
   {
      const inchi_Atom &inchi_atom = inchi_output.atom[i];

      int idx = atom_indices[i];
      mol.setAtomCharge(idx, inchi_atom.charge);
      if (inchi_atom.isotopic_mass)
         mol.setAtomIsotope(idx, inchi_atom.isotopic_mass);
      if (inchi_atom.radical)
         mol.setAtomRadical(idx, inchi_atom.radical);
      mol.setImplicitH(idx, inchi_atom.num_iso_H[0]);
   }

   neutralizeV5Nitrogen(mol);

   // Process stereoconfiguration
   for (int i = 0; i < inchi_output.num_stereo0D; i++)
   {
      inchi_Stereo0D &stereo0D = inchi_output.stereo0D[i];
      if (stereo0D.type == INCHI_StereoType_DoubleBond)
      {
         if (stereo0D.parity != INCHI_PARITY_ODD && stereo0D.parity != INCHI_PARITY_EVEN)
            continue;

         int bond = mol.findEdgeIndex(stereo0D.neighbor[1], stereo0D.neighbor[2]);

         bool valid = mol.cis_trans.registerBondAndSubstituents(bond);
         if (!valid)
            throw IndigoError("Indigo-InChI: Unsupported cis-trans configuration for "
               "bond %d (atoms %d-%d-%d-%d)", bond, stereo0D.neighbor[0], stereo0D.neighbor[1], 
               stereo0D.neighbor[2], stereo0D.neighbor[3]);

         int vb, ve;
         const Edge &edge = mol.getEdge(bond);
         if (edge.beg == stereo0D.neighbor[1])
         {
            vb = stereo0D.neighbor[0];
            ve = stereo0D.neighbor[3];
         }
         else if (edge.beg == stereo0D.neighbor[2])
         {
            vb = stereo0D.neighbor[3];
            ve = stereo0D.neighbor[0];
         }
         else
            throw IndigoError("Indigo-InChI: Internal error: cannot find cis-trans bond indices");

         const int *subst = mol.cis_trans.getSubstituents(bond);
         bool same_side;
         if (subst[0] == vb)
            same_side = (subst[2] == ve);
         else if (subst[1] == vb)
            same_side = (subst[3] == ve);
         else
            throw IndigoError("Indigo-InChI: Internal error: cannot find cis-trans bond indices (#2)");

         if (stereo0D.parity == INCHI_PARITY_EVEN)
            same_side = !same_side;

         mol.cis_trans.setParity(bond, same_side ? MoleculeCisTrans::CIS : MoleculeCisTrans::TRANS);
      }
      else if (stereo0D.type == INCHI_StereoType_Tetrahedral)
      {
         if (stereo0D.parity != INCHI_PARITY_ODD && stereo0D.parity != INCHI_PARITY_EVEN)
            continue;

         int pyramid[4];
         if (stereo0D.central_atom == stereo0D.neighbor[0])
         {
            pyramid[1] = stereo0D.neighbor[1];
            pyramid[0] = stereo0D.neighbor[2];
            pyramid[2] = stereo0D.neighbor[3];
            pyramid[3] = -1;
         }
         else
         {
            pyramid[0] = stereo0D.neighbor[0];
            pyramid[1] = stereo0D.neighbor[1];
            pyramid[2] = stereo0D.neighbor[2];
            pyramid[3] = stereo0D.neighbor[3];
         }
         if (stereo0D.parity == INCHI_PARITY_ODD)
            std::swap(pyramid[0], pyramid[1]);

         mol.stereocenters.add(stereo0D.central_atom, MoleculeStereocenters::ATOM_ABS, 0, pyramid);
      }
   }

}

inchi_BondType IndigoInchi::getInchiBondType (int bond_order)
{
   switch (bond_order)
   {
   case BOND_SINGLE:
      return INCHI_BOND_TYPE_SINGLE;
   case BOND_DOUBLE:
      return INCHI_BOND_TYPE_DOUBLE;
   case BOND_TRIPLE:
      return INCHI_BOND_TYPE_TRIPLE;
   case BOND_AROMATIC:
      return INCHI_BOND_TYPE_ALTERN;
   }
   throw IndigoError("Indigo-InChI: unexpected bond order %d", bond_order);
}

void IndigoInchi::generateInchiInput (Molecule &mol, inchi_Input &input, 
   Array<inchi_Atom> &atoms, Array<inchi_Stereo0D> &stereo)
{
   QS_DEF(Array<int>, mapping);
   mapping.clear_resize(mol.vertexEnd());
   mapping.fffill();
   int index = 0;
   for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
      mapping[v] = index++;
   atoms.clear_resize(index);
   atoms.zerofill();

   stereo.clear();
   for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
   {
      inchi_Atom &atom = atoms[mapping[v]];
      
      int atom_number = mol.getAtomNumber(v);
      if (atom_number == ELEM_PSEUDO)
         throw IndigoError("Molecule with pseudoatom (%s) cannot be converted into InChI", mol.getPseudoAtom(v));
      if (atom_number == ELEM_RSITE)
         throw IndigoError("Molecule with RGroups cannot be converted into InChI");
      strncpy(atom.elname, Element::toString(atom_number), ATOM_EL_LEN);

      Vec3f &c = mol.getAtomXyz(v);
      atom.x = c.x;
      atom.y = c.y;
      atom.z = c.z;
                              
      // connectivity
      const Vertex &vtx = mol.getVertex(v);
      int nei_idx = 0;
      for (int nei = vtx.neiBegin(); nei != vtx.neiEnd(); nei = vtx.neiNext(nei))
      {
         int v_nei = vtx.neiVertex(nei);
         atom.neighbor[nei_idx] = mapping[v_nei];
         int edge_idx = vtx.neiEdge(nei);
         atom.bond_type[nei_idx] = getInchiBondType(mol.getBondOrder(edge_idx));

         int bond_stereo = INCHI_BOND_STEREO_NONE;
         if (mol.cis_trans.isIgnored(edge_idx))
            bond_stereo = INCHI_BOND_STEREO_DOUBLE_EITHER;
         else
         {
            int dir = mol.getBondDirection2(v, v_nei);
            if (mol.getBondDirection2(v, v_nei) == BOND_EITHER)
               bond_stereo = INCHI_BOND_STEREO_SINGLE_1EITHER;
            else if (mol.getBondDirection2(v_nei, v) == BOND_EITHER)
               bond_stereo = INCHI_BOND_STEREO_SINGLE_2EITHER;
         }
         atom.bond_stereo[nei_idx] = bond_stereo;
         nei_idx++;
      }
      atom.num_bonds = vtx.degree();

      // Other properties
      atom.isotopic_mass = mol.getAtomIsotope(v);
      atom.radical = mol.getAtomRadical(v);
      atom.charge = mol.getAtomCharge(v);

      // Hydrogens
      int hcount = -1;
      if (Molecule::shouldWriteHCount(mol, v) || mol.isExplicitValenceSet(v) || mol.isImplicitHSet(v))
      {
         if (atom_number == ELEM_C && atom.charge == 0)
         {
            // Do not set number of implicit hydrogens here as InChI throws an exception on
            // the molecule B1=CB=c2cc3B=CC=c3cc12
            ;
         }
         else
            // set -1 to tell InChI add implicit hydrogens automatically
            hcount = mol.getImplicitH_NoThrow(v, -1); 
      }
      atom.num_iso_H[0] = hcount;
   }
  
   // Process cis-trans bonds
   for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
   {
      if (mol.cis_trans.getParity(e) == 0)
         continue;

      int subst[4];
      mol.cis_trans.getSubstituents_All(e, subst);

      const Edge &edge = mol.getEdge(e);

      inchi_Stereo0D &st = stereo.push();

      // Write it as
      // #0 - #1 = #2 - #3
      st.neighbor[0] = mapping[subst[0]];
      st.neighbor[1] = mapping[edge.beg];
      st.neighbor[2] = mapping[edge.end];
      st.neighbor[3] = mapping[subst[2]];

      if (mol.cis_trans.getParity(e) == MoleculeCisTrans::CIS)
         st.parity = INCHI_PARITY_ODD;
      else
         st.parity = INCHI_PARITY_EVEN;

      st.central_atom = NO_ATOM;
      st.type = INCHI_StereoType_DoubleBond;
   }

   // Process tetrahedral stereocenters
   for (int i = mol.stereocenters.begin(); i != mol.stereocenters.end(); i = mol.stereocenters.next(i))
   {
      int v = mol.stereocenters.getAtomIndex(i);

      int type, group, pyramid[4];
      mol.stereocenters.get(v, type, group, pyramid);
      if (type == MoleculeStereocenters::ATOM_ANY)
         continue;

      for (int i = 0; i < 4; i++)
         if (pyramid[i] != -1)
            pyramid[i] = mapping[pyramid[i]];

      inchi_Stereo0D &st = stereo.push();

      /*
         4 neighbors

                  X                    neighbor[4] : {#W, #X, #Y, #Z}
                  |                    central_atom: #A
               W--A--Y                 type        : INCHI_StereoType_Tetrahedral
                  |
                  Z
         parity: if (X,Y,Z) are clockwize when seen from W then parity is 'e' otherwise 'o'
         Example (see AXYZW above): if W is above the plane XYZ then parity = 'e'

         3 neighbors

                    Y          Y       neighbor[4] : {#A, #X, #Y, #Z}
                   /          /        central_atom: #A
               X--A  (e.g. O=S   )     type        : INCHI_StereoType_Tetrahedral
                   \          \
                    Z          Z
      */
      int offset = 0;
      if (pyramid[3] == -1)
         offset = 1;

      st.neighbor[offset] = mapping[pyramid[0]];
      st.neighbor[offset + 1] = mapping[pyramid[1]];
      st.neighbor[offset + 2] = mapping[pyramid[2]];
      if (offset == 0)
         st.neighbor[3] = mapping[pyramid[3]];
      else
         st.neighbor[0] = mapping[v];

      st.parity = INCHI_PARITY_ODD;
      if (offset != 0)
         st.parity = INCHI_PARITY_ODD;
      else
         st.parity = INCHI_PARITY_EVEN;
      st.central_atom = mapping[v];
      st.type = INCHI_StereoType_Tetrahedral;
   }

   input.atom = atoms.ptr();
   input.num_atoms = atoms.size();
   input.stereo0D = stereo.ptr();
   input.num_stereo0D = stereo.size();
   input.szOptions = options.ptr();
}
 
void IndigoInchi::saveMoleculeIntoInchi (Molecule &mol, Array<char> &inchi)
{
   inchi_Input input;
   QS_DEF(Array<inchi_Atom>, atoms);
   QS_DEF(Array<inchi_Stereo0D>, stereo);

   // Check if structure has aromatic bonds
   bool has_aromatic = false;
   for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
      if (mol.getBondOrder(e) == BOND_AROMATIC)
      {
         has_aromatic = true;
         break;
      }

   Molecule *target = &mol;
   Obj<Molecule> dearom;
   if (has_aromatic)
   {
      dearom.create();
      dearom->clone(mol, 0, 0);
      try
      {
         dearom->dearomatize();
      }
      catch (DearomatizationsGroups::Error &)
      {
      }
      target = dearom.get();
   }
   generateInchiInput(*target, input, atoms, stereo);

   inchi_Output output;
   
   int ret = GetINCHI(&input, &output);

   if (output.szMessage)
      warning.readString(output.szMessage, true);
   if (output.szLog)
      log.readString(output.szLog, true);
   if (output.szAuxInfo)
      auxInfo.readString(output.szAuxInfo, true);

   if (ret != inchi_Ret_OKAY && ret != inchi_Ret_WARNING)
   {
      // Construct error before dispoing inchi output to preserve error message
      IndigoError error("Indigo-InChI: InChI generation failed: %s. Code: %d.", output.szMessage, ret);
      FreeINCHI(&output);
      throw error;
   }

   inchi.readString(output.szInChI, true);

   FreeINCHI(&output);
}


//
// Session Inchi instance
//

_SessionLocalContainer<IndigoInchi> indigo_inchi_self;

IndigoInchi &indigoInchiGetInstance ()
{
   return indigo_inchi_self.getLocalCopy();
}

// 
// C interface functions
//

CEXPORT int indigoInchiResetOptions (void)
{
   IndigoInchi &indigo_inchi = indigoInchiGetInstance();
   indigo_inchi.clear();
   return 0;
}

CEXPORT int indigoInchiLoadMolecule (const char *inchi_string)
{
   INDIGO_BEGIN
   {
      IndigoInchi &indigo_inchi = indigoInchiGetInstance();

      AutoPtr<IndigoMolecule> mol_obj(new IndigoMolecule());

      indigo_inchi.loadMoleculeFromInchi(inchi_string, mol_obj->mol);
      return self.addObject(mol_obj.release());
   }
   INDIGO_END(-1)
}

CEXPORT const char* indigoInchiGetInchi (int molecule)
{
   INDIGO_BEGIN
   {
      IndigoInchi &indigo_inchi = indigoInchiGetInstance();
      IndigoObject &obj = self.getObject(molecule);

      indigo_inchi.saveMoleculeIntoInchi(obj.getMolecule(), self.tmp_string);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0)
}

CEXPORT const char* indigoInchiGetInchiKey (const char *inchi_string)
{
   INDIGO_BEGIN
   {
      self.tmp_string.resize(30);
      self.tmp_string.zerofill();
      int ret = GetINCHIKeyFromINCHI(inchi_string, 0, 0, self.tmp_string.ptr(), 0, 0);
      if (ret != INCHIKEY_OK)
      {
         if (ret == INCHIKEY_UNKNOWN_ERROR)
            throw IndigoError("Indigo-InChI: INCHIKEY_UNKNOWN_ERROR");
         else if (ret == INCHIKEY_EMPTY_INPUT)
            throw IndigoError("Indigo-InChI: INCHIKEY_EMPTY_INPUT");
         else if (ret == INCHIKEY_INVALID_INCHI_PREFIX)
            throw IndigoError("Indigo-InChI: INCHIKEY_INVALID_INCHI_PREFIX");
         else if (ret == INCHIKEY_NOT_ENOUGH_MEMORY)
            throw IndigoError("Indigo-InChI: INCHIKEY_NOT_ENOUGH_MEMORY");
         else if (ret == INCHIKEY_INVALID_INCHI)
            throw IndigoError("Indigo-InChI: INCHIKEY_INVALID_INCHI");
         else if (ret == INCHIKEY_INVALID_STD_INCHI)
            throw IndigoError("Indigo-InChI: INCHIKEY_INVALID_STD_INCHI");
         else
            throw IndigoError("Indigo-InChI: Undefined error");
      }
      return self.tmp_string.ptr();
   }
   INDIGO_END(0)
}

CEXPORT const char* indigoInchiGetWarning ()
{
   IndigoInchi &indigo_inchi = indigoInchiGetInstance();
   if (indigo_inchi.warning.size() != 0)
      return indigo_inchi.warning.ptr();
   return "";
}

CEXPORT const char* indigoInchiGetLog ()
{
   IndigoInchi &indigo_inchi = indigoInchiGetInstance();
   if (indigo_inchi.log.size() != 0)
      return indigo_inchi.log.ptr();
   return "";
}

CEXPORT const char* indigoInchiGetAuxInfo ()
{
   IndigoInchi &indigo_inchi = indigoInchiGetInstance();
   if (indigo_inchi.auxInfo.size() != 0)
      return indigo_inchi.auxInfo.ptr();
   return "";
}

//
// Options
//

void indigoInchiSetInchiOptions (const char *options)
{
   IndigoInchi &inchi = indigoInchiGetInstance();
   inchi.options.readString(options, true);
}

class _IndigoInchiOptionsHandlersSetter
{
public:
   _IndigoInchiOptionsHandlersSetter ();
};

_IndigoInchiOptionsHandlersSetter::_IndigoInchiOptionsHandlersSetter ()
{
   OptionManager &mgr = indigoGetOptionManager();
   OsLocker locker(mgr.lock);
   
   mgr.setOptionHandlerString("inchi-options", indigoInchiSetInchiOptions);
}

_IndigoInchiOptionsHandlersSetter _indigo_inchi_options_handlers_setter;


