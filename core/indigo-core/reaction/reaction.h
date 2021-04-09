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

#ifndef __reaction_h__
#define __reaction_h__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "molecule/molecule.h"
#include "reaction/base_reaction.h"

namespace indigo
{

    // Stereo changes during reaction
    enum
    {
        STEREO_UNMARKED = 0,
        STEREO_INVERTS = 1,
        STEREO_RETAINS = 2
    };

    // Reacting centers
    enum
    {
        RC_NOT_CENTER = -1,
        RC_UNMARKED = 0,
        RC_CENTER = 1,
        RC_UNCHANGED = 2,
        RC_MADE_OR_BROKEN = 4,
        RC_ORDER_CHANGED = 8,
        RC_TOTAL = 16
    };

    class DLLEXPORT Reaction : public BaseReaction
    {
    public:
        Reaction();
        ~Reaction() override;

        void clear() override;

        BaseReaction* neu() override;

        Molecule& getMolecule(int index);

        bool aromatize(const AromaticityOptions& options) override;
        bool dearomatize(const AromaticityOptions& options) override;

        Reaction& asReaction() override;

        /*
        void dearomatizeBonds();
        void aromatizeQueryBonds();
        bool isAllConnected() const;*/

        static void saveBondOrders(Reaction& reaction, ObjArray<Array<int>>& bond_types);
        static void loadBondOrders(Reaction& reaction, ObjArray<Array<int>>& bond_types);

        static void checkForConsistency(Reaction& rxn);

        void unfoldHydrogens();

        DECL_ERROR;

    protected:
        int _addBaseMolecule(int side) override;
    };

} // namespace indigo

#endif
