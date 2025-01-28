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
#include "molecule/ket_document_json_loader.h"
#include "molecule/monomers_template_library.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(KetDocumentJsonLoader, "KetDocument json loader");

void KetDocumentJsonLoader::parseJson(const std::string& json_str, KetDocument& document, lib_ref /* library */)
{
    Document data;
    auto& ket = data.Parse(json_str.c_str());
    if (ket.HasParseError())
        throw Error("Parse error at offset %llu: %s", ket.GetErrorOffset(), GetParseError_En(ket.GetParseError()));

    Value& root = ket["root"];
    if (root.HasMember("templates"))
    {
        Value& templates = root["templates"];
        for (rapidjson::SizeType i = 0; i < templates.Size(); ++i)
        {
            if (templates[i].HasMember("$ref"))
            {
                std::string ref = templates[i]["$ref"].GetString();
                Value& templ = ket[ref.c_str()];
                std::string templ_type = templ["type"].GetString();
                if (templ_type == "monomerTemplate")
                {
                    parseMonomerTemplate(templ, document);
                }
                else if (templ_type == "ambiguousMonomerTemplate")
                {
                    parseVariantMonomerTemplate(templ, document);
                }
                else
                    throw Error("Unknows template type: %s", templ_type.c_str());
            }
        }
        document.processAmbiguousMonomerTemplates();
    }
    if (root.HasMember("nodes"))
    {
        Value& nodes = root["nodes"];
        for (rapidjson::SizeType i = 0; i < nodes.Size(); ++i)
        {
            if (nodes[i].HasMember("$ref"))
            {
                std::string ref = nodes[i]["$ref"].GetString();
                Value& node = ket[ref.c_str()];
                std::string node_type = node["type"].GetString();
                if (node_type == "molecule")
                {
                    // parseKetMolecule(ref, node, document);
                    document.addMolecule(node, ref);
                }
                else if (node_type == "rgroup")
                {
                    // parseKetRgroup(ref, node, document);
                    document.addRGroup(node);
                }
                else if (node_type == "monomer")
                {
                    parseKetMonomer(ref, node, document);
                }
                else if (node_type == "ambiguousMonomer")
                {
                    parseKetVariantMonomer(ref, node, document);
                }
                else if (node_type == "monomerShape")
                {
                    parseKetMonomerShape(ref, node, document);
                }
                else
                    throw Error("Unknows node type: %s", node_type.c_str());
            }
            else if (nodes[i].HasMember("type"))
            {
                document.addMetaObject(nodes[i]);
            }
            else
                throw Error("Unsupported node for molecule");
        }
    }
    if (root.HasMember("connections"))
    {
        Value& connections = root["connections"];
        for (rapidjson::SizeType i = 0; i < connections.Size(); ++i)
        {
            Value& connection = connections[i];
            std::string connection_type = connection["connectionType"].GetString();
            if (connection_type == KetConnectionSingle || connection_type == KetConnectionHydro)
            {
                KetConnectionEndPoint ep1, ep2;
                ep1.parseOptsFromKet(connection["endpoint1"]);
                ep2.parseOptsFromKet(connection["endpoint2"]);
                auto& conn = document.addConnection(connection_type, ep1, ep2);
                conn.parseOptsFromKet(connection);
            }
            else
                throw Error("Unknown connection type: %s", connection_type.c_str());
        }
    }
}

static void parseIdtAlias(const rapidjson::Value& parent, std::string& idt_alias_base, bool& idt_has_modifications, std::string& idt_five_prime_end,
                          std::string& idt_internal, std::string& idt_three_prime_end)
{
    auto& idt_alias_node = parent["idtAliases"];
    if (idt_alias_node.HasMember("base"))
        idt_alias_base = idt_alias_node["base"].GetString();
    if (idt_alias_node.HasMember("modifications"))
    {
        idt_has_modifications = true;
        auto& idt_modifications_node = idt_alias_node["modifications"];
        if (idt_modifications_node.HasMember("endpoint5"))
            idt_five_prime_end = idt_modifications_node["endpoint5"].GetString();
        if (idt_modifications_node.HasMember("internal"))
            idt_internal = idt_modifications_node["internal"].GetString();
        if (idt_modifications_node.HasMember("endpoint3"))
            idt_three_prime_end = idt_modifications_node["endpoint3"].GetString();
    }
}

