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

#ifndef __sequence_saver__
#define __sequence_saver__

#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "molecule/monomers_lib.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Output;
    class BaseMolecule;

    class DLLEXPORT SequenceSaver
    {
    public:
        enum class SeqFormat
        {
            Sequence,
            FASTA,
            IDT
        };

        static constexpr uint32_t SEQ_LINE_LENGTH = 80;

        DECL_ERROR;

        SequenceSaver(Output& output);
        ~SequenceSaver();

        void saveMolecule(BaseMolecule& mol, SeqFormat sf = SeqFormat::Sequence);

    protected:
        TGroup& getTGroup();
        std::string saveIdt(BaseMolecule& mol, std::deque<int>& sequence);

    private:
        SequenceSaver(const SequenceSaver&); // no implicit copy
        Output& _output;
        const MonomerTemplates& _mon_lib;
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> _templates;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
