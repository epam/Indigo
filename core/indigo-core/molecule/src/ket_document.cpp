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
#include "molecule/crippen.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_mass.h"
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
    if (((cl1 == MonomerClass::DNA || cl1 == MonomerClass::RNA) && (cl2 == MonomerClass::Phosphate || cl2 == MonomerClass::Sugar) &&
         (ap1 == "R2" && ap2 == "R1")) ||
        ((cl2 == MonomerClass::DNA || cl2 == MonomerClass::RNA) && (cl1 == MonomerClass::Phosphate || cl1 == MonomerClass::Sugar) &&
         (ap2 == "R2" && ap1 == "R1")))
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

void KetDocument::CalculateMacroProps(Output& output, bool pretty_json)
{
    auto is_base = [&](MonomerClass monomer_class) -> bool {
        return monomer_class == MonomerClass::Base || monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA;
    };
    auto move_to_next_base = [&](auto& it, const auto& end) {
        while (it != end)
        {
            it++;
            if (it != end && is_base(getMonomerClass(*it)))
                return;
        }
    };
    struct chain
    {
        std::deque<std::string> sequence, secondary_sequence;
        chain(std::deque<std::string>& seq) : sequence(seq), secondary_sequence(){};
        chain(std::deque<std::string>& seq, std::deque<std::string>& secondary_seq) : sequence(seq), secondary_sequence(secondary_seq){};
    };
    std::vector<std::deque<std::string>> sequences;
    std::vector<chain> joined_sequences;
    std::map<std::string, std::pair<std::string, std::string>> monomer_to_molecule;
    parseSimplePolymers(sequences, false);
    for (auto connection : _non_sequence_connections)
    {
        auto& ep1 = connection.ep1();
        auto& ep2 = connection.ep2();
        if (ep1.hasStringProp("monomerId") && ep2.hasStringProp("monomerId"))
        {
            auto& left_monomer = _monomers.at(_monomer_ref_to_id.at(ep1.getStringProp("monomerId")));
            auto& right_monomer = _monomers.at(_monomer_ref_to_id.at(ep2.getStringProp("monomerId")));
            if (connection.connectionType() == KetConnectionHydro)
            {
                left_monomer->addHydrogenConnection(right_monomer->ref());
                right_monomer->addHydrogenConnection(left_monomer->ref());
            }
            else if (connection.connectionType() == KetConnectionSingle)
            {
                auto& ap_left = ep1.getStringProp("attachmentPointId");
                auto& ap_right = ep2.getStringProp("attachmentPointId");
                left_monomer->connectAttachmentPointTo(ap_left, right_monomer->ref(), ap_right);
                right_monomer->connectAttachmentPointTo(ap_right, left_monomer->ref(), ap_left);
            }
        }
        else if ((ep1.hasStringProp("monomerId") && ep2.hasStringProp("moleculeId")) || (ep1.hasStringProp("moleculeId") && ep2.hasStringProp("monomerId")))
        {
            auto monomer_id =
                ep1.hasStringProp("monomerId") ? _monomer_ref_to_id.at(ep1.getStringProp("monomerId")) : _monomer_ref_to_id.at(ep2.getStringProp("monomerId"));
            auto molecule_id = ep1.hasStringProp("moleculeId") ? ep1.getStringProp("moleculeId") : ep2.getStringProp("moleculeId");
            auto monomer_ap = ep1.hasStringProp("monomerId") ? ep1.getStringProp("attachmentPointId") : ep2.getStringProp("attachmentPointId");
            if (monomer_ap == kAttachmentPointR1 || monomer_ap == kAttachmentPointR2)
                monomer_to_molecule.emplace(monomer_id, std::make_pair(molecule_id, monomer_ap));
        }
    }
    std::map<std::string, size_t> five_prime_monomers, three_prime_monomers;
    for (size_t i = 0; i < sequences.size(); i++)
    {
        five_prime_monomers.emplace(sequences[i].front(), i);
        three_prime_monomers.emplace(sequences[i].back(), i);
    }
    std::set<size_t> used_sequences;
    std::set<size_t> possible_double_chains;
    std::map<size_t, std::set<size_t>> bases_count_to_variants;
    while (five_prime_monomers.size() > 0)
    {
        auto idx = five_prime_monomers.begin()->second;
        auto cur_monomer_id = five_prime_monomers.begin()->first;
        five_prime_monomers.erase(five_prime_monomers.begin());
        used_sequences.emplace(idx);
        // If no chains connected to 5' and 3' ends of DNA/RNA chain - add it to possible double chain
        if (_monomers.at(sequences[idx].front())->connections().count(kAttachmentPointR1) == 0 &&
            _monomers.at(sequences[idx].back())->connections().count(kAttachmentPointR2) == 0)
        {
            auto& first_monomer = _monomers.at(sequences[idx].front());
            auto first_monomer_class = getMonomerClass(*first_monomer);
            switch (first_monomer_class)
            {
            case MonomerClass::DNA:
            case MonomerClass::RNA:
            case MonomerClass::Sugar:
            case MonomerClass::Phosphate:
            case MonomerClass::Base: {
                bool found_no_hydrogen = false;
                size_t bases_count = 0;
                // check that all bases have hydrogen connections
                for (auto& monomer_id : sequences[idx])
                {
                    auto& monomer = _monomers.at(monomer_id);
                    if (is_base(getMonomerClass(*monomer)))
                    {
                        if (monomer->hydrogenConnections().size() == 0) // no hydrogen connections
                        {
                            found_no_hydrogen = true;
                            break;
                        }
                        bases_count++;
                    }
                }
                if (found_no_hydrogen == false)
                {
                    possible_double_chains.emplace(idx);
                    if (bases_count_to_variants.count(bases_count) == 0)
                        bases_count_to_variants.emplace(bases_count, std::set<size_t>());
                    bases_count_to_variants.at(bases_count).emplace(idx);
                    continue;
                }
            }
            default:
                break;
            }
        }
        auto& sequence = joined_sequences.emplace_back(sequences[idx]).sequence;
        while (true)
        {
            auto& front_connections = _monomers.at(sequence.front())->connections();
            if (front_connections.count(kAttachmentPointR1) > 0)
            {
                auto& connection = front_connections.at(kAttachmentPointR1);
                if (connection.second == kAttachmentPointR2)
                {
                    auto& monomer_id = _monomer_ref_to_id.at(connection.first);
                    auto it = three_prime_monomers.find(monomer_id);
                    if (it == three_prime_monomers.end())
                        throw Error("Internal error. Connection to monomer %s not found", monomer_id.c_str());
                    auto letf_idx = it->second;
                    if (used_sequences.count(letf_idx) == 0)
                    {
                        sequence.insert(sequence.begin(), sequences[letf_idx].begin(), sequences[letf_idx].end());
                        five_prime_monomers.erase(sequences[letf_idx].front()); // remove from 5' monomers
                        used_sequences.emplace(letf_idx);
                        continue;
                    }
                }
            }
            auto& back_connections = _monomers.at(sequence.back())->connections();
            if (back_connections.count(kAttachmentPointR2) > 0)
            {
                auto& connection = back_connections.at(kAttachmentPointR2);
                if (connection.second == kAttachmentPointR1)
                {
                    auto& monomer_id = _monomer_ref_to_id.at(connection.first);
                    if (monomer_id != cur_monomer_id) // avoid cycle
                    {
                        auto it = five_prime_monomers.find(monomer_id);
                        if (it == five_prime_monomers.end())
                            throw Error("Internal error. Connection to monomer %s not found", monomer_id.c_str());
                        auto right_idx = it->second;
                        if (used_sequences.count(right_idx) == 0)
                        {
                            sequence.insert(sequence.end(), sequences[right_idx].begin(), sequences[right_idx].end());
                            five_prime_monomers.erase(sequences[right_idx].front()); // remove from 5' monomers
                            used_sequences.emplace(right_idx);
                            continue;
                        }
                    }
                }
            }
            break;
        }
    }
    std::set<size_t> verified_options;
    for (auto idx : possible_double_chains)
    {
        if (verified_options.count(idx) > 0)
            continue;
        auto& sequence = sequences[idx];
        verified_options.emplace(idx);
        size_t bases_count = 0;
        for (auto it = sequence.begin(); it != sequence.end(); it++)
        {
            if (is_base(getMonomerClass(*it)))
                bases_count++;
        }
        bases_count_to_variants.at(bases_count).erase(idx);
        bool pair_found = false;
        for (auto& variant_idx : bases_count_to_variants.at(bases_count))
        {
            auto& variant = sequences[variant_idx];
            auto variant_it = variant.rbegin();
            auto sequence_it = sequence.begin();
            if (!is_base(getMonomerClass(*variant_it)))
                move_to_next_base(variant_it, variant.rend());
            if (!is_base(getMonomerClass(*sequence_it)))
                move_to_next_base(sequence_it, sequence.end());
            while (sequence_it != sequence.end() && variant_it != variant.rend())
            {
                auto& base = _monomers.at(*sequence_it);
                auto& variant_base = _monomers.at(*variant_it);
                if (base->hydrogenConnections().count(variant_base->ref()) == 0)
                    break;
                move_to_next_base(variant_it, variant.rend());
                move_to_next_base(sequence_it, sequence.end());
            }
            if (sequence_it == sequence.end() && variant_it == variant.rend())
            {
                pair_found = true;
                verified_options.emplace(variant_idx);
                bases_count_to_variants.at(bases_count).erase(variant_idx);
                joined_sequences.emplace_back(sequence, variant);
                break;
            }
        }
        if (!pair_found)
        {
            joined_sequences.emplace_back(sequence);
        }
    }
    // Sequences generated. Calculate macro properties
    rapidjson::StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    writer.StartArray();
    for (auto& sequence_arr : joined_sequences)
    {
        writer.StartObject();
        std::deque<std::string> sequence{sequence_arr.sequence};
        sequence.insert(sequence.begin(), sequence_arr.secondary_sequence.begin(), sequence_arr.secondary_sequence.end());
        std::vector<double> pKa_values;
        // in kDa(1000g/mol) (all chains)
        double mass_sum = -1;
        std::map<char, size_t> atoms_count;
        GROSS_UNITS gross_units;
        gross_units.resize(1);
        auto merge_gross_data = [&gross_units](const GROSS_UNITS& gross) {
            for (int i = 0; i < gross.size(); i++)
            {
                for (auto it : gross.at(i).isotopes)
                {
                    if (gross_units[0].isotopes.count(it.first) == 0)
                        gross_units[0].isotopes[it.first] = it.second;
                    else
                        gross_units[0].isotopes[it.first] += it.second;
                }
            }
        };
        for (auto& monomer_id : sequence)
        {
            auto& monomer = _monomers.at(monomer_id);
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            {
                mass_sum = -1;
                atoms_count.clear();
                break;
            }
            auto& monomer_template = _templates.at(monomer->templateId());
            if (monomer_template.unresolved())
            {
                mass_sum = -1;
                atoms_count.clear();
                break;
            }
            std::vector<int> leaved_atoms;
            auto& connections = monomer->connections();
            auto& att_points = monomer->attachmentPoints();
            for (auto& conn : connections)
            {
                auto it = att_points.find(conn.first);
                if (it == att_points.end())
                    throw Error("Internal error. Attachment point %s not found in monomer %s", conn.first.c_str(), monomer_id.c_str());
                auto& leaving_group = it->second.leavingGroup();
                if (!leaving_group.has_value())
                    continue;
                auto& leaved = leaving_group.value();
                leaved_atoms.insert(leaved_atoms.end(), leaved.begin(), leaved.end());
            }
            std::sort(leaved_atoms.rbegin(), leaved_atoms.rend());
            auto tgroup = monomer_template.getTGroup();
            auto* pmol = static_cast<Molecule*>(tgroup->fragment.get());
            Array<int> atom_filt;
            atom_filt.expandFill(pmol->vertexCount(), 1);
            for (auto& idx : leaved_atoms)
                atom_filt[idx] = 0;
            Filter atom_filter(atom_filt.ptr(), Filter::EQ, 1);
            pmol->selectAtoms(atom_filter);
            MoleculeMass mass;
            mass.mass_options.skip_error_on_pseudoatoms = true;
            mass_sum += mass.molecularWeight(*pmol);
            if (getMonomerClass(*monomer) == MonomerClass::AminoAcid)
            {
                pKa_values.emplace_back(Crippen::pKa(*pmol));
            }
            auto gross = MoleculeGrossFormula::collect(*pmol, true);
            merge_gross_data(*gross);
        }
        std::vector<std::string> sequence_ends;
        sequence_ends.emplace_back(sequence.front());
        if (sequence.back() != sequence.front())
            sequence_ends.emplace_back(sequence.back());
        for (auto& monomer_id : sequence_ends)
        {
            if (monomer_to_molecule.count(monomer_id) > 0)
            {
                auto& molecule_id = monomer_to_molecule.at(monomer_id).first;
                auto& mol_json = _json_molecules[_mol_ref_to_idx[molecule_id]];
                rapidjson::Value marr(rapidjson::kArrayType);
                marr.PushBack(_json_document.CopyFrom(mol_json, _json_document.GetAllocator()), _json_document.GetAllocator());
                MoleculeJsonLoader loader(marr);
                BaseMolecule* pbmol;
                Molecule mol;
                QueryMolecule qmol;
                try
                {
                    loader.loadMolecule(mol);
                    pbmol = &mol;
                    MoleculeMass mass;
                    mass.mass_options.skip_error_on_pseudoatoms = true;
                    mass_sum += mass.molecularWeight(mol);
                    auto gross = MoleculeGrossFormula::collect(*pbmol, true);
                    merge_gross_data(*gross);
                }
                catch (...)
                {
                    // query molecule just skipped
                }
            }
        }
        Array<char> gross_str;
        MoleculeGrossFormula::toString(gross_units, gross_str);
        writer.Key("grossFormula");
        writer.String(gross_str.ptr());
        if (mass_sum > 0)
        {
            writer.Key("mass");
            writer.Double(mass_sum);
        }

        // pKa (only peptides)
        auto pka_count = pKa_values.size();
        if (pka_count > 0)
        {
            double pKa;
            if (pka_count > 1)
            {
                std::sort(pKa_values.begin(), pKa_values.end());
                if (pka_count & 1) // odd
                {
                    pKa = pKa_values[pka_count / 2];
                }
                else // even - get average
                {
                    pKa = (pKa_values[pka_count / 2 - 1] + pKa_values[pka_count / 2]) / 2;
                }
            }
            else // only one value
            {
                pKa = pKa_values[0];
            }
            writer.Key("pKa");
            writer.Double(pKa);
        }

        // Melting temperature (only double stranded DNA)
        if (sequence_arr.secondary_sequence.size() > 0)
        {
            std::deque<std::string> bases;
            auto it = sequence_arr.sequence.begin();
            if (!is_base(getMonomerClass(*it)))
                move_to_next_base(it, sequence_arr.sequence.end());
            while (it != sequence_arr.sequence.end())
            {
                auto& monomer = _monomers.at(*it);
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    continue;
                auto& monomer_template = _templates.at(monomer->templateId());
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                {
                    bases.emplace_back(monomer_template.getStringProp("naturalAnalogShort"));
                }
                move_to_next_base(it, sequence_arr.sequence.end());
            }
            if (bases.size() > 0)
            {
                std::string left = bases.front();
                bases.pop_front();
                if (left == "U")
                    left = "T";
                size_t base_count = 1;
                size_t total_strength = 0;
                static const std::map<std::pair<std::string, std::string>, size_t> STRENGTH_PARAMS{
                    {{"C", "G"}, 13}, {{"C", "C"}, 11}, {{"G", "G"}, 11}, {{"C", "G"}, 10}, {{"A", "C"}, 10}, {{"T", "C"}, 8},
                    {{"A", "G"}, 8},  {{"T", "G"}, 7},  {{"G", "T"}, 10}, {{"C", "T"}, 8},  {{"G", "A"}, 8},  {{"C", "A"}, 7},
                    {{"A", "T"}, 7},  {{"T", "T"}, 5},  {{"A", "A"}, 5},  {{"T", "A"}, 4}};
                while (bases.size() > 0)
                {
                    auto right = bases.front();
                    bases.pop_front();
                    if (right == "U")
                        right = "T";
                    auto str_it = STRENGTH_PARAMS.find({left, right});
                    if (str_it == STRENGTH_PARAMS.end())
                        throw Error("Internal error. No strength params for %s and %s", left.c_str(), right.c_str());
                    total_strength += str_it->second;
                    base_count++;
                    left = right;
                }
                writer.Key("Tm");
                writer.Double(static_cast<double>(total_strength) / base_count);
            }
        }

        // Extinction coefficient (only peptides)
        {
            static const std::map<std::string, size_t> extinction_coefficients{{"C", 125}, {"W", 5500}, {"Y", 1490}};
            std::map<std::string, size_t> extinction_counts;
            for (auto& it : extinction_coefficients)
                extinction_counts.emplace(it.first, 0);
            size_t peptides_count = 0;
            for (auto monomer_id : sequence)
            {
                auto& monomer = _monomers.at(monomer_id);
                if (getMonomerClass(*monomer) != MonomerClass::AminoAcid)
                    continue;
                peptides_count++;
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    continue;
                auto& monomer_template = _templates.at(monomer->templateId());
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                {
                    auto it = extinction_counts.find(monomer_template.getStringProp("naturalAnalogShort"));
                    if (it != extinction_counts.end())
                    {
                        it->second++;
                    }
                }
            }
            if (peptides_count > 0)
            {
                size_t e_calc = 0;
                for (auto& it : extinction_counts)
                {
                    e_calc += it.second * extinction_coefficients.at(it.first);
                }
                writer.Key("extinctionCoefficient");
                writer.Uint64(e_calc);
            }
        }

        // Hydrophobicity (only peptides)
        std::vector<double> hydrophobicity;
        static const std::map<std::string, double> hydrophobicity_coefficients{
            {"A", 0.616}, {"G", 0.501}, {"M", 0.738}, {"S", 0.359}, {"C", 0.680}, {"H", 0.165}, {"N", 0.236}, {"T", 0.450}, {"D", 0.028}, {"I", 0.943},
            {"P", 0.711}, {"V", 0.825}, {"E", 0.043}, {"K", 0.283}, {"Q", 0.251}, {"W", 0.878}, {"F", 1.000}, {"L", 0.943}, {"R", 0.000}, {"Y", 0.880}};
        for (auto& monomer_id : sequence)
        {
            if (getMonomerClass(monomer_id) != MonomerClass::AminoAcid)
                continue;
            auto& monomer = _monomers.at(monomer_id);
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                continue;
            auto& monomer_template = _templates.at(monomer->templateId());
            if (monomer_template.hasStringProp("naturalAnalogShort"))
            {

                auto it = hydrophobicity_coefficients.find(monomer_template.getStringProp("naturalAnalogShort"));
                if (it != hydrophobicity_coefficients.end())
                    hydrophobicity.emplace_back(it->second);
            }
        }
        if (hydrophobicity.size() > 0)
        {
            writer.Key("hydrophobicity");
            writer.StartArray();
            for (auto value : hydrophobicity)
                writer.Double(value);
            writer.EndArray();
        }

        // Monomer count
        static const std::string peptides = "ACDEFGHIKLMNPQRSTVWY";
        static const std::string nucleotides = "ACGTU";
        std::map<std::string, size_t> peptides_count;
        std::map<std::string, size_t> nucleotides_count;
        for (auto ch : peptides)
            peptides_count.emplace(std::string(1, ch), 0);
        for (auto ch : nucleotides)
            nucleotides_count.emplace(std::string(1, ch), 0);
        static const std::string OTHER = "Other";
        peptides_count.emplace(OTHER, 0);
        nucleotides_count.emplace(OTHER, 0);
        for (auto& monomer_id : sequence)
        {
            auto& monomer = _monomers.at(monomer_id);
            auto monomer_class = getMonomerClass(*monomer);
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            {
                if (monomer_class == MonomerClass::AminoAcid)
                    peptides_count[OTHER]++;
                else if (monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA || monomer_class == MonomerClass::Sugar ||
                         monomer_class == MonomerClass::Phosphate || monomer_class == MonomerClass::Base)
                    nucleotides_count[OTHER]++;
                continue;
            }
            std::string natural_analog = "";
            auto& monomer_template = _templates.at(monomer->templateId());
            if (monomer_template.monomerClass() == MonomerClass::AminoAcid)
            {
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                    natural_analog = monomer_template.getStringProp("naturalAnalogShort");
                auto it = peptides_count.find(natural_analog);
                if (it == peptides_count.end())
                    peptides_count[OTHER]++;
                else
                    it->second++;
            }
            else if (is_base(monomer_template.monomerClass()))
            {
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                    natural_analog = monomer_template.getStringProp("naturalAnalogShort");
                auto it = nucleotides_count.find(natural_analog);
                if (it == nucleotides_count.end())
                    nucleotides_count[OTHER]++;
                else
                    it->second++;
            }
        }
        writer.Key("monomerCount");
        writer.StartObject();
        writer.Key("peptides");
        writer.StartObject();
        for (const auto& it : peptides_count)
        {
            if (it.second > 0)
            {
                writer.Key(it.first);
                writer.Uint64(it.second);
            }
        }
        writer.EndObject(); // peptides
        writer.Key("nucleotides");
        writer.StartObject();
        for (const auto& it : nucleotides_count)
        {
            if (it.second > 0)
            {
                writer.Key(it.first);
                writer.Uint64(it.second);
            }
        }
        writer.EndObject(); // nucleotides
        writer.EndObject(); // monomerCount
        writer.EndObject();
    }
    writer.EndArray();
    std::stringstream result;
    result << s.GetString();
    output.printf("%s", result.str().c_str());
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