static IdtAlias parseIdtAlias(const rapidjson::Value& parent)
{
    std::string template_class, idt_alias_base, idt_five_prime_end, idt_internal, idt_three_prime_end;
    bool idt_has_modifications = false;
    parseIdtAlias(parent, idt_alias_base, idt_has_modifications, idt_five_prime_end, idt_internal, idt_three_prime_end);
    if (idt_has_modifications)
        return IdtAlias(idt_alias_base, idt_five_prime_end, idt_internal, idt_three_prime_end);
    else
        return IdtAlias(idt_alias_base);
}

void KetDocumentJsonLoader::parseMonomerTemplate(const rapidjson::Value& mt_json, template_add_func addMonomerTemplate)
{
    if (!mt_json.HasMember("id"))
        throw Error("Monomer template without id");

    std::string id = mt_json["id"].GetString();

    if (!mt_json.HasMember("class"))
        throw Error("Monomer template without class");
    std::string monomer_class = mt_json["class"].GetString();

    bool unresolved = false;
    if (mt_json.HasMember("unresolved"))
        unresolved = mt_json["unresolved"].GetBool();

    IdtAlias idt_alias;
    if (mt_json.HasMember("idtAliases"))
    {
        idt_alias = parseIdtAlias(mt_json);
        auto& idt_base = idt_alias.getBase();
        if (idt_base.size() == 0)
            throw Error("Monomer template %s contains IDT alias without base.", id.c_str());
        if (unresolved) // For unresoved all modifications should be equal to base
            idt_alias.setModifications(idt_base, idt_base, idt_base);
    }
    else if (unresolved)
    {
        throw Error("Unresoved monomer '%s' without IDT alias.", id.c_str());
    }

    auto& mon_template = addMonomerTemplate(id, monomer_class, idt_alias, unresolved);
    mon_template.parseOptsFromKet(mt_json);

    // parse atoms
    mon_template.parseAtoms(mt_json["atoms"]);

    // parse bonds
    if (mt_json.HasMember("bonds"))
    {
        mon_template.parseBonds(mt_json["bonds"]);
    }

    if (mt_json.HasMember("attachmentPoints"))
    {
        auto& att_points = mt_json["attachmentPoints"];
        int side_count = 0;
        for (SizeType i = 0; i < att_points.Size(); i++)
        {
            auto& ap = att_points[i];
            std::string ap_id = "R";
            int attachment_atom;
            if (ap.HasMember("type"))
            {
                std::string t = ap["type"].GetString();
                if (t == "left")
                    ap_id = "R1";
                else if (t == "right")
                    ap_id = "R2";
                else if (t == "side")
                    ap_id += '3' + static_cast<char>(side_count++);
                else
                    throw Error("Unknown attachment point type %s", t.c_str());
            }
            else if (ap.HasMember("label"))
                ap_id = ap["label"].GetString();
            else
                ap_id += '1' + static_cast<char>(i);
            attachment_atom = ap["attachmentAtom"].GetInt();
            auto& att_point = mon_template.AddAttachmentPointId(ap_id, attachment_atom);
            att_point.parseOptsFromKet(ap);
            if (ap.HasMember("leavingGroup"))
            {
                auto& lv = ap["leavingGroup"];
                if (lv.HasMember("atoms"))
                {
                    std::vector<int> leaving_group;
                    auto& atoms = lv["atoms"];
                    for (SizeType j = 0; j < atoms.Size(); j++)
                    {
                        leaving_group.emplace_back(atoms[j].GetInt());
                    }
                    att_point.setLeavingGroup(leaving_group);
                }
            }
        }
    }
}

void KetDocumentJsonLoader::parseMonomerTemplate(const rapidjson::Value& mt_json, KetDocument& document)
{
    template_add_func func = [&document](const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved) -> MonomerTemplate& {
        return document.addMonomerTemplate(id, monomer_class, idt_alias, unresolved);
    };
    parseMonomerTemplate(mt_json, func);
}

void KetDocumentJsonLoader::parseMonomerTemplate(const rapidjson::Value& mt_json, MonomerTemplateLibrary& library)
{
    template_add_func func = [&library](const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved) -> MonomerTemplate& {
        return library.addMonomerTemplate(id, monomer_class, idt_alias, unresolved);
    };
    parseMonomerTemplate(mt_json, func);
}

