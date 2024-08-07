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

#include "molecule/ket_document.h"
#include "base_cpp/exception.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"

using namespace indigo;

IMPL_ERROR(KetDocument, "Ket Document")

KetMolecule& KetDocument::addMolecule(const std::string& ref)
{
    _molecule_refs.emplace_back(ref);
    auto res = _molecules.emplace(ref, KetMolecule());
    if (!res.second)
        throw Error("Molecule with ref='%s' already exists", ref.c_str());
    return res.first->second;
}

std::unique_ptr<KetBaseMonomer>& KetDocument::addMonomer(const std::string& id, const std::string& alias, const std::string& template_id,
                                                         const std::string& ref)
{
    auto& mon = addMonomer(id, alias, template_id);
    _monomer_ref_to_id.erase(mon->ref());
    mon->setRef(ref);
    _monomer_ref_to_id.emplace(ref, id);
    return mon;
}

std::unique_ptr<KetBaseMonomer>& KetDocument::addMonomer(const std::string& id, const std::string& alias, const std::string& template_id)
{
    auto res = _monomers.emplace(id, std::make_unique<KetMonomer>(id, alias, template_id));
    if (!res.second)
        throw Error("Monomer with id='%s' already exists", id.c_str());
    _monomer_ref_to_id.emplace(res.first->second->ref(), id);
    _monomers_ids.emplace_back(id);
    return res.first->second;
}

std::unique_ptr<KetBaseMonomer>& KetDocument::addMonomer(const std::string& alias, const std::string& template_id)
{
    std::string id = std::to_string(_monomers.size());
    return addMonomer(id, alias, template_id);
}

const std::unique_ptr<KetBaseMonomer>& KetDocument::getMonomerById(const std::string& id) const
{
    auto it = _monomers.find(id);
    if (it == _monomers.end())
        throw Error("Monomer with id '%s' not found.", id.c_str());
    return it->second;
}

MonomerTemplate& KetDocument::addMonomerTemplate(const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved)
{
    auto it = _templates.try_emplace(id, id, monomer_class, idt_alias, unresolved);
    if (!it.second)
        throw Error("Template with id '%s' already added", id.c_str());
    _template_id_to_type.emplace(id, KetBaseMonomerTemplate::TemplateType::MonomerTemplate);
    _templates_ids.emplace_back(id);
    return it.first->second;
}

void KetDocument::addMonomerTemplate(const MonomerTemplate& monomer_template)
{

    auto it = _templates.try_emplace(monomer_template.id(), monomer_template.id(), monomer_template.monomerClass(), monomer_template.idtAlias(),
                                     monomer_template.unresolved());
    if (!it.second)
        throw Error("Template with id '%s' already added", monomer_template.id().c_str());
    _template_id_to_type.emplace(monomer_template.id(), KetBaseMonomerTemplate::TemplateType::MonomerTemplate);
    _templates_ids.emplace_back(monomer_template.id());
    it.first->second.copy(monomer_template);
}

KetVariantMonomerTemplate& KetDocument::addVariantMonomerTemplate(const std::string& subtype, const std::string& id, const std::string& name,
                                                                  IdtAlias idt_alias, std::vector<KetVariantMonomerOption>& options)
{
    if (_variant_templates.find(id) != _variant_templates.end())
        throw Error("Variant monomer template '%s' already exists.", id.c_str());
    _variant_templates_ids.emplace_back(id);
    auto it = _variant_templates.try_emplace(id, subtype, id, name, idt_alias, options);
    _template_id_to_type.emplace(id, KetBaseMonomerTemplate::TemplateType::VariantMonomerTemplate);
    return it.first->second;
};

std::unique_ptr<KetBaseMonomer>& KetDocument::addVariantMonomer(const std::string& id, const std::string& alias, const std::string& template_id,
                                                                const std::string& ref)
{
    auto& mon = addVariantMonomer(id, alias, template_id);
    _monomer_ref_to_id.erase(mon->ref());
    mon->setRef(ref);
    _monomer_ref_to_id.emplace(ref, id);
    return mon;
};

std::unique_ptr<KetBaseMonomer>& KetDocument::addVariantMonomer(const std::string& id, const std::string& alias, const std::string& template_id)
{
    if (_monomers.find(id) != _monomers.end())
        throw Error("Variant monomer '%s' already exists.", id.c_str());
    auto it = _monomers.try_emplace(id, std::make_unique<KetVariantMonomer>(id, alias, template_id));
    _monomer_ref_to_id.emplace(it.first->second->ref(), id);
    _monomers_ids.emplace_back(id);
    return it.first->second;
};

