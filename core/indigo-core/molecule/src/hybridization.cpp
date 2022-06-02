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

#include "molecule/hybridization.h"

#include "molecule/elements.h"
#include "molecule/molecule.h"

namespace
{
    using namespace indigo;

    int getTotalBondsOrder(Molecule& molecule, int atomIndex)
    {
        const auto& atom = molecule.getVertex(atomIndex);
        int order = 0;
        for (auto neiIndex = atom.neiBegin(); neiIndex != atom.neiEnd(); neiIndex = atom.neiNext(neiIndex))
        {
            order += molecule.getBondOrder(atom.neiEdge(neiIndex));
        }
        order += molecule.getImplicitH(atomIndex);
        return order;
    }

    Hybridization getCarbonHybridization(Molecule& molecule, int atomIndex)
    {
        const auto numNeighbors = molecule.getVertex(atomIndex).degree() + molecule.getImplicitH(atomIndex);
        switch (numNeighbors)
        {
        case 4:
            return Hybridization::SP3;
        case 3:
            return Hybridization::SP2;
        case 2:
        case 1:
            return Hybridization::SP;
        default:
            throw HybridizationCalculator::Error("Couldn't calculate C hybridization properly");
        }
    }

    bool matchMinusInduction(Molecule& molecule, int atomIndex)
    {
        const auto& atom = molecule.getVertex(atomIndex);
        for (auto neiIndex = atom.neiBegin(); neiIndex != atom.neiEnd(); neiIndex = atom.neiNext(neiIndex))
        {
            const auto bondOrder = molecule.getBondOrder(atom.neiEdge(neiIndex));
            if (bondOrder == BOND_DOUBLE || bondOrder == BOND_AROMATIC)
            {
                return true;
            }
        }
        return false;
    }

    bool hasLonePair(Molecule& molecule, int atomIndex, int totalBondsOrder)
    {
        return Element::getNumOuterElectrons(molecule.getAtomNumber(atomIndex)) - totalBondsOrder >= 2;
    }

    int lonePairs(Molecule& molecule, int atomIndex, int totalBondsOrder)
    {
        const auto numOuterElectrons = Element::getNumOuterElectrons(molecule.getAtomNumber(atomIndex));
        return (numOuterElectrons - totalBondsOrder) / 2;
    }

    Hybridization getNitrogenHybridization(Molecule& molecule, int atomIndex, int totalBondsOrder)
    {
        const auto nOrbs = molecule.getVertex(atomIndex).degree() + static_cast<int>(hasLonePair(molecule, atomIndex, totalBondsOrder));
        bool minusInduction = false;
        const auto& atom = molecule.getVertex(atomIndex);
        for (auto neiIndex = atom.neiBegin(); neiIndex != atom.neiEnd(); neiIndex = atom.neiNext(neiIndex))
        {
            const auto neiVertexIndex = atom.neiVertex(neiIndex);
            if (molecule.getAtomNumber(neiVertexIndex) == ELEM_C && matchMinusInduction(molecule, neiVertexIndex))
            {
                minusInduction = true;
                break;
            }
        }
        if (nOrbs == 4 && minusInduction)
        {
            return Hybridization(nOrbs - 1);
        }
        if (nOrbs <= 4)
        {
            return Hybridization(nOrbs);
        }
        throw HybridizationCalculator::Error("Couldn't calculate N hybridization properly");
    }

    Hybridization getOxygenHybridization(Molecule& molecule, int atomIndex)
    {
        const auto& atom = molecule.getVertex(atomIndex);
        for (auto neiIndex = atom.neiBegin(); neiIndex < atom.neiEnd(); neiIndex = atom.neiNext(neiIndex))
        {
            const auto bondOrder = molecule.getBondOrder(atom.neiEdge(neiIndex));
            const auto neiVertexIndex = atom.neiVertex(neiIndex);

            if (bondOrder == BOND_DOUBLE)
            {
                return Hybridization::SP2;
            }
            if (bondOrder == BOND_TRIPLE)
            {
                // for CO, but some sources do not recognize "sp" for oxygen
                return Hybridization::SP;
            }

            if (molecule.getAtomNumber(neiVertexIndex) == ELEM_C && matchMinusInduction(molecule, neiVertexIndex))
            {
                return Hybridization::SP2;
            }
        }
        return Hybridization::SP3;
    }

