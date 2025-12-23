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
#include "layout/molecule_layout.h"
#include "layout/sequence_layout.h"
#include "molecule/elements.h"
#include "molecule/ket_document.h"
#include "molecule/meta_commons.h"
#include "molecule/molecule.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/sequence_loader.h"
#include "molecule/smiles_loader.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

void SequenceLoader::loadAxoLabs(KetDocument& document)
{
    _row = 0;
    std::string data;
    _scanner.readAll(data);
    auto data_size = data.size();

    constexpr char* CRLF = "\r\n";
    constexpr char* LF = "\n";
    const auto MIN_AXO_SIZE = sizeof(AXOLABS_PREFIX) + sizeof(AXOLABS_SUFFIX) - 1;

    size_t start = 0, end = 0;
    auto search_line_end = [&data, &CRLF, &LF, data_size](size_t start, size_t& pos) -> size_t {
        auto crlf = data.find(CRLF, start);
        auto lf = data.find(LF, start);
        if (lf == std::string::npos && crlf == std::string::npos)
        {
            pos = data_size;
            return std::string::npos;
        }

        if (lf < crlf)
        {
            pos = lf;
            return 1;
        }
        else
        {
            pos = crlf;
            return 2;
        };
    };
    auto search_data = [&data_size, &search_line_end](size_t& start, size_t& end) {
        auto count = search_line_end(start, end);
        while (end - start == 0 && start != data_size) // skip empty strings
        {
            start += count;
            count = search_line_end(start, end);
        }
    };

    search_data(start, end);
    if (start == end)
        throw Error("Empty string");

    class link
    {
    public:
        link(const std::string& monomer) : mon_id(monomer), base_id(){};
        link(const std::string& monomer, const std::string& base) : mon_id(monomer), base_id(base){};
        std::string mon_id;
        std::string base_id;
    };

    std::string sense;
    std::vector<link> sense_chain;
    std::string antisense;
    std::vector<link> antisense_chain;
    auto constexpr non_pairing = 'X';
    bool is_sense = true;

    auto get_analog_short = [non_pairing](const MonomerTemplate& templ) {
        if (hasKetStrProp(templ, naturalAnalogShort))
            return getKetStrProp(templ, naturalAnalogShort)[0];
        if (hasKetStrProp(templ, alias))
        {
            auto alias = getKetStrProp(templ, alias);
            if (alias.size() == 1)
                return alias[0];
        }
        return non_pairing;
    };

    auto add_link = [&](const MonomerTemplate& monomer_template) {
        if (is_sense)
        {
            sense.append(1, get_analog_short(monomer_template));
            sense_chain.emplace_back(monomer_template.id());
        }
        else
        {
            antisense.append(1, get_analog_short(monomer_template));
            antisense_chain.emplace_back(monomer_template.id());
        }
    };

    while (start != end)
    {
        std::string sequence = data.substr(start, end - start);
        if (sequence.size() < MIN_AXO_SIZE)
            throw Error("Sequence too short: '%s'", data.substr(start, end - start).c_str());
        std::string affix = sequence.substr(0, sizeof(AXOLABS_PREFIX) - 1);
        if (affix != AXOLABS_PREFIX)
            throw Error("Invalid AxoLabs sequence: expected %s got %s", AXOLABS_PREFIX, affix.c_str());
        affix = sequence.substr(sequence.size() - sizeof(AXOLABS_SUFFIX) + 1, sizeof(AXOLABS_SUFFIX) - 1);
        if (affix != AXOLABS_SUFFIX)
            throw Error("Invalid AxoLabs sequence: expected %s got %s", AXOLABS_SUFFIX, affix.c_str());
        // remove prefix and suffix
        sequence.erase(sequence.size() - sizeof(AXOLABS_SUFFIX) + 1, sizeof(AXOLABS_SUFFIX) - 1);
        sequence.erase(0, sizeof(AXOLABS_PREFIX) - 1);

        std::size_t pos = 0;
        std::size_t length = sequence.size();
        while (pos < length)
        {
            std::string group{sequence[pos++]};
            if (pos == 1 && group == "s")
                throw Error("Invalid AxoLabs sequence: phosphate 's' can only be internal.");
            if (group == "p")
            {
                if (pos != 1 && pos != length)
                    throw Error("Invalid AxoLabs sequence: phosphate 'p' can only be terminal.");
                auto& monomer_template_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, "P");
                const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                checkAddTemplate(document, monomer_template);
                add_link(monomer_template);
                continue;
            }
            std::string mgt_id;
            if (group.front() == '(')
            { // read till ')'
                while (pos < length)
                {
                    group += sequence[pos++];
                    if (group.back() == ')')
                        break;
                    if (pos >= length)
                        throw Error("Unexpected end of data");
                }
                if (group.back() != ')')
                    throw Error("Unexpected end of data");
            }
            else if (pos < length && STANDARD_NUCLEOTIDES.count(group) > 0 && std::islower(sequence[pos]) && sequence[pos] != 's' && sequence[pos] != 'p')
            {
                group += sequence[pos++];
            }
            else
            {
                mgt_id = _library.getMGTidByAliasAxoLabs(group);
                if (mgt_id.size() == 0) // if not a/c/t/g - read second char
                {
                    if (pos >= length)
                        throw Error("Unexpected end of data");
                    group += sequence[pos++];
                }
            }
            std::string phosphate = "P";
            std::string sugar, base;
            if (pos < length && sequence[pos] == 's')
            {
                pos++;
                if (pos >= length || (pos == length - 1 && sequence[pos] == 'p'))
                    throw Error("Invalid AxoLabs sequence: phosphate 's' can only be internal.");
                phosphate = "sP";
            }
            if (mgt_id.size() == 0)
            {
                mgt_id = _library.getMGTidByAliasAxoLabs(group);
            }
            if (mgt_id.size() > 0)
            {
                auto& mgt = _library.getMonomerGroupTemplateById(mgt_id);
                std::string base_id;
                auto base_analog = non_pairing;
                if (mgt.hasTemplate(MonomerClass::Base))
                {
                    auto& base_template = mgt.getTemplateByClass(MonomerClass::Base);
                    checkAddTemplate(document, base_template);
                    base_id = base_template.id();
                    base_analog = get_analog_short(base_template);
                }
                auto& sugar_template = mgt.getTemplateByClass(MonomerClass::Sugar);
                checkAddTemplate(document, sugar_template);
                if (is_sense)
                {
                    sense.append(1, base_analog);
                    sense_chain.emplace_back(sugar_template.id(), base_id);
                }
                else
                {
                    antisense.append(1, base_analog);
                    antisense_chain.emplace_back(sugar_template.id(), base_id);
                }
                if (!(pos >= length || (pos == length - 1 && sequence[pos] == 'p'))) // last nuleotide without phosphate, this is not last nucleotide
                {
                    auto& phosphate_template = group.front() == '('
                                                   ? mgt.getTemplateByClass(MonomerClass::Phosphate)
                                                   : _library.getMonomerTemplateById(_library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, phosphate));
                    add_link(phosphate_template);
                    checkAddTemplate(document, phosphate_template);
                }
            }
            else
            {
                std::string monomer_template_id = _library.getMonomerTemplateIdByAliasAxoLabs(group);
                if (monomer_template_id.size() > 0)
                {
                    const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                    checkAddTemplate(document, monomer_template);
                    add_link(monomer_template);
                }
                else
                {
                    if (group[0] != '(')
                        throw Error("The following string cannot be interpreted as an AxoLabs string: %s", group.c_str()); // unresolved should be in ()
                    monomer_template_id = group;
                    MonomerTemplate monomer_template(monomer_template_id, MonomerClass::CHEM, IdtAlias(), true);
                    setKetStrProp(monomer_template, alias, group);
                    setKetStrProp(monomer_template, aliasAxoLabs, group);
                    for (auto ap : {"R1", "R2"})
                        monomer_template.AddAttachmentPoint(ap, -1);
                    checkAddTemplate(document, monomer_template);
                    add_link(monomer_template);
                }
            }
        }
        start = end;
        search_data(start, end);
        if (start == end || !is_sense) // last sequence or end of antisense
        {
            auto add_link_monomers = [&](const link& cur_link, int& last_monomer_idx, bool reverse_base = false, std::string* base_id = nullptr) {
                Vec3f pos = getBackboneMonomerPosition();
                auto monomer_idx = document.monomers().size();
                auto& monomer_template = document.templates().at(cur_link.mon_id);
                if (monomer_template.monomerClass() != MonomerClass::Phosphate)
                    _seq_id++;
                auto& monomer = document.addMonomer(getKetStrProp(monomer_template, alias), monomer_template.id());
                setKetIntProp(*monomer, seqid, _seq_id);
                monomer->setPosition(pos);
                if (last_monomer_idx >= 0)
                    addMonomerConnection(document, last_monomer_idx, monomer_idx);
                if (cur_link.base_id.size() > 0)
                {
                    auto base_idx = document.monomers().size();
                    auto& base_template = document.templates().at(cur_link.base_id);
                    auto& base = document.addMonomer(getKetStrProp(base_template, alias), base_template.id());
                    if (base_id != nullptr)
                        base_id->assign(base->id());
                    setKetIntProp(*base, seqid, _seq_id);
                    auto base_shift = LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH;
                    if (reverse_base)
                        base_shift = -base_shift;
                    Vec3f base_pos(pos.x, pos.y - base_shift, 0);
                    base->setPosition(base_pos);
                    addMonomerConnection(document, monomer_idx, base_idx, true);
                }
                last_monomer_idx = static_cast<int>(monomer_idx);
            };
            if (is_sense) // last sequence is sense - just add it
            {
                _col = 0;
                int last_monomer_idx = -1;
                for (auto it = sense_chain.begin(); it != sense_chain.end(); it++)
                {
                    add_link_monomers(*it, last_monomer_idx);
                    _col++;
                }
            }
            else // end of antisense sequence - add doube-chain
            {
                std::vector<std::pair<size_t, size_t>> pairs;
                bool shift_sense;
                std::reverse(antisense.begin(), antisense.end());
                auto offset = best_allign(sense, antisense, pairs, shift_sense);
                std::map<size_t, std::string> sense_ids;
                std::map<size_t, std::string> antisense_ids;
                for (auto it = pairs.begin(); it != pairs.end(); it++)
                {
                    sense_ids.emplace(it->first, "");
                    // fix antisense index - it is reversed
                    auto antisense_idx = antisense_chain.size() - 1 - it->second;
                    it->second = antisense_idx;
                    antisense_ids.emplace(antisense_idx, "");
                }
                // save sense
                _col = static_cast<int>(shift_sense ? offset : 0);
                int last_monomer_idx = -1;
                for (size_t idx = 0; idx < sense_chain.size(); idx++)
                {
                    if (sense_ids.count(idx) > 0) // if id in pairs - save base id
                        add_link_monomers(sense_chain.at(idx), last_monomer_idx, false, &sense_ids.at(idx));
                    else
                        add_link_monomers(sense_chain.at(idx), last_monomer_idx);
                    _col++;
                }
                // save antisense (from right to left)
                _row += 3;
                _col = static_cast<int>(shift_sense ? antisense_chain.size() - 1 : offset + antisense_chain.size() - 1);
                last_monomer_idx = -1;
                for (size_t idx = 0; idx < antisense_chain.size(); idx++)
                {

                    if (antisense_ids.count(idx) > 0) // if id in pairs - save base id
                        add_link_monomers(antisense_chain.at(idx), last_monomer_idx, true, &antisense_ids.at(idx));
                    else
                        add_link_monomers(antisense_chain.at(idx), last_monomer_idx, true);
                    _col--;
                }
                // add hydrogen bonds
                for (auto it = pairs.begin(); it != pairs.end(); it++)
                {
                    document.addConnection(document.monomers().at(sense_ids.at(it->first))->ref(), HelmHydrogenPair,
                                           document.monomers().at(antisense_ids.at(it->second))->ref(), HelmHydrogenPair);
                }
                sense.clear();
                antisense.clear();
                sense_ids.clear();
                antisense_ids.clear();
                sense_chain.clear();
                antisense_chain.clear();
                is_sense = !is_sense;
                _row += 1;
            }
        }
        else // not last sequence and not antisense - next will be antisense
        {
            is_sense = !is_sense;
        }
    }
}

