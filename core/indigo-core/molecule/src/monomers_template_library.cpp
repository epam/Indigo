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

#include <functional>
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/base_cpp/scanner.h"
#include "layout/molecule_layout.h"

#include "molecule/elements.h"
#include "molecule/ket_document_json_loader.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_basic_templates.h"
#include "molecule/monomers_lib.h"
#include "molecule/monomers_template_library.h"
#include "molecule/smiles_loader.h"

namespace indigo
{
    using template_add_func = std::function<MonomerTemplate&(const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved)>;
    static std::string EMPTY_STRING;

    IMPL_ERROR(MonomerTemplate, "MonomerTemplate");

    const std::map<std::string, int>& MonomerTemplate::getStringPropStrToIdx() const
    {
        static std::map<std::string, int> str_to_idx{
            {"alias", toUType(StringProps::alias)},
            {"classHELM", toUType(StringProps::classHELM)},
            {"fullName", toUType(StringProps::fullName)},
            {"naturalAnalog", toUType(StringProps::naturalAnalog)},
            {"naturalAnalogShort", toUType(StringProps::naturalAnalogShort)},
            {"aliasHELM", toUType(StringProps::aliasHELM)},
            {"aliasAxoLabs", toUType(StringProps::aliasAxoLabs)},
        };
        return str_to_idx;
    };

    size_t MonomerTemplate::AddAtom(const std::string& label, Vec3f location)
    {
        _atoms.push_back(std::make_unique<KetAtom>(label));
        (*_atoms.rbegin())->setLocation(location);
        return _atoms.size() - 1;
    }

    size_t MonomerTemplate::AddBond(int bond_type, int atom1, int atom2)
    {
        _bonds.emplace_back(bond_type, atom1, atom2);
        return _bonds.size() - 1;
    }

    KetAttachmentPoint& MonomerTemplate::AddAttachmentPoint(const std::string& label, int att_atom)
    {
        std::string ap_id = label.size() != 0 ? label : "R" + std::to_string(1 + _attachment_points.size());
        auto& ap = AddAttachmentPointId(ap_id, att_atom);
        if (label.size())
            setKetStrProp(ap, label, label);
        return ap;
    }

    KetAttachmentPoint& MonomerTemplate::AddAttachmentPointId(const std::string& id, int att_atom)
    {
        auto it = _attachment_points.emplace(id, att_atom);
        return it.first->second;
    }

    void MonomerTemplate::addSuperatomAttachmentPoints(const Superatom& sa)
    {
        std::map<std::string, int> sorted_attachment_points;
        if (sa.attachment_points.size())
        {
            for (int i = sa.attachment_points.begin(); i != sa.attachment_points.end(); i = sa.attachment_points.next(i))
            {
                auto& atp = sa.attachment_points[i];
                std::string atp_id_str(atp.apid.ptr());
                if (atp_id_str.size())
                    sorted_attachment_points.insert(std::make_pair(atp_id_str, i));
            }

            if (sorted_attachment_points.size())
            {
                int order = 0;
                for (const auto& kvp : sorted_attachment_points)
                {
                    auto& atp = sa.attachment_points[kvp.second];
                    auto& atp_ket = AddAttachmentPointId(kvp.first, atp.aidx);
                    std::vector<int> lgrp{{atp.lvidx}};
                    atp_ket.setLeavingGroup(lgrp);
                    if (!isAttachmentPointsInOrder(order++, kvp.first))
                    {
                        if (kvp.first == kLeftAttachmentPoint || kvp.first == kAttachmentPointR1)
                            setKetStrProp(atp_ket, type, "left");
                        else if (kvp.first == kRightAttachmentPoint || kvp.first == kAttachmentPointR2)
                            setKetStrProp(atp_ket, type, "right");
                        else
                            setKetStrProp(atp_ket, type, "side");
                        setKetStrProp(atp_ket, label, convertAPToHELM(kvp.first));
                    }
                }
            }
        }
    }