void KetDocumentJsonLoader::parseKetMolecule(std::string& ref, rapidjson::Value& json, KetDocument& document)
{
    KetMolecule& mol = document.addMolecule(ref);
    if (!json.HasMember("atoms"))
        throw Error("Expected atoms block not found");

    // parse atoms
    mol.parseKetAtoms(json["atoms"]);

    // parse bonds
    if (json.HasMember("bonds"))
    {
        mol.parseKetBonds(json["bonds"]);
    }

    // if (json.HasMember("highlight"))
    // {
    //     parseHighlight(json["highlight"], *pmol);
    // }

    // parse SGroups
    if (json.HasMember("sgroups"))
    {
        mol.parseKetSGroups(json["sgroups"]);
    }

    // if (json.HasMember("properties"))
    // {
    //     parseProperties(json["properties"], *pmol);
    // }
}

void KetDocumentJsonLoader::parseKetRgroup(std::string& /* ref */, rapidjson::Value& /* json */, KetDocument& /* document */)
{
}

void KetDocumentJsonLoader::parseKetMonomer(std::string& ref, rapidjson::Value& json, KetDocument& document)
{
    std::string template_id = json["templateId"].GetString();
    // if (_monomers.)
    auto& monomer = document.addMonomer(json["id"].GetString(), json["alias"].GetString(), template_id, ref);
    monomer->parseOptsFromKet(json);
    if (json.HasMember("position"))
    {
        Vec2f location;
        const Value& coords = json["position"];
        location.x = coords["x"].GetFloat();
        location.y = coords["y"].GetFloat();
        monomer->setPosition(location);
    }
    monomer->setAttachmentPoints(document.templates().at(template_id).attachmentPoints());
}

void KetDocumentJsonLoader::parseKetVariantMonomer(std::string& ref, rapidjson::Value& json, KetDocument& document)
{
    std::string template_id = json["templateId"].GetString();
    auto& monomer = document.addAmbiguousMonomer(json["id"].GetString(), json["alias"].GetString(), template_id, ref);
    monomer->parseOptsFromKet(json);
    if (json.HasMember("position"))
    {
        Vec2f location;
        const Value& coords = json["position"];
        location.x = coords["x"].GetFloat();
        location.y = coords["y"].GetFloat();
        monomer->setPosition(location);
    }
    auto& variant_monomer_template = document.ambiguousTemplates().at(template_id);
    monomer->setAttachmentPoints(variant_monomer_template.attachmentPoints());
}

void KetDocumentJsonLoader::parseVariantMonomerTemplate(const rapidjson::Value& json, KetDocument& document)
{
    std::vector<KetAmbiguousMonomerOption> options;
    auto& opts = json["options"];
    for (SizeType i = 0; i < opts.Size(); i++)
    {
        auto& opt = opts[i];
        options.emplace_back(opt["templateId"].GetString());
        if (opt.HasMember("probability"))
            options.rbegin()->setProbability(opt["probability"].GetFloat());
        if (opt.HasMember("ratio"))
            options.rbegin()->setRatio(opt["ratio"].GetFloat());
    }
    std::string id = json["id"].GetString();
    IdtAlias idt_alias;
    if (json.HasMember("idtAliases"))
    {
        idt_alias = parseIdtAlias(json);
        if (idt_alias.getBase().size() == 0)
            throw Error("Monomer template %s contains IDT alias without base.", id.c_str());
    }
    auto& monomer_template = document.addAmbiguousMonomerTemplate(json["subtype"].GetString(), id, json["alias"].GetString(), idt_alias, options);
    monomer_template.parseOptsFromKet(json);
}

void KetDocumentJsonLoader::parseKetMonomerShape(std::string& /* ref */, rapidjson::Value& json, KetDocument& document)
{
    Vec2f position;
    const Value& coords = json["position"];
    position.x = coords["x"].GetFloat();
    position.y = coords["y"].GetFloat();
    std::vector<std::string> monomers;
    auto& mons = json["monomers"];
    for (SizeType i = 0; i < mons.Size(); i++)
    {
        monomers.emplace_back(mons[i].GetString());
    }
    document.addMonomerShape(json["id"].GetString(), json["collapsed"].GetBool(), json["shape"].GetString(), position, monomers);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif