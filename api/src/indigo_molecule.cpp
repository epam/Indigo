/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#include "indigo_molecule.h"
#include "indigo_io.h"
#include "indigo_array.h"
#include "molecule/molecule_auto_loader.h"
#include "base_cpp/output.h"
#include "molecule/gross_formula.h"
#include "molecule/molecule_mass.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/molecule_inchi.h"
#include "base_c/bitarray.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/elements.h"
#include "molecule/molecule_automorphism_search.h"
#include "base_cpp/scanner.h"
#include "indigo_mapping.h"

IndigoGross::IndigoGross() : IndigoObject(GROSS)
{
}

IndigoGross::~IndigoGross ()
{
}

void IndigoGross::toString (Array<char> &str)
{
   GrossFormula::toString_Hill(gross, str);
}

IndigoBaseMolecule::IndigoBaseMolecule (int type_) : IndigoObject(type_)
{
}

IndigoBaseMolecule::~IndigoBaseMolecule ()
{
}

RedBlackStringObjMap< Array<char> > * IndigoBaseMolecule::getProperties ()
{
   return &properties;
}

const char * IndigoBaseMolecule::debugInfo ()
{
   return "<base molecule>";
}

IndigoMolecule::IndigoMolecule () : IndigoBaseMolecule(MOLECULE)
{
}

bool IndigoBaseMolecule::is (IndigoObject &object)
{
   int type = object.type;

   if (type == MOLECULE || type == QUERY_MOLECULE ||
       type == REACTION_MOLECULE || type == SCAFFOLD ||
       type == RGROUP_FRAGMENT || type == RDF_MOLECULE || type == SMILES_MOLECULE || type == CML_MOLECULE)
      return true;

   if (type == ARRAY_ELEMENT)
      return is(((IndigoArrayElement &)object).get());

   return false;
}

IndigoMolecule::~IndigoMolecule ()
{
}

Molecule & IndigoMolecule::getMolecule ()
{
   return mol;
}

BaseMolecule & IndigoMolecule::getBaseMolecule ()
{
   return mol;
}

const char * IndigoMolecule::getName ()
{
   if (mol.name.ptr() == 0)
      return "";
   return mol.name.ptr();
}

IndigoMolecule * IndigoMolecule::cloneFrom (IndigoObject & obj)
{
   AutoPtr<IndigoMolecule> molptr(new IndigoMolecule());
   QS_DEF(Array<int>, mapping);

   molptr->mol.clone(obj.getMolecule(), 0, &mapping);

   RedBlackStringObjMap< Array<char> > *props = obj.getProperties();
   if (props != 0)
      molptr->copyProperties(*props);

   return molptr.release();
}

IndigoQueryMolecule::IndigoQueryMolecule () : IndigoBaseMolecule(QUERY_MOLECULE)
{
   _nei_counters_edit_revision = -1;
}

IndigoQueryMolecule::~IndigoQueryMolecule ()
{
}

QueryMolecule & IndigoQueryMolecule::getQueryMolecule ()
{
   return qmol;
}

IndigoQueryMolecule * IndigoQueryMolecule::cloneFrom( IndigoObject & obj )
{
   AutoPtr<IndigoQueryMolecule> molptr(new IndigoQueryMolecule());
   QS_DEF(Array<int>, mapping);

   molptr->qmol.clone(obj.getQueryMolecule(), 0, &mapping);

   RedBlackStringObjMap< Array<char> > *props = obj.getProperties();
   if (props != 0)
      molptr->copyProperties(*props);

   return molptr.release();
}

void IndigoQueryMolecule::parseAtomConstraint (const char* type, const char* value, 
   AutoPtr<QueryMolecule::Atom>& atom)
{
   enum KeyType { Int, Bool };
   struct Mapping
   {
      const char *key;
      QueryMolecule::OpType value;
      KeyType key_type;
   };

   static Mapping mappingForKeys[] = 
   {
      { "atomic-number", QueryMolecule::ATOM_NUMBER, Int },
      { "charge", QueryMolecule::ATOM_CHARGE, Int },
      { "isotope", QueryMolecule::ATOM_ISOTOPE, Int },
      { "radical", QueryMolecule::ATOM_RADICAL, Int },
      { "valence", QueryMolecule::ATOM_VALENCE, Int },
      { "connectivity", QueryMolecule::ATOM_CONNECTIVITY, Int },
      { "total-bond-order", QueryMolecule::ATOM_TOTAL_BOND_ORDER, Int },
      { "hydrogens", QueryMolecule::ATOM_TOTAL_H, Int },
      { "substituents", QueryMolecule::ATOM_SUBSTITUENTS, Int },
      { "ring", QueryMolecule::ATOM_SSSR_RINGS, Int },
      { "smallest-ring-size", QueryMolecule::ATOM_SMALLEST_RING_SIZE, Int },
      { "ring-bonds", QueryMolecule::ATOM_RING_BONDS, Int },
      { "rsite-mask", QueryMolecule::ATOM_RSITE, Int },
      { "highlighting", QueryMolecule::HIGHLIGHTING, Bool },
   };

   for (int i = 0; i < NELEM(mappingForKeys); i++)
   {
      if(strcasecmp(type, mappingForKeys[i].key) == 0)
      {
         int int_value = 0;
         if (value != NULL)
         {
            if (mappingForKeys[i].key_type == Int)
            {
               BufferScanner buf_scanner(value);
               int_value = buf_scanner.readInt();
            }
            else if (mappingForKeys[i].key_type == Bool)
            {
               if (strcasecmp(value, "true") == 0)
                  int_value = 1;
               else if (strcasecmp(value, "false") == 0)
                  int_value = 0;
               else
               {
                  BufferScanner buf_scanner(value);
                  int_value = buf_scanner.readInt();
               }
            }
         }
         atom.reset(new QueryMolecule::Atom(mappingForKeys[i].value, int_value));
         return;
      }
   }

   if (strcasecmp(type, "rsite") == 0)
   {
      int int_value = 0;
      if (value != NULL)
      {
         BufferScanner buf_scanner(value);
         int_value = buf_scanner.readInt();
      }
      atom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 1 << int_value));
      return;
   }
   else if (strcasecmp(type, "smarts") == 0)
   {
      if (value == NULL)
         throw IndigoError("Internal error: value argument in parseAtomConstraint has null value");
      atom.reset(parseAtomSMARTS(value));
      return;
   }
   else if (strcasecmp(type, "aromaticity") == 0)
   {
      int int_value = 0;
      if (value != NULL)
      {
         if (strcasecmp(value, "aromatic") == 0)
            int_value = ATOM_AROMATIC;
         else if (strcasecmp(value, "aliphatic") == 0)
            int_value = ATOM_ALIPHATIC;
         else
            throw IndigoError("unsupported aromaticity type: %s", value);
      }
      atom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, int_value));
      return;
   }

   throw IndigoError("unsupported constraint type: %s", type);
}

QueryMolecule::Atom* IndigoQueryMolecule::parseAtomSMARTS (const char *string)
{
   if (strlen(string) == 0)
      return new QueryMolecule::Atom();

   QS_DEF(QueryMolecule, qmol);
   qmol.clear();

   BufferScanner scanner(string);
   SmilesLoader loader(scanner);

   loader.loadSMARTS(qmol);
   if (qmol.vertexCount() != 1)
      throw IndigoError("cannot parse '%s' as a single-atom", string);

   return qmol.releaseAtom(qmol.vertexBegin());
}

BaseMolecule & IndigoQueryMolecule::getBaseMolecule ()
{
   return qmol;
}

const char * IndigoQueryMolecule::getName ()
{
   if (qmol.name.ptr() == 0)
      return "";
   return qmol.name.ptr();
}


IndigoAtom::IndigoAtom (BaseMolecule &mol_, int idx_) :
IndigoObject (ATOM),
mol(mol_)
{
   idx = idx_;
}

IndigoAtom::~IndigoAtom ()
{
}

int IndigoAtom::getIndex ()
{
   return idx;
}

const char * IndigoAtom::debugInfo ()
{
   return "<atom>";
}

bool IndigoAtom::is (IndigoObject &obj)
{
   if (obj.type == IndigoObject::ATOM || obj.type == IndigoObject::ATOM_NEIGHBOR)
      return true;
   if (obj.type == IndigoObject::ARRAY_ELEMENT)
      return is(((IndigoArrayElement &)obj).get());
   return false;
}

IndigoAtom & IndigoAtom::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::ATOM || obj.type == IndigoObject::ATOM_NEIGHBOR)
      return (IndigoAtom &)obj;
   if (obj.type == IndigoObject::ARRAY_ELEMENT)
      return cast(((IndigoArrayElement &)obj).get());
   throw IndigoError("%s does not represent an atom", obj.debugInfo());
}

void IndigoAtom::remove ()
{
   mol.removeAtom(idx);
}

IndigoObject * IndigoAtom::clone ()
{
   return new IndigoAtom(mol, idx);
}

IndigoAtomsIter::IndigoAtomsIter (BaseMolecule *mol, int type_) : IndigoObject(ATOMS_ITER)
{
   _mol = mol;
   _type = type_;
   _idx = -1;
}

IndigoAtomsIter::~IndigoAtomsIter ()
{
}

int IndigoAtomsIter::_shift (int idx)
{
   if (_type == PSEUDO)
   {
      for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
         if (_mol->isPseudoAtom(idx))
            break;
   }
   else if (_type == RSITE)
   {
      for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
         if (_mol->isRSite(idx))
            break;
   }
   else if (_type == STEREOCENTER)
   {
      for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
         if (_mol->stereocenters.getType(idx) != 0)
            break;
   }
   else if (_type == ALLENE_CENTER)
   {
      for (; idx != _mol->vertexEnd(); idx = _mol->vertexNext(idx))
         if (_mol->allene_stereo.isCenter(idx))
            break;
   }

   return idx;
}