    // const MonomerAttachmentPoint& MonomerTemplate::getAttachmenPointById(const std::string& att_point_id)
    // {
    //     if (!hasAttachmenPointWithId(att_point_id))
    //         throw Error("Attachment point with id %s not found in monomer template %s.", att_point_id.c_str(), _id.c_str());
    //     return _attachment_points.at(att_point_id);
    // }

    std::unique_ptr<TGroup> MonomerTemplate::getTGroup(bool for_smiles) const
    {
        auto tgroup = std::make_unique<TGroup>();
        // save template to ket
        rapidjson::StringBuffer string_buffer;
        auto writer_ptr = JsonWriter::createJsonWriter(false);
        JsonWriter& writer = *writer_ptr;
        writer.Reset(string_buffer);
        writer.StartObject();
        if (for_smiles)
        { // Replace leaving groups with RSites
            MonomerTemplate tmpl(_id, _monomer_class, IdtAlias(), _unresolved);
            tmpl.copy(*this);
            for (auto att_point : _attachment_points)
            {
                std::string label = getKetStrProp(att_point.second, label);
                label.replace(0, 1, "rg-");
                auto& leaving = att_point.second.leavingGroup();
                if (leaving.has_value())
                {
                    for (auto atom : leaving.value())
                    {
                        tmpl._atoms[atom] = std::make_unique<KetRgLabel>();
                        auto* atom_ptr = tmpl._atoms[atom].get();
                        KetRgLabel* r_ptr = static_cast<KetRgLabel*>(atom_ptr);
                        std::vector<std::string> ref_list;
                        ref_list.emplace_back(label);
                        r_ptr->setRefs(ref_list);
                    }
                }
            }
            KetDocumentJsonSaver::saveMonomerTemplate(writer, tmpl);
        }
        else
        {
            KetDocumentJsonSaver::saveMonomerTemplate(writer, *this);
        }
        writer.EndObject();
        std::string ket(string_buffer.GetString());
        // read TGroup
        rapidjson::Document document;
        document.Parse(ket.c_str());
        auto template_id = _ref;
        auto& mon_template = document[template_id.c_str()];
        Molecule mol;
        QueryMolecule qmol;
        BaseMolecule* bmol = &mol;
        StereocentersOptions stereo_opt;
        stereo_opt.ignore_errors = true;
        int idx = -1;
        try
        {
            idx = MoleculeJsonLoader::parseMonomerTemplate(mon_template, mol, stereo_opt);
        }
        catch (MoleculeJsonLoader::Error&) // try as query molecule
        {
            idx = MoleculeJsonLoader::parseMonomerTemplate(mon_template, qmol, stereo_opt);
            bmol = &qmol;
        }

        auto& tg = bmol->tgroups.getTGroup(idx);
        if (_unresolved)
        {
            tg.unresolved = _unresolved;
            auto& sa = (Superatom&)tg.fragment->sgroups.getSGroup(0, SGroup::SG_TYPE_SUP);
            sa.unresolved = _unresolved;
        }
        tgroup->copy(tg);

        return tgroup;
    }

    IMPL_ERROR(MonomerGroupTemplate, "MonomerGroupTemplate");

    void MonomerGroupTemplate::addTemplate(MonomerTemplateLibrary& library, const std::string& template_id)
    {
        auto [it, inserted] = _monomer_templates.emplace(template_id, std::cref(library.getMonomerTemplateById(template_id)));
        if (inserted)
            _template_ids.push_back(template_id);
    };

    const MonomerTemplate& MonomerGroupTemplate::getTemplateByClass(MonomerClass monomer_class) const
    {
        for (auto& id_template : _monomer_templates)
        {
            if (id_template.second.get().monomerClass() == monomer_class)
                return id_template.second;
        }
        throw Error("Monomer template with class %s not found in monomer temmplate group %s.", MonomerTemplate::MonomerClassToStr(monomer_class).c_str(),
                    _id.c_str());
    }

    bool MonomerGroupTemplate::hasTemplate(MonomerClass monomer_class, const std::string monomer_id) const
    {
        for (auto& id_template : _monomer_templates)
        {
            if (id_template.second.get().monomerClass() == monomer_class) // for now only one monomer of each class
                return id_template.second.get().id() == monomer_id;
        }
        return false;
    }

