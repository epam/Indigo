/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"	);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/
#pragma once

#include <cstdint>
#include <map>

namespace indigo
{
    class Molecule;

    class TPSA
    {
    public:
        static double calculate(Molecule& molecule, bool includeSP = false);

    private:
        struct Key
        {
            uint32_t atomNumber = 0;
            uint32_t nH = 0;
            int32_t charge = 0;
            uint32_t maxBondOrder = 0;
            uint32_t bondOrderSum = 0;
            uint32_t nNeighbors = 0;
            uint32_t nSingleBonds = 0;
            uint32_t nDoubleBonds = 0;
            uint32_t nTripleBonds = 0;
            uint32_t nAromaticBonds = 0;
            bool isIn3MemberedRing = false;

            bool operator<(const Key& b) const noexcept;
        };

        static const std::map<TPSA::Key, double>& getAtomContributionsNO();
        static const std::map<TPSA::Key, double>& getAtomContributionsNOSP();
    };
}