bool IndigoAtomsIter::hasNext ()
{
   if (_idx == _mol->vertexEnd())
      return false;

   int next_idx;

   if (_idx == -1)
      next_idx = _shift(_mol->vertexBegin());
   else
      next_idx = _shift(_mol->vertexNext(_idx));

   return next_idx != _mol->vertexEnd();
}

IndigoObject * IndigoAtomsIter::next ()
{
   if (_idx == -1)
      _idx = _mol->vertexBegin();
   else
      _idx = _mol->vertexNext(_idx);

   _idx = _shift(_idx);

   if (_idx == _mol->vertexEnd())
      return 0;

   AutoPtr<IndigoAtom> atom(new IndigoAtom(*_mol, _idx));
   
   return atom.release();
}

IndigoBond::IndigoBond (BaseMolecule &mol_, int idx_) :
IndigoObject(BOND),
mol(mol_)
{
   idx = idx_;
}

IndigoBond::~IndigoBond ()
{
}

int IndigoBond::getIndex ()
{
   return idx;
}

const char * IndigoBond::debugInfo ()
{
   return "<bond>";
}

bool IndigoBond::is (IndigoObject &obj)
{
   if (obj.type == IndigoObject::BOND)
      return true;
   if (obj.type == IndigoObject::ARRAY_ELEMENT)
      return is(((IndigoArrayElement &)obj).get());
   return false;
}

IndigoBond & IndigoBond::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::BOND)
      return (IndigoBond &)obj;
   if (obj.type == IndigoObject::ARRAY_ELEMENT)
      return cast(((IndigoArrayElement &)obj).get());
   throw IndigoError("%s does not represent a bond", obj.debugInfo());
}

void IndigoBond::remove ()
{
   mol.removeBond(idx);
}

IndigoBondsIter::IndigoBondsIter (BaseMolecule &mol) :
IndigoObject(BONDS_ITER),
_mol(mol)
{
   _mol = mol;
   _idx = -1;
}

IndigoBondsIter::~IndigoBondsIter ()
{
}

bool IndigoBondsIter::hasNext ()
{
   if (_idx == _mol.edgeEnd())
      return false;

   int next_idx;

   if (_idx == -1)
      next_idx = _mol.edgeBegin();
   else
      next_idx = _mol.edgeNext(_idx);

   return next_idx != _mol.edgeEnd();
}

IndigoObject * IndigoBondsIter::next ()
{
   if (_idx == -1)
      _idx = _mol.edgeBegin();
   else
      _idx = _mol.edgeNext(_idx);

   if (_idx == _mol.edgeEnd())
      return 0;

   AutoPtr<IndigoBond> bond(new IndigoBond(_mol, _idx));

   return bond.release();
}

CEXPORT int indigoLoadMolecule (int source)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(source);

      MoleculeAutoLoader loader(IndigoScanner::get(obj));

      loader.ignore_stereocenter_errors = self.ignore_stereochemistry_errors;
      loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
      loader.ignore_noncritical_query_features = self.ignore_noncritical_query_features;
      loader.skip_3d_chirality = self.skip_3d_chirality;

      AutoPtr<IndigoMolecule> molptr(new IndigoMolecule());

      Molecule &mol = molptr->mol;

      loader.loadMolecule(mol);
      molptr->properties.copy(loader.properties);

      return self.addObject(molptr.release());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoLoadQueryMolecule (int source)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(source);
      MoleculeAutoLoader loader(IndigoScanner::get(obj));

      loader.ignore_stereocenter_errors = self.ignore_stereochemistry_errors;
      loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;

      AutoPtr<IndigoQueryMolecule> molptr(new IndigoQueryMolecule());

      QueryMolecule &qmol = molptr->qmol;

      loader.loadQueryMolecule(qmol);
      molptr->properties.copy(loader.properties);

      return self.addObject(molptr.release());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoLoadSmarts (int source)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(source);
      SmilesLoader loader(IndigoScanner::get(obj));

      AutoPtr<IndigoQueryMolecule> molptr(new IndigoQueryMolecule());

      QueryMolecule &qmol = molptr->qmol;

      loader.loadSMARTS(qmol);
      return self.addObject(molptr.release());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoGrossFormula (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      AutoPtr<IndigoGross> grossptr(new IndigoGross());

      GrossFormula::collect(mol, grossptr->gross);
      return self.addObject(grossptr.release());
   }
   INDIGO_END(-1)
}

CEXPORT float indigoMolecularWeight (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      MoleculeMass mass;
      return mass.molecularWeight(mol);
   }
   INDIGO_END(-1)
}

CEXPORT float indigoMostAbundantMass (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      MoleculeMass mass;
      return mass.mostAbundantMass(mol);
   }
   INDIGO_END(-1)
}

CEXPORT float indigoMonoisotopicMass (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      MoleculeMass mass;
      return mass.monoisotopicMass(mol);
   }
   INDIGO_END(-1)
}

IndigoMoleculeComponent::IndigoMoleculeComponent (BaseMolecule &mol_, int index_) :
IndigoObject(COMPONENT),
mol(mol_)
{
   index = index_;
}

IndigoMoleculeComponent::~IndigoMoleculeComponent ()
{
}

int IndigoMoleculeComponent::getIndex ()
{
   return index;
}

IndigoObject * IndigoMoleculeComponent::clone ()
{
   AutoPtr<IndigoObject> res;
   BaseMolecule *newmol;

   if (mol.isQueryMolecule())
   {
      res.reset(new IndigoQueryMolecule());
      newmol = &(((IndigoQueryMolecule *)res.get())->qmol);
   }
   else
   {
      res.reset(new IndigoMolecule());
      newmol = &(((IndigoMolecule *)res.get())->mol);
   }
   
   Filter filter(mol.getDecomposition().ptr(), Filter::EQ, index);
   newmol->makeSubmolecule(mol, filter, 0, 0);
   return res.release();
}

IndigoComponentsIter::IndigoComponentsIter (BaseMolecule &mol_) :
IndigoObject(COMPONENT),
mol(mol_)
{
   _idx = -1;
}

IndigoComponentsIter::~IndigoComponentsIter ()
{
}

bool IndigoComponentsIter::hasNext ()
{
   return _idx + 1 < mol.countComponents();
}

IndigoObject * IndigoComponentsIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;
   return new IndigoMoleculeComponent(mol, _idx);
}

IndigoSGroupAtomsIter::IndigoSGroupAtomsIter (BaseMolecule &mol, BaseMolecule::SGroup &sgroup) :
IndigoObject(SGROUP_ATOMS_ITER),
_mol(mol),
_sgroup(sgroup)
{
   _idx = -1;
}

IndigoSGroupAtomsIter::~IndigoSGroupAtomsIter ()
{
}

bool IndigoSGroupAtomsIter::hasNext ()
{
   return _idx + 1 < _sgroup.atoms.size();
}

IndigoObject * IndigoSGroupAtomsIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;
   return new IndigoAtom(_mol, _sgroup.atoms[_idx]);
}

IndigoSGroupBondsIter::IndigoSGroupBondsIter (BaseMolecule &mol, BaseMolecule::SGroup &sgroup) :
IndigoObject(SGROUP_ATOMS_ITER),
_mol(mol),
_sgroup(sgroup)
{
   _idx = -1;
}

IndigoSGroupBondsIter::~IndigoSGroupBondsIter ()
{
}

bool IndigoSGroupBondsIter::hasNext ()
{
   return _idx + 1 < _sgroup.bonds.size();
}

IndigoObject * IndigoSGroupBondsIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;
   return new IndigoBond(_mol, _sgroup.bonds[_idx]);
}

int _indigoIterateAtoms (Indigo &self, int molecule, int type)
{
   BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

   AutoPtr<IndigoAtomsIter> newiter(new IndigoAtomsIter(&mol, type));

   return self.addObject(newiter.release());
}

CEXPORT int indigoIterateAtoms (int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);

      if (obj.type == IndigoObject::COMPONENT)
      {
         IndigoMoleculeComponent &mc = (IndigoMoleculeComponent &)obj;
         return self.addObject(new IndigoComponentAtomsIter(mc.mol, mc.index));
      }
      if (obj.type == IndigoObject::SUBMOLECULE)
      {
         IndigoSubmolecule &sm = (IndigoSubmolecule &)obj;
         return self.addObject(new IndigoSubmoleculeAtomsIter(sm));
      }
      if (obj.type == IndigoObject::DATA_SGROUP)
      {
         IndigoDataSGroup &dsg = IndigoDataSGroup::cast(obj);
         return self.addObject(new IndigoSGroupAtomsIter(*dsg.mol, dsg.mol->data_sgroups[dsg.idx]));
      }
      if (obj.type == IndigoObject::SUPERATOM)
      {
         IndigoSuperatom &sa = IndigoSuperatom::cast(obj);
         return self.addObject(new IndigoSGroupAtomsIter(sa.mol, sa.mol.superatoms[sa.idx]));
      }
      if (obj.type == IndigoObject::REPEATING_UNIT)
      {
         IndigoRepeatingUnit &ru = IndigoRepeatingUnit::cast(obj);
         return self.addObject(new IndigoSGroupAtomsIter(ru.mol, ru.mol.repeating_units[ru.idx]));
      }
      if (obj.type == IndigoObject::MULTIPLE_GROUP)
      {
         IndigoMultipleGroup &mr = IndigoMultipleGroup::cast(obj);
         return self.addObject(new IndigoSGroupAtomsIter(mr.mol, mr.mol.multiple_groups[mr.idx]));
      }
      if (obj.type == IndigoObject::GENERIC_SGROUP)
      {
         IndigoGenericSGroup &gg = IndigoGenericSGroup::cast(obj);
         return self.addObject(new IndigoSGroupAtomsIter(gg.mol, gg.mol.generic_sgroups[gg.idx]));
      }


      return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::ALL);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIterateBonds (int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);

      if (obj.type == IndigoObject::COMPONENT)
      {
         IndigoMoleculeComponent &mc = (IndigoMoleculeComponent &)obj;
         return self.addObject(new IndigoComponentBondsIter(mc.mol, mc.index));
      }
      if (obj.type == IndigoObject::SUBMOLECULE)
      {
         IndigoSubmolecule &sm = (IndigoSubmolecule &)obj;
         return self.addObject(new IndigoSubmoleculeBondsIter(sm));
      }
      if (obj.type == IndigoObject::DATA_SGROUP)
      {
         IndigoDataSGroup &dsg = IndigoDataSGroup::cast(obj);
         return self.addObject(new IndigoSGroupBondsIter(*dsg.mol, dsg.mol->data_sgroups[dsg.idx]));
      }
      if (obj.type == IndigoObject::SUPERATOM)
      {
         IndigoSuperatom &sa = IndigoSuperatom::cast(obj);
         return self.addObject(new IndigoSGroupBondsIter(sa.mol, sa.mol.superatoms[sa.idx]));
      }
      if (obj.type == IndigoObject::REPEATING_UNIT)
      {
         IndigoRepeatingUnit &ru = IndigoRepeatingUnit::cast(obj);
         return self.addObject(new IndigoSGroupBondsIter(ru.mol, ru.mol.repeating_units[ru.idx]));
      }
      if (obj.type == IndigoObject::MULTIPLE_GROUP)
      {
         IndigoMultipleGroup &mr = IndigoMultipleGroup::cast(obj);
         return self.addObject(new IndigoSGroupBondsIter(mr.mol, mr.mol.multiple_groups[mr.idx]));
      }
      if (obj.type == IndigoObject::GENERIC_SGROUP)
      {
         IndigoGenericSGroup &gg = IndigoGenericSGroup::cast(obj);
         return self.addObject(new IndigoSGroupBondsIter(gg.mol, gg.mol.generic_sgroups[gg.idx]));
      }
      BaseMolecule &mol = obj.getBaseMolecule();

      AutoPtr<IndigoBondsIter> newiter(new IndigoBondsIter(mol));

      return self.addObject(newiter.release());
   }
   INDIGO_END(-1);
}