    bool MonomerGroupTemplate::hasTemplate(MonomerClass monomer_class) const
    {
        for (auto& id_template : _monomer_templates)
        {
            if (id_template.second.get().monomerClass() == monomer_class)
                return true;
        }
        return false;
    }

    bool MonomerGroupTemplate::hasIdtAlias(const std::string& alias, IdtModification mod)
    {
        if (_idt_alias.hasModification(mod) && (_idt_alias.getModification(mod) == alias))
            return true;
        return false;
    }

    bool MonomerGroupTemplate::hasIdtAliasBase(const std::string& alias_base)
    {
        if (_idt_alias.getBase() == alias_base)
            return true;
        return false;
    }

    bool MonomerGroupTemplate::isValid() const
    {
        if (_connections.empty())
            return true;

        // map template id to index in _template_ids for quick lookup
        std::unordered_map<std::string, int> template_id_to_idx;
        template_id_to_idx.reserve(_template_ids.size());
        for (int i = 0; i < static_cast<int>(_template_ids.size()); ++i)
            template_id_to_idx.emplace(_template_ids[i], i);

        auto resolve_template_idx = [&template_id_to_idx](const KetConnectionEndPoint& endpoint) -> int {
            if (!hasKetStrProp(endpoint, templateId))
                return -1;

            const auto& endpoint_template_id = getKetStrProp(endpoint, templateId);
            if (auto it = template_id_to_idx.find(endpoint_template_id); it != template_id_to_idx.end())
                return it->second;

            if (endpoint_template_id.rfind(MonomerTemplate::ref_prefix, 0) == 0)
            {
                auto it = template_id_to_idx.find(endpoint_template_id.substr(MonomerTemplate::ref_prefix.size()));
                if (it != template_id_to_idx.end())
                    return it->second;
            }

            return -1;
        };

        std::vector<std::vector<int>> adjacency(_template_ids.size());
        for (const auto& connection : _connections)
        {
            int ep1_idx = resolve_template_idx(connection.ep1());
            int ep2_idx = resolve_template_idx(connection.ep2());
            if (ep1_idx < 0 || ep2_idx < 0)
                return false;

            if (ep1_idx == ep2_idx)
                continue;

            adjacency[ep1_idx].push_back(ep2_idx);
            adjacency[ep2_idx].push_back(ep1_idx);
        }

        // dfs to check connectivity
        std::vector<bool> visited(_template_ids.size(), false);
        std::vector<int> stack{0};
        int visited_count = 1;
        visited[0] = true;

        while (!stack.empty())
        {
            int current = stack.back();
            stack.pop_back();

            for (int next : adjacency[current])
            {
                if (visited[next])
                    continue;

                visited[next] = true;
                ++visited_count; // count visited templates
                stack.push_back(next);
            }
        }

        return visited_count == static_cast<int>(_template_ids.size());
    }

    IMPL_ERROR(MonomerTemplateLibrary, "MonomerTemplateLibrary");

