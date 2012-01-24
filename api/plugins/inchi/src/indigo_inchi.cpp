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

using namespace indigo;

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

   void generateInchiInput (Molecule &mol, inchi_Input &input, Array<inchi_Atom> &atoms);
   void saveMoleculeIntoInchi (Molecule &mol, Array<char> &inchi);

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
      mol.setAtomCharge(idx, inchi_atom.charge);
      if (inchi_atom.isotopic_mass)
         mol.setAtomIsotope(idx, inchi_atom.isotopic_mass);
      if (inchi_atom.radical)
         mol.setAtomRadical(idx, inchi_atom.radical);
      mol.setImplicitH(idx, inchi_atom.num_iso_H[0]);
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

void IndigoInchi::generateInchiInput (Molecule &mol, inchi_Input &input, Array<inchi_Atom> &atoms)
{
   QS_DEF(Array<char>, mapping);
   mapping.clear_resize(mol.vertexEnd());
   mapping.fffill();
   int index = 0;
   for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
      mapping[v] = index++;
   atoms.clear_resize(index);
   atoms.zerofill();

   for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
   {
      inchi_Atom &atom = atoms[mapping[v]];
      
      strncpy(atom.elname, Element::toString(mol.getAtomNumber(v)), ATOM_EL_LEN);

      Vec3f &c = mol.getAtomXyz(v);
      atom.x = c.x;
      atom.y = c.y;
      atom.z = c.z;

      // connectivity
      const Vertex &vtx = mol.getVertex(v);
      int nei_idx = 0;
      for (int nei = vtx.neiBegin(); nei != vtx.neiEnd(); nei = vtx.neiNext(nei))
      {
         atom.neighbor[nei_idx] = mapping[vtx.neiVertex(nei)];
         int edge_idx = vtx.neiEdge(nei);
         atom.bond_type[nei_idx] = getInchiBondType(mol.getBondOrder(edge_idx));
         nei_idx++;
      }
      atom.num_bonds = vtx.degree();

      // Other properties
      atom.num_iso_H[0] = mol.getImplicitH_NoThrow(v, -1); // -1 means INCHI adds implicit H automatically
      atom.isotopic_mass = mol.getAtomIsotope(v);
      atom.radical = mol.getAtomRadical(v);
      atom.charge = mol.getAtomCharge(v);
   }
  
   input.atom = atoms.ptr();
   input.num_atoms = atoms.size();
   input.stereo0D = 0;
   input.szOptions = options.ptr();
}
 
void IndigoInchi::saveMoleculeIntoInchi (Molecule &mol, Array<char> &inchi)
{
   inchi_Input input;
   QS_DEF(Array<inchi_Atom>, atoms);
   generateInchiInput(mol, input, atoms);

   inchi_Output output;
   
   int ret = GetINCHI(&input, &output);

   if (output.szMessage)
      warning.readString(output.szMessage, true);
   if (output.szLog)
      log.readString(output.szLog, true);
   if (output.szAuxInfo)
      auxInfo.readString(output.szAuxInfo, true);

   if (ret != inchi_Ret_OKAY && ret != inchi_Ret_WARNING)
      throw IndigoError("Indigo-InChI: InChI generation failed: %s. Code: %d.", 
         output.szMessage, ret);

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