CEXPORT int indigoCountAtoms (int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);

      if (obj.type == IndigoObject::COMPONENT)
      {
         IndigoMoleculeComponent &mc = (IndigoMoleculeComponent &)obj;
         return mc.mol.countComponentVertices(mc.index);
      }
      if (obj.type == IndigoObject::SUBMOLECULE)
      {
         IndigoSubmolecule &sm = (IndigoSubmolecule &)obj;
         return sm.vertices.size();
      }
      if (obj.type == IndigoObject::DATA_SGROUP)
      {
         IndigoDataSGroup &dsg = IndigoDataSGroup::cast(obj);
         return dsg.get().atoms.size();
      }
      if (obj.type == IndigoObject::SUPERATOM)
      {
         IndigoSuperatom &sa = IndigoSuperatom::cast(obj);
         return sa.get().atoms.size();
      }

      BaseMolecule &mol = obj.getBaseMolecule();
      
      return mol.vertexCount();
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountBonds (int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);

      if (obj.type == IndigoObject::COMPONENT)
      {
         IndigoMoleculeComponent &mc = (IndigoMoleculeComponent &)obj;
         return mc.mol.countComponentEdges(mc.index);
      }
      if (obj.type == IndigoObject::SUBMOLECULE)
      {
         IndigoSubmolecule &sm = (IndigoSubmolecule &)obj;
         return sm.edges.size();
      }
      if (obj.type == IndigoObject::DATA_SGROUP)
      {
         IndigoDataSGroup &dsg = IndigoDataSGroup::cast(obj);
         return dsg.get().bonds.size();
      }
      if (obj.type == IndigoObject::SUPERATOM)
      {
         IndigoSuperatom &sa = IndigoSuperatom::cast(obj);
         return sa.get().bonds.size();
      }

      BaseMolecule &mol = obj.getBaseMolecule();

      return mol.edgeCount();
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountPseudoatoms (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      int i, res = 0;

      for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
         if (mol.isPseudoAtom(i))
            res++;

      return res;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountRSites (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      int i, res = 0;

      for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
         if (mol.isRSite(i))
            res++;

      return res;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIteratePseudoatoms (int molecule)
{
   INDIGO_BEGIN
   {
      return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::PSEUDO);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIterateRSites (int molecule)
{
   INDIGO_BEGIN
   {
      return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::RSITE);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIterateStereocenters (int molecule)
{
   INDIGO_BEGIN
   {
      return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::STEREOCENTER);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIterateAlleneCenters (int molecule)
{
   INDIGO_BEGIN
   {
      return _indigoIterateAtoms(self, molecule, IndigoAtomsIter::ALLENE_CENTER);
   }
   INDIGO_END(-1);
}

CEXPORT const char * indigoSymbol (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      ia.mol.getAtomSymbol(ia.idx, self.tmp_string);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0);
}

CEXPORT int indigoIsPseudoatom (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      if (ia.mol.isPseudoAtom(ia.idx))
         return 1;
      return 0;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIsRSite (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      if (ia.mol.isRSite(ia.idx))
         return 1;
      return 0;
   }
   INDIGO_END(-1);
}


CEXPORT int indigoSingleAllowedRGroup (int rsite)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(rsite));

      return ia.mol.getSingleAllowedRGroup(ia.idx);
   }
   INDIGO_END(-1);
}

IndigoRGroup::IndigoRGroup () : IndigoObject(RGROUP)
{
}

IndigoRGroup::~IndigoRGroup ()
{
}

int IndigoRGroup::getIndex ()
{
   return idx;
}

IndigoRGroup & IndigoRGroup::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::RGROUP)
      return (IndigoRGroup &)obj;
   throw IndigoError("%s is not an rgroup", obj.debugInfo());
}

IndigoRGroupsIter::IndigoRGroupsIter (BaseMolecule *mol) : IndigoObject(RGROUPS_ITER)
{
   _mol = mol;
   _idx = 0;
}

IndigoRGroupsIter::~IndigoRGroupsIter ()
{
}

CEXPORT int indigoIterateRGroups (int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);

      if (IndigoBaseMolecule::is(obj))
      {
         BaseMolecule &mol = obj.getBaseMolecule();

         return self.addObject(new IndigoRGroupsIter(&mol));
      }

      throw IndigoError("%s can not have r-groups", obj.debugInfo());
   }
   INDIGO_END(-1);
}

IndigoRGroupFragment::IndigoRGroupFragment (IndigoRGroup &rgp, int idx) : IndigoObject(RGROUP_FRAGMENT)
{
   rgroup.idx = rgp.idx;
   rgroup.mol = rgp.mol;
   frag_idx = idx;
}

IndigoRGroupFragment::IndigoRGroupFragment (BaseMolecule *mol, int rgroup_idx, int fragment_idx) :
IndigoObject(RGROUP_FRAGMENT)
{
   rgroup.mol = mol;
   rgroup.idx = rgroup_idx;
   frag_idx = fragment_idx;
}

IndigoRGroupFragment::~IndigoRGroupFragment ()
{
}

int IndigoRGroupFragment::getIndex ()
{
   return frag_idx;
}

void IndigoRGroupFragment::remove ()
{
   rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments.remove(frag_idx);
}

QueryMolecule & IndigoRGroupFragment::getQueryMolecule ()
{
   return rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx]->asQueryMolecule();
}

Molecule & IndigoRGroupFragment::getMolecule ()
{
   return rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx]->asMolecule();
}

BaseMolecule & IndigoRGroupFragment::getBaseMolecule ()
{
   return *rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx];
}

IndigoObject * IndigoRGroupFragment::clone ()
{
   BaseMolecule *mol = rgroup.mol->rgroups.getRGroup(rgroup.idx).fragments[frag_idx];

   AutoPtr<IndigoBaseMolecule> molptr;

   if (mol->isQueryMolecule())
   {
      molptr.reset(new IndigoQueryMolecule());
      molptr->getQueryMolecule().clone(*mol, 0, 0);
   }
   else
   {
      molptr.reset(new IndigoMolecule());
      molptr->getMolecule().clone(*mol, 0, 0);
   }

   return molptr.release();
}


IndigoRGroupFragmentsIter::IndigoRGroupFragmentsIter (IndigoRGroup& rgp) :
IndigoObject(RGROUP_FRAGMENTS_ITER)
{
   _mol = rgp.mol;
   _rgroup_idx = rgp.idx;
   _frag_idx = -1;
}

IndigoRGroupFragmentsIter::~IndigoRGroupFragmentsIter ()
{
}

bool IndigoRGroupFragmentsIter::hasNext ()
{
   PtrPool<BaseMolecule> &frags = _mol->rgroups.getRGroup(_rgroup_idx).fragments;

   if (_frag_idx == -1)
      return frags.begin() != frags.end();
   return frags.next(_frag_idx) != frags.end();
}

IndigoObject * IndigoRGroupFragmentsIter::next ()
{
   if (!hasNext())
      return 0;

   PtrPool<BaseMolecule> &frags = _mol->rgroups.getRGroup(_rgroup_idx).fragments;

   if (_frag_idx == -1)
      _frag_idx = frags.begin();
   else
      _frag_idx = frags.next(_frag_idx);

   AutoPtr<IndigoRGroupFragment> rgroup(new IndigoRGroupFragment(_mol, _rgroup_idx, _frag_idx));

   return rgroup.release();
}

CEXPORT int indigoIterateRGroupFragments (int rgroup)
{
   INDIGO_BEGIN
   {
      IndigoRGroup &rgp = IndigoRGroup::cast(self.getObject(rgroup));

      AutoPtr<IndigoRGroupFragmentsIter> newiter(new IndigoRGroupFragmentsIter(rgp));
      return self.addObject(newiter.release());
   }
   INDIGO_END(-1);
}

bool IndigoRGroupsIter::hasNext ()
{
   bool result = false;
   /*
    * Skip empty fragments
    */
   while((_idx < _mol->rgroups.getRGroupCount()) &&
           (_mol->rgroups.getRGroup(_idx + 1).fragments.size() == 0)) {
      ++_idx;
   }
   
   if(_idx < _mol->rgroups.getRGroupCount())
      result = true;
   
   return result;
}

