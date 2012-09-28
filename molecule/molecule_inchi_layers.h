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

#ifndef __molecule_inchi_layers_h__
#define __molecule_inchi_layers_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "molecule/molecule_inchi_utils.h"

namespace indigo {

class Molecule;
class Output;
class MoleculeStereocenters;

// Namespace with layers
// Each layers are independent and contains all 
// nessesary information 
namespace MoleculeInChILayers
{

// Abtract layer
class AbstractLayer
{
public:
   AbstractLayer ();
   virtual ~AbstractLayer () {};

   // Method for constructing internal layer information
   void construct (Molecule &mol);

   DECL_ERROR;
protected:
   Molecule& _getMolecule ();

   virtual void _construct () {};

private:
   Molecule *_mol;
};

// Main layer formula
class MainLayerFormula : public AbstractLayer
{
public:
   void printFormula (Array<char> &result);

   static int compareComponentsAtomsCountNoH       (MainLayerFormula &comp1, 
                                                    MainLayerFormula &comp2);
   static int compareComponentsTotalHydrogensCount (MainLayerFormula &comp1, 
                                                    MainLayerFormula &comp2);
protected:
   virtual void _construct ();

private:
   Array<int> _atoms_count;

   void _printAtom (Output &output, int label) const;
   void _collectAtomsCount ();
};

// Main layer connections
class MainLayerConnections : public AbstractLayer
{
public:
   void printConnectionTable (Array<char> &result);

   int  compareMappings (const MoleculeInChIUtils::Mapping &m1, 
                         const MoleculeInChIUtils::Mapping &m2);

   static int compareComponentsConnectionTables (MainLayerConnections &comp1,
                                                 MainLayerConnections &comp2);

protected:
   virtual void _construct ();

private:
   Array<int> _connection_table;

   void _linearizeConnectionTable ();
};

// Layer with hydrogens
class HydrogensLayer : public AbstractLayer
{
public:
   static int compareComponentsHydrogens (HydrogensLayer &comp1, 
                                          HydrogensLayer &comp2);

   bool checkAutomorphism (const Array<int> &mapping);
   int  compareMappings (MoleculeInChIUtils::Mapping &m1, 
                         MoleculeInChIUtils::Mapping &m2);

   void print (Array<char> &result);

protected:
   virtual void _construct ();

private:
   // Number of immobile hydrogens for each atom
   Array<int> _per_atom_immobile;
   // Atom indices in the 'mol' to avoid vertexBegin/vertexEnd iterations
   // when comparing components
   Array<int> _atom_indices; 

   // TODO: Mobile hydrogens, fixed hydrogens
};

// Cis-trans stereochemistry
class CisTransStereochemistryLayer : public AbstractLayer
{
public:
   void print (Array<char> &result);

   bool checkAutomorphism (const Array<int> &mapping);
   int  compareMappings (const MoleculeInChIUtils::Mapping &m1, 
                         const MoleculeInChIUtils::Mapping &m2);

   static int compareComponents (CisTransStereochemistryLayer &comp1, 
                                 CisTransStereochemistryLayer &comp2);
protected:
   virtual void _construct ();

private:
   Array<int> bond_is_cis_trans;
};

// Tetrahedral stereochemistry
class TetrahedralStereochemistryLayer : public AbstractLayer
{
public:
   void print (Array<char> &result);

   void printEnantiomers (Array<char> &result);

   bool checkAutomorphism (const Array<int> &mapping);
   int  compareMappings (const MoleculeInChIUtils::Mapping &m1, 
                         const MoleculeInChIUtils::Mapping &m2);

   static int compareComponentsEnantiomers (TetrahedralStereochemistryLayer &comp1, 
                                            TetrahedralStereochemistryLayer &comp2);

   static int compareComponents (TetrahedralStereochemistryLayer &comp1, 
                                 TetrahedralStereochemistryLayer &comp2);
private:
   int _getMappingSign (const MoleculeStereocenters &stereocenters, 
      const MoleculeInChIUtils::Mapping *m, int index);

   int _getFirstSign ();
};

};

}

#endif // __molecule_inchi_layers_h__

