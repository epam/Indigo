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

#ifndef __rsmiles_saver__
#define __rsmiles_saver__

#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"
#include "reaction.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Output;
    class BaseReaction;
    class QueryReaction;
    class Reaction;

    class DLLEXPORT RSmilesSaver
    {
    public:
        DECL_ERROR;

        RSmilesSaver(Output& output);

        void saveReaction(Reaction& reaction);
        void saveQueryReaction(QueryReaction& reaction);

        bool smarts_mode;
        bool chemaxon;

    protected:
        BaseReaction* _brxn;
        QueryReaction* _qrxn;
        Reaction* _rxn;

        void _saveReaction();

        struct _Idx
        {
            int mol;
            int idx;
        };

        Output& _output;

        CP_DECL;
        TL_CP_DECL(Array<_Idx>, _written_atoms);
        TL_CP_DECL(Array<_Idx>, _written_bonds);
        TL_CP_DECL(Array<int>, _ncomp);

        void _writeMolecule(int i);
        void _writeFragmentsInfo();
        void _writeStereogroups();
        void _writeRadicals();
        void _writePseudoAtoms();
        void _writeHighlighting();

        bool _comma;

    private:
        RSmilesSaver(const RSmilesSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
