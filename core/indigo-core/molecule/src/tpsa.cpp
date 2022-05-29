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

#include "molecule/tpsa.h"

#include "molecule/elements.h"
#include "molecule/molecule.h"

using namespace std;

namespace indigo
{
    bool TPSA::Key::operator<(const TPSA::Key& other) const noexcept
    {
        return tie(atomNumber, nH, charge, maxBondOrder, bondOrderSum, nNeighbors, nSingleBonds, nDoubleBonds, nTripleBonds, nAromaticBonds,
                   isIn3MemberedRing) < tie(other.atomNumber, other.nH, other.charge, other.maxBondOrder, other.bondOrderSum, other.nNeighbors,
                                            other.nSingleBonds, other.nDoubleBonds, other.nTripleBonds, other.nAromaticBonds, other.isIn3MemberedRing);
    }

    double TPSA::calculate(Molecule& rawMolecule, const bool includeSP)
    {
        Molecule molecule;
        molecule.clone(rawMolecule);
        if (!molecule.isAromatized())
        {
            molecule.aromatize(AromaticityOptions());
        }

        double tpsa = 0.0;
        const auto& atomContributions = includeSP ? getAtomContributionsNOSP() : getAtomContributionsNO();
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            TPSA::Key key{};
            key.atomNumber = molecule.getAtomNumber(i);
            if (key.atomNumber != ELEM_N && key.atomNumber != ELEM_O && key.atomNumber != ELEM_S && key.atomNumber != ELEM_P)
            {
                continue;
            }
            key.nH = molecule.getAtomTotalH(i);
            key.charge = molecule.getAtomCharge(i);

            key.isIn3MemberedRing = molecule.vertexSmallestRingSize(i) == 3;

            const Vertex& vertex = molecule.getVertex(i);
            for (auto ni = vertex.neiBegin(); ni != vertex.neiEnd(); ni = vertex.neiNext(ni))
            {
                ++key.nNeighbors;

                const auto bondOrder = molecule.getBondOrder(vertex.neiEdge(ni));
                switch (bondOrder)
                {
                case BOND_SINGLE:
                    ++key.nSingleBonds;
                    key.bondOrderSum += 10;
                    break;
                case BOND_AROMATIC:
                    ++key.nAromaticBonds;
                    key.bondOrderSum += 15;
                    break;
                case BOND_DOUBLE:
                    ++key.nDoubleBonds;
                    key.bondOrderSum += 20;
                    break;
                case BOND_TRIPLE:
                    ++key.nTripleBonds;
                    key.bondOrderSum += 30;
                    break;
                default:
                    break;
                }
            }

            const auto nImplicitH = molecule.getImplicitH(i);
            key.nSingleBonds += nImplicitH;
            key.nNeighbors += nImplicitH;
            key.bondOrderSum += nImplicitH * 10;

            if (key.nTripleBonds)
            {
                key.maxBondOrder = 30;
            }
            else if (key.nDoubleBonds)
            {
                key.maxBondOrder = 20;
            }
            else if (key.nAromaticBonds)
            {
                key.maxBondOrder = 15;
            }
            else if (key.nSingleBonds)
            {
                key.maxBondOrder = 10;
            }

            if (atomContributions.count(key) > 0)
            {
                const auto contrib = atomContributions.at(key);
                tpsa += contrib;
            }
        }

