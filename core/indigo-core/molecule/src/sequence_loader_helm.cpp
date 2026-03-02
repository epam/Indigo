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

static std::set<std::string> polymer_types{kHELMPolymerTypePEPTIDE, kHELMPolymerTypeRNA, kHELMPolymerTypeCHEM, kHELMPolymerTypeUnknown};
static const char* reserved_helm_chars = "${}|.,-:[]()";
static const char* unexpected_eod = "Unexpected end of data";

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

void SequenceLoader::loadHELM(KetDocument& document)
{
    _row = 0;
    _seq_id = 1;
    std::string simple_polymer_name = "";
    std::string simple_polymer_type = "";
    int monomer_idx = 0;
    int unknown_count = 0;
    size_t last_rna_branch_monomer = 0;
    _unknown_ambiguous_count = 0;
    using polymer_map = std::map<std::string, std::map<int, size_t>>;
    polymer_map used_polymer_nums;
    polymer_map::iterator cur_polymer_map;
    _opts_to_template_id.clear();
    enum class helm_parts
    {
        ListOfSimplePolymers,
        ListOfConnections,
        ListOfPolymerGroups,
        ExtendedAnnotation,
        End
    };
    helm_parts helm_part = helm_parts::ListOfSimplePolymers;
    while (!_scanner.isEOF())
    {
        if (helm_part == helm_parts::ListOfSimplePolymers)
        {
            auto ch = _scanner.lookNext();
            if (simple_polymer_name.size() == 0) // Read simple polymer_name
            {
                _col = 0;
                simple_polymer_type = readHelmSimplePolymerName(simple_polymer_name);
                if (used_polymer_nums.count(simple_polymer_name))
                    throw Error("Simple polymer '%s' defined more than once.", simple_polymer_name.c_str());
                if (simple_polymer_name == simple_polymer_type)
                    throw Error("Polymer '%s' without number not allowed.", simple_polymer_name.c_str());
                ch = _scanner.lookNext();
                if (ch != '{')
                    throw Error(". Expected '{' but found '%c'.", ch);
                _scanner.skip(1); // skip '{'
                if (used_polymer_nums.count(simple_polymer_name))
                    throw Error("Simple polymer '%s' defined more than once.", simple_polymer_name.c_str());
                auto res = used_polymer_nums.emplace(std::make_pair(simple_polymer_name, std::map<int, size_t>()));
                if (res.second)
                    cur_polymer_map = res.first;
                else
                    throw Error("Internal error - cannot emplace polymer map.");
            }
            else if (ch != '}')
            {
                monomer_idx++;
                Vec3f pos(_col * LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, -LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH * _row, 0);
                _col++;
                if (simple_polymer_type == kHELMPolymerTypeUnknown)
                {
                    std::string name;
                    _scanner.readWord(name, reserved_helm_chars);
                    // skip blob for now
                    ch = _scanner.lookNext();
                    if (ch != '}')
                        throw Error("Unexpected symbol. Expected '}' but found '%c'.", ch);
                    continue;
                }
                const auto& monomer_class = MonomerTemplates::getStrToMonomerType().at(simple_polymer_type);
                auto monomer_info = readHelmMonomer(document, monomer_class == MonomerClass::RNA ? MonomerClass::Sugar : monomer_class);
                if (monomer_class == MonomerClass::CHEM)
                {
                    ch = _scanner.lookNext();
                    if (ch != '}')
                        throw Error("Unexpected symbol. Expected '}' but found '%c'.", ch); // only one monomer in chem

                    auto& alias = std::get<0>(monomer_info);
                    if (alias == "*") // if monomer_alias == "*"
                    {
                        alias = "unknown_monomer_" + std::to_string(unknown_count++);
                        MonomerTemplate monomer_template(alias, MonomerClass::CHEM, IdtAlias(alias, alias, alias, alias), true);
                        setKetStrProp(monomer_template, alias, alias);
                        for (auto ap : {"R1", "R2", "R3", "R4"})
                            monomer_template.AddAttachmentPoint(ap, -1);
                        checkAddTemplate(document, monomer_template);
                        _added_templates.emplace(monomer_class, alias);
                        _alias_to_id.emplace(std::make_pair(monomer_class, alias), monomer_template.id());
                    }
                    cur_polymer_map->second[monomer_idx] = addHelmMonomer(document, monomer_info, monomer_class, pos);
                }
                else if (monomer_class == MonomerClass::AminoAcid)
                {
                    auto amino_idx = addHelmMonomer(document, monomer_info, monomer_class, pos);
                    cur_polymer_map->second[monomer_idx] = amino_idx;
                    if (monomer_idx > 1)
                        addMonomerConnection(document, amino_idx - 1, amino_idx);
                    ch = _scanner.lookNext();
                    if (ch == '.')
                        _scanner.skip(1);
                    else if (ch != '}')
                        throw Error("Unexpected symbol. Expected '.' or '}' but found '%c'.", ch);
                }
                else // kHELMPolymerTypeRNA
                {
                    if (std::get<1>(monomer_info)) // branch_monomer
                    {
                        Vec3f base_pos(pos.x, pos.y - LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, 0);
                        auto base_idx = addHelmMonomer(document, monomer_info, MonomerClass::Base, base_pos);
                        cur_polymer_map->second[monomer_idx] = base_idx;
                        if (monomer_idx > 1)
                            addMonomerConnection(document, last_rna_branch_monomer, base_idx, true);
                        continue;
                    }
                    ch = _scanner.lookNext();
                    if (ch == '.' || ch == '}') // single monomer - could be unsplitted RNA or phosphate or sugar
                    {
                        size_t added_idx;
                        auto& monomer_alias = std::get<0>(monomer_info);
                        auto tmpl_it = _alias_to_id.find(std::make_pair(MonomerClass::Sugar, monomer_alias));
                        // if found sugar id - add sugar, else if found phosphate id - add phosphate, else - add unsplitted RNA(may be unresolved)
                        if (tmpl_it != _alias_to_id.end() || _library.getMonomerTemplateIdByAlias(MonomerClass::Sugar, monomer_alias).size() > 0 ||
                            _library.getMonomerTemplateIdByAliasHELM(MonomerClass::Sugar, monomer_alias).size() > 0)
                            added_idx = addHelmMonomer(document, monomer_info, MonomerClass::Sugar, pos);
                        else if (_library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, monomer_alias).size() > 0 ||
                                 _library.getMonomerTemplateIdByAliasHELM(MonomerClass::Phosphate, monomer_alias).size() > 0)
                            added_idx = addHelmMonomer(document, monomer_info, MonomerClass::Phosphate, pos);
                        else
                            added_idx = addHelmMonomer(document, monomer_info, MonomerClass::RNA, pos);

                        cur_polymer_map->second[monomer_idx] = added_idx;
                        if (monomer_idx > 1)
                            addMonomerConnection(document, last_rna_branch_monomer, added_idx);
                        last_rna_branch_monomer = added_idx;
                        if (ch == '.')
                            _scanner.skip(1);
                        continue;
                    }
                    auto sugar_idx = addHelmMonomer(document, monomer_info, MonomerClass::Sugar, pos);
                    cur_polymer_map->second[monomer_idx] = sugar_idx;
                    if (monomer_idx > 1)
                        addMonomerConnection(document, last_rna_branch_monomer, sugar_idx);
                    last_rna_branch_monomer = sugar_idx;
                    monomer_idx++;
                    if (ch == '(') // base after sugar
                    {
                        auto base_info = readHelmMonomer(document, MonomerClass::Base);
                        Vec3f base_pos(pos.x, pos.y - LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, 0);
                        auto base_idx = addHelmMonomer(document, base_info, MonomerClass::Base, base_pos);
                        cur_polymer_map->second[monomer_idx] = base_idx;
                        addMonomerConnection(document, sugar_idx, base_idx, true);
                    }
                    ch = _scanner.lookNext();
                    if (ch == '.') // end of nucleo
                    {
                        _scanner.skip(1);
                        continue;
                    }
                    if (ch == '}')
                        continue;
                    auto phosphate_info = readHelmMonomer(document, MonomerClass::Phosphate);
                    monomer_idx++;
                    Vec3f phosphate_pos(_col * LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, -LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH * _row, 0);
                    _col++;
                    auto phosphate_idx = addHelmMonomer(document, phosphate_info, MonomerClass::Phosphate, phosphate_pos);
                    cur_polymer_map->second[monomer_idx] = phosphate_idx;
                    addMonomerConnection(document, sugar_idx, phosphate_idx);
                    last_rna_branch_monomer = phosphate_idx;
                    ch = _scanner.lookNext();
                    if (ch != '.' && ch != '}')
                        throw Error("Unexpected symbol. Expected '.' or '}' but found '%c'.", ch);
                    if (ch == '.')
                        _scanner.skip(1);
                }
            }
            else // end of polymer - }
            {
                _scanner.skip(1); // skip '}'
                ch = _scanner.lookNext();
                if (ch == '"')
                {
                    Array<char> annotation;
                    _scanner.skip(1);
                    _scanner.readWord(annotation, "\"");
                    if (_scanner.lookNext() != '"')
                        throw Error("Unexpected symbol. Expected '\"' but found '%c'.", _scanner.lookNext());
                    _scanner.skip(1);
                    // skip annotation for now
                    // 2do add annotation
                    // add monomer group
                    // add annotation
                    ch = _scanner.lookNext();
                }
                _row++;
                _col = 0;
                monomer_idx = 0;
                if (simple_polymer_type == kHELMPolymerTypeRNA)
                    _row++; // additional row for bases in RNA
                if (ch == '|')
                {
                    // cleanup to go to next simple polymer
                    simple_polymer_name = "";
                    simple_polymer_type = "";
                }
                else if (ch == '$')
                {
                    helm_part = helm_parts::ListOfConnections;
                }
                else if (ch == -1)
                {
                    throw Error(unexpected_eod);
                }
                else
                {
                    throw Error("Unexpected symbol. Expected '|' or '$' but found '%c'.", ch);
                }
                _scanner.skip(1);
            }
        }
        else if (helm_part == helm_parts::ListOfConnections)
        {
            auto ch = _scanner.lookNext();
            if (ch == '$')
            {
                helm_part = helm_parts::ListOfPolymerGroups;
                _scanner.skip(1);
                continue;
            }
            // CHEM1,RNA1,32:R1-12:R2"annotation"|.....
            std::string left_polymer, right_polymer;
            std::ignore = readHelmSimplePolymerName(left_polymer);
            auto left_polymer_nums = used_polymer_nums.find(left_polymer);
            if (left_polymer_nums == used_polymer_nums.end())
                throw Error("Polymer '%s' not found.", left_polymer.c_str());
            ch = _scanner.lookNext();
            if (ch != ',')
                throw Error("Unexpected symbol. Expected ',' but found '%c'.", _scanner.lookNext());
            _scanner.skip(1);
            std::ignore = readHelmSimplePolymerName(right_polymer);
            auto right_polymer_nums = used_polymer_nums.find(right_polymer);
            if (right_polymer_nums == used_polymer_nums.end())
                throw Error("Polymer '%s' not found.", right_polymer.c_str());
            ch = _scanner.lookNext();
            if (ch != ',')
                throw Error("Unexpected symbol. Expected ',' but found '%c'.", _scanner.lookNext());
            _scanner.skip(1);
            // read monomer position
            int left_monomer_idx, right_monomer_idx;
            std::string left_ap, right_ap;
            std::string position;
            size_t error_pos;
            _scanner.readWord(position, ":");
            _scanner.skip(1);
            left_monomer_idx = std::stoi(position, &error_pos);
            if (error_pos != position.size())
                throw Error("Only direct connections supported now.");
            _scanner.readWord(left_ap, "-");
            _scanner.skip(1);
            position.clear();
            _scanner.readWord(position, ":");
            _scanner.skip(1);
            right_monomer_idx = std::stoi(position, &error_pos);
            if (error_pos != position.size())
                throw Error("Only direct connections supported now.");
            _scanner.readWord(right_ap, "\"|$");
            auto left_mon_it = left_polymer_nums->second.find(left_monomer_idx);
            if (left_mon_it == left_polymer_nums->second.end())
                throw Error("Polymer '%s' does not contains monomer with number %d.", left_polymer.c_str(), left_monomer_idx);
            auto right_mon_it = right_polymer_nums->second.find(right_monomer_idx);
            if (right_mon_it == right_polymer_nums->second.end())
                throw Error("Polymer '%s' does not contains monomer with number %d.", right_polymer.c_str(), right_monomer_idx);
            auto& connection = document.addConnection(document.monomers().at(std::to_string(left_mon_it->second))->ref(), left_ap,
                                                      document.monomers().at(std::to_string(right_mon_it->second))->ref(), right_ap);
            if (_scanner.isEOF())
                throw Error(unexpected_eod);
            ch = _scanner.lookNext();
            if (ch == '"')
            {
                _scanner.skip(1);
                std::string annotation;
                _scanner.readWord(annotation, "\"");
                if (_scanner.isEOF())
                    throw Error(unexpected_eod);
                if (_scanner.lookNext() != '"')
                    throw Error("Unexpected symbol. Expected '\"' but found '%c'.", _scanner.lookNext());
                _scanner.skip(1); // skip '"'
                if (_scanner.isEOF())
                    throw Error(unexpected_eod);
                connection.setTextAnnotation(annotation);
                ch = _scanner.lookNext();
            }
            if (ch == '|')
                _scanner.skip(1);
            else if (ch != '$')
                throw Error("Unexpected symbol. Expected '|' or '$' but found '%c'.", _scanner.lookNext());
        }
        else if (helm_part == helm_parts::ListOfPolymerGroups)
        {
            std::string groups;
            _scanner.readWord(groups, "$");
            // skip groups for now
            helm_part = helm_parts::ExtendedAnnotation;
            _scanner.skip(1);
        }
        else // helm_parts::ExtendedAnnotation
        {
            // read rest of data
            std::string rest_of_helm;
            _scanner.readAll(rest_of_helm);
            auto it = rest_of_helm.find_last_of('$');
            if (it == rest_of_helm.npos)
                throw Error("Incorrect format. Last '$' not found.");
            // read annotation
            if (it > 0)
            {
                std::string annotation = rest_of_helm.substr(0, it);
                rapidjson::Document data;
                auto& ann = document.addAnnotation();
                auto& extended = data.Parse(annotation.c_str());
                ann->setExtended(extended);
            }
            std::string signature = rest_of_helm.substr(it + 1);
            // split by last '$' and check if right part eq “V2.0”
            // if (signature != "v2.0")
            //     throw Error("Expected HELM V2.0 but got '%s'.", signature.c_str());
            // check that left part is valid json - TODO
            helm_part = helm_parts::End;
        }
    }
    if (helm_part != helm_parts::End)
        throw Error(unexpected_eod);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