    Hybridization getComplexHybridization(Molecule& molecule, int atomIndex)
    {
        const auto& numNeighbors = molecule.getVertex(atomIndex).degree();
        const auto atomNumber = molecule.getAtomNumber(atomIndex);
        switch (numNeighbors)
        {
        case 4:
            if (atomNumber == ELEM_Pt || atomNumber == ELEM_Ni || atomNumber == ELEM_Cu || atomNumber == ELEM_Au || atomNumber == ELEM_Pd)
            {
                return Hybridization::SP2D;
            }
            return Hybridization::SP3;
        case 5:
            return Hybridization::SP3D;
        case 6:
            return Hybridization::SP3D2;
        case 7:
            return Hybridization::SP3D3;
        case 8:
            return Hybridization::SP3D4;
        default:
            throw HybridizationCalculator::Error("Could not calculate %s hybridization properly", Element::toString(atomNumber));
        }
    }

    bool isAtomInAromaticRing(Molecule& rawMolecule, const int atomIndex)
    {
        Molecule molecule;
        molecule.clone(rawMolecule);
        if (!molecule.isAromatized())
        {
            molecule.aromatize({});
        }
        return molecule.getAtomAromaticity(atomIndex) == ATOM_AROMATIC;
        //        const auto& atom = molecule.getVertex(atomIndex);
        //        int order = 0;
        //        for (auto neiIndex = atom.neiBegin(); neiIndex != atom.neiEnd(); neiIndex = atom.neiNext(neiIndex))
        //        {
        //            if (molecule.getBondTopology(atom.neiEdge(neiIndex)) == TOPOLOGY_RING
        //        }
        return false;
    }
}

namespace indigo
{
    IMPL_ERROR(HybridizationCalculator, "HybridizationCalculator");

    Hybridization HybridizationCalculator::calculate(Molecule& molecule, int atomIndex)
    {
        const auto atomNumber = molecule.getAtomNumber(atomIndex);
        if (atomNumber == 0)
        {
            throw Error("Atomic number is undefined or ambiguous");
        }
        if (atomNumber >= 57)
        {
            throw Error("Hybridization calculation is not implemented for atomic numbers >= 57");
        }
        if (atomNumber == ELEM_H)
        {
            return Hybridization::S;
        }

        auto totalBondsOrder = getTotalBondsOrder(molecule, atomIndex);

        if (totalBondsOrder == 0)
        {
            return Hybridization::S;
        }

        if ((atomNumber == ELEM_C || atomNumber == ELEM_N || atomNumber == ELEM_O || atomNumber == ELEM_P || atomNumber == ELEM_S || atomNumber == ELEM_B) &&
            isAtomInAromaticRing(molecule, atomIndex))
        {
            return Hybridization::SP2;
        }

        if (atomNumber == ELEM_C)
        {
            return getCarbonHybridization(molecule, atomIndex);
        }
        if (atomNumber == ELEM_N)
        {
            return getNitrogenHybridization(molecule, atomIndex, totalBondsOrder);
        }
        if (atomNumber == ELEM_O)
        {
            return getOxygenHybridization(molecule, atomIndex);
        }

        if (molecule.getVertex(atomIndex).degree() >= 4)
        {
            return getComplexHybridization(molecule, atomIndex);
        }

        const auto nOrbs = molecule.getVertex(atomIndex).degree() + lonePairs(molecule, atomIndex, totalBondsOrder);
        return Hybridization(nOrbs);
    }
}
