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

#ifndef __crf_saver__
#define __crf_saver__

#include "base_cpp/obj.h"
#include "lzw/lzw_encoder.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class Reaction;
    class LzwDict;

    class DLLEXPORT CrfSaver
    {
    public:
        // external dictionary, internal encoder
        explicit CrfSaver(LzwDict& dict, Output& output);

        // no dictionary, no encoder
        explicit CrfSaver(Output& output);

        void saveReaction(Reaction& reaction);

        Output* xyz_output;

        bool save_bond_dirs;
        bool save_highlighting;
        bool save_mapping;

        DECL_ERROR;

    protected:
        void _init();

        void _writeReactionInfo(Reaction& reaction);
        void _writeAam(const int* aam, const Array<int>& sequence);
        void _writeMolecule(Molecule& molecule);
        void _writeReactionMolecule(Reaction& reaction, int idx);

        Output& _output;
        Obj<LzwEncoder> _encoder;

        const int* _atom_stereo_flags;
        const int* _bond_rc_flags;
        const int* _aam;

    private:
        CrfSaver(const CrfSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