// Shared constants for HELM and AxoLabs parsers (declared in sequence_loader.cpp)
extern std::set<std::string> polymer_types;
extern const char* reserved_helm_chars;
extern const char* unexpected_eod;

std::string SequenceLoader::readHelmMonomerAlias(KetDocument& document, MonomerClass monomer_class, bool inside_parentheses)
{
    std::string monomer_alias;
    char ch = static_cast<char>(_scanner.lookNext());

    if (ch == '*')
    {
        if (monomer_class != MonomerClass::CHEM)
            throw Error("'*' could be used only for CHEM monomers for now.");
        _scanner.skip(1);
        return "*";
    }

    if (ch == '[')
    {
        _scanner.skip(1);
        bool smiles = false;
        for (int bracket_count = 1; bracket_count != 0 && !_scanner.isEOF();)
        {
            ch = _scanner.readChar();
            switch (ch)
            {
            case '[':
                bracket_count++;
                smiles = true;
                break;
            case ']':
                bracket_count--;
                break;
            case '(':
            case ')':
            case '=':
            case '#':
            case '/':
            case '\\':
            case '+':
                smiles = true;
                break;
            default:
                break;
            }
            if (ch != ']' || bracket_count > 0)
                monomer_alias += ch;
        }
        if (_scanner.isEOF())
            throw Error(unexpected_eod);
        if (ch != ']')
            throw Error("Unexpected symbol. Expected ']' but found '%c'.", ch);
        if (smiles) // Monomer alias not found in library - try read as smiles
        {
            // Convert smiles to molecule
            BufferScanner scanner(monomer_alias.c_str());
            SmilesLoader loader(scanner);
            Molecule mol{};
            loader.stereochemistry_options.ignore_errors = true;
            // loader.ignore_cistrans_errors = true;
            loader.loadMolecule(mol);
            MoleculeLayout ml(mol, true);
            ml.layout_orientation = UNCPECIFIED;
            ml.make();
            mol.clearBondDirections();
            try
            {
                mol.markBondsStereocenters();
                mol.markBondsAlleneStereo();
            }
            catch (Exception e)
            {
            }
            // create template based on molecule
            monomer_alias = "Mod" + std::to_string(_unknown_ambiguous_count++);
            auto& mon_template = document.addMonomerTemplate(monomer_alias, MonomerTemplate::MonomerClassToStr(monomer_class), IdtAlias());
            setKetStrProp(mon_template, alias, monomer_alias);
            _added_templates.emplace(monomer_class, monomer_alias);
            _alias_to_id.emplace(std::make_pair(monomer_class, monomer_alias), mon_template.id());
            std::map<int, int> rgroups;
            std::map<int, int> rg_to_attatom;
            std::vector<KetBond> bonds;
            for (auto i : mol.vertices())
            {
                if (mol.isRSite(i))
                {
                    const auto& vertex = mol.getVertex(i);
                    if (vertex.degree() != 1)
                        throw Error("Attachment point should be connected to single atom");
                    rg_to_attatom.emplace(i, vertex.neiVertex(vertex.neiBegin()));
                    rgroups.emplace(i, mol.getSingleAllowedRGroup(i));
                    mon_template.AddAtom("H", mol.getAtomXyz(i));
                }
                else
                {
                    int anum = mol.getAtomNumber(i);
                    std::string label;
                    if (anum == VALUE_UNKNOWN)
                        throw Error("Unknown element");
                    int isotope = mol.getAtomIsotope(i);
                    auto atom_idx = mon_template.AddAtom(Element::toString(anum, isotope), mol.getAtomXyz(i));
                    auto stereo_type = mol.stereocenters.getType(i);
                    if (stereo_type > 0)
                    {
                        auto& base_atom = static_cast<KetBaseAtom&>(*mon_template.getAtom(atom_idx));
                        switch (stereo_type)
                        {
                        case MoleculeStereocenters::ATOM_ANY:
                            setKetStrProp(base_atom, stereoLabel, "any");
                            break;
                        case MoleculeStereocenters::ATOM_AND:
                            setKetStrProp(base_atom, stereoLabel, "rac");
                            break;
                        case MoleculeStereocenters::ATOM_OR:
                            setKetStrProp(base_atom, stereoLabel, "rel");
                            break;
                        case MoleculeStereocenters::ATOM_ABS:
                            setKetStrProp(base_atom, stereoLabel, "abs");
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            for (auto i : mol.edges())
            {
                auto& edge = mol.getEdge(i);
                int bond_order = mol.getBondOrder(i);
                if (bond_order == BOND_ZERO)
                {
                    bond_order = _BOND_COORDINATION;
                    if ((mol.getAtomNumber(edge.beg) == ELEM_H) || (mol.getAtomNumber(edge.end) == ELEM_H))
                        bond_order = _BOND_HYDROGEN;
                }
                auto bond_idx = mon_template.AddBond(bond_order, edge.beg, edge.end);
                int stereo = mol.getBondDirection(i);
                switch (stereo)
                {
                case BOND_UP:
                    stereo = BIOVIA_STEREO_UP;
                    break;
                case BOND_EITHER:
                    stereo = BIOVIA_STEREO_ETHER;
                    break;
                case BOND_DOWN:
                    stereo = BIOVIA_STEREO_DOWN;
                    break;
                default:
                    stereo = BIOVIA_STEREO_NO;
                }
                if (stereo != 0)
                {
                    setKetIntProp(mon_template.getBond(bond_idx), stereo, stereo);
                }
            }
            for (auto& it : rgroups)
            {
                std::string label = 'R' + std::to_string(it.second);
                auto& att_point = mon_template.AddAttachmentPoint(label, rg_to_attatom.at(it.first));
                std::vector<int> lg;
                lg.emplace_back(it.first);
                att_point.setLeavingGroup(lg);
            }
        }
    }
    else if (!_scanner.isEOF())
    {
        _scanner.skip(1);
        monomer_alias = ch;
        while (!_scanner.isEOF())
        {
            ch = static_cast<char>(_scanner.lookNext());
            if (ch == '(' || ch == '[' || ch == '\'' || ch == '"') // repeating or annotation
                break;
            if (ch == ']')
                throw Error("Unexpected symbol ']'.");
            bool end_of_name = false;
            if (inside_parentheses)
                end_of_name = ch == ',' || ch == '+' || ch == ':' || ch == ')';
            else
                end_of_name = ch == '.' || ch == '}';
            if (end_of_name)
                break;
            monomer_alias += ch;
            _scanner.skip(1);
        }
    }
    if (_scanner.isEOF())
        throw Error(unexpected_eod);
    return monomer_alias;
}

std::string SequenceLoader::readHelmRepeating()
{
    std::string repeating;
    if (_scanner.lookNext() == '\'')
    {
        // repeating
        Array<char> name;
        _scanner.skip(1);
        _scanner.readWord(name, "'");
        if (_scanner.lookNext() != '\'')
            throw Error("Unexpected symbol. Expected ''' but found '%c'.", _scanner.lookNext());
        _scanner.skip(1); // skip "'"
        repeating = name.ptr();
    }
    return repeating;
}

std::string SequenceLoader::readHelmAnnotation()
{
    std::string annotation;
    if (_scanner.lookNext() == '"') // inline annotation
    {
        Array<char> name;
        _scanner.skip(1);
        _scanner.readWord(name, "\"");
        if (_scanner.lookNext() != '"')
            throw Error("Unexpected symbol. Expected '\"' but found '%c'.", _scanner.lookNext());
        _scanner.skip(1); // skip '"'
        annotation = name.ptr();
    }
    return annotation;
}

int SequenceLoader::readCount(std::string& count)
{
    count.clear();
    char ch = static_cast<char>(_scanner.lookNext());
    if (ch == ':')
    {
        _scanner.skip(1);
        ch = static_cast<char>(_scanner.lookNext());
        while ((std::isdigit(ch) || ch == '.') && !_scanner.isEOF())
        {
            _scanner.skip(1);
            if (ch == '.')
            {
                if (count.size() == 0)
                    count += '0';
                else if (count.find(ch, 0) != count.npos) // second dot
                    throw Error("Enexpected symbol. Second dot in number");
            }
            count += ch;
            ch = static_cast<char>(_scanner.lookNext());
        }
        if (count.size() == 0)
            throw Error("Unexpected symbol. Expected digit but found '%c'", ch);
    }
    return ch;
}

SequenceLoader::MonomerInfo SequenceLoader::readHelmMonomer(KetDocument& document, MonomerClass monomer_class)
{
    std::string monomer_alias, repeating, annotation;
    ambiguous_template_opts options;
    int ch = _scanner.lookNext();
    bool inside_parentheses = false;
    if (ch == '(')
    {
        _scanner.skip(1);
        inside_parentheses = true;
    }
    monomer_alias = readHelmMonomerAlias(document, monomer_class, inside_parentheses);
    ch = _scanner.lookNext();
    bool is_ambiguous = false;

    if (ch == ',' || ch == '+' || ch == ':') // looks like ambiguous
    {
        if (!inside_parentheses)
            throw Error("Unexpected symbol '%c'. Ambiguous monomers should be defined in ().");
        std::string count;
        is_ambiguous = true;
        ch = readCount(count); // in ch==':' read count and return next char
        bool is_mixture = false;
        if (ch == '+')
            is_mixture = true;
        else if (ch != ',')
            throw Error("Unexpected symbol. Expected '+' or ',' but found '%c'", ch);
        _scanner.skip(1);

        std::set<std::string> aliases;
        std::string opt_alias = monomer_alias;
        options.first = is_mixture;
        bool no_counts = true;
        while (true)
        {
            aliases.emplace(opt_alias);
            auto& opt = options.second.emplace_back(opt_alias, std::optional<float>());
            if (count.size() > 0)
            {
                try
                {
                    opt.second = std::stof(count);
                }
                catch (...)
                {
                    throw Error("Invalid number '%s'", count.c_str());
                }
                no_counts = false;
            }
            if (ch == ')')
                break;
            opt_alias = readHelmMonomerAlias(document, monomer_class, inside_parentheses);
            if (aliases.count(opt_alias) > 0)
                throw Error("Ivalid ambiguous monomer. Monomer '%s' repeated more than once.", opt_alias.c_str());
            ch = readCount(count);
            if (is_mixture && ch != '+' && ch != ')')
                throw Error("Invalid ambiguous monomer. Expected '+' but found '%c'", ch);
            else if (!is_mixture && ch != ',' && ch != ')')
                throw Error("Invalid ambiguous monomer. Expected ',' but found '%c'", ch);
            _scanner.skip(1); // skip delimiter or ')'
        }
        if (monomer_class == MonomerClass::AminoAcid)
        {
            if (STANDARD_MIXED_PEPTIDES_TO_ALIAS.count(aliases) > 0)
                if (!is_mixture && no_counts)
                    monomer_alias = STANDARD_MIXED_PEPTIDES_TO_ALIAS.at(aliases);
                else
                    monomer_alias = STANDARD_MIXED_PEPTIDES_TO_ALIAS.at(aliases) + std::to_string(_unknown_ambiguous_count++);
            else
                monomer_alias = "Var" + std::to_string(_unknown_ambiguous_count++);
        }
        else if (monomer_class == MonomerClass::Base)
        {
            if (!is_mixture && STANDARD_MIXED_BASES_TO_ALIAS.count(aliases) > 0)
            {
                monomer_alias = STANDARD_MIXED_BASES_TO_ALIAS.at(aliases);
                if (monomer_alias[0] == 'r')
                    monomer_alias.erase(0, 1);
                if (is_mixture || !no_counts)
                    monomer_alias += std::to_string(_unknown_ambiguous_count++);
            }
            else
                monomer_alias = "Var" + std::to_string(_unknown_ambiguous_count++);
        }
        else
        {
            monomer_alias = "Var" + std::to_string(_unknown_ambiguous_count++);
        }
    }
    repeating = readHelmRepeating();
    annotation = readHelmAnnotation();
    ch = _scanner.lookNext();
    bool branch_monomer = false;
    if (inside_parentheses && !is_ambiguous) // for variants ')' already processed
    {
        if (ch != ')')
            throw Error("Unexpected symbol. Expected ')' but found '%c'.", ch);
        _scanner.skip(1); // single monomer in () - branch monomer
        branch_monomer = true;
    }
    return std::make_tuple(monomer_alias, branch_monomer, repeating, annotation, options);
}

std::string SequenceLoader::readHelmSimplePolymerName(std::string& polymer_name)
{
    auto ch = _scanner.lookNext();
    while (std::isalpha(ch) && !_scanner.isEOF())
    {
        _scanner.skip(1);
        polymer_name += static_cast<char>(std::toupper(ch));
        ch = _scanner.lookNext();
    }
    std::string polymer_type = polymer_name;
    if (polymer_types.count(polymer_name) == 0)
        throw Error("Unknown polymer type '%s'.", polymer_name.c_str());
    while (std::isdigit(ch) && !_scanner.isEOF())
    {
        _scanner.skip(1);
        polymer_name += static_cast<char>(ch);
        ch = _scanner.lookNext();
    }
    return polymer_type;
}

const std::string SequenceLoader::checkAddAmbiguousMonomerTemplate(KetDocument& document, const std::string& alias, MonomerClass monomer_class,
                                                                   ambiguous_template_opts& options)
{
    std::string ambiguous_template_id;
    const auto& it = _opts_to_template_id.find(options);
    if (it != _opts_to_template_id.end())
    {
        ambiguous_template_id = it->second;
    }
    else
    {
        bool is_mixture = options.first;
        std::string subtype = is_mixture ? "mixture" : "alternatives";
        std::vector<KetAmbiguousMonomerOption> opts;
        std::set<std::string> vt_ap_names;
        bool not_inited = true;
        std::map<std::string, KetAttachmentPoint> att_points;
        for (auto& option : options.second)
        {
            auto& helm_alias = option.first;
            auto opt_template_id = _library.getMonomerTemplateIdByAliasHELM(monomer_class, helm_alias);
            std::string opt_alias;
            if (opt_template_id.size() > 0)
            {
                _aliasHELM_to_id.emplace(make_pair(monomer_class, helm_alias), opt_template_id);
                opt_alias = getKetStrProp(_library.getMonomerTemplateById(opt_template_id), alias);
                checkAddTemplate(document, monomer_class, opt_alias);
            }
            else
            {
                opt_alias = helm_alias;
                opt_template_id = _library.getMonomerTemplateIdByAlias(monomer_class, opt_alias);
                if (opt_template_id.size() > 0)
                {
                    checkAddTemplate(document, _library.getMonomerTemplateById(opt_template_id));
                }
                else
                {
                    auto tmpl_it = _alias_to_id.find(std::make_pair(monomer_class, opt_alias));
                    if (tmpl_it != _alias_to_id.end())
                    {
                        opt_template_id = tmpl_it->second;
                    }
                    else
                    { // unknown monomer - add unresolved template
                        MonomerTemplate monomer_template(MonomerTemplate::MonomerClassToStr(monomer_class) + opt_alias, monomer_class, IdtAlias(), true);
                        setKetStrProp(monomer_template, alias, opt_alias);
                        for (auto ap : {"R1", "R2", "R3", "R4"})
                            monomer_template.AddAttachmentPoint(ap, -1);
                        checkAddTemplate(document, monomer_template);
                        _alias_to_id.emplace(make_pair(monomer_class, opt_alias), monomer_template.id());
                        opt_template_id = monomer_template.id();
                    }
                }
            }
            auto& opt = opts.emplace_back(opt_template_id);
            if (option.second.has_value())
                if (is_mixture)
                    opt.setRatio(option.second.value());
                else
                    opt.setProbability(option.second.value());
            auto& opt_template = document.getMonomerTemplate(opt_template_id);
            std::set<std::string> ap_names;
            for (auto& ap_it : opt_template.attachmentPoints())
            {
                ap_names.emplace(ap_it.first);
            }
            if (not_inited)
            {
                vt_ap_names = ap_names;
                att_points = opt_template.attachmentPoints();
            }
            else
            {
                for (auto ap_it : vt_ap_names)
                    if (ap_names.count(ap_it) == 0)
                        vt_ap_names.erase(ap_it);
            }
        }
        ambiguous_template_id = alias;
        if (document.hasAmbiguousMonomerTemplateWithId(ambiguous_template_id))
        {
            int idx = 0;
            do
            {
                ambiguous_template_id = alias + std::to_string(idx++);
            } while (document.hasAmbiguousMonomerTemplateWithId(ambiguous_template_id));
        }
        auto& var_template = document.addAmbiguousMonomerTemplate(subtype, ambiguous_template_id, alias, IdtAlias(), opts);
        var_template.setAttachmentPoints(att_points);
        _opts_to_template_id.emplace(options, ambiguous_template_id);
    }
    return ambiguous_template_id;
}

size_t SequenceLoader::addHelmMonomer(KetDocument& document, MonomerInfo info, MonomerClass monomer_class, const Vec3f& pos)
{
    auto [helm_alias, branch_monomer, repeating, annotaion, options] = info;
    if (repeating.size() && monomer_class == MonomerClass::CHEM)
        throw Error("Chem cannot be repeated.");
    if (repeating.size() && (monomer_class == MonomerClass::Base || monomer_class == MonomerClass::Sugar || monomer_class == MonomerClass::Phosphate))
        throw Error("RNA parts cannot be repeated.");
    if (repeating.size() > 0)
        throw Error("Repeating not supported now.");
    auto monomer_idx = document.monomers().size();
    if (options.second.size() > 0) // ambiguous monomer
    {
        std::string template_id = checkAddAmbiguousMonomerTemplate(document, helm_alias, monomer_class, options);
        const auto var_templ = document.ambiguousTemplates().at(template_id);
        helm_alias = var_templ.alias();
        auto& monomer = document.addAmbiguousMonomer(helm_alias, template_id);
        monomer->setAttachmentPoints(var_templ.attachmentPoints());
        setKetIntProp(*monomer, seqid, _seq_id++);
        monomer->setPosition(pos);
        if (annotaion.size())
            monomer->setTextAnnotation(annotaion);
    }
    else
    {
        const auto& it = _aliasHELM_to_id.find(make_pair(monomer_class, helm_alias));
        std::string template_id;
        if (it != _aliasHELM_to_id.end())
        {
            template_id = it->second;
        }
        else
        {
            template_id = _library.getMonomerTemplateIdByAliasHELM(monomer_class, helm_alias);
            std::string alias;
            if (template_id.size() > 0)
            {
                _aliasHELM_to_id.emplace(make_pair(monomer_class, helm_alias), template_id);
                alias = getKetStrProp(_library.getMonomerTemplateById(template_id), alias);
                template_id = checkAddTemplate(document, monomer_class, alias);
            }
            else
            {
                alias = helm_alias;
                auto& id = _library.getMonomerTemplateIdByAlias(monomer_class, alias);
                if (id.size() > 0)
                {
                    template_id = checkAddTemplate(document, monomer_class, alias);
                }
                else
                {
                    auto tmpl_it = _alias_to_id.find(std::make_pair(monomer_class, alias));
                    if (tmpl_it != _alias_to_id.end())
                    {
                        template_id = tmpl_it->second;
                    }
                    else
                    {
                        // unknown monomer - add unresolved template
                        MonomerTemplate monomer_template(MonomerTemplate::MonomerClassToStr(monomer_class) + alias, monomer_class, IdtAlias(), true);
                        setKetStrProp(monomer_template, alias, alias);
                        setKetStrProp(monomer_template, aliasHELM, alias);
                        for (auto ap : {"R1", "R2", "R3", "R4"})
                            monomer_template.AddAttachmentPoint(ap, -1);
                        checkAddTemplate(document, monomer_template);
                        template_id = monomer_template.id();
                    }
                }
            }
            _alias_to_id.emplace(make_pair(monomer_class, alias), template_id);
        }
        auto& monomer = document.addMonomer(helm_alias, template_id);
        monomer->setAttachmentPoints(document.templates().at(template_id).attachmentPoints());
        setKetIntProp(*monomer, seqid, _seq_id++);
        monomer->setPosition(pos);
        if (annotaion.size())
            monomer->setTextAnnotation(annotaion);
    }
    return monomer_idx;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