IndigoObject * IndigoRGroupsIter::next ()
{
   if (!hasNext())
      return 0;
   
   _idx += 1;
   AutoPtr<IndigoRGroup> rgroup(new IndigoRGroup());

   rgroup->mol = _mol;
   rgroup->idx = _idx;
   return rgroup.release();
}

CEXPORT int indigoCountAttachmentPoints (int rgroup)
{
   INDIGO_BEGIN
   {
      IndigoObject &object = self.getObject(rgroup);
      if (IndigoBaseMolecule::is(object))
         return object.getBaseMolecule().attachmentPointCount();

      IndigoRGroup &rgp = IndigoRGroup::cast(object);

      return rgp.mol->rgroups.getRGroup(rgp.idx).fragments[0]->attachmentPointCount();
   }
   INDIGO_END(-1);
}

CEXPORT int indigoDegree (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      return ia.mol.getVertex(ia.idx).degree();
   }
   INDIGO_END(-1);
}

CEXPORT int indigoGetCharge (int atom, int *charge)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      int ch = ia.mol.getAtomCharge(ia.idx);
      if (ch == CHARGE_UNKNOWN)
      {
         *charge = 0;
         return 0;
      }
      *charge = ch;
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoValence (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      return ia.mol.asMolecule().getAtomValence(ia.idx);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoGetExplicitValence (int atom, int *valence)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      int val = ia.mol.getExplicitValence(ia.idx);
      if (val == -1)
      {
         *valence = 0;
         return 0;
      }
      *valence = val;
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoSetExplicitValence (int atom, int valence)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      ia.mol.asMolecule().setExplicitValence(ia.idx, valence);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIsotope (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      int iso = ia.mol.getAtomIsotope(ia.idx);
      return iso == -1 ? 0 : iso;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoAtomicNumber (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      if (ia.mol.isPseudoAtom(ia.idx))
         throw IndigoError("indigoAtomicNumber() called on a pseudoatom");
      if (ia.mol.isRSite(ia.idx))
         throw IndigoError("indigoAtomicNumber() called on an R-site");

      int num = ia.mol.getAtomNumber(ia.idx);
      return num == -1 ? 0 : num;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoGetRadicalElectrons (int atom, int *electrons)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      int rad = ia.mol.getAtomRadical(ia.idx);

      if (rad == -1)
      {
         *electrons = 0;
         return 0;
      }
      *electrons = Element::radicalElectrons(rad);
      return 1;
   }
   INDIGO_END(-1);
}

static int mapRadicalToIndigoRadical (int radical)
{
   switch (radical)
   {
   case 0: return 0;
   case RADICAL_SINGLET: return INDIGO_SINGLET;
   case RADICAL_DOUBLET: return INDIGO_DOUBLET;
   case RADICAL_TRIPLET: return INDIGO_TRIPLET;
   default: throw IndigoError("Unknown radical type");
   }
}

static int mapIndigoRadicalToRadical (int indigo_radical)
{
   switch (indigo_radical)
   {
   case 0: return 0;
   case INDIGO_SINGLET: return RADICAL_SINGLET;
   case INDIGO_DOUBLET: return RADICAL_DOUBLET;
   case INDIGO_TRIPLET: return RADICAL_TRIPLET;
   default: throw IndigoError("Unknown radical type");
   }
}

CEXPORT int indigoGetRadical (int atom, int *radical)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      int rad = ia.mol.getAtomRadical(ia.idx);

      if (rad == -1)
      {
         *radical = 0;
         return 0;
      }
      *radical = mapRadicalToIndigoRadical(rad);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoSetRadical (int atom, int radical)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      ia.mol.asMolecule().setAtomRadical(ia.idx, mapIndigoRadicalToRadical(radical));
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT float * indigoXYZ (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      Vec3f &pos = mol.getAtomXyz(ia.idx);
      self.tmp_xyz[0] = pos.x;
      self.tmp_xyz[1] = pos.y;
      self.tmp_xyz[2] = pos.z;
      return self.tmp_xyz;
   }
   INDIGO_END(0)
}

CEXPORT int indigoSetXYZ (int atom, float x, float y, float z)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      Vec3f &pos = mol.getAtomXyz(ia.idx);
      pos.set(x, y, z);
      return 1;
   }
   INDIGO_END(0)
}

CEXPORT int indigoResetCharge (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      if (mol.isQueryMolecule())
         mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_CHARGE);
      else
         mol.asMolecule().setAtomCharge(ia.idx, 0);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoResetExplicitValence (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      if (mol.isQueryMolecule())
         mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_VALENCE);
      else
         mol.asMolecule().resetExplicitValence(ia.idx);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoResetRadical (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      if (mol.isQueryMolecule())
         mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_RADICAL);
      else
         mol.asMolecule().setAtomRadical(ia.idx, 0);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoResetIsotope (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      if (mol.isQueryMolecule())
         mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_ISOTOPE);
      else
         mol.asMolecule().setAtomIsotope(ia.idx, 0);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoResetRsite (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      BaseMolecule &mol = ia.mol;

      mol.asQueryMolecule().getAtom(ia.idx).removeConstraints(QueryMolecule::ATOM_RSITE);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoSetAttachmentPoint (int atom, int order)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      ia.mol.addAttachmentPoint(order, ia.idx);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoClearAttachmentPoints (int item)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(item).getBaseMolecule();
      mol.removeAttachmentPoints();
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoRemoveConstraints (int item, const char *str_type)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(item));
      QueryMolecule &qmol = ia.mol.asQueryMolecule();

      if(strcasecmp(str_type, "smarts") == 0)
         throw IndigoError("indigoRemoveConstraints(): type 'smarts' is not supported", str_type);

      AutoPtr<QueryMolecule::Atom> atom;
      IndigoQueryMolecule::parseAtomConstraint(str_type, NULL, atom);

      if (atom->children.size() != 0)
         throw IndigoError("indigoRemoveConstraints(): can not parse type: %s", str_type);

      qmol.getAtom(ia.idx).removeConstraints(atom->type);
      qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoAddConstraint (int atom, const char *type, const char *value)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      QueryMolecule& qmol = ia.mol.asQueryMolecule();
      AutoPtr<QueryMolecule::Atom> atom_constraint;

      IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);
      
      qmol.resetAtom(ia.idx, QueryMolecule::Atom::und(qmol.releaseAtom(ia.idx), atom_constraint.release()));
      qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoAddConstraintNot (int atom, const char *type, const char *value)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      QueryMolecule& qmol = ia.mol.asQueryMolecule();
      AutoPtr<QueryMolecule::Atom> atom_constraint;

      IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

      qmol.resetAtom(ia.idx, QueryMolecule::Atom::und(qmol.releaseAtom(ia.idx), QueryMolecule::Atom::nicht(atom_constraint.release())));
      qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoAddConstraintOr(int atom, const char* type, const char* value)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      QueryMolecule& qmol = ia.mol.asQueryMolecule();
      AutoPtr<QueryMolecule::Atom> atom_constraint;

      IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

      qmol.resetAtom(ia.idx, QueryMolecule::Atom::oder(qmol.releaseAtom(ia.idx), atom_constraint.release()));
      qmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ALL);

      return 1;
   }
   INDIGO_END(-1);
}

/*
CEXPORT int indigoAddConstraintOrNot(int atom, const char* type, const char* value)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      BaseMolecule *mol = ia.mol;
      QueryMolecule& qmol = mol->asQueryMolecule();
      AutoPtr<QueryMolecule::Atom> atom_constraint;

      IndigoQueryMolecule::parseAtomConstraint(type, value, atom_constraint);

      qmol.resetAtom(ia.idx, QueryMolecule::Atom::oder(qmol.releaseAtom(ia.idx), QueryMolecule::Atom::nicht(atom_constraint.release())));

      return 1;
   }
   INDIGO_END(-1);
}
 * */

CEXPORT const char * indigoCanonicalSmiles (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      ArrayOutput output(self.tmp_string);
      CanonicalSmilesSaver saver(output);
      
      saver.saveMolecule(mol);
      self.tmp_string.push(0);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0);
}

CEXPORT const int * indigoSymmetryClasses (int molecule, int *count_out)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      QS_DEF(Molecule, m2);
      m2.clone_KeepIndices(mol);
      m2.aromatize(self.arom_options);

      QS_DEF(Array<int>, ignored);
      ignored.clear_resize(m2.vertexEnd());
      ignored.zerofill();

      for (int i = m2.vertexBegin(); i < m2.vertexEnd(); i = m2.vertexNext(i))
         if (m2.convertableToImplicitHydrogen(i))
            ignored[i] = 1;

      MoleculeAutomorphismSearch of;

      QS_DEF(Array<int>, orbits);
      of.find_canonical_ordering = true;
      of.ignored_vertices = ignored.ptr();
      of.process(m2);
      of.getCanonicallyOrderedOrbits(orbits);

      self.tmp_string.resize(orbits.sizeInBytes());
      self.tmp_string.copy((char*)orbits.ptr(), orbits.sizeInBytes());

      if (count_out != 0)
         *count_out= orbits.size();

      return (const int*)self.tmp_string.ptr();
   }
   INDIGO_END(0);
}

CEXPORT const char * indigoLayeredCode (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      ArrayOutput output(self.tmp_string);

      MoleculeInChI inchi_saver(output);
      inchi_saver.outputInChI(mol);

      self.tmp_string.push(0);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0);
}

CEXPORT int indigoCreateSubmolecule (int molecule, int nvertices, int *vertices)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      
      QS_DEF(Array<int>, vertices_arr);

      vertices_arr.copy(vertices, nvertices);

      if (mol.isQueryMolecule())
      {
         AutoPtr<IndigoQueryMolecule> molptr(new IndigoQueryMolecule());

         molptr->qmol.makeSubmolecule(mol, vertices_arr, 0, 0);
         return self.addObject(molptr.release());
      }
      else
      {
         AutoPtr<IndigoMolecule> molptr(new IndigoMolecule());

         molptr->mol.makeSubmolecule(mol, vertices_arr, 0, 0);
         return self.addObject(molptr.release());
      }
   }
   INDIGO_END(-1)
}

