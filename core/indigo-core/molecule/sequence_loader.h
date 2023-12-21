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

        void loadSequence(Molecule& mol, SeqType sq);
        Scanner& _scanner;

    private:
        void addAminoAcid(Molecule& mol, char ch);
        void addRNANucleotide(Molecule& mol, char ch);
        void addDNANucleotide(Molecule& mol, char ch);
        SequenceLoader(const SequenceLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
