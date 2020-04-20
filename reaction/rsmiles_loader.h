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

#ifndef __rsmiles_loader__
#define __rsmiles_loader__

#include "base_cpp/exception.h"
#include "molecule/molecule_stereocenter_options.h"

namespace indigo
{

    class Scanner;
    class BaseReaction;
    class Reaction;
    class QueryReaction;

    class DLLEXPORT RSmilesLoader
    {
    public:
        DECL_ERROR;

        RSmilesLoader(Scanner& scanner);

        void loadReaction(Reaction& rxn);
        void loadQueryReaction(QueryReaction& rxn);

        // see comment in SmilesLoader
        bool ignore_closing_bond_direction_mismatch;
        bool smarts_mode;
        bool ignore_cistrans_errors;
        bool ignore_bad_valence;
        StereocentersOptions stereochemistry_options;

    protected:
        struct _Atom
        {
            int mol_idx;
            int atom_idx;
        };

        int _selectGroup(int& idx, int rcnt, int ccnt, int pcnt) const;
        int _selectGroupByPair(int& lead_idx, int& idx, int rcnt, int ccnt, int pcnt) const;

        Scanner& _scanner;

        BaseReaction* _brxn;
        QueryReaction* _qrxn;
        Reaction* _rxn;

        void _loadReaction();

    private:
        RSmilesLoader(const RSmilesLoader&); // no implicit copy
    };

} // namespace indigo

#endif