CEXPORT int indigoGetSubmolecule (int molecule, int nvertices, int *vertices)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      
      QS_DEF(Array<int>, vertices_arr);
      vertices_arr.copy(vertices, nvertices);

      // Collect edges by vertices
      QS_DEF(Array<int>, vertices_mask);
      vertices_mask.clear_resize(mol.vertexEnd());
      vertices_mask.zerofill();
      for (int i = 0; i < nvertices; i++)
         vertices_mask[vertices[i]] = 1;

      QS_DEF(Array<int>, edges);
      edges.clear();
      for (int i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
      {
         const Edge &edge = mol.getEdge(i);
         if (vertices_mask[edge.beg] && vertices_mask[edge.end])
            edges.push(i);
      }

      AutoPtr<IndigoSubmolecule> subptr(new IndigoSubmolecule(mol, vertices_arr, edges));
      return self.addObject(subptr.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCreateEdgeSubmolecule (int molecule, int nvertices, int *vertices,
                                                       int nedges, int *edges)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      QS_DEF(Array<int>, vertices_arr);
      QS_DEF(Array<int>, edges_arr);

      vertices_arr.copy(vertices, nvertices);
      edges_arr.copy(edges, nedges);

      if (mol.isQueryMolecule())
      {
         AutoPtr<IndigoQueryMolecule> molptr(new IndigoQueryMolecule());

         molptr->qmol.makeEdgeSubmolecule(mol, vertices_arr, edges_arr, 0, 0);
         return self.addObject(molptr.release());
      }
      else
      {
         AutoPtr<IndigoMolecule> molptr(new IndigoMolecule());

         molptr->mol.makeEdgeSubmolecule(mol, vertices_arr, edges_arr, 0, 0);
         return self.addObject(molptr.release());
      }
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRemoveAtoms (int molecule, int nvertices, int *vertices)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      QS_DEF(Array<int>, indices);

      indices.copy(vertices, nvertices);

      mol.removeAtoms(indices);
      return 1;
   }
   INDIGO_END(-1)
}

IndigoObject * IndigoMolecule::clone ()
{
   return cloneFrom(*this);
}

const char * IndigoMolecule::debugInfo ()
{
   return "<molecule>";
}

IndigoObject * IndigoQueryMolecule::clone ()
{
   return cloneFrom(*this);
}

const char * IndigoQueryMolecule::debugInfo ()
{
   return "<query molecule>";
}

const MoleculeAtomNeighbourhoodCounters& IndigoQueryMolecule::getNeiCounters ()
{
   // TODO: implement query.getAtomEdit(...) instead of getAtom(...) to update nei counters
   // automatically. Current approach is too complictated because
   // we need to call updateEditRevision manually after changing an atom.
   //if (_nei_counters_edit_revision != qmol.getEditRevision())
   {
      _nei_counters.calculate(qmol);
      _nei_counters_edit_revision = qmol.getEditRevision();
   }
   return _nei_counters;
}

