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

#ifndef __crf_loader__
#define __crf_loader__

#include "base_cpp/obj.h"
#include "crf_saver.h"
#include "lzw/lzw_decoder.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Reaction;

    class DLLEXPORT CrfLoader
    {
    public:
        // external dictionary, internal encoder
        explicit CrfLoader(LzwDict& dict, Scanner& scanner);

        // no dictionary, no encoder
        explicit CrfLoader(Scanner& scanner);

        void loadReaction(Reaction& reaction);

        Scanner* xyz_scanner;
        int version; // By default the latest version 2 is used

        DECL_ERROR;

    protected:
        void _init();

        void _loadMolecule(Molecule& molecule);
        void _loadReactionMolecule(Reaction& reaction, int index, bool have_aam);

        Scanner& _scanner;

        Obj<LzwDecoder> _decoder;
        LzwDict* _dict;

        Array<int>* _bond_rc_flags;
        Array<int>* _atom_stereo_flags;
        Array<int>* _aam;

    private:
        CrfLoader(const CrfLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
