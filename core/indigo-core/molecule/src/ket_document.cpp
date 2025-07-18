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
#include "molecule/monomer_commons.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

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
    res.first->second->setAttachmentPoints(_templates.at(template_id).attachmentPoints());
    _monomer_ref_to_id.emplace(res.first->second->ref(), id);
    _monomers_ids.emplace_back(id);
    return res.first->second;
}

std::unique_ptr<KetBaseMonomer>& KetDocument::addMonomer(const std::string& alias, const std::string& template_id)
{
    std::string id = std::to_string(_monomers.size());
    return addMonomer(id, alias, template_id);
}

std::unique_ptr<KetBaseMonomer>& KetDocument::getMonomerById(const std::string& id)
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

bool KetDocument::hasAmbiguousMonomerTemplateWithId(const std::string& id) const
{
    if (_ambiguous_templates.find(id) == _ambiguous_templates.end())
        return false;
    return true;
}

KetAmbiguousMonomerTemplate& KetDocument::addAmbiguousMonomerTemplate(const std::string& subtype, const std::string& id, const std::string& name,
                                                                      IdtAlias idt_alias, std::vector<KetAmbiguousMonomerOption>& options)
{
    if (_ambiguous_templates.find(id) != _ambiguous_templates.end())
        throw Error("Ambiguous monomer template '%s' already exists.", id.c_str());
    _ambiguous_templates_ids.emplace_back(id);
    auto it = _ambiguous_templates.try_emplace(id, subtype, id, name, idt_alias, options);
    _template_id_to_type.emplace(id, KetBaseMonomerTemplate::TemplateType::AmbiguousMonomerTemplate);
    return it.first->second;
};

std::unique_ptr<KetBaseMonomer>& KetDocument::addAmbiguousMonomer(const std::string& id, const std::string& alias, const std::string& template_id,
                                                                  const std::string& ref)
{
    auto& mon = addAmbiguousMonomer(id, alias, template_id);
    _monomer_ref_to_id.erase(mon->ref());
    mon->setRef(ref);
    _monomer_ref_to_id.emplace(ref, id);
    return mon;
};

std::unique_ptr<KetBaseMonomer>& KetDocument::addAmbiguousMonomer(const std::string& id, const std::string& alias, const std::string& template_id)
{
    if (_monomers.find(id) != _monomers.end())
        throw Error("Ambiguous monomer '%s' already exists.", id.c_str());
    auto it = _monomers.try_emplace(id, std::make_unique<KetAmbiguousMonomer>(id, alias, template_id));
    it.first->second->setAttachmentPoints(_ambiguous_templates.at(template_id).attachmentPoints());
    _monomer_ref_to_id.emplace(it.first->second->ref(), id);
    _monomers_ids.emplace_back(id);
    return it.first->second;
};

std::unique_ptr<KetBaseMonomer>& KetDocument::addAmbiguousMonomer(const std::string& alias, const std::string& template_id)
{
    std::string id = std::to_string(_monomers.size());
    return addAmbiguousMonomer(id, alias, template_id);
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
        std::ignore = data.Parse(json.c_str());
        // auto& res = data.Parse(json.c_str());
        // if res.hasParseError()
        MoleculeJsonLoader loader(data);
        loader.stereochemistry_options.ignore_errors = true;
        loader.ignore_noncritical_query_features = true;
        molecule.emplace(std::make_unique<Molecule>());
        loader.loadMolecule(*molecule.value().get());
    }
    return *molecule.value().get();
}

KetConnection& KetDocument::addConnection(const std::string& conn_type, KetConnectionEndPoint ep1, KetConnectionEndPoint ep2)
{
    _connections.emplace_back(conn_type, ep1, ep2);
    return *_connections.rbegin();
}

KetConnection& KetDocument::addConnection(KetConnectionEndPoint ep1, KetConnectionEndPoint ep2)
{
    return addConnection(KetConnectionSingle, ep1, ep2);
}

KetConnection& KetDocument::addConnection(const std::string& mon1, const std::string& ap1, const std::string& mon2, const std::string& ap2)
{
    KetConnectionEndPoint ep1, ep2;
    ep1.setStringProp("monomerId", mon1);
    ep2.setStringProp("monomerId", mon2);
    if (ap1 == HelmHydrogenPair && ap2 == HelmHydrogenPair)
    {
        _connections.emplace_back(KetConnection::TYPE::HYDROGEN, ep1, ep2);
    }
    else if (ap1 == HelmHydrogenPair || ap2 == HelmHydrogenPair)
    {
        throw Error("Wrong hydrogen connection - both attachment point should be '%s' but got '%s' and '%s'.", HelmHydrogenPair.c_str(), ap1.c_str(),
                    ap2.c_str());
    }
    else
    {
        connectMonomerTo(mon1, ap1, mon2, ap2);
        connectMonomerTo(mon2, ap2, mon1, ap1);
        ep1.setStringProp("attachmentPointId", ap1);
        ep2.setStringProp("attachmentPointId", ap2);
        _connections.emplace_back(ep1, ep2);
    }
    return *_connections.rbegin();
}