CEXPORT int indigoIsChiral (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return mol.isChrial();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoBondOrder (int bond)
{
   INDIGO_BEGIN
   {
      IndigoBond &ib = IndigoBond::cast(self.getObject(bond));

      int num = ib.mol.getBondOrder(ib.idx);
      return num == -1 ? 0 : num;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoTopology (int bond)
{
   INDIGO_BEGIN
   {
      IndigoBond &ib = IndigoBond::cast(self.getObject(bond));

      int topology = ib.mol.getBondTopology(ib.idx);
      if (topology == TOPOLOGY_RING)
         return INDIGO_RING;
      if (topology == TOPOLOGY_CHAIN)
         return INDIGO_CHAIN;
      return 0;
   }
   INDIGO_END(-1);
}


CEXPORT int indigoGetAtom (int molecule, int idx)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoAtom(mol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoGetBond (int molecule, int idx)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoBond(mol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSource (int bond)
{
   INDIGO_BEGIN
   {
      IndigoBond &ib = IndigoBond::cast(self.getObject(bond));
      return self.addObject(new IndigoAtom(ib.mol, ib.mol.getEdge(ib.idx).beg));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDestination (int bond)
{
   INDIGO_BEGIN
   {
      IndigoBond &ib = IndigoBond::cast(self.getObject(bond));
      return self.addObject(new IndigoAtom(ib.mol, ib.mol.getEdge(ib.idx).end));
   }
   INDIGO_END(-1)
}

IndigoAtomNeighbor::IndigoAtomNeighbor (BaseMolecule &mol_, int atom_idx, int bond_idx_) :
         IndigoAtom(mol_, atom_idx)
{
   type = ATOM_NEIGHBOR;

   bond_idx = bond_idx_;
}

IndigoAtomNeighbor::~IndigoAtomNeighbor ()
{
}

IndigoAtomNeighborsIter::IndigoAtomNeighborsIter (BaseMolecule &molecule, int atom_idx) :
IndigoObject(ATOM_NEIGHBORS_ITER),
_mol(molecule)
{
   _atom_idx = atom_idx;
   _nei_idx = -1;
}

IndigoAtomNeighborsIter::~IndigoAtomNeighborsIter ()
{
}

IndigoObject * IndigoAtomNeighborsIter::next ()
{
   const Vertex &vertex = _mol.getVertex(_atom_idx);

   if (_nei_idx == -1)
      _nei_idx = vertex.neiBegin();
   else if (_nei_idx != vertex.neiEnd())
      _nei_idx = vertex.neiNext(_nei_idx);

   if (_nei_idx == vertex.neiEnd())
      return 0;

   return new IndigoAtomNeighbor(_mol, vertex.neiVertex(_nei_idx), vertex.neiEdge(_nei_idx));
}

bool IndigoAtomNeighborsIter::hasNext ()
{
   const Vertex &vertex = _mol.getVertex(_atom_idx);

   if (_nei_idx == -1)
      return vertex.neiBegin() != vertex.neiEnd();

   if (_nei_idx == vertex.neiEnd())
      return false;

   return vertex.neiNext(_nei_idx) != vertex.neiEnd();
}

CEXPORT int indigoIterateNeighbors (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      
      return self.addObject(new IndigoAtomNeighborsIter(ia.mol, ia.idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoBond (int nei)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(nei);

      if (obj.type != IndigoObject::ATOM_NEIGHBOR)
         throw IndigoError("indigoBond(): not applicable to %s", obj.debugInfo());

      IndigoAtomNeighbor &atomnei = (IndigoAtomNeighbor &)obj;

      return self.addObject(new IndigoBond(atomnei.mol, atomnei.bond_idx));
   }
   INDIGO_END(-1)
}

CEXPORT float indigoAlignAtoms (int molecule, int natoms, int *atom_ids, float *desired_xyz)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      QS_DEF(Array<Vec3f>, points);
      QS_DEF(Array<Vec3f>, goals);
      int i;

      if (natoms < 1)
         throw IndigoError("indigoAlignAtoms(): can not align %d atoms", natoms);

      if (atom_ids == 0 || desired_xyz == 0)
         throw IndigoError("indigoAlignAtoms(): zero pointer given as input");
         
      points.clear();
      goals.clear();

      for (i= 0; i < natoms; i++)
      {
         points.push(mol.getAtomXyz(atom_ids[i]));
         goals.push(Vec3f(desired_xyz[i * 3], desired_xyz[i * 3 + 1], desired_xyz[i * 3 + 2]));
      }

      if (points.size() < 1)
         return true;

      float sqsum;
      Transform3f matr;

      if (!matr.bestFit(points.size(), points.ptr(), goals.ptr(), &sqsum))
         return false;

      for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
         mol.getAtomXyz(i).transformPoint(matr);

      return (float)(sqrt(sqsum / natoms));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountSuperatoms (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return mol.superatoms.size();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountDataSGroups (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return mol.data_sgroups.size();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountRepeatingUnits (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return mol.repeating_units.size();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountMultipleGroups (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return mol.multiple_groups.size();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountGenericSGroups (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return mol.generic_sgroups.size();
   }
   INDIGO_END(-1)
}


IndigoDataSGroupsIter::IndigoDataSGroupsIter (BaseMolecule &molecule) :
        IndigoObject(DATA_SGROUPS_ITER),
        _mol(molecule)
{
   _idx = -1;
}

IndigoDataSGroupsIter::~IndigoDataSGroupsIter ()
{
}

bool IndigoDataSGroupsIter::hasNext ()
{
   if (_idx == -1)
      return _mol.data_sgroups.begin() != _mol.data_sgroups.end();
   return _mol.data_sgroups.next(_idx) != _mol.data_sgroups.end();
}

IndigoObject * IndigoDataSGroupsIter::next ()
{
   if (!hasNext())
      return 0;

   if (_idx == -1)
      _idx = _mol.data_sgroups.begin();
   else
      _idx = _mol.data_sgroups.next(_idx);

   AutoPtr<IndigoDataSGroup> sgroup(new IndigoDataSGroup(_mol, _idx));
   return sgroup.release();
}


CEXPORT int indigoIterateDataSGroups (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoDataSGroupsIter(mol));
   }
   INDIGO_END(-1)
}

IndigoDataSGroup::IndigoDataSGroup (BaseMolecule &mol_, int idx_) :
         IndigoObject(DATA_SGROUP)
{
   mol = &mol_;
   idx = idx_;
}

IndigoDataSGroup & IndigoDataSGroup::cast (IndigoObject &obj)
{
   if (obj.type != DATA_SGROUP)
      throw IndigoError("%s is not a data sgroup", obj.debugInfo());
   return (IndigoDataSGroup &)obj;
}

BaseMolecule::DataSGroup & IndigoDataSGroup::get ()
{
   return mol->data_sgroups[idx];
}

void IndigoDataSGroup::remove ()
{
   mol->data_sgroups.remove(idx);
}

IndigoDataSGroup::~IndigoDataSGroup ()
{
}

int IndigoDataSGroup::getIndex ()
{
   return idx;
}

IndigoSuperatom::IndigoSuperatom (BaseMolecule &mol_, int idx_) :
IndigoObject(SUPERATOM),
mol(mol_)
{
   idx = idx_;
}

IndigoSuperatom::~IndigoSuperatom ()
{
}

int IndigoSuperatom::getIndex ()
{
   return idx;
}

void IndigoSuperatom::remove ()
{
   mol.superatoms.remove(idx);
}

const char * IndigoSuperatom::getName ()
{
   return mol.superatoms[idx].subscript.ptr();
}

IndigoSuperatom & IndigoSuperatom::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::SUPERATOM)
      return (IndigoSuperatom &)obj;

   throw IndigoError("%s is not a superatom", obj.debugInfo());
}

Molecule::Superatom & IndigoSuperatom::get ()
{
   return mol.superatoms[idx];
}


IndigoSuperatomsIter::IndigoSuperatomsIter (BaseMolecule &molecule) :
IndigoObject(SUPERATOMS_ITER),
_mol(molecule)
{
   _idx = -1;
}

IndigoSuperatomsIter::~IndigoSuperatomsIter ()
{
}

bool IndigoSuperatomsIter::hasNext ()
{
   if (_idx == -1)
      return _mol.superatoms.begin() != _mol.superatoms.end();
   return _mol.superatoms.next(_idx) != _mol.superatoms.end();
}

IndigoObject * IndigoSuperatomsIter::next ()
{
   if (!hasNext())
      return 0;

   if (_idx == -1)
      _idx = _mol.superatoms.begin();
   else
      _idx = _mol.superatoms.next(_idx);

   return new IndigoSuperatom(_mol, _idx);
}

IndigoRepeatingUnit::IndigoRepeatingUnit (BaseMolecule &mol_, int idx_) :
IndigoObject(REPEATING_UNIT),
mol(mol_)
{
   idx = idx_;
}

IndigoRepeatingUnit::~IndigoRepeatingUnit ()
{
}

int IndigoRepeatingUnit::getIndex ()
{
   return idx;
}

void IndigoRepeatingUnit::remove ()
{
   mol.repeating_units.remove(idx);
}

IndigoRepeatingUnit & IndigoRepeatingUnit::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::REPEATING_UNIT)
      return (IndigoRepeatingUnit &)obj;

   throw IndigoError("%s is not a repeating unit", obj.debugInfo());
}

Molecule::RepeatingUnit & IndigoRepeatingUnit::get ()
{
   return mol.repeating_units[idx];
}

IndigoRepeatingUnitsIter::IndigoRepeatingUnitsIter (BaseMolecule &molecule) :
IndigoObject(REPEATING_UNITS_ITER),
_mol(molecule)
{
   _idx = -1;
}

IndigoRepeatingUnitsIter::~IndigoRepeatingUnitsIter ()
{
}

bool IndigoRepeatingUnitsIter::hasNext ()
{
   if (_idx == -1)
      return _mol.repeating_units.begin() != _mol.repeating_units.end();
   return _mol.repeating_units.next(_idx) != _mol.repeating_units.end();
}

IndigoObject * IndigoRepeatingUnitsIter::next ()
{
   if (!hasNext())
      return 0;

   if (_idx == -1)
      _idx = _mol.repeating_units.begin();
   else
      _idx = _mol.repeating_units.next(_idx);

   return new IndigoRepeatingUnit(_mol, _idx);
}

IndigoMultipleGroup::IndigoMultipleGroup (BaseMolecule &mol_, int idx_) :
IndigoObject(MULTIPLE_GROUP),
mol(mol_)
{
   idx = idx_;
}

IndigoMultipleGroup::~IndigoMultipleGroup ()
{
}

int IndigoMultipleGroup::getIndex ()
{
   return idx;
}

void IndigoMultipleGroup::remove ()
{
   mol.multiple_groups.remove(idx);
}

IndigoMultipleGroup & IndigoMultipleGroup::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::MULTIPLE_GROUP)
      return (IndigoMultipleGroup &)obj;

   throw IndigoError("%s is not a multiple group", obj.debugInfo());
}

Molecule::MultipleGroup & IndigoMultipleGroup::get ()
{
   return mol.multiple_groups[idx];
}

IndigoMultipleGroupsIter::IndigoMultipleGroupsIter (BaseMolecule &molecule) :
IndigoObject(MULTIPLE_GROUPS_ITER),
_mol(molecule)
{
   _idx = -1;
}

IndigoMultipleGroupsIter::~IndigoMultipleGroupsIter ()
{
}

bool IndigoMultipleGroupsIter::hasNext ()
{
   if (_idx == -1)
      return _mol.multiple_groups.begin() != _mol.multiple_groups.end();
   return _mol.multiple_groups.next(_idx) != _mol.multiple_groups.end();
}

IndigoObject * IndigoMultipleGroupsIter::next ()
{
   if (!hasNext())
      return 0;

   if (_idx == -1)
      _idx = _mol.multiple_groups.begin();
   else
      _idx = _mol.multiple_groups.next(_idx);

   return new IndigoMultipleGroup(_mol, _idx);
}

IndigoGenericSGroup::IndigoGenericSGroup (BaseMolecule &mol_, int idx_) :
IndigoObject(GENERIC_SGROUP),
mol(mol_)
{
   idx = idx_;
}

IndigoGenericSGroup::~IndigoGenericSGroup ()
{
}

int IndigoGenericSGroup::getIndex ()
{
   return idx;
}

void IndigoGenericSGroup::remove ()
{
   mol.generic_sgroups.remove(idx);
}

IndigoGenericSGroup & IndigoGenericSGroup::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::GENERIC_SGROUP)
      return (IndigoGenericSGroup &)obj;

   throw IndigoError("%s is not a generic sgroup", obj.debugInfo());
}

Molecule::SGroup & IndigoGenericSGroup::get ()
{
   return mol.generic_sgroups[idx];
}

IndigoGenericSGroupsIter::IndigoGenericSGroupsIter (BaseMolecule &molecule) :
IndigoObject(GENERIC_SGROUPS_ITER),
_mol(molecule)
{
   _idx = -1;
}

IndigoGenericSGroupsIter::~IndigoGenericSGroupsIter ()
{
}

bool IndigoGenericSGroupsIter::hasNext ()
{
   if (_idx == -1)
      return _mol.generic_sgroups.begin() != _mol.generic_sgroups.end();
   return _mol.generic_sgroups.next(_idx) != _mol.generic_sgroups.end();
}

IndigoObject * IndigoGenericSGroupsIter::next ()
{
   if (!hasNext())
      return 0;

   if (_idx == -1)
      _idx = _mol.generic_sgroups.begin();
   else
      _idx = _mol.generic_sgroups.next(_idx);

   return new IndigoGenericSGroup(_mol, _idx);
}

CEXPORT int indigoIterateGenericSGroups (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoGenericSGroupsIter(mol));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateRepeatingUnits (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoRepeatingUnitsIter(mol));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateMultipleGroups (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoMultipleGroupsIter(mol));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateSuperatoms (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoSuperatomsIter(mol));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoGetSuperatom (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoSuperatom(mol, index));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoGetDataSGroup (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoDataSGroup(mol, index));
   }
   INDIGO_END(-1)
}

CEXPORT const char * indigoDescription (int data_sgroup)
{
   INDIGO_BEGIN
   {
      IndigoDataSGroup &dsg = IndigoDataSGroup::cast(self.getObject(data_sgroup));
      if (dsg.get().description.size() < 1)
         return "";
      return dsg.get().description.ptr();
   }
   INDIGO_END(0)
}

