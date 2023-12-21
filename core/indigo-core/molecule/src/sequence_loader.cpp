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

#include <cctype>
#include <memory>
#include <regex>
#include <unordered_set>

#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/sequence_loader.h"

using namespace indigo;

IMPL_ERROR(SequenceLoader, "SEQUENCE loader");

SequenceLoader::SequenceLoader(Scanner& scanner) : _scanner(scanner)
{
}

SequenceLoader::~SequenceLoader()
{
}

void SequenceLoader::loadSequence(Molecule& mol, SeqType sq)
{
    mol.clear();
    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        switch (sq)
        {
        case SeqType::PEPTIDESeq:
            addAminoAcid(mol, ch);
            break;
        case SeqType::RNASeq:
            addRNANucleotide(mol, ch);
            break;
        case SeqType::DNASeq:
            addDNANucleotide(mol, ch);
            break;
        }
    }
}

void SequenceLoader::addAminoAcid(Molecule& mol, char ch)
{

}

void SequenceLoader::addRNANucleotide(Molecule& mol, char ch)
{

}

void SequenceLoader::addDNANucleotide(Molecule& mol, char ch)
{

}