void KetDocument::connectMonomerTo(const std::string& mon1, const std::string& ap1, const std::string& mon2, const std::string& ap2)
{
    getMonomerById(_monomer_ref_to_id.at(mon1))->connectAttachmentPointTo(ap1, mon2, ap2);
}

void KetDocument::processAmbiguousMonomerTemplates()
{
    for (auto& it : _ambiguous_templates)
    {
        // check that all options has same option
        bool has_ratio = false;
        bool has_probability = false;
        auto& options = it.second.options();
        if (options.size() == 0)
            continue;
        for (auto& opt_it : options)
        {
            if (opt_it.ratio().has_value())
                has_ratio = true;
            if (opt_it.probability().has_value())
                has_probability = true;
        }
        if (has_ratio && has_probability)
            throw Error("Ambiguous monomer template '%s' has options with both ratio and probability set.", it.first.c_str());
        MonomerClass monomer_class = _templates.at(options[0].templateId()).monomerClass();
        for (auto& opt_it : options)
        {
            if (_templates.at(opt_it.templateId()).monomerClass() != monomer_class)
            {
                monomer_class = MonomerClass::Unknown;
                break;
            }
        }
        it.second.setMonomerClass(monomer_class);
        // calc attachment points
        std::map<std::string, KetAttachmentPoint> var_att_points;
        for (auto opt_it : options)
        {
            if ((opt_it.ratio().has_value() && opt_it.ratio().value() == 0) || (opt_it.probability().has_value() && opt_it.probability().value() == 0))
                continue;
            auto& opt_att_points = _templates.at(opt_it.templateId()).attachmentPoints();
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
    if ((cl1 == MonomerClass::Sugar && cl2 == MonomerClass::Sugar) && ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
        return true;
    if (((cl1 == MonomerClass::DNA || cl1 == MonomerClass::RNA) && (cl2 == MonomerClass::Phosphate || cl2 == MonomerClass::Sugar) &&
         (ap1 == "R2" && ap2 == "R1")) ||
        ((cl1 == MonomerClass::Phosphate || cl1 == MonomerClass::Sugar) && (cl2 == MonomerClass::DNA || cl2 == MonomerClass::RNA) &&
         (ap1 == "R2" && ap2 == "R1")))
        return true;
    if (((cl1 == MonomerClass::DNA && cl2 == MonomerClass::DNA) || (cl1 == MonomerClass::RNA && cl2 == MonomerClass::RNA)) &&
        ((ap1 == "R2" && ap2 == "R1") || (ap1 == "R1" && ap2 == "R2")))
        return true;
    return false;
}

static bool isIdtConnection(MonomerClass cl1, const std::string& ap1, MonomerClass cl2, const std::string& ap2)
{
    bool is_simple_pol_connection = isSimplePolymerConnection(cl1, ap1, cl2, ap2);
    if (is_simple_pol_connection)
        return true;
    static const std::set<MonomerClass> idt_backbone = {MonomerClass::CHEM, MonomerClass::DNA, MonomerClass::RNA, MonomerClass::Sugar, MonomerClass::Phosphate};
    if (idt_backbone.count(cl1) > 0 && idt_backbone.count(cl2) > 0 && ((ap1 == "R1" && ap2 == "R2") || (ap1 == "R2" && ap2 == "R1")))
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
        if (used_monomers.count(monomer_id) == 0 && monomers.erase(monomer_id))
        {
            used_monomers.emplace(monomer_id);
            if (left_side)
                sequence.emplace_front(monomer_id);
            else
                sequence.emplace_back(monomer_id);
        }

        const ket_connections_type& connections = _monomers.at(monomer_id)->connections();

        if (auto side_it = connections.find(left_side ? kAttachmentPointR1 : kAttachmentPointR2); side_it == connections.end())
            has_monomer_id = false;
        else
        {
            monomer_id = _monomer_ref_to_id.at(side_it->second.first);
            if (used_monomers.count(monomer_id) != 0) // This monomer already in sequence - this is cycle, connection should be stored as no-sequence
            {
                if (left_side) // add only when go left to avoid duplicate
                    _non_sequence_connections.emplace_back(ap_to_connection.at(std::make_pair(monomer_id, side_it->second.second)));
                break;
            }
        }
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
    else if (monomer.monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
        return _ambiguous_templates.at(monomer.templateId()).monomerClass();
    else
        throw Error("Unknonwn monomer type");
}

MonomerClass KetDocument::getMonomerClass(const std::string& monomer_id) const
{
    return getMonomerClass(*_monomers.at(monomer_id));
}

void KetDocument::addMetaObject(const rapidjson::Value& node)
{
    _meta_objects.PushBack(_json_document.CopyFrom(node, _json_document.GetAllocator()), _json_document.GetAllocator());
}

void KetDocument::addRGroup(const rapidjson::Value& node)
{
    _r_groups.PushBack(_json_document.CopyFrom(node, _json_document.GetAllocator()), _json_document.GetAllocator());
}

void KetDocument::addMolecule(const rapidjson::Value& node, std::string& ref)
{
    _molecule_refs.emplace_back(ref);
    _mol_ref_to_idx.emplace(ref, _json_molecules.Size());
    _json_molecules.PushBack(_json_document.CopyFrom(node, _json_document.GetAllocator()), _json_document.GetAllocator());
}

const KetBaseMonomerTemplate& KetDocument::getMonomerTemplate(const std::string& template_id) const
{
    if (_template_id_to_type.at(template_id) == KetBaseMonomerTemplate::TemplateType::MonomerTemplate)
        return _templates.at(template_id);
    else if (_template_id_to_type.at(template_id) == KetBaseMonomerTemplate::TemplateType::AmbiguousMonomerTemplate)
        return _ambiguous_templates.at(template_id);
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

void KetDocument::parseSimplePolymers(std::vector<std::deque<std::string>>& sequences, bool for_idt)
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

    for (auto& connection : _connections)
    {
        auto& ep1 = connection.ep1();
        auto& ep2 = connection.ep2();
        bool has_mol_1 = ep1.hasStringProp("moleculeId");
        bool has_mon_1 = ep1.hasStringProp("monomerId");
        bool has_mol_2 = ep2.hasStringProp("moleculeId");
        bool has_mon_2 = ep2.hasStringProp("monomerId");
        if ((has_mon_1 || has_mol_1) != (has_mon_2 || has_mol_2))
            throw Error("Connection with only one end point.");
        if (!(has_mon_1 || has_mol_1))
            throw Error("Connection with empty point.");
        if (connection.connType() == KetConnection::TYPE::HYDROGEN)
        {
            _non_sequence_connections.emplace_back(connection);
            continue;
        }
        bool has_ap_1 = ep1.hasStringProp("attachmentPointId");
        bool has_atom_1 = ep1.hasStringProp("atomId");
        bool has_ap_2 = ep2.hasStringProp("attachmentPointId");
        bool has_atom_2 = ep2.hasStringProp("atomId");
        if ((has_ap_1 || has_atom_1) != (has_ap_2 || has_atom_2))
            throw Error("Connection with only one attachment point id.");
        if (!(has_ap_1 || has_atom_1))
            throw Error("Connection with empty attachment point.");
        if ((has_mon_1 != has_ap_1) || (has_mon_2 != has_ap_2))
            throw Error("Wrong connection point");
        auto& mon_ref_1 = has_mon_1 ? ep1.getStringProp("monomerId") : ep1.getStringProp("moleculeId");
        auto& mon_ref_2 = has_mon_2 ? ep2.getStringProp("monomerId") : ep2.getStringProp("moleculeId");

        auto& mon_id_1 = has_mon_1 ? _monomer_ref_to_id.at(mon_ref_1) : mon_ref_1;
        auto& mon_id_2 = has_mon_2 ? _monomer_ref_to_id.at(mon_ref_2) : mon_ref_2;

        // molecules saved in helm as CHEM
        if (has_mol_1)
            id_to_class.emplace(mon_ref_1, MonomerClass::CHEM);
        if (has_mol_2)
            id_to_class.emplace(mon_ref_2, MonomerClass::CHEM);

        auto& mon1_class = id_to_class.at(mon_id_1);
        auto& mon2_class = id_to_class.at(mon_id_2);

        auto& ap_id_1 = has_mon_1 ? ep1.getStringProp("attachmentPointId") : ep1.getStringProp("atomId");
        auto& ap_id_2 = has_mon_2 ? ep2.getStringProp("attachmentPointId") : ep2.getStringProp("atomId");

        ap_to_connection.emplace(std::make_pair(mon_id_1, ap_id_1), connection);
        ap_to_connection.emplace(std::make_pair(mon_id_2, ap_id_2), connection);

        bool sequence_connection = false;
        if (has_mon_1 && has_mon_2) // any connection to molecule is not in sequence
            if (for_idt)
                sequence_connection = isIdtConnection(mon1_class, ap_id_1, mon2_class, ap_id_2);
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

    auto it = _monomers_ids.begin();
    while (monomers.size() > 0)
    {
        std::string start_monomer_id = "";
        // use _monomer_ids to follow original monomer order
        while (it != _monomers_ids.end() && start_monomer_id.size() == 0)
        {
            if (monomers.count(*it) > 0 && is_backbone_class(id_to_class.at(*it)))
            {
                start_monomer_id = *it;
            }
            it++;
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

const std::string& KetDocument::monomerIdByRef(const std::string& ref)
{
    const auto& it = _monomer_ref_to_id.find(ref);
    if (it == _monomer_ref_to_id.end())
        throw Error("Monomer with ref %s not found", ref.c_str());
    return it->second;
}

int KetDocument::moleculeIdxByRef(const std::string& ref)
{
    const auto& it = _mol_ref_to_idx.find(ref);
    if (it == _mol_ref_to_idx.end())
        throw Error("Molecule with ref %s not found", ref.c_str());
    return it->second;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
