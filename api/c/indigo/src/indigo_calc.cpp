/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <base_cpp/output.h>
#include <molecule/lipinski.h>
#include <molecule/molecule_gross_formula.h>
#include <molecule/molecule_mass.h>
#include <molecule/tpsa.h>
#include <reaction/reaction_gross_formula.h>

#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "molecule/crippen.h"

IndigoMoleculeGross::IndigoMoleculeGross() : IndigoObject(GROSS_MOLECULE), iupacFormula(false)
{
}

IndigoMoleculeGross::~IndigoMoleculeGross()
{
}

void IndigoMoleculeGross::toString(Array<char>& str)
{
    Indigo& self = indigoGetInstance();
    MoleculeGrossFormula::toString_Hill(*gross, str, self.gross_formula_options.add_rsites, iupacFormula);
}

IndigoReactionGross::IndigoReactionGross() : IndigoObject(GROSS_REACTION), iupacFormula(false)
{
}

IndigoReactionGross::~IndigoReactionGross()
{
}

void IndigoReactionGross::toString(Array<char>& str)
{
    Indigo& self = indigoGetInstance();
    ReactionGrossFormula::toString_Hill(*gross, str, self.gross_formula_options.add_rsites, iupacFormula);
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
            auto& rxn = self.getObject(object).getBaseReaction();
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

CEXPORT int indigoMolecularFormula(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& indigoObject = self.getObject(object);
        if (IndigoBaseMolecule::is(indigoObject))
        {
            BaseMolecule& mol = self.getObject(object).getBaseMolecule();
            std::unique_ptr<IndigoMoleculeGross> grossptr = std::make_unique<IndigoMoleculeGross>();
            grossptr->gross = MoleculeGrossFormula::collect(mol, self.gross_formula_options.add_isotopes);
            grossptr->iupacFormula = true;
            return self.addObject(grossptr.release());
        }
        else if (IndigoBaseReaction::is(indigoObject))
        {
            auto& rxn = self.getObject(object).getBaseReaction();
            std::unique_ptr<IndigoReactionGross> grossptr = std::make_unique<IndigoReactionGross>();
            grossptr->gross = ReactionGrossFormula::collect(rxn, self.gross_formula_options.add_isotopes);
            grossptr->iupacFormula = true;
            return self.addObject(grossptr.release());
        }
        else
        {
            throw IndigoError("incorrect object type for molecular formula: %s", indigoObject.debugInfo());
        }
    }
    INDIGO_END(-1);
}

static BaseMolecule& _indigoPrepareMass(IndigoObject& obj, MoleculeMass mass)
{
    if (IndigoBaseMolecule::is(obj))
    {
        auto& mol = obj.getBaseMolecule();
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
        return mass.molecularWeight(mol);
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
        return mass.mostAbundantMass(mol);
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
        return mass.monoisotopicMass(mol);
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
        mass.massComposition(mol, tmp.string);

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
    INDIGO_END(-1);
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
        throw IndigoError("incorrect object type for logP calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoPkaValues(const int molecule)
{
    INDIGO_BEGIN
    {
        auto& obj = self.getObject(molecule);
        if (IndigoMolecule::is(obj))
        {
            auto& mol = obj.getMolecule();
            Array<double> pka_values;
            Crippen::getPKaValues(mol, pka_values);
            auto& tmp = self.getThreadTmpData();
            tmp.clear();
            for (auto& pka_val : pka_values)
            {
                if (tmp.string.size())
                    tmp.string.appendString(",", true);
                tmp.string.appendString(std::to_string(pka_val).c_str(), true);
            }
            return tmp.string.ptr();
        }
        else
            throw IndigoError("incorrect object type for pKa calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(0);
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
        throw IndigoError("incorrect object type for pKa calculation: %s, should be molecule", obj.debugInfo());
    }
    INDIGO_END(-1);
}
