#include <base_cpp/output.h>
#include <molecule/lipinski.h>
#include <molecule/molecule_gross_formula.h>
#include <molecule/molecule_mass.h>
#include <molecule/tpsa.h>
#include <reaction/reaction_gross_formula.h>

#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "molecule/crippen.h"

IndigoMoleculeGross::IndigoMoleculeGross() : IndigoObject(GROSS_MOLECULE)
{
}

IndigoMoleculeGross::~IndigoMoleculeGross()
{
}

void IndigoMoleculeGross::toString(Array<char>& str)
{
    Indigo& self = indigoGetInstance();
    MoleculeGrossFormula::toString_Hill(*gross, str, self.gross_formula_options.add_rsites);
}

IndigoReactionGross::IndigoReactionGross() : IndigoObject(GROSS_REACTION)
{
}

IndigoReactionGross::~IndigoReactionGross()
{
}

void IndigoReactionGross::toString(Array<char>& str)
{
    Indigo& self = indigoGetInstance();
    ReactionGrossFormula::toString_Hill(*gross, str, self.gross_formula_options.add_rsites);
}

CEXPORT int indigoGrossFormula(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& indigoObject = self.getObject(object);
        if (IndigoBaseMolecule::is(indigoObject))
        {
            BaseMolecule& mol = self.getObject(object).getBaseMolecule();
            std::unique_ptr<IndigoMoleculeGross> grossptr = std::make_unique<IndigoMoleculeGross>();
            grossptr->gross = MoleculeGrossFormula::collect(mol, self.gross_formula_options.add_isotopes);
            return self.addObject(grossptr.release());
        }
        else if (IndigoBaseReaction::is(indigoObject))
        {
            BaseReaction& rxn = self.getObject(object).getBaseReaction();
            std::unique_ptr<IndigoReactionGross> grossptr = std::make_unique<IndigoReactionGross>();
            grossptr->gross = ReactionGrossFormula::collect(rxn, self.gross_formula_options.add_isotopes);
            return self.addObject(grossptr.release());
        }
        else
        {
            throw IndigoError("incorrect object type for gross formula: %s", indigoObject.debugInfo());
        }
    }
    INDIGO_END(-1);
}

static BaseMolecule& _indigoPrepareMass(IndigoObject& obj, MoleculeMass mass)
{
    if (IndigoBaseMolecule::is(obj))
    {
        auto& mol = obj.getBaseMolecule();
        if (mol.isQueryMolecule())
        {
            throw IndigoError("can not calculate mass for query molecule");
        }
        return mol;
    }
    else
    {
        throw IndigoError("incorrect object type for mass: %s", obj.debugInfo());
    }
}

CEXPORT double indigoMolecularWeight(int molecule)
{
    INDIGO_BEGIN
    {
        MoleculeMass mass;
        auto& mol = _indigoPrepareMass(self.getObject(molecule), mass);
        mass.mass_options = self.mass_options;
        return mass.molecularWeight(mol.asMolecule());
    }
    INDIGO_END(-1);
}

CEXPORT double indigoMostAbundantMass(int molecule)
{
    INDIGO_BEGIN
    {
        MoleculeMass mass;
        auto& mol = _indigoPrepareMass(self.getObject(molecule), mass);
        mass.mass_options = self.mass_options;
        return mass.mostAbundantMass(mol.asMolecule());
    }
    INDIGO_END(-1);
}

CEXPORT double indigoMonoisotopicMass(int molecule)
{
    INDIGO_BEGIN
    {
        MoleculeMass mass;
        auto& mol = _indigoPrepareMass(self.getObject(molecule), mass);
        mass.mass_options = self.mass_options;
        return mass.monoisotopicMass(mol.asMolecule());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoMassComposition(int molecule)
{
    INDIGO_BEGIN
    {
        MoleculeMass mass;
        auto& mol = _indigoPrepareMass(self.getObject(molecule), mass);
        mass.mass_options = self.mass_options;

        auto& tmp = self.getThreadTmpData();
        mass.massComposition(mol.asMolecule(), tmp.string);

        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT double indigoTPSA(const int molecule, const int includeSP)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return TPSA::calculate(mol, static_cast<bool>(includeSP));
        }
        throw IndigoError("incorrect object type for TPSA calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoNumRotatableBonds(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return Lipinski::getNumRotatableBonds(mol);
        }
        throw IndigoError("incorrect object type for calculation number of rotatable bonds: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoNumHydrogenBondAcceptors(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return Lipinski::getNumHydrogenBondAcceptors(mol);
        }
        throw IndigoError("incorrect object type for calculation number of hydrogen bond acceptors: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoNumHydrogenBondDonors(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return Lipinski::getNumHydrogenBondDonors(mol);
        }
        throw IndigoError("incorrect object type for calculation number of hydrogen bond donors: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT double indigoLogP(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return Crippen::logP(mol);
        }
        throw IndigoError("incorrect object type for logP calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1000.0);
}

CEXPORT double indigoMolarRefractivity(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return Crippen::molarRefractivity(mol);
        }
        throw IndigoError("incorrect object type for MR calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1000.0);
}

CEXPORT double indigoPka(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            return Crippen::pKa(mol);
        }
        throw IndigoError("incorrect object type for logP calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1000.0);
}