std::unique_ptr<KetBaseMonomer>& KetDocument::addVariantMonomer(const std::string& alias, const std::string& template_id)
{
    std::string id = std::to_string(_monomers.size());
    return addVariantMonomer(id, alias, template_id);
}

BaseMolecule& KetDocument::getBaseMolecule()
{
    static thread_local std::optional<std::unique_ptr<Molecule>> molecule; // Temporary until direct conversion to molecule supported
    if (!molecule.has_value())
    {
        // save to ket
        std::string json;
        StringOutput out(json);
        KetDocumentJsonSaver saver(out);
        saver.saveKetDocument(*this);
        // load molecule from ket
        rapidjson::Document data;
        auto& res = data.Parse(json.c_str());
        // if res.hasParseError()
        MoleculeJsonLoader loader(data);
        loader.stereochemistry_options.ignore_errors = true;
        loader.ignore_noncritical_query_features = true;
        molecule.emplace(std::make_unique<Molecule>());
        loader.loadMolecule(*molecule.value().get());
    }
    return *molecule.value().get();
}

KetConnection& KetDocument::addConnection(KetConnectionEndPoint ep1, KetConnectionEndPoint ep2)
{
    _connections.emplace_back(ep1, ep2);
    return *_connections.rbegin();
}

void KetDocument::connectMonomerTo(const std::string& mon1, const std::string& ap1, const std::string& mon2, const std::string& ap2)
{
    std::string mon1_id = _monomer_ref_to_id.at(mon1);
    auto it = _monomers.find(mon1_id);
    if (it == _monomers.end())
        throw Error("Unknown monomer '%s'", mon1.c_str());
    it->second->connectAttachmentPointTo(ap1, mon2, ap2);
}

void KetDocument::processVariantMonomerTemplates()
{
    for (auto& it : _variant_templates)
    {
        // check that all options has same option
        bool has_ratio = false;
        bool has_probability = false;
        auto& options = it.second.options();
        if (options.size() == 0)
            continue;
        for (auto& it : options)
        {
            if (it.ratio().has_value())
                has_ratio = true;
            if (it.probability().has_value())
                has_probability = true;
        }
        if (has_ratio && has_probability)
            throw Error("Variant monomer template '%s' has options with both ratio and probability set.", it.first.c_str());
        MonomerClass monomer_class = _templates.at(options[0].templateId()).monomerClass();
        for (auto& it : options)
        {
            if (_templates.at(it.templateId()).monomerClass() != monomer_class)
            {
                monomer_class = MonomerClass::Unknown;
                break;
            }
        }
        it.second.setMonomerClass(monomer_class);
        // calc attachment points
        std::map<std::string, KetAttachmentPoint> var_att_points;
        for (auto it : options)
        {
            if ((it.ratio().has_value() && it.ratio().value() == 0) || (it.probability().has_value() && it.probability().value() == 0))
                continue;
            auto& opt_att_points = _templates.at(it.templateId()).attachmentPoints();
            if (var_att_points.size() == 0) // first option
            {
                var_att_points = opt_att_points;
            }
            std::vector<std::string> to_remove;
            for (auto& att_point_it : var_att_points)
            {
                if (opt_att_points.count(att_point_it.first) == 0)
                {
                    to_remove.emplace_back(att_point_it.first);
                    continue;
                    // should we check attachment points content?
                }
            }
            for (auto id : to_remove)
            {
                var_att_points.erase(id);
            }
            if (var_att_points.size() == 0) // no attachment point
                break;
        }
        it.second.setAttachmentPoints(var_att_points);
    }
}

