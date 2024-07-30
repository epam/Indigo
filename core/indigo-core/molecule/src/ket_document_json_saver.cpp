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

#include <memory>
#include <set>

#include "molecule/ket_commons.h"
#include "molecule/ket_document.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/monomers_template_library.h"
#include <base_cpp/scanner.h>

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(KetDocumentJsonSaver, "KetDocument json saver");

static void saveNativeFloat(JsonWriter& writer, float f_value)
{
    std::string val = std::to_string(f_value);
    writer.RawValue(val.c_str(), val.length(), kStringType);
}

static void saveNonEmptyStr(JsonWriter& writer, const char* name, const std::string& str)
{
    if (str.size())
    {
        writer.Key(name);
        writer.String(str);
    }
}

static void saveStr(JsonWriter& writer, const char* name, const std::string& str)
{
    writer.Key(name);
    writer.String(str);
}

static void saveMonomerTemplateAttachmentPoints(JsonWriter& writer, const MonomerTemplate& monomer_template)
{
    if (monomer_template.attachemntPoints().size() == 0)
        return;

    writer.Key("attachmentPoints");
    writer.StartArray();
    for (auto& it : monomer_template.attachemntPoints())
    {
        auto& att_point = it.second;
        writer.StartObject();

        writer.Key("attachmentAtom");
        writer.Int(att_point.attachment_atom());
        att_point.saveOptsToKet(writer);

        auto& leaving_group = att_point.leavingGroup();

        if (leaving_group.has_value())
        {
            writer.Key("leavingGroup");
            writer.StartObject();
            writer.Key("atoms");
            writer.StartArray();
            for (auto idx : leaving_group.value())
            {
                writer.Int(idx);
            }
            writer.EndArray();
            writer.EndObject();
        }
        writer.EndObject();
    }
    writer.EndArray();
}

static void saveKetAtom(JsonWriter& writer, const KetBaseAtomType* base_atom)
{
    if (base_atom == nullptr)
        return;
    writer.StartObject();
    switch (base_atom->getType())
    {
    case KetBaseAtomType::atype::atom: {
        const KetAtom* atom = static_cast<const KetAtom*>(base_atom);
        saveStr(writer, "label", atom->label());
        auto& custom_query = atom->customQuery();
        auto& query_propertes = atom->queryProperties();
        if (custom_query.has_value() || query_propertes.has_value())
        {
            writer.Key("queryProperties");
            writer.StartObject();
            if (custom_query.has_value())
            {
                saveStr(writer, "customQuery", custom_query.value());
            }
            else if (query_propertes.has_value())
            {
                query_propertes.value().saveOptsToKet(writer);
            }
            writer.EndObject();
        }
        break;
    }
    case KetBaseAtomType::atype::atom_list: {
        saveStr(writer, "type", "atom-list");
        const KetAtomList* atom_list = static_cast<const KetAtomList*>(base_atom);
        auto& query_propertes = atom_list->queryProperties();
        writer.Key("elements");
        writer.StartArray();
        for (auto& label : atom_list->atomList())
        {
            writer.String(label);
        }
        writer.EndArray();
        if (query_propertes.has_value())
        {
            writer.Key("queryProperties");
            writer.StartObject();
            query_propertes.value().saveOptsToKet(writer);
            writer.EndObject();
        }
        break;
    }
    case KetBaseAtomType::atype::rg_label: {
        saveStr(writer, "type", "rg-label");
        const KetRgLabel* rg_label = static_cast<const KetRgLabel*>(base_atom);
        auto& attachment_order = rg_label->attachmentOrder();
        if (attachment_order.has_value())
        {
            writer.Key("attachmentOrder");
            writer.StartArray();
            for (auto& att : attachment_order.value())
            {
                writer.Key("attachmentAtom");
                writer.Int(att.first);
                writer.Key("attachmentId");
                writer.Int(att.second);
            }
            writer.EndArray();
        }
        auto& refs = rg_label->refs();
        if (refs.has_value())
        {
            writer.Key("$refs");
            writer.StartArray();
            for (auto& ref : refs.value())
                writer.String(ref);
            writer.EndArray();
        }
        break;
    }
    }
    auto& location = base_atom->location();
    if (location.has_value())
    {
        writer.Key("location");
        writer.StartArray();
        auto& loc = location.value();
        saveNativeFloat(writer, loc.x);
        saveNativeFloat(writer, loc.y);
        saveNativeFloat(writer, loc.z);
        writer.EndArray();
    }
    base_atom->saveOptsToKet(writer);
    writer.EndObject();
}

static void saveKetAtoms(JsonWriter& writer, const KetMolecule::atoms_type& atoms)
{
    writer.Key("atoms");
    writer.StartArray();
    for (auto& atom : atoms)
        saveKetAtom(writer, atom.get());
    writer.EndArray();
};

static void saveKetBonds(JsonWriter& writer, const std::vector<KetBond>& bonds)
{
    writer.Key("bonds");
    writer.StartArray();
    for (auto& bond : bonds)
    {
        writer.StartObject();
        writer.Key("type");
        writer.Int(static_cast<int>(bond.getType()));
        writer.Key("atoms");
        writer.StartArray();
        writer.Int(bond.atoms().first);
        writer.Int(bond.atoms().second);
        writer.EndArray();
        bond.saveOptsToKet(writer);
        writer.EndObject();
    }
    writer.EndArray();
};