CEXPORT int indigoAddDataSGroup (int molecule, int natoms, int *atoms,
        int nbonds, int *bonds, const char *description, const char *data)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      int idx = mol.data_sgroups.add();
      BaseMolecule::DataSGroup &dsg =  mol.data_sgroups.at(idx);
      int i;
      if (atoms != 0)
         for (i = 0; i < natoms; i++)
            dsg.atoms.push(atoms[i]);
      if (bonds != 0)
         for (i = 0; i < nbonds; i++)
            dsg.bonds.push(bonds[i]);
      if (data != 0)
         dsg.data.readString(data, false);
      if (description != 0)
         dsg.description.readString(description, true);
      return self.addObject(new IndigoDataSGroup(mol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoAddSuperatom (int molecule, int natoms, int *atoms, const char *name)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      int idx = mol.superatoms.add();
      BaseMolecule::Superatom &satom =  mol.superatoms.at(idx);
      satom.subscript.appendString(name, true);
      if (atoms == 0)
         throw IndigoError("indigoAddSuperatom(): atoms were not specified");

      for (int i = 0; i < natoms; i++)
         satom.atoms.push(atoms[i]);

      return self.addObject(new IndigoSuperatom(mol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetDataSGroupXY (int sgroup, float x, float y, const char *options)
{
   INDIGO_BEGIN
   {
      BaseMolecule::DataSGroup &dsg = IndigoDataSGroup::cast(self.getObject(sgroup)).get();

      dsg.display_pos.x = x;
      dsg.display_pos.y = y;
      dsg.detached = true;

      if (options != 0 && options[0] != 0)
      {
         if (strcasecmp(options, "absolute") == 0)
            dsg.relative = false;
         else if (strcasecmp(options, "relative") == 0)
            dsg.relative = true;
         else
            throw IndigoError("indigoSetDataSGroupXY(): invalid options string");
      }

      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountHeavyAtoms (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();
      int i, cnt = 0;

      for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
         if (!mol.possibleAtomNumber(i, ELEM_H))
            cnt++;

      return cnt;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountComponents (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      return bm.countComponents();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCloneComponent (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();
      if (index < 0 || index >= bm.countComponents())
         throw IndigoError("indigoCloneComponent(): bad index %d (0-%d allowed)",
                 bm.countComponents() - 1);

      Filter filter(bm.getDecomposition().ptr(), Filter::EQ, index);
      AutoPtr<IndigoMolecule> im(new IndigoMolecule());
      im->mol.makeSubmolecule(bm, filter, 0, 0);
      return self.addObject(im.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoComponentIndex (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      return ia.mol.vertexComponent(ia.idx);
   }
   INDIGO_END(-1)
}

CEXPORT int indigoComponent (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      if (index < 0 || index >= bm.countComponents())
         throw IndigoError("indigoComponent(): bad index %d (0-%d allowed)",
                 bm.countComponents() - 1);

      return self.addObject(new IndigoMoleculeComponent(bm, index));
   }
   INDIGO_END(-1)
}


IndigoComponentAtomsIter::IndigoComponentAtomsIter (BaseMolecule &mol, int cidx) :
IndigoObject(COMPONENT_ATOMS_ITER),
_mol(mol)
{
   if (cidx < 0 || cidx > mol.countComponents())
      throw IndigoError("%d is not a valid component number (0-%d allowed)",
              cidx, _mol.countComponents() - 1);
   _idx = -1;
   _cidx = cidx;
}

IndigoComponentAtomsIter::~IndigoComponentAtomsIter ()
{
}

bool IndigoComponentAtomsIter::hasNext ()
{
   return _next() != _mol.vertexEnd();
}

IndigoObject * IndigoComponentAtomsIter::next ()
{
   int idx = _next();

   if (idx == _mol.vertexEnd())
      return 0;
   _idx = idx;
   return new IndigoAtom(_mol, idx);
}

int IndigoComponentAtomsIter::_next ()
{
   int idx;

   if (_idx == -1)
      idx = _mol.vertexBegin();
   else
      idx = _mol.vertexNext(_idx);

   for (; idx != _mol.vertexEnd(); idx = _mol.vertexNext(idx))
      if (_mol.vertexComponent(idx) == _cidx)
         break;
   return idx;
}

IndigoComponentBondsIter::IndigoComponentBondsIter (BaseMolecule &mol, int cidx) :
IndigoObject(COMPONENT_BONDS_ITER),
_mol(mol)
{
   if (cidx < 0 || cidx > _mol.countComponents())
      throw IndigoError("%d is not a valid component number (0-%d allowed)",
              cidx, _mol.countComponents() - 1);
   _idx = -1;
   _cidx = cidx;
}

IndigoComponentBondsIter::~IndigoComponentBondsIter ()
{
}

bool IndigoComponentBondsIter::hasNext ()
{
   return _next() != _mol.edgeEnd();
}

IndigoObject * IndigoComponentBondsIter::next ()
{
   int idx = _next();

   if (idx == _mol.edgeEnd())
      return 0;
   _idx = idx;
   return new IndigoBond(_mol, idx);
}

int IndigoComponentBondsIter::_next ()
{
   int idx;

   if (_idx == -1)
      idx = _mol.edgeBegin();
   else
      idx = _mol.edgeNext(_idx);

   for (; idx != _mol.edgeEnd(); idx = _mol.edgeNext(idx))
   {
      const Edge &edge = _mol.getEdge(idx);

      int comp = _mol.vertexComponent(edge.beg);

      if (comp != _mol.vertexComponent(edge.end))
         throw IndigoError("internal: edge ends belong to different components");

      if (comp == _cidx)
         break;
   }
   return idx;
}

CEXPORT int indigoIterateComponents (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoComponentsIter(bm));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateComponentAtoms (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoComponentAtomsIter(bm, index));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateComponentBonds (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoComponentBondsIter(bm, index));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountComponentAtoms (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      return bm.countComponentVertices(index);
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountComponentBonds (int molecule, int index)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(molecule).getBaseMolecule();

      return bm.countComponentEdges(index);
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCreateMolecule ()
{
   INDIGO_BEGIN
   {
      AutoPtr<IndigoMolecule> obj(new IndigoMolecule());
      return self.addObject(obj.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCreateQueryMolecule ()
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoQueryMolecule());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoMerge (int where, int what)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol_where = self.getObject(where).getBaseMolecule();
      BaseMolecule &mol_what = self.getObject(what).getBaseMolecule();

      AutoPtr<IndigoMapping> res(new IndigoMapping(mol_what, mol_where));

      mol_where.mergeWithMolecule(mol_what, &res->mapping, 0);

      return self.addObject(res.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoAddAtom (int molecule, const char *symbol)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);
      BaseMolecule &bmol = obj.getBaseMolecule();

      int idx;
      if (bmol.isQueryMolecule())
      {
         QueryMolecule &qmol = bmol.asQueryMolecule();
         idx = qmol.addAtom(IndigoQueryMolecule::parseAtomSMARTS(symbol));
      }
      else
      {
         Molecule &mol = bmol.asMolecule();
         int elem = Element::fromString2(symbol);

         if (elem > 0)
            idx = mol.addAtom(elem);
         else
         {
            idx = mol.addAtom(ELEM_PSEUDO);
            mol.setPseudoAtom(idx, symbol);
         }
      }

      return self.addObject(new IndigoAtom(bmol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoResetAtom (int atom, const char *symbol)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      BaseMolecule &bmol = ia.mol;

      if (bmol.isQueryMolecule())
      {
         QueryMolecule &qmol = bmol.asQueryMolecule();
         qmol.resetAtom(ia.idx, IndigoQueryMolecule::parseAtomSMARTS(symbol));
      }
      else
      {
         Molecule &mol = ia.mol.asMolecule();

         int elem = Element::fromString2(symbol);

         if (elem > 0)
            mol.resetAtom(ia.idx, elem);
         else
         {
            mol.resetAtom(ia.idx, ELEM_PSEUDO);
            mol.setPseudoAtom(ia.idx, symbol);
         }
      }
      bmol.invalidateAtom(ia.idx, BaseMolecule::CHANGED_ATOM_NUMBER);

      return 1;
   }
   INDIGO_END(-1)
}

static void _parseRSites (const char *name, Array<int> &rsites)
{
   BufferScanner scanner(name);
   rsites.clear();
   while (!scanner.isEOF())
   {
      scanner.skipSpace();
      if (scanner.lookNext() != 'R')
         throw IndigoError("indigoAddRSite(): cannot parse '%s' as r-site name(s)", name);
      scanner.readChar();
      if (scanner.isEOF())
         break;
      if (isdigit(scanner.lookNext()))
      {
         int idx = scanner.readInt();
         rsites.push(idx);
      }

      scanner.skipSpace();
      if (scanner.lookNext() == ',' || scanner.lookNext() == ';')
         scanner.readChar();
   }
}

static void _indigoSetRSite (Molecule &mol, int atom_index, const char *name)
{
   // Parse r-sites
   QS_DEF(Array<int>, rsites);
   _parseRSites(name, rsites);
   mol.resetAtom(atom_index, ELEM_RSITE);
   mol.setRSiteBits(atom_index, 0);
   for (int i = 0; i < rsites.size(); i++)
      mol.allowRGroupOnRSite(atom_index, rsites[i]);
}

CEXPORT int indigoAddRSite (int molecule, const char *name)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      int idx = mol.addAtom(ELEM_RSITE);
      try 
      {
         _indigoSetRSite(mol, idx, name);
      }
      catch (...)
      {
         // Remove atom if there is an exception (r-site index is very big for example)
         mol.removeAtom(idx);
         throw;
      }

      return self.addObject(new IndigoAtom(mol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetRSite (int atom, const char *name)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      Molecule &mol = ia.mol.asMolecule();

      _indigoSetRSite(mol, ia.idx, name);

      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetCharge (int atom, int charge)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      ia.mol.asMolecule().setAtomCharge(ia.idx, charge);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetIsotope (int atom, int isotope)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      ia.mol.asMolecule().setAtomIsotope(ia.idx, isotope);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetImplicitHCount (int atom, int impl_h)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      ia.mol.asMolecule().setImplicitH(ia.idx, impl_h);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoAddBond (int source, int destination, int order)
{
   INDIGO_BEGIN
   {
      IndigoAtom &s_atom = IndigoAtom::cast(self.getObject(source));
      IndigoAtom &d_atom = IndigoAtom::cast(self.getObject(destination));

      if (&s_atom.mol != &d_atom.mol)
         throw IndigoError("indigoAddBond(): molecules do not match");

      int idx;
      if (s_atom.mol.isQueryMolecule())
         idx = s_atom.mol.asQueryMolecule().addBond(s_atom.idx, d_atom.idx, 
            new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order));
      else
         idx = s_atom.mol.asMolecule().addBond(s_atom.idx, d_atom.idx, order);

      return self.addObject(new IndigoBond(s_atom.mol, idx));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetBondOrder (int bond, int order)
{
   INDIGO_BEGIN
   {
      IndigoBond &ib = IndigoBond::cast(self.getObject(bond));

      ib.mol.asMolecule().setBondOrder(ib.idx, order, false);
      return 1;
   }
   INDIGO_END(-1)
}

IndigoSubmolecule::IndigoSubmolecule (BaseMolecule &mol_, Array<int> &vertices_, Array<int> &edges_) :
IndigoObject(SUBMOLECULE),
mol(mol_)
{
   vertices.copy(vertices_);
   edges.copy(edges_);
   idx = -1;
}

IndigoSubmolecule::IndigoSubmolecule (BaseMolecule &mol_, List<int> &vertices_, List<int> &edges_) :
IndigoObject(SUBMOLECULE),
mol(mol_)
{
   int i;
   
   vertices.clear();
   edges.clear();
   
   for (i = vertices_.begin(); i != vertices_.end(); i = vertices_.next(i))
      vertices.push(vertices_[i]);

   for (i = edges_.begin(); i != edges_.end(); i = edges_.next(i))
      edges.push(edges_[i]);
   
   idx = -1;
}

IndigoSubmolecule::~IndigoSubmolecule ()
{
}

BaseMolecule & IndigoSubmolecule::getBaseMolecule ()
{
   return mol;
}

int IndigoSubmolecule::getIndex ()
{
   if (idx == -1)
      throw IndigoError("index not set");

   return idx;
}

IndigoObject * IndigoSubmolecule::clone ()
{
   AutoPtr<IndigoObject> res;
   BaseMolecule *newmol;

   if (mol.isQueryMolecule())
   {
      res.reset(new IndigoQueryMolecule());
      newmol = &(((IndigoQueryMolecule *)res.get())->qmol);
   }
   else
   {
      res.reset(new IndigoMolecule());
      newmol = &(((IndigoMolecule *)res.get())->mol);
   }

   newmol->makeEdgeSubmolecule(mol, vertices, edges, 0, 0);
   return res.release();
}

IndigoSubmoleculeAtomsIter::IndigoSubmoleculeAtomsIter (IndigoSubmolecule &submol) :
IndigoObject(SUBMOLECULE_ATOMS_ITER),
_submol(submol)
{
   _idx = -1;
}

IndigoSubmoleculeAtomsIter::~IndigoSubmoleculeAtomsIter ()
{
}

bool IndigoSubmoleculeAtomsIter::hasNext ()
{
   return _idx + 1 < _submol.vertices.size();
}

IndigoObject * IndigoSubmoleculeAtomsIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;

   return new IndigoAtom(_submol.mol, _submol.vertices[_idx]);
}

IndigoSubmoleculeBondsIter::IndigoSubmoleculeBondsIter (IndigoSubmolecule &submol) :
IndigoObject(SUBMOLECULE_BONDS_ITER),
_submol(submol)
{
   _idx = -1;
}

IndigoSubmoleculeBondsIter::~IndigoSubmoleculeBondsIter ()
{
}

bool IndigoSubmoleculeBondsIter::hasNext ()
{
   return _idx + 1 < _submol.edges.size();
}

IndigoObject * IndigoSubmoleculeBondsIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;

   return new IndigoBond(_submol.mol, _submol.edges[_idx]);
}

CEXPORT int indigoCountSSSR (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return mol.sssrCount();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateSSSR (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoSSSRIter(mol));
   }
   INDIGO_END(-1)
}

IndigoSSSRIter::IndigoSSSRIter (BaseMolecule &mol) :
IndigoObject(SSSR_ITER),
_mol(mol)
{
   _idx = -1;
}

IndigoSSSRIter::~IndigoSSSRIter ()
{
}

bool IndigoSSSRIter::hasNext ()
{
   return _idx + 1 < _mol.sssrCount();
}

IndigoObject * IndigoSSSRIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;
   List<int> &vertices = _mol.sssrVertices(_idx);
   List<int> &edges = _mol.sssrEdges(_idx);

   AutoPtr<IndigoSubmolecule> res(new IndigoSubmolecule(_mol, vertices, edges));
   res->idx = _idx;
   return res.release();
}

IndigoSubtreesIter::IndigoSubtreesIter (BaseMolecule &mol, int min_vertices, int max_vertices) :
IndigoObject(SUBTREES_ITER),
_mol(mol),
_enumerator(mol)
{
   _enumerator.min_vertices = min_vertices;
   _enumerator.max_vertices = max_vertices;
   _enumerator.context = this;
   _enumerator.callback = _handleTree;
   _enumerator.process();
   _idx = -1;
}

IndigoSubtreesIter::~IndigoSubtreesIter ()
{
}

void IndigoSubtreesIter::_handleTree (Graph &graph, const int *v_mapping, const int *e_mapping, void *context)
{
   IndigoSubtreesIter *self = (IndigoSubtreesIter *)context;

   Array<int> &vertices = self->_vertices.push();
   Array<int> &edges = self->_edges.push();

   Graph::filterVertices(graph, v_mapping, FILTER_NEQ, -1, vertices);
   Graph::filterEdges(graph, e_mapping, FILTER_NEQ, -1, edges);
}

bool IndigoSubtreesIter::hasNext ()
{
   return _idx + 1 < _vertices.size();
}

IndigoObject * IndigoSubtreesIter::next ()
{
   if (!hasNext())
      return 0;
   
   _idx++;
   AutoPtr<IndigoSubmolecule> res(new IndigoSubmolecule(_mol, _vertices[_idx], _edges[_idx]));
   res->idx = _idx;
   return res.release();
}

CEXPORT int indigoIterateSubtrees (int molecule, int min_atoms, int max_atoms)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoSubtreesIter(mol, min_atoms, max_atoms));
   }
   INDIGO_END(-1)
}

IndigoRingsIter::IndigoRingsIter (BaseMolecule &mol, int min_vertices, int max_vertices) :
IndigoObject(RINGS_ITER),
_mol(mol),
_enumerator(mol)
{
   _enumerator.min_length = min_vertices;
   _enumerator.max_length = max_vertices;
   _enumerator.context = this;
   _enumerator.cb_handle_cycle = _handleCycle;
   _enumerator.process();
   _idx = -1;
}

IndigoRingsIter::~IndigoRingsIter ()
{
}

bool IndigoRingsIter::_handleCycle (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context)
{
   IndigoRingsIter *self = (IndigoRingsIter *)context;

   self->_vertices.push().copy(vertices);
   self->_edges.push().copy(edges);
   return true;
}

bool IndigoRingsIter::hasNext ()
{
   return _idx + 1 < _vertices.size();
}

IndigoObject * IndigoRingsIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;
   AutoPtr<IndigoSubmolecule> res(new IndigoSubmolecule(_mol, _vertices[_idx], _edges[_idx]));
   res->idx = _idx;
   return res.release();
}

CEXPORT int indigoIterateRings (int molecule, int min_atoms, int max_atoms)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoRingsIter(mol, min_atoms, max_atoms));
   }
   INDIGO_END(-1)
}

IndigoEdgeSubmoleculeIter::IndigoEdgeSubmoleculeIter (BaseMolecule &mol, int min_edges, int max_edges) :
IndigoObject(EDGE_SUBMOLECULE_ITER),
_mol(mol),
_enumerator(mol)
{
   _enumerator.min_edges = min_edges;
   _enumerator.max_edges = max_edges;
   _enumerator.userdata = this;
   _enumerator.cb_subgraph = _handleSubgraph;
   _enumerator.process();
   _idx = -1;
}

IndigoEdgeSubmoleculeIter::~IndigoEdgeSubmoleculeIter ()
{
}

void IndigoEdgeSubmoleculeIter::_handleSubgraph (Graph &graph, const int *v_mapping, const int *e_mapping, void *context)
{
   IndigoEdgeSubmoleculeIter *self = (IndigoEdgeSubmoleculeIter *)context;

   Array<int> &vertices = self->_vertices.push();
   Array<int> &edges = self->_edges.push();

   Graph::filterVertices(graph, v_mapping, FILTER_NEQ, -1, vertices);
   Graph::filterEdges(graph, e_mapping, FILTER_NEQ, -1, edges);
}

bool IndigoEdgeSubmoleculeIter::hasNext ()
{
   return _idx + 1 < _vertices.size();
}

IndigoObject * IndigoEdgeSubmoleculeIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;
   AutoPtr<IndigoSubmolecule> res(new IndigoSubmolecule(_mol, _vertices[_idx], _edges[_idx]));
   res->idx = _idx;
   return res.release();
}

CEXPORT int indigoIterateEdgeSubmolecules (int molecule, int min_bonds, int max_bonds)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoEdgeSubmoleculeIter(mol, min_bonds, max_bonds));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountHydrogens (int item, int *hydro)
{
   INDIGO_BEGIN
   {
      if (hydro == 0)
         throw IndigoError("indigoCountHydrogens(): null pointer");

      IndigoObject &obj = self.getObject(item);

      if (IndigoAtom::is(obj))
      {
         IndigoAtom &ia = IndigoAtom::cast(obj);

         int res = ia.mol.getAtomTotalH(ia.idx);

         if (res == -1)
            return 0;

         *hydro = res;
      }
      else if (IndigoBaseMolecule::is(obj))
      {
         Molecule &mol = obj.getMolecule();
         *hydro = 0;

         for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
         {
            if (mol.getAtomNumber(i) == ELEM_H)
               (*hydro)++;
            else if (!mol.isPseudoAtom(i) && !mol.isRSite(i))
               (*hydro) += mol.getImplicitH(i);
         }
      }
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountImplicitHydrogens (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      if (IndigoAtom::is(obj))
      {
         IndigoAtom &ia = IndigoAtom::cast(obj);

         return ia.mol.asMolecule().getImplicitH(ia.idx);
      }
      else if (IndigoBaseMolecule::is(obj))
      {
         Molecule &mol = obj.getMolecule();
         int i, sum = 0;

         for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            sum += mol.getImplicitH(i);
         return sum;
      }
      else
         throw IndigoError("indigoCountImplicitHydrogens: %s is not a molecule nor an atom",
                 obj.debugInfo());
   }
   INDIGO_END(-1)
}

//
// IndigoAttachmentPointsIter
//

IndigoAttachmentPointsIter::IndigoAttachmentPointsIter (BaseMolecule &mol, int order) : 
   IndigoObject(ATTACHMENT_POINTS_ITER), _mol(mol)
{
   _order = order;
   _index = -1;
}

IndigoObject * IndigoAttachmentPointsIter::next ()
{
   if (!hasNext())
      return 0;
   _index++;
   int atom_index = _mol.getAttachmentPoint(_order, _index);
   if (atom_index == -1)
      throw IndigoError("Internal error in IndigoAttachmentPointsIter::next");
   return new IndigoAtom(_mol, atom_index);
}

bool IndigoAttachmentPointsIter::hasNext ()
{
   return _mol.getAttachmentPoint(_order, _index + 1) != -1;
}

CEXPORT int indigoIterateAttachmentPoints (int molecule, int order)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return self.addObject(new IndigoAttachmentPointsIter(mol, order));
   }
   INDIGO_END(-1)
}