static bool isSimplePolymerConnection(MonomerClass cl1, const std::string& ap1, MonomerClass cl2, const std::string& ap2)
{
    if ((cl1 == MonomerClass::Sugar && ap1 == "R3" && cl2 == MonomerClass::Base && ap2 == "R1") ||
        (cl2 == MonomerClass::Sugar && ap2 == "R3" && cl1 == MonomerClass::Base && ap1 == "R1"))
        return true;
    if (cl1 == MonomerClass::AminoAcid && cl2 == MonomerClass::AminoAcid && ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
        return true;
    if (((cl1 == MonomerClass::Phosphate && cl2 == MonomerClass::Sugar) || (cl2 == MonomerClass::Phosphate && cl1 == MonomerClass::Sugar)) &&
        ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
        return true;
    if ((cl1 == MonomerClass::DNA && cl2 == MonomerClass::Sugar && ap1 == "R2" && ap2 == "R1") ||
        (cl2 == MonomerClass::DNA && cl1 == MonomerClass::Sugar && ap2 == "R2" && ap1 == "R1"))
        return true;
    if ((cl1 == MonomerClass::Phosphate && cl2 == MonomerClass::DNA && ap1 == "R2" && ap2 == "R1") ||
        (cl2 == MonomerClass::Phosphate && cl1 == MonomerClass::DNA && ap2 == "R2" && ap1 == "R1"))
        return true;
    return false;
}

static bool isSequenceConnection(MonomerClass cl1, const std::string& ap1, MonomerClass cl2, const std::string& ap2)
{
    bool is_simple_pol_connection = isSimplePolymerConnection(cl1, ap1, cl2, ap2);
    if (is_simple_pol_connection)
        return true;
    if ((cl1 == MonomerClass::CHEM || cl2 == MonomerClass::CHEM) && ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
        return true;
    if (((cl1 == MonomerClass::Phosphate && cl2 == MonomerClass::Sugar) || (cl2 == MonomerClass::CHEM && cl1 == MonomerClass::Sugar)) &&
        ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
        return true;
    if ((cl1 == MonomerClass::DNA && cl2 == MonomerClass::Sugar && ap1 == "R2" && ap2 == "R1") ||
        (cl2 == MonomerClass::DNA && cl1 == MonomerClass::Sugar && ap2 == "R2" && ap1 == "R1"))
        return true;
    if ((cl1 == MonomerClass::Phosphate && cl2 == MonomerClass::DNA && ap1 == "R2" && ap2 == "R1") ||
        (cl2 == MonomerClass::Phosphate && cl1 == MonomerClass::DNA && ap2 == "R2" && ap1 == "R1"))
        return true;
    if ((cl1 == MonomerClass::Phosphate && cl2 == MonomerClass::Phosphate) && ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
        return true;
    return false;
}

void KetDocument::collect_sequence_side(const std::string& start_monomer_id, bool left_side, std::set<std::string>& monomers,
                                        std::set<std::string>& used_monomers, std::deque<std::string>& sequence,
                                        std::map<std::pair<std::string, std::string>, const KetConnection&>& ap_to_connection)
{
    bool has_monomer_id = true;
    std::string monomer_id = start_monomer_id;
    while (has_monomer_id)
    {
        if (used_monomers.count(monomer_id) == 0)
        {
            used_monomers.emplace(monomer_id);
            monomers.erase(monomer_id);
            if (left_side)
                sequence.emplace_front(monomer_id);
            else
                sequence.emplace_back(monomer_id);
        }

        const ket_connections_type& connections = _monomers.at(monomer_id)->connections();

        if (auto side_it = connections.find(left_side ? "R1" : "R2"); side_it == connections.end())
            has_monomer_id = false;
        else if (used_monomers.count(side_it->second.first) == 0)
            monomer_id = _monomer_ref_to_id.at(side_it->second.first);
        else // This monomer already in sequence - this is cycle, connection should be stored as no-sequence
            _non_sequence_connections.emplace_back(ap_to_connection.at(std::make_pair(side_it->second.first, side_it->second.second)));
        // When collect left side - base should be placed before sugar, when right - after
        if (!left_side || has_monomer_id)
        {
            auto& conns = left_side ? _monomers.at(monomer_id)->connections() : connections;
            if (auto base_it = conns.find("R3"); base_it != conns.end())
            {
                auto& base_id = _monomer_ref_to_id.at(base_it->second.first);
                used_monomers.emplace(base_id);
                monomers.erase(base_id);
                if (left_side)
                    sequence.emplace_front(base_id);
                else
                    sequence.emplace_back(base_id);
            }
        }
    }
}

MonomerClass KetDocument::getMonomerClass(const KetBaseMonomer& monomer) const
{
    if (monomer.monomerType() == KetBaseMonomer::MonomerType::Monomer)
        return _templates.at(monomer.templateId()).monomerClass();
    else if (monomer.monomerType() == KetBaseMonomer::MonomerType::VarianMonomer)
        return _variant_templates.at(monomer.templateId()).monomerClass();
    else
        throw Error("Unknonwn monomer type");
}

MonomerClass KetDocument::getMonomerClass(const std::string& monomer_id) const
{
    return getMonomerClass(*_monomers.at(monomer_id));
}

const KetBaseMonomerTemplate& KetDocument::getMonomerTemplate(const std::string& template_id) const
{
    if (_template_id_to_type.at(template_id) == KetBaseMonomerTemplate::TemplateType::MonomerTemplate)
        return _templates.at(template_id);
    else if (_template_id_to_type.at(template_id) == KetBaseMonomerTemplate::TemplateType::VariantMonomerTemplate)
        return _variant_templates.at(template_id);
    else
        throw Error("Unknonwn monomer template type");
}

bool is_backbone_class(MonomerClass monomer_class)
{
    switch (monomer_class)
    {
    case MonomerClass::AminoAcid:
    case MonomerClass::CHEM:
    case MonomerClass::DNA:
    case MonomerClass::Phosphate:
    case MonomerClass::RNA:
    case MonomerClass::Sugar:
        return true;
    }
    return false;
}

void KetDocument::parseSimplePolymers(std::vector<std::deque<std::string>>& sequences, bool for_sequence)
{
    std::map<std::string, MonomerClass> id_to_class;
    std::set<std::string> monomers;
    std::set<std::string> used_monomers;

    for (auto& it : _monomers)
    {
        id_to_class.emplace(it.first, getMonomerClass(*it.second));
        monomers.emplace(it.first);
    }

    std::map<std::pair<std::string, std::string>, const KetConnection&> ap_to_connection;
    int conn_idx = 0;
    for (auto& connection : _connections)
    {
        auto& ep1 = connection.ep1();
        auto& ep2 = connection.ep2();
        bool has_mon_1 = ep1.hasStringProp("monomerId");
        bool has_mon_2 = ep2.hasStringProp("monomerId");
        if (has_mon_1 != has_mon_2)
            throw Error("Connection with only one end point.");
        if (!has_mon_1)
            throw Error("Connection with empty point.");
        bool has_ap_1 = ep1.hasStringProp("attachmentPointId");
        bool has_ap_2 = ep2.hasStringProp("attachmentPointId");
        if (has_ap_1 != has_ap_2)
            throw Error("Connection with only one attachment point id.");
        if (!has_ap_1)
            throw Error("Connection with empty attachment point.");

        auto& mon_ref_1 = ep1.getStringProp("monomerId");
        auto& mon_ref_2 = ep2.getStringProp("monomerId");

        auto& mon_id_1 = _monomer_ref_to_id.at(mon_ref_1);
        auto& mon_id_2 = _monomer_ref_to_id.at(mon_ref_2);

        auto& mon1_class = id_to_class.at(mon_id_1);
        auto& mon2_class = id_to_class.at(mon_id_2);

        auto& ap_id_1 = ep1.getStringProp("attachmentPointId");
        auto& ap_id_2 = ep2.getStringProp("attachmentPointId");

        ap_to_connection.emplace(std::make_pair(mon_id_1, ap_id_1), connection);
        ap_to_connection.emplace(std::make_pair(mon_id_2, ap_id_2), connection);

        bool sequence_connection = false;
        if (for_sequence)
            sequence_connection = isSequenceConnection(mon1_class, ap_id_1, mon2_class, ap_id_2);
        else
            sequence_connection = isSimplePolymerConnection(mon1_class, ap_id_1, mon2_class, ap_id_2);
        if (!sequence_connection)
        {
            _non_sequence_connections.emplace_back(connection);
            continue;
        }
        connectMonomerTo(mon_ref_1, ap_id_1, mon_ref_2, ap_id_2);
        connectMonomerTo(mon_ref_2, ap_id_2, mon_ref_1, ap_id_1);
    }

    while (monomers.size() > 0)
    {
        std::string start_monomer_id = "";
        for (auto& id : monomers)
        {
            if (is_backbone_class(id_to_class.at(id)))
            {
                start_monomer_id = id;
                break;
            }
        }
        if (start_monomer_id.size() == 0) // no backbone monomers left - create sequence for each monomers
        {
            for (auto& monomer_id : monomers)
            {
                auto& sequence = sequences.emplace_back();
                sequence.emplace_back(monomer_id);
            }
            break;
        }
        auto& sequence = sequences.emplace_back();

        collect_sequence_side(start_monomer_id, false, monomers, used_monomers, sequence, ap_to_connection);
        collect_sequence_side(start_monomer_id, true, monomers, used_monomers, sequence, ap_to_connection);
    }
}