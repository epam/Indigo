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

#ifndef __sequence_loader__
#define __sequence_loader__

#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "molecule/monomers_lib.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Scanner;
    class BaseMolecule;
    class Molecule;
    class QueryMolecule;

    class DLLEXPORT SequenceLoader
    {
    public:
        enum class SeqType
        {
            PEPTIDESeq,
            RNASeq,
            DNASeq
        };
        DECL_ERROR;

        SequenceLoader(Scanner& scanner);

        ~SequenceLoader();

        void loadSequence(BaseMolecule& mol, SeqType seq_type);
        void loadSequence(BaseMolecule& mol, const std::string& seq_type_str);

    private:
        void addMonomer(BaseMolecule& mol, char ch, SeqType seq_type);
        void addTemplate(BaseMolecule& mol, char ch, SeqType seq_type);

        void addAminoAcid(BaseMolecule& mol, char ch);
        void addNucleotide(BaseMolecule& mol, char ch, const std::string& sugar_alias);
        bool addMonomerTemplate(BaseMolecule& mol, MonomerType mt, const std::string& alias);
        SequenceLoader(const SequenceLoader&); // no implicit copy
        Scanner& _scanner;
        std::unordered_set<std::pair<SeqType, char>, pair_hash> _added_templates;
        const MonomerTemplates& _mon_lib;
        Array<char> _left_apid;
        Array<char> _right_apid;
        Array<char> _xlink_apid;
        int _seq_id;
        int _last_sugar_idx;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