    MonomerTemplate& MonomerTemplateLibrary::addMonomerTemplate(TGroup& tg, const IdtAlias& idt_alias)
    {
        if (tg.tgroup_name.ptr() && tg.tgroup_class.ptr())
        {
            template_add_func add_new_template;
            std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
            std::string alias(monomerAlias(tg));
            std::optional<std::reference_wrapper<MonomerTemplate>> mt_ref;
            auto lib_id = getMonomerTemplateIdByAlias(MonomerTemplate::StrToMonomerClass(template_class), alias);
            if (lib_id.size() > 0)
            {
                // monomer with same name and class - check inchi
                mt_ref = _monomer_templates.at(lib_id);
                auto inchi_key_lib = monomerInchi(*mt_ref->get().getTGroup());
                auto inchi_key_new = monomerInchi(tg);
                if (inchi_key_new != inchi_key_lib)
                    throw Error("Monomer %s with alias %s and same structure already in the library.", template_class.c_str(), alias.c_str());
            }
            else
            {
                // new monomer
                auto id = monomerTemplateId(tg);
                // check if id is unique
                if (_monomer_templates.find(id) != _monomer_templates.end())
                {
                    if (_duplicate_names_count.count(id) == 0)
                        _duplicate_names_count.emplace(id, 0);
                    do
                    {
                        id = id + "_" + std::to_string(++_duplicate_names_count[id]);
                    } while (_monomer_templates.find(id) != _monomer_templates.end());
                }
                add_new_template = [this, id, template_class, idt_alias, &mt_ref](const std::string&, const std::string&, IdtAlias, bool) -> MonomerTemplate& {
                    auto& result = this->addMonomerTemplate(id, template_class, idt_alias, false);
                    mt_ref = result;
                    return result;
                };

                auto jwriter_ptr = JsonWriter::createJsonDocumentWriter();
                DocumentJsonWriter& jwriter = static_cast<DocumentJsonWriter&>(*jwriter_ptr);
                NullOutput null_out;
                MoleculeJsonSaver json_saver(null_out);
                json_saver.saveMonomerTemplate(tg, jwriter);
                auto& doc = jwriter.GetDocument();

                // Debug output
                KetDocumentJsonLoader::parseMonomerTemplate(doc, add_new_template);
            }

            auto& mt = mt_ref->get();
            if (!tg.tgroup_full_name.size())
                setKetStrProp(mt, fullName, tg.tgroup_name.ptr());
            return mt;
        }
        throw Error("TGroup should have name and class to be converted to monomer template.");
    }

    MonomerTemplate& MonomerTemplateLibrary::addMonomerTemplate(const std::string& id, const std::string& monomer_class, const IdtAlias& idt_alias,
                                                                bool unresolved)
    {
        auto res = _monomer_templates.try_emplace(id, id, monomer_class, idt_alias, unresolved);
        if (!res.second)
            throw Error("Monomer template '%s' already exists", id.c_str());
        for (auto modification : {IdtModification::FIVE_PRIME_END, IdtModification::INTERNAL, IdtModification::THREE_PRIME_END})
        {
            if (res.first->second.idtAlias().hasModification(modification))
            {
                const std::string& alias = res.first->second.idtAlias().getModification(modification);
                _id_alias_to_monomer_templates.emplace(alias, std::make_pair(std::ref(res.first->second), modification));
            }
        }
        return res.first->second;
    }

    const MonomerTemplate& MonomerTemplateLibrary::getMonomerTemplateById(const std::string& monomer_template_id) const
    {
        if (_monomer_templates.count(monomer_template_id) == 0)
            throw Error("Monomer template with id %s not found.", monomer_template_id.c_str());
        return _monomer_templates.at(monomer_template_id);
    }

    const std::string& MonomerTemplateLibrary::getMonomerTemplateIdByAlias(MonomerClass monomer_class, const std::string& monomer_template_alias)
    {
        for (auto& it : _monomer_templates)
        {
            if (it.second.monomerClass() == monomer_class && hasKetStrProp(it.second, alias) && getKetStrProp(it.second, alias) == monomer_template_alias)
                return it.second.id();
        }
        return EMPTY_STRING;
    }

    const std::string& MonomerTemplateLibrary::getMonomerTemplateIdByAliasHELM(MonomerClass monomer_class, const std::string& alias)
    {
        for (auto& it : _monomer_templates)
        {
            if (it.second.monomerClass() == monomer_class && hasKetStrProp(it.second, aliasHELM) && getKetStrProp(it.second, aliasHELM) == alias)
                return it.second.id();
        }
        return EMPTY_STRING;
    }

    const std::string& MonomerTemplateLibrary::getMonomerTemplateIdByAliasAxoLabs(const std::string& alias)
    {
        for (auto& it : _monomer_templates)
        {
            if (hasKetStrProp(it.second, aliasAxoLabs) && getKetStrProp(it.second, aliasAxoLabs) == alias)
                return it.second.id();
        }
        return EMPTY_STRING;
    }