void KetDocumentJsonSaver::saveMolecule(JsonWriter& writer, const KetMolecule& molecule)
{
    writer.StartObject();
    saveKetAtoms(writer, molecule.atoms());
    saveKetBonds(writer, molecule.bonds());
    // save sgroups
    writer.EndObject();
}

void KetDocumentJsonSaver::saveMonomer(JsonWriter& writer, const KetMonomer& monomer)
{
    writer.StartObject();
    saveStr(writer, "type", "monomer");
    saveStr(writer, "id", monomer.id());
    monomer.saveOptsToKet(writer);
    auto& pos = monomer.position();
    if (pos.has_value())
    {
        writer.Key("position");
        writer.StartObject();
        writer.Key("x");
        saveNativeFloat(writer, pos.value().x);
        writer.Key("y");
        saveNativeFloat(writer, pos.value().y);
        writer.EndObject();
    }
    saveStr(writer, "alias", monomer.alias());
    saveStr(writer, "templateId", monomer.templateId());
    writer.EndObject();
}

void KetDocumentJsonSaver::saveMonomerTemplate(JsonWriter& writer, const MonomerTemplate& monomer_template)
{
    writer.Key(MonomerTemplate::ref_prefix + monomer_template.id());
    writer.StartObject();
    saveStr(writer, "type", "monomerTemplate");
    saveStr(writer, "id", monomer_template.id());
    saveNonEmptyStr(writer, "class", monomer_template.monomerClassStr());
    monomer_template.saveOptsToKet(writer);
    if (monomer_template.unresolved())
    {
        writer.Key("unresolved");
        writer.Bool(monomer_template.unresolved());

        IdtAlias idt_alias = monomer_template.idtAlias().getBase();
        if (idt_alias.getBase().size()) // Save IDT alias only for unresolved
        {
            writer.Key("idtAliases");
            writer.StartObject();
            saveStr(writer, "base", idt_alias.getBase());
            if (idt_alias.hasModifications())
            {
                writer.Key("modifications");
                writer.StartObject();
                if (idt_alias.hasFivePrimeEnd())
                    saveStr(writer, "endpoint5", idt_alias.getFivePrimeEnd());
                if (idt_alias.hasInternal())
                    saveStr(writer, "internal", idt_alias.getInternal());
                if (idt_alias.hasThreePrimeEnd())
                    saveStr(writer, "endpoint3", idt_alias.getThreePrimeEnd());
                writer.EndObject();
            }
            writer.EndObject();
        }
    }

    saveMonomerTemplateAttachmentPoints(writer, monomer_template);
    saveKetAtoms(writer, monomer_template.atoms());
    saveKetBonds(writer, monomer_template.bonds());
    writer.EndObject();
}

void KetDocumentJsonSaver::saveKetDocument(JsonWriter& writer, const KetDocument& document)
{
    auto& molecules = document.molecules();
    auto& monomers = document.monomers();
    auto& connections = document.connections();
    auto& templates = document.templates();
    writer.StartObject(); // start
    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();
    for (auto& it : molecules)
    {
        writer.StartObject();
        saveStr(writer, "$ref", it.first);
        writer.EndObject();
    }
    for (auto& it : document.monomersRefs())
    {
        writer.StartObject();
        saveStr(writer, "$ref", it);
        writer.EndObject();
    }
    writer.EndArray(); // nodes
    writer.Key("connections");
    writer.StartArray();
    for (auto it : connections)
    {
        writer.StartObject();
        saveStr(writer, "connectionType", it.connectionType());
        it.saveOptsToKet(writer);
        writer.Key("endpoint1");
        writer.StartObject();
        it.ep1().saveOptsToKet(writer);
        writer.EndObject();
        writer.Key("endpoint2");
        writer.StartObject();
        it.ep2().saveOptsToKet(writer);
        writer.EndObject();
        writer.EndObject();
    }
    writer.EndArray(); // connections
    if (document.templatesRefs().size() > 0)
    {
        writer.Key("templates");
        writer.StartArray();
        for (auto& it : document.templatesRefs())
        {
            writer.StartObject();
            saveStr(writer, "$ref", it);
            writer.EndObject();
        }
        writer.EndArray(); // templates
    }
    writer.EndObject(); // root

    for (auto& it : document.moleculesRefs())
    {
        writer.Key(it);
        saveMolecule(writer, molecules.at(it));
    }
    for (auto& it : document.monomersRefs())
    {
        writer.Key(it);
        saveMonomer(writer, monomers.at(it));
    }
    if (templates.size() > 0)
    {
        for (auto& it : document.templatesRefs())
            saveMonomerTemplate(writer, templates.at(it));
    }
    writer.EndObject(); // end
}

void KetDocumentJsonSaver::saveKetDocument(const KetDocument& document)
{
    rapidjson::StringBuffer string_buffer;
    JsonWriter writer(pretty_json);
    writer.Reset(string_buffer);
    saveKetDocument(writer, document);
    std::stringstream result;
    _output.writeString(string_buffer.GetString());
}