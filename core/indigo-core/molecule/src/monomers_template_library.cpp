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
#include "molecule/json_writer.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_basic_templates.h"
#include "molecule/monomers_lib.h"
#include "molecule/monomers_template_library.h"
#include "molecule/smiles_loader.h"

namespace indigo
{
    static std::string EMPTY_STRING;

    IMPL_ERROR(MonomerTemplateAttachmentPoint, "MonomerTemplateAttachmentPoint");

    const std::map<std::string, int>& MonomerTemplateAttachmentPoint::getStringPropStrToIdx() const
    {
        static std::map<std::string, int> str_to_idx{
            {"label", toUType(StringProps::label)},
            {"type", toUType(StringProps::type)},
        };
        return str_to_idx;
    }
    IMPL_ERROR(MonomerTemplate, "MonomerTemplate");

    const std::map<std::string, int>& MonomerTemplate::getStringPropStrToIdx() const
    {
        static std::map<std::string, int> str_to_idx{
            {"alias", toUType(StringProps::alias)},
            {"classHELM", toUType(StringProps::classHELM)},
            {"fullName", toUType(StringProps::fullName)},
            {"naturalAnalog", toUType(StringProps::naturalAnalog)},
            {"naturalAnalogShort", toUType(StringProps::naturalAnalogShort)},
        };
        return str_to_idx;
    };

    MonomerTemplateAttachmentPoint& MonomerTemplate::AddAttachmentPoint(const std::string& label, int att_atom)
    {
        std::string ap_id = label.size() != 0 ? label : "R" + std::to_string(1 + _attachment_points.size());
        auto it = _attachment_points.emplace(ap_id, att_atom);
        if (label.size())
            it.first->second.setStringProp("label", label);
        return it.first->second;
    }

    // const MonomerAttachmentPoint& MonomerTemplate::getAttachmenPointById(const std::string& att_point_id)
    // {
    //     if (!hasAttachmenPointWithId(att_point_id))
    //         throw Error("Attachment point with id %s not found in monomer template %s.", att_point_id.c_str(), _id.c_str());
    //     return _attachment_points.at(att_point_id);
    // }

    std::unique_ptr<TGroup> MonomerTemplate::getTGroup() const
    {
        auto tgroup = std::make_unique<TGroup>();
        // save template to ket
        rapidjson::StringBuffer string_buffer;
        JsonWriter writer;
        writer.Reset(string_buffer);
        writer.StartObject();
        KetDocumentJsonSaver::saveMonomerTemplate(writer, ref_prefix + id(), *this);
        writer.EndObject();
        std::string ket(string_buffer.GetString());
        // read TGroup
        rapidjson::Document document;
        document.Parse(ket.c_str());
        auto template_id = ref_prefix + _id;
        auto& mon_template = document[template_id.c_str()];
        Molecule mol;
        StereocentersOptions stereo_opt;
        stereo_opt.ignore_errors = true;
        int idx = MoleculeJsonLoader::parseMonomerTemplate(mon_template, mol, stereo_opt);
        auto& tg = mol.tgroups.getTGroup(idx);
        if (_unresolved)
        {
            tg.unresolved = _unresolved;
            auto& sa = (Superatom&)tg.fragment->sgroups.getSGroup(0, SGroup::SG_TYPE_SUP);
            sa.unresolved = _unresolved;
        }
        tgroup->copy(tg);

        return tgroup;
    }

    bool MonomerTemplate::hasIdtAlias(const std::string& alias, IdtModification mod)
    {
        if (_idt_alias.hasModification(mod) && (_idt_alias.getModification(mod) == alias))
            return true;
        return false;
    }

    bool MonomerTemplate::hasIdtAliasBase(const std::string& alias_base)
    {
        if (_idt_alias.getBase() == alias_base)
            return true;
        return false;
    }

    IMPL_ERROR(MonomerGroupTemplate, "MonomerGroupTemplate");

    void MonomerGroupTemplate::addTemplate(MonomerTemplateLibrary& library, const std::string& template_id)
    {
        _monomer_templates.emplace(template_id, std::cref(library.getMonomerTemplateById(template_id)));
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

    IMPL_ERROR(MonomerTemplateLibrary, "MonomerTemplateLibrary");

    MonomerTemplate& MonomerTemplateLibrary::addMonomerTemplate(const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved)
    {
        auto res = _monomer_templates.try_emplace(id, id, MonomerTemplate::StrToMonomerClass(monomer_class), idt_alias, unresolved);
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

    const MonomerTemplate& MonomerTemplateLibrary::getMonomerTemplateById(const std::string& monomer_template_id)
    {
        if (_monomer_templates.count(monomer_template_id) == 0)
            throw Error("Monomert template with id %s not found.", monomer_template_id.c_str());
        return _monomer_templates.at(monomer_template_id);
    }

    const std::string& MonomerTemplateLibrary::getMonomerTemplateIdByAlias(MonomerClass monomer_class, const std::string& monomer_template_alias)
    {
        for (auto& it : _monomer_templates)
        {
            if (it.second.monomerClass() == monomer_class && it.second.hasStringProp("alias") && it.second.getStringProp("alias") == monomer_template_alias)
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
        if (auto it = _id_alias_to_monomer_group_templates.find(alias); it != _id_alias_to_monomer_group_templates.end())
        {
            mod = it->second.second;
            return it->second.first.id();
        }
        return EMPTY_STRING;
    };

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
}
