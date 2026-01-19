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

#include <map>
#include <string>
#include <strings.h>

#include "molecule/molecule_json_saver.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"

using namespace indigo;

int MoleculeJsonSaver::getMonomerNumber(int mon_idx)
{
    auto mon_it = _monomers_enum.find(mon_idx);
    if (mon_it != _monomers_enum.end())
        return mon_it->second;
    else
        throw Error("Monomer index: %d not found", mon_idx);
    return -1;
}

void MoleculeJsonSaver::saveMonomerTemplate(TGroup& tg, JsonWriter& writer)
{
    std::string template_id("monomerTemplate-");
    std::string tg_id(monomerId(tg));
    std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
    std::string helm_class(monomerHELMClass(tg.tgroup_class.ptr()));
    template_id += tg_id;
    writer.Key(template_id.c_str());
    writer.StartObject();
    writer.Key("type");
    writer.String("monomerTemplate");
    writer.Key("id");
    writer.String(tg_id.c_str());
    if (tg.tgroup_class.size())
    {
        writer.Key("class");
        if (strcasecmp(template_class.c_str(), kMonomerClassLINKER) == 0)
            writer.String(kMonomerClassCHEM);
        else
            writer.String(template_class.c_str());
        writer.Key("classHELM");
        writer.String(helm_class.c_str());
    }

    writer.Key("alias");
    writer.String(monomerAlias(tg).c_str());

    if (tg.tgroup_name.size())
    {
        writer.Key("name");
        writer.String(tg.tgroup_name.ptr());
    }

    if (tg.tgroup_full_name.size())
    {
        writer.Key("fullName");
        writer.String(tg.tgroup_full_name.ptr());
    }

    std::string natreplace;
    if (tg.tgroup_natreplace.size() == 0)
    {
        auto alias = monomerAlias(tg);
        if (isBasicAminoAcid(template_class, alias))
        {
            natreplace = alias;
        }
        else if (tg.tgroup_name.size() > 0)
        {
            std::string name = tg.tgroup_name.ptr();
            alias = monomerAliasByName(tg.tgroup_class.ptr(), name);
            if (alias.size() > 0 && alias.size() != name.size())
                natreplace = alias;
        }
    }
    else
        natreplace = tg.tgroup_natreplace.ptr();

    if (natreplace.size())
    {
        auto analog = extractMonomerName(natreplace);
        auto nat_alias = monomerAliasByName(tg.tgroup_class.ptr(), analog);
        writer.Key("naturalAnalogShort");
        writer.String(nat_alias.c_str());
        if (analog.size() > 1)
        {
            writer.Key("naturalAnalog");
            writer.String(analog.c_str());
        }
    }

    if (tg.tgroup_comment.size())
    {
        writer.Key("comment");
        writer.String(tg.tgroup_comment.ptr());
    }

    if (tg.unresolved)
    {
        writer.Key("unresolved");
        writer.Bool(tg.unresolved);

        if (tg.idt_alias.size()) // Save IDT alias only for unresolved
        {
            writer.Key("idtAliases");
            writer.StartObject();
            writer.Key("base");
            writer.String(tg.idt_alias.ptr());
            writer.Key("modifications");
            writer.StartObject();
            writer.Key("endpoint5");
            writer.String(tg.idt_alias.ptr());
            writer.Key("internal");
            writer.String(tg.idt_alias.ptr());
            writer.Key("endpoint3");
            writer.String(tg.idt_alias.ptr());
            writer.EndObject();
            writer.EndObject();
        }
    }
    if (tg.modification_types.size() > 0)
    {
        writer.Key("modificationTypes");
        writer.StartArray();
        for (int i = 0; i < tg.modification_types.size(); i++)
        {
            writer.String(tg.modification_types[i].ptr());
        }
        writer.EndArray();
    }

    if (tg.different_aliasHELM)
    {
        writer.Key("aliasHELM");
        if (tg.aliasHELM.size() > 0)
            writer.String(tg.aliasHELM.ptr());
        else
            writer.String("");
    }

    if (tg.aliasAxoLabs.size() > 0)
    {
        writer.Key("aliasAxoLabs");
        writer.String(tg.aliasAxoLabs.ptr());
    }

    saveMonomerAttachmentPoints(tg, writer);
    saveFragment(*tg.fragment, writer);
    writer.EndObject();
}

void MoleculeJsonSaver::saveAmbiguousMonomerTemplate(TGroup& tg, JsonWriter& writer)
{
    std::string template_id("ambiguousMonomerTemplate-");
    std::string tg_id(monomerId(tg));
    std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
    std::string helm_class(monomerHELMClass(tg.tgroup_class.ptr()));
    template_id += tg_id;
    writer.Key(template_id.c_str());
    writer.StartObject();
    writer.Key("type");
    writer.String("ambiguousMonomerTemplate");
    writer.Key("subtype");
    writer.String(tg.mixture ? "mixture" : "alternatives");
    writer.Key("id");
    writer.String(tg_id.c_str());
    writer.Key("alias");
    writer.String(tg.tgroup_alias.ptr());
    writer.Key("options");
    writer.StartArray();
    const char* num_name = tg.mixture ? "ratio" : "probability";
    for (int i = 0; i < tg.aliases.size(); i++)
    {
        writer.StartObject();
        writer.Key("templateId");
        writer.String(tg.aliases[i].ptr());
        if (tg.ratios[i] >= 0)
        {
            writer.Key(num_name);
            saveNativeFloat(writer, tg.ratios[i], this->native_precision);
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
}

void MoleculeJsonSaver::saveSuperatomAttachmentPoints(Superatom& sa, JsonWriter& writer)
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
            writer.Key("attachmentPoints");
            writer.StartArray();
            int order = 0;
            for (const auto& kvp : sorted_attachment_points)
            {
                writer.StartObject();
                auto& atp = sa.attachment_points[kvp.second];
                std::string atp_id_str(atp.apid.ptr());
                if (!isAttachmentPointsInOrder(order++, atp_id_str))
                {
                    if (atp_id_str.size())
                    {
                        writer.Key("id");
                        writer.String(atp_id_str.c_str());
                    }
                    writer.Key("type");
                    if (atp_id_str == kLeftAttachmentPoint || atp_id_str == kAttachmentPointR1)
                        writer.String("left");
                    else if (atp_id_str == kRightAttachmentPoint || atp_id_str == kAttachmentPointR2)
                        writer.String("right");
                    else
                        writer.String("side");
                    writer.Key("label");
                    writer.String(convertAPToHELM(atp_id_str).c_str());
                }
                writer.Key("attachmentAtom");
                writer.Int(atp.aidx);
                if (atp.lvidx >= 0)
                {
                    writer.Key("leavingGroup");
                    writer.StartObject();
                    writer.Key("atoms");
                    writer.StartArray();
                    writer.Int(atp.lvidx);
                    writer.EndArray();
                    writer.EndObject(); // leavingGroup
                }
                writer.EndObject(); // attachmentAtom
            }
            writer.EndArray();
        }
    }
}

void MoleculeJsonSaver::saveMonomerAttachmentPoints(TGroup& tg, JsonWriter& writer)
{
    auto& sgroups = tg.fragment->sgroups;
    for (int j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
    {
        SGroup& sg = sgroups.getSGroup(j);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            saveSuperatomAttachmentPoints((Superatom&)sg, writer);
            sgroups.remove(j);
        }
    }
}