    MonomerGroupTemplate& MonomerTemplateLibrary::getMonomerGroupTemplateById(const std::string& monomer_group_template_id)
    {
        if (_monomer_group_templates.count(monomer_group_template_id) == 0)
            throw Error("Monomert group template with id %s not found.", monomer_group_template_id.c_str());
        return _monomer_group_templates.at(monomer_group_template_id);
    }

    const std::string& MonomerTemplateLibrary::getMonomerTemplateIdByIdtAliasBase(const std::string& alias_base)
    {
        for (auto& monomer_template : _monomer_templates)
        {
            if (monomer_template.second.hasIdtAliasBase(alias_base))
                return monomer_template.first;
        };
        return EMPTY_STRING;
    };

    const std::string& MonomerTemplateLibrary::getMGTidByIdtAliasBase(const std::string& alias_base)
    {
        for (auto& mgt : _monomer_group_templates)
        {
            if (mgt.second.hasIdtAliasBase(alias_base))
                return mgt.first;
        };
        return EMPTY_STRING;
    };

    const std::string& MonomerTemplateLibrary::getMonomerTemplateIdByIdtAlias(const std::string& alias, IdtModification& mod)
    {
        if (auto it = _id_alias_to_monomer_templates.find(alias); it != _id_alias_to_monomer_templates.end())
        {
            mod = it->second.second;
            return it->second.first.id();
        }
        return EMPTY_STRING;
    };

    const std::string& MonomerTemplateLibrary::getMGTidByIdtAlias(const std::string& alias, IdtModification& mod)
    {
        if (auto it = _idt_alias_to_monomer_group_templates.find(alias); it != _idt_alias_to_monomer_group_templates.end())
        {
            mod = it->second.second;
            return it->second.first.id();
        }
        return EMPTY_STRING;
    };

    const std::string& MonomerTemplateLibrary::getMGTidByAliasAxoLabs(const std::string& alias)
    {
        for (auto& it : _monomer_group_templates)
        {
            auto axolabs_alias = it.second.aliasAxoLabs();
            if (axolabs_alias.has_value() && *axolabs_alias == alias)
                return it.second.id();
        }
        return EMPTY_STRING;
    };

    const std::string& MonomerTemplateLibrary::getMGTidByComponents(const std::string sugar_id, const std::string base_id, const std::string phosphate_id)
    {
        for (auto& mgt : _monomer_group_templates)
        {
            if (!mgt.second.hasTemplate(MonomerClass::Sugar, sugar_id))
                continue;
            if (!mgt.second.hasTemplate(MonomerClass::Phosphate, phosphate_id))
                continue;
            if (base_id.size())
            {
                if (!mgt.second.hasTemplate(MonomerClass::Base, base_id))
                    continue;
            }
            else // If no base - group template should not contain base template
            {
                if (mgt.second.hasTemplate(MonomerClass::Base))
                    continue;
            }
            return mgt.second.id();
        }
        return EMPTY_STRING;
    }

    const std::string& MonomerTemplateLibrary::getIdtAliasByModification(IdtModification modification, const std::string sugar_id, const std::string base_id,
                                                                         const std::string phosphate_id)
    {
        for (auto& mgt : _monomer_group_templates)
        {
            if (mgt.second.idtAlias().hasModification(modification))
            {
                if (!mgt.second.hasTemplate(MonomerClass::Sugar, sugar_id))
                    continue;
                if (modification != IdtModification::THREE_PRIME_END)
                {
                    if (!mgt.second.hasTemplate(MonomerClass::Phosphate, phosphate_id))
                        continue;
                }
                if (base_id.size())
                {
                    if (!mgt.second.hasTemplate(MonomerClass::Base, base_id))
                        continue;
                }
                else // If no base - group template should not contain base template
                {
                    if (mgt.second.hasTemplate(MonomerClass::Base))
                        continue;
                }
                return mgt.second.idtAlias().getModification(modification);
            }
        }
        return EMPTY_STRING;
    }

