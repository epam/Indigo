#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "base_cpp/output.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_mass.h"
#include "reaction/reaction_gross_formula.h"


IndigoMoleculeGross::IndigoMoleculeGross() : IndigoObject(GROSS_MOLECULE)
{
}

IndigoMoleculeGross::~IndigoMoleculeGross()
{
}

void IndigoMoleculeGross::toString (Array<char> &str)
{
   MoleculeGrossFormula::toString_Hill(gross, str);
}

IndigoReactionGross::IndigoReactionGross() : IndigoObject(GROSS_REACTION)
{
}

IndigoReactionGross::~IndigoReactionGross()
{
}

void IndigoReactionGross::toString (Array<char> &str)
{
    ReactionGrossFormula::toString_Hill(gross, str);
}


CEXPORT int indigoGrossFormula (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject & indigoObject = self.getObject(object);
      if (IndigoBaseMolecule::is(indigoObject))
      {
          BaseMolecule &mol = self.getObject(object).getBaseMolecule();
          AutoPtr<IndigoMoleculeGross> grossptr(new IndigoMoleculeGross());

          MoleculeGrossFormula::collect(mol, grossptr->gross);
          return self.addObject(grossptr.release());
      }
      else if (IndigoBaseReaction::is(indigoObject))
      {
          BaseReaction &rxn = self.getObject(object).getBaseReaction();
          AutoPtr<IndigoReactionGross> grossptr(new IndigoReactionGross());
          
          ReactionGrossFormula::collect(rxn, grossptr->gross);
          return self.addObject(grossptr.release());
      }
      else
      {
          throw IndigoError("incorrect object type for gross formula: %s", indigoObject.type);
      }
   }
   INDIGO_END(-1)
}

CEXPORT double indigoMolecularWeight (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      MoleculeMass mass;
      return mass.molecularWeight(mol);
   }
   INDIGO_END(-1)
}

CEXPORT double indigoMostAbundantMass (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      MoleculeMass mass;
      return mass.mostAbundantMass(mol);
   }
   INDIGO_END(-1)
}

CEXPORT double indigoMonoisotopicMass (int molecule)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      MoleculeMass mass;
      return mass.monoisotopicMass(mol);
   }
   INDIGO_END(-1)
}

CEXPORT const char * indigoMassComposition (int molecule)
{
    INDIGO_BEGIN
    {
        Molecule &mol = self.getObject(molecule).getMolecule();
        auto &tmp = self.getThreadTmpData();
        MoleculeMass mass;
        mass.massComposition(mol, tmp.string);
        return tmp.string.ptr();
    }
    INDIGO_END(0)
}