        return tpsa;
    }

    const map<TPSA::Key, double>& TPSA::getAtomContributionsNO()
    {
        static map<TPSA::Key, double> atomContributions{
            {{ELEM_N, 0, 0, 10, 30, 3, 3, 0, 0, 0, false}, 3.24},  {{ELEM_N, 0, 0, 20, 30, 2, 1, 1, 0, 0, false}, 12.36},
            {{ELEM_N, 0, 0, 30, 30, 1, 0, 0, 1, 0, false}, 23.79}, {{ELEM_N, 0, 0, 20, 50, 3, 1, 2, 0, 0, false}, 11.68},
            {{ELEM_N, 0, 0, 30, 50, 2, 0, 1, 1, 0, false}, 13.6},  {{ELEM_N, 0, 0, 10, 30, 3, 3, 0, 0, 0, true}, 3.01},
            {{ELEM_N, 1, 0, 10, 30, 3, 3, 0, 0, 0, false}, 12.03}, {{ELEM_N, 1, 0, 10, 30, 3, 3, 0, 0, 0, true}, 21.94},
            {{ELEM_N, 1, 0, 20, 30, 2, 1, 1, 0, 0, false}, 23.85}, {{ELEM_N, 2, 0, 10, 30, 3, 3, 0, 0, 0, false}, 26.02},
            {{ELEM_N, 0, 1, 10, 40, 4, 4, 0, 0, 0, false}, 0.0},   {{ELEM_N, 0, 1, 20, 40, 3, 2, 1, 0, 0, false}, 3.01},
            {{ELEM_N, 0, 1, 30, 40, 2, 1, 0, 1, 0, false}, 4.36},  {{ELEM_N, 1, 1, 10, 40, 4, 4, 0, 0, 0, false}, 4.44},
            {{ELEM_N, 1, 1, 20, 40, 3, 2, 1, 0, 0, false}, 13.97}, {{ELEM_N, 2, 1, 10, 40, 4, 4, 0, 0, 0, false}, 16.61},
            {{ELEM_N, 2, 1, 20, 40, 3, 2, 1, 0, 0, false}, 25.59}, {{ELEM_N, 3, 1, 10, 40, 4, 4, 0, 0, 0, false}, 27.64},
            {{ELEM_N, 0, 0, 15, 30, 2, 0, 0, 0, 2, false}, 12.89}, {{ELEM_N, 0, 0, 15, 45, 3, 0, 0, 0, 3, false}, 4.41},
            {{ELEM_N, 0, 0, 15, 40, 3, 1, 0, 0, 2, false}, 4.93},  {{ELEM_N, 0, 0, 20, 50, 3, 0, 1, 0, 2, false}, 8.39},
            {{ELEM_N, 1, 0, 15, 40, 3, 1, 0, 0, 2, false}, 15.79}, {{ELEM_N, 0, 1, 15, 45, 3, 0, 0, 0, 3, false}, 4.1},
            {{ELEM_N, 0, 1, 15, 40, 3, 1, 0, 0, 2, false}, 3.88},  {{ELEM_N, 1, 1, 15, 40, 3, 1, 0, 0, 2, false}, 14.14},
            {{ELEM_O, 0, 0, 10, 20, 2, 2, 0, 0, 0, false}, 9.23},  {{ELEM_O, 0, 0, 10, 20, 2, 2, 0, 0, 0, true}, 12.53},
            {{ELEM_O, 0, 0, 20, 20, 1, 0, 1, 0, 0, false}, 17.07}, {{ELEM_O, 0, -1, 10, 10, 1, 1, 0, 0, 0, false}, 23.06},
            {{ELEM_O, 1, 0, 10, 20, 2, 2, 0, 0, 0, false}, 20.23}, {{ELEM_O, 0, 0, 15, 30, 2, 0, 0, 0, 2, false}, 13.14}};
        return atomContributions;
    }

    const map<TPSA::Key, double>& TPSA::getAtomContributionsNOSP()
    {
        static map<TPSA::Key, double> atomContributions{
            {{ELEM_N, 0, 0, 10, 30, 3, 3, 0, 0, 0, false}, 3.24},  {{ELEM_N, 0, 0, 20, 30, 2, 1, 1, 0, 0, false}, 12.36},
            {{ELEM_N, 0, 0, 30, 30, 1, 0, 0, 1, 0, false}, 23.79}, {{ELEM_N, 0, 0, 20, 50, 3, 1, 2, 0, 0, false}, 11.68},
            {{ELEM_N, 0, 0, 30, 50, 2, 0, 1, 1, 0, false}, 13.6},  {{ELEM_N, 0, 0, 10, 30, 3, 3, 0, 0, 0, true}, 3.01},
            {{ELEM_N, 1, 0, 10, 30, 3, 3, 0, 0, 0, false}, 12.03}, {{ELEM_N, 1, 0, 10, 30, 3, 3, 0, 0, 0, true}, 21.94},
            {{ELEM_N, 1, 0, 20, 30, 2, 1, 1, 0, 0, false}, 23.85}, {{ELEM_N, 2, 0, 10, 30, 3, 3, 0, 0, 0, false}, 26.02},
            {{ELEM_N, 0, 1, 10, 40, 4, 4, 0, 0, 0, false}, 0.0},   {{ELEM_N, 0, 1, 20, 40, 3, 2, 1, 0, 0, false}, 3.01},
            {{ELEM_N, 0, 1, 30, 40, 2, 1, 0, 1, 0, false}, 4.36},  {{ELEM_N, 1, 1, 10, 40, 4, 4, 0, 0, 0, false}, 4.44},
            {{ELEM_N, 1, 1, 20, 40, 3, 2, 1, 0, 0, false}, 13.97}, {{ELEM_N, 2, 1, 10, 40, 4, 4, 0, 0, 0, false}, 16.61},
            {{ELEM_N, 2, 1, 20, 40, 3, 2, 1, 0, 0, false}, 25.59}, {{ELEM_N, 3, 1, 10, 40, 4, 4, 0, 0, 0, false}, 27.64},
            {{ELEM_N, 0, 0, 15, 30, 2, 0, 0, 0, 2, false}, 12.89}, {{ELEM_N, 0, 0, 15, 45, 3, 0, 0, 0, 3, false}, 4.41},
            {{ELEM_N, 0, 0, 15, 40, 3, 1, 0, 0, 2, false}, 4.93},  {{ELEM_N, 0, 0, 20, 50, 3, 0, 1, 0, 2, false}, 8.39},
            {{ELEM_N, 1, 0, 15, 40, 3, 1, 0, 0, 2, false}, 15.79}, {{ELEM_N, 0, 1, 15, 45, 3, 0, 0, 0, 3, false}, 4.1},
            {{ELEM_N, 0, 1, 15, 40, 3, 1, 0, 0, 2, false}, 3.88},  {{ELEM_N, 1, 1, 15, 40, 3, 1, 0, 0, 2, false}, 14.14},
            {{ELEM_O, 0, 0, 10, 20, 2, 2, 0, 0, 0, false}, 9.23},  {{ELEM_O, 0, 0, 10, 20, 2, 2, 0, 0, 0, true}, 12.53},
            {{ELEM_O, 0, 0, 20, 20, 1, 0, 1, 0, 0, false}, 17.07}, {{ELEM_O, 0, -1, 10, 10, 1, 1, 0, 0, 0, false}, 23.06},
            {{ELEM_O, 1, 0, 10, 20, 2, 2, 0, 0, 0, false}, 20.23}, {{ELEM_O, 0, 0, 15, 30, 2, 0, 0, 0, 2, false}, 13.14},
            {{ELEM_S, 0, 0, 10, 20, 2, 2, 0, 0, 0, false}, 25.3},  {{ELEM_S, 0, 0, 20, 20, 1, 0, 1, 0, 0, false}, 32.09},
            {{ELEM_S, 0, 0, 20, 40, 3, 2, 1, 0, 0, false}, 19.21}, {{ELEM_S, 0, 0, 20, 60, 4, 2, 2, 0, 0, false}, 8.38},
            {{ELEM_S, 1, 0, 10, 20, 2, 2, 0, 0, 0, false}, 38.8},  {{ELEM_S, 0, 0, 15, 30, 2, 0, 0, 0, 2, false}, 28.24},
            {{ELEM_S, 0, 0, 20, 50, 3, 0, 1, 0, 2, false}, 21.7},  {{ELEM_P, 0, 0, 10, 30, 3, 3, 0, 0, 0, false}, 13.59},
            {{ELEM_P, 0, 0, 20, 30, 3, 1, 1, 0, 0, false}, 34.14}, {{ELEM_P, 0, 0, 20, 50, 4, 3, 1, 0, 0, false}, 9.81},
            {{ELEM_P, 1, 0, 20, 50, 4, 3, 1, 0, 0, false}, 23.47}};
        return atomContributions;
    }
}
