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

#include "molecule/ket_document.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/meta_commons.h"
#include "molecule/monomers_template_library.h"
#include <base_cpp/scanner.h>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(KetDocumentJsonSaver, "KetDocument json saver");

template <class T>
const std::string get_ref(const T& obj)
{
    return T::ref_prefix + obj.id();
}

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
    if (monomer_template.attachmentPoints().size() == 0)
        return;

    writer.Key("attachmentPoints");
    writer.StartArray();
    for (auto& it : monomer_template.attachmentPoints())
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

void KetDocumentJsonSaver::saveMolecule(JsonWriter& writer, const std::string& ref, const KetMolecule& molecule)
{
    writer.Key(ref);
    writer.StartObject();
    saveKetAtoms(writer, molecule.atoms());
    saveKetBonds(writer, molecule.bonds());
    // save sgroups
    writer.EndObject();
}

void KetDocumentJsonSaver::saveMonomer(JsonWriter& writer, const KetMonomer& monomer)
{
    writer.Key(monomer.ref());
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
    writer.Key(get_ref(monomer_template));
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

void KetDocumentJsonSaver::saveVariantMonomer(JsonWriter& writer, const KetAmbiguousMonomer& monomer)
{
    writer.Key(monomer.ref());
    writer.StartObject();
    saveStr(writer, "type", "ambiguousMonomer");
    saveStr(writer, "id", monomer.id());
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
    monomer.saveOptsToKet(writer);
    saveStr(writer, "alias", monomer.alias());
    saveStr(writer, "templateId", monomer.templateId());
    writer.EndObject();
}

void KetDocumentJsonSaver::saveVariantMonomerTemplate(JsonWriter& writer, const KetAmbiguousMonomerTemplate& monomer_template)
{
    writer.Key(get_ref(monomer_template));
    writer.StartObject();
    saveStr(writer, "type", "ambiguousMonomerTemplate");
    saveStr(writer, "subtype", monomer_template.subtype());
    saveStr(writer, "id", monomer_template.id());
    saveStr(writer, "alias", monomer_template.alias());
    writer.Key("options");
    writer.StartArray();
    for (auto& it : monomer_template.options())
    {
        writer.StartObject();
        saveStr(writer, "templateId", it.templateId());
        auto& ratio = it.ratio();
        if (ratio.has_value())
        {
            writer.Key("ratio");
            saveNativeFloat(writer, ratio.value());
        }
        auto& probability = it.probability();
        if (probability.has_value())
        {
            writer.Key("probability");
            saveNativeFloat(writer, probability.value());
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
}

void KetDocumentJsonSaver::saveMonomerShape(JsonWriter& writer, const KetMonomerShape& monomer_shape)
{
    writer.Key(get_ref(monomer_shape));
    writer.StartObject();
    saveStr(writer, "type", "monomerShape");
    saveStr(writer, "id", monomer_shape.id());
    writer.Key("collapsed");
    writer.Bool(monomer_shape.collapsed());
    saveStr(writer, "shape", KetMonomerShape::shapeTypeToStr(monomer_shape.shape()).c_str());
    writer.Key("position");
    Vec2f pos = monomer_shape.position();
    writer.StartObject();
    writer.Key("x");
    saveNativeFloat(writer, pos.x);
    writer.Key("y");
    saveNativeFloat(writer, pos.y);
    writer.EndObject();
    writer.Key("monomers");
    writer.StartArray();
    for (auto& monomer_id : monomer_shape.monomers())
    {
        writer.String(monomer_id);
    }
    writer.EndArray();
    writer.EndObject();
}

void KetDocumentJsonSaver::saveKetDocument(JsonWriter& writer, const KetDocument& document)
{
    // auto& molecules = document.molecules();
    auto& monomers = document.monomers();
    auto& connections = document.connections();
    auto& templates = document.templates();
    auto& variant_templates = document.ambiguousTemplates();
    writer.StartObject(); // start
    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();
    /*
    for (auto& it : molecules)
    {
        writer.StartObject();
        saveStr(writer, "$ref", it.first);
        writer.EndObject();
    }
    // */
    std::string molecule_prefix = "mol";
    for (rapidjson::SizeType i = 0; i < document.jsonMolecules().Size(); i++)
    {
        writer.StartObject();
        saveStr(writer, "$ref", molecule_prefix + std::to_string(i));
        writer.EndObject();
    }
    std::string rgroup_prefix = "rg";
    for (rapidjson::SizeType i = 0; i < document.rgroups().Size(); i++)
    {
        writer.StartObject();
        saveStr(writer, "$ref", rgroup_prefix + std::to_string(i));
        writer.EndObject();
    }
    for (auto& id : document.monomersIds())
    {
        writer.StartObject();
        saveStr(writer, "$ref", monomers.at(id)->ref());
        writer.EndObject();
    }
    for (auto& shape : document.monomerShapes())
    {

        writer.StartObject();
        saveStr(writer, "$ref", get_ref(shape));
        writer.EndObject();
    }
    auto& meta_objects = document.metaObjects();
    for (rapidjson::SizeType i = 0; i < meta_objects.Size(); i++)
    {
        meta_objects[i].Accept(writer);
    }
    writer.EndArray(); // nodes
    if (connections.size() > 0)
    {
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
    }
    if (document.templatesIds().size() + document.ambiguousTemplatesIds().size() > 0)
    {
        writer.Key("templates");
        writer.StartArray();
        if (document.templatesIds().size() > 0)
        {
            for (auto& it : document.templatesIds())
            {
                writer.StartObject();
                saveStr(writer, "$ref", MonomerTemplate::ref_prefix + it);
                writer.EndObject();
            }
        }
        if (document.ambiguousTemplatesIds().size() > 0)
        {
            for (auto& it : document.ambiguousTemplatesIds())
            {
                writer.StartObject();
                saveStr(writer, "$ref", KetAmbiguousMonomerTemplate::ref_prefix + it);
                writer.EndObject();
            }
        }
        writer.EndArray(); // templates
    }
    writer.EndObject(); // root

    auto& json_molecules = document.jsonMolecules();
    for (rapidjson::SizeType i = 0; i < json_molecules.Size(); i++)
    {
        writer.Key(molecule_prefix + std::to_string(i));
        json_molecules[i].Accept(writer);
    }

    auto& rgroups = document.rgroups();
    for (rapidjson::SizeType i = 0; i < rgroups.Size(); i++)
    {
        writer.Key(rgroup_prefix + std::to_string(i));
        rgroups[i].Accept(writer);
    }

    // for (auto& it : document.moleculesRefs())
    //     saveMolecule(writer, it, molecules.at(it));

    for (auto& it : document.monomersIds())
    {
        auto& monomer = monomers.at(it);
        if (monomer->monomerType() == KetBaseMonomer::MonomerType::Monomer)
            saveMonomer(writer, *static_cast<KetMonomer*>(monomer.get()));
        else if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            saveVariantMonomer(writer, *static_cast<KetAmbiguousMonomer*>(monomer.get()));
        else
            throw Error("Unknown monomer type");
    }

    for (auto& it : document.templatesIds())
        saveMonomerTemplate(writer, templates.at(it));

    for (auto& it : document.ambiguousTemplatesIds())
        saveVariantMonomerTemplate(writer, variant_templates.at(it));

    for (auto& shape : document.monomerShapes())
        saveMonomerShape(writer, shape);

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif