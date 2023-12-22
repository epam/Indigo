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
#include "molecule/monomer_commons.h"

using namespace indigo;

IMPL_ERROR(SequenceLoader, "SEQUENCE loader");

SequenceLoader::SequenceLoader(Scanner& scanner) : _scanner(scanner)
{
}

SequenceLoader::~SequenceLoader()
{
}

void SequenceLoader::loadSequence(Molecule& mol, SeqType sqtype)
{
    mol.clear();
    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        addMonomer(mol, ch, sqtype);
    }
}

void SequenceLoader::addTemplate(Molecule& mol, char ch, SeqType sqtype)
{
    int tg_idx = mol.tgroups.addTGroup();
    auto& tg = mol.tgroups.getTGroup(tg_idx);
    if (sqtype == SeqType::PEPTIDESeq)
    {

    }
    /* const auto& class_str = MonomerTemplates::kNucleotideComponentTypeStr.at();
    tg.tgroup_class.appendString(class_str.c_str(), true);
    tg.tgroup_name.appendString(nuc.first.second.c_str(), true);
    tg.tgroup_alias.appendString(monomerAliasByName(class_str, nuc.first.second).c_str(), true);
    tg.tgroup_id = tg_idx;
    tg.tgroup_natreplace.appendString(nuc.second.natreplace.c_str(), true);
    tg.fragment.reset(nuc.second.monomer->neu());
    tg.fragment->clone(*nuc.second.monomer);
    _added_templates.emplace(sqtype, ch);*/
}

void SequenceLoader::addMonomer(Molecule& mol, char ch, SeqType sqtype)
{
    //if (_added_templates.find(std::make_pair(sqtype, ch)) != _added_templates.end())
    {
        // add template
    }
    switch (sqtype)
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

void SequenceLoader::addAminoAcid(Molecule& mol, char ch)
{
}

void SequenceLoader::addRNANucleotide(Molecule& mol, char ch)
{
}

void SequenceLoader::addDNANucleotide(Molecule& mol, char ch)
{
}