    void MonomerTemplateLibrary::addMonomersFromMolecule(Molecule& mol, PropertiesMap& properties)
    {
        // read common properties values first
        std::string mon_type = "monomerTemplate", alias_helm;
        IdtAlias idt_alias;
        if (properties.contains("type"))
            mon_type = properties.at("type");
        if (properties.contains("aliasHELM"))
            alias_helm = properties.at("aliasHELM");
        // read idtAliases
        if (properties.contains("idtAliases"))
        {
            auto idt_alias_parts = split(properties.at("idtAliases"), ';');
            if (idt_alias_parts.size() == 1)
                idt_alias_parts = split(properties.at("idtAliases"), ',');
            for (const auto& idt_alias_str : idt_alias_parts)
            {
                auto kvp_vec = split(idt_alias_str, '=');
                if (kvp_vec.size() == 2)
                {
                    if (kvp_vec[0] == "base")
                        idt_alias.setBase(kvp_vec[1]);
                    else if (kvp_vec[0] == "ep5")
                        idt_alias.setModification(IdtModification::FIVE_PRIME_END, kvp_vec[1]);
                    else if (kvp_vec[0] == "ep3")
                        idt_alias.setModification(IdtModification::THREE_PRIME_END, kvp_vec[1]);
                    else if (kvp_vec[0] == "internal")
                        idt_alias.setModification(IdtModification::INTERNAL, kvp_vec[1]);
                }
            }
        }

        // single monomer template
        if (mon_type == "monomerTemplate")
        {
            std::string modification_types;
            // read monomer template specific properties
            if (properties.contains("modificationTypes"))
                modification_types = properties.at("modificationTypes");
            if (mol.tgroups.getTGroupCount() == 1)
            {
                try
                {
                    if (properties.contains("templateId"))
                        mol.tgroups.getTGroup(0).tgroup_text_id.readString(properties.at("templateId"), true);
                    auto& mt = addMonomerTemplate(mol.tgroups.getTGroup(0), idt_alias);
                    if (properties.contains("templateRef"))
                        mt.setRef(properties.at("templateRef"));
                    if (modification_types.size())
                    {
                        for (auto& modification_type : split(modification_types, ';'))
                            mt.addModificationType(modification_type);
                    }
                    if (alias_helm.size())
                    {
                        setKetStrProp(mt, aliasHELM, alias_helm);
                    }
                }
                catch (const Error& /* e */)
                {
                    // just suppress the error here
                    // throw Error("Error adding monomer template from molecule: %s", e.message());
                }
            }
            else
                throw Error("Molecule should contain exactly one TGroup to be converted to monomer template.");
        }
        else // add multiple monomer templates
        {
            std::optional<std::reference_wrapper<MonomerGroupTemplate>> mgt;
            std::unordered_map<int, std::string> template_id_by_tgroup_idx;
            std::unordered_map<std::pair<std::string, std::string>, std::string, pair_hash> template_id_by_name_class;
            if (mon_type == "monomerGroupTemplate")
            {
                std::string group_class, group_name, group_id;
                // mandatory fields
                if (properties.contains("groupClass") && properties.contains("groupName"))
                {
                    group_class = properties.at("groupClass");
                    group_name = properties.at("groupName");
                    group_id = properties.contains("groupId") ? properties.at("groupId") : group_name;
                    addMonomerGroupTemplate(
                        MonomerGroupTemplate(group_id, group_name, group_class, idt_alias.hasModifications() ? idt_alias : idt_alias.getBase()));
                    mgt = getMonomerGroupTemplateById(group_id);
                    if (properties.contains("groupRef"))
                        mgt->get().setRef(properties.at("groupRef"));
                    if (properties.contains("aliasAxoLabs"))
                        mgt->get().setAliasAxoLabs(properties.at("aliasAxoLabs"));
                }
            }
            for (int i = mol.tgroups.begin(); i != mol.tgroups.end(); i = mol.tgroups.next(i))
            {
                try
                {
                    auto& tg = mol.tgroups.getTGroup(i);
                    auto& mt = addMonomerTemplate(tg, IdtAlias());
                    template_id_by_tgroup_idx.emplace(i, mt.id());
                    if (tg.tgroup_name.size() && tg.tgroup_class.size())
                        template_id_by_name_class.emplace(std::make_pair(std::string(tg.tgroup_name.ptr()), std::string(tg.tgroup_class.ptr())), mt.id());
                    if (tg.tgroup_alias.size() && tg.tgroup_class.size())
                        template_id_by_name_class.emplace(std::make_pair(std::string(tg.tgroup_alias.ptr()), std::string(tg.tgroup_class.ptr())), mt.id());
                    if (mgt.has_value())
                        mgt->get().addTemplate(*this, mt.id());
                }
                catch (const Error& /* e */) // ignore monomer if already exists
                {
                }
            }

            if (mgt.has_value())
            {
                auto resolve_template_id = [this, &mol, &template_id_by_tgroup_idx, &template_id_by_name_class](int atom_idx) -> std::string {
                    int tg_idx = mol.getTemplateAtomTemplateIndex(atom_idx);
                    auto tg_it = template_id_by_tgroup_idx.find(tg_idx);
                    if (tg_it != template_id_by_tgroup_idx.end())
                        return tg_it->second;

                    auto* atom_class = mol.getTemplateAtomClass(atom_idx);
                    std::string alias = mol.getTemplateAtom(atom_idx);
                    std::string class_name = atom_class ? atom_class : "";
                    auto name_class_it = template_id_by_name_class.find(std::make_pair(alias, class_name));
                    if (name_class_it != template_id_by_name_class.end())
                        return name_class_it->second;

                    auto resolved = getMonomerTemplateIdByAlias(MonomerTemplate::StrToMonomerClass(atom_class ? atom_class : ""), alias);
                    if (resolved.size())
                        return resolved;

                    if (tg_idx >= 0 && tg_idx < mol.tgroups.getTGroupCount())
                    {
                        auto& tg = mol.tgroups.getTGroup(tg_idx);
                        auto [it, inserted] = template_id_by_tgroup_idx.emplace(tg_idx, monomerId(tg));
                        return it->second;
                    }

                    throw Error("Template atom '%s' can not be resolved to monomer template id.", alias.c_str());
                };

                auto set_attachment_point = [&mol](int atom_idx, int other_idx, KetConnectionEndPoint& endpoint) {
                    Array<char> ap_id;
                    for (int order = 0; order < mol.getTemplateAtomAttachmentPointsCount(atom_idx); ++order)
                    {
                        if (mol.getTemplateAtomAttachmentPoint(atom_idx, order) != other_idx)
                            continue;

                        mol.getTemplateAtomAttachmentPointId(atom_idx, order, ap_id);
                        setKetStrProp(endpoint, attachmentPointId, convertAPToHELM(ap_id.ptr()));
                        return;
                    }
                };

                auto get_connection_type = [&mol](int bond_idx) {
                    auto bond_order = mol.getBondOrder(bond_idx);
                    if (bond_order == BOND_SINGLE)
                        return KetConnectionSingle;
                    if (bond_order == _BOND_HYDROGEN)
                        return KetConnectionHydro;
                    throw Error("Unsupported bond type %d in monomer group template.", bond_order);
                };

                const auto& bond_annotations = mol.getBondAnnotations();
                for (auto bond_idx : mol.edges())
                {
                    const auto& edge = mol.getEdge(bond_idx);
                    if (!mol.isTemplateAtom(edge.beg) || !mol.isTemplateAtom(edge.end))
                        continue;

                    KetConnectionEndPoint ep1;
                    KetConnectionEndPoint ep2;
                    setKetStrProp(ep1, templateId, resolve_template_id(edge.beg));
                    setKetStrProp(ep2, templateId, resolve_template_id(edge.end));
                    set_attachment_point(edge.beg, edge.end, ep1);
                    set_attachment_point(edge.end, edge.beg, ep2);

                    auto& connection = mgt->get().addConnection(get_connection_type(bond_idx), ep1, ep2);
                    if (bond_annotations.count(bond_idx))
                        connection.setAnnotation(bond_annotations.at(bond_idx));
                }

                if (!mgt->get().isValid())
                    throw Error("Monomer template group %s has disconnected templates.", mgt->get().id().c_str());
            }
        }
    }
}
