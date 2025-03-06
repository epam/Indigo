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

#include "molecule/ket_objects.h"
#include "molecule/json_writer.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(KetObjWithProps, "Ket Options")

static std::map<std::string, int> empty_str_to_idx;

const std::map<std::string, int>& KetObjWithProps::getBoolPropStrToIdx() const
{
    return empty_str_to_idx;
};

const std::map<std::string, int>& KetObjWithProps::getIntPropStrToIdx() const
{
    return empty_str_to_idx;
};

const std::map<std::string, int>& KetObjWithProps::getStringPropStrToIdx() const
{
    return empty_str_to_idx;
};

void KetObjWithProps::setBoolProp(std::string name, bool value)
{
    auto& map = getBoolPropStrToIdx();
    auto it = map.find(name);
    if (it == map.end())
        throw Error("Unknown bool property '%s'", name.c_str());
    setBoolProp(it->second, value);
}

void KetObjWithProps::setIntProp(std::string name, int value)
{
    auto& map = getIntPropStrToIdx();
    auto it = map.find(name);
    if (it == map.end())
        throw Error("Unknown int property '%s'", name.c_str());
    setIntProp(it->second, value);
}

void KetObjWithProps::setStringProp(std::string name, std::string value)
{
    auto& map = getStringPropStrToIdx();
    auto it = map.find(name);
    if (it == map.end())
        throw Error("Unknown string property '%s'", name.c_str());
    setStringProp(it->second, value);
}

bool KetObjWithProps::getBoolProp(int idx) const
{
    auto it = _bool_props.find(idx);
    if (it == _bool_props.end())
        throw Error("Option %d not found", idx);
    return it->second;
};

int KetObjWithProps::getIntProp(int idx) const
{
    auto it = _int_props.find(idx);
    if (it == _int_props.end())
        throw Error("Option %d not found", idx);
    return it->second;
};

const std::string& KetObjWithProps::getStringProp(int idx) const
{
    auto it = _string_props.find(idx);
    if (it == _string_props.end())
        throw Error("Option %d not found", idx);
    return it->second;
};

static std::pair<bool, int> find_prop_idx(const std::map<std::string, int>& map, const std::string& name)
{
    auto it = map.find(name);
    if (it == map.end())
        return std::make_pair(false, -1);
    return std::make_pair(true, it->second);
}

std::pair<bool, int> KetObjWithProps::getBoolPropIdx(const std::string& name) const
{
    return find_prop_idx(getBoolPropStrToIdx(), name);
}

std::pair<bool, int> KetObjWithProps::getIntPropIdx(const std::string& name) const
{
    return find_prop_idx(getIntPropStrToIdx(), name);
}

std::pair<bool, int> KetObjWithProps::getStringPropIdx(const std::string& name) const
{
    return find_prop_idx(getStringPropStrToIdx(), name);
}

bool KetObjWithProps::getBoolProp(const std::string& name) const
{
    auto res = getBoolPropIdx(name);
    if (!res.first)
        throw Error("Bool property %s not found", name.c_str());
    return getBoolProp(res.second);
}
int KetObjWithProps::getIntProp(const std::string& name) const
{
    auto res = getIntPropIdx(name);
    if (!res.first)
        throw Error("Int property %s not found", name.c_str());
    return getIntProp(res.second);
};
const std::string& KetObjWithProps::getStringProp(const std::string& name) const
{
    auto res = getStringPropIdx(name);
    if (!res.first)
        throw Error("String property %s not found", name.c_str());
    return getStringProp(res.second);
};

void KetObjWithProps::parseOptsFromKet(const rapidjson::Value& json)
{
    // Parse bool props
    for (auto it : getBoolPropStrToIdx())
    {
        if (json.HasMember(it.first.c_str()))
            setBoolProp(it.second, json[it.first.c_str()].GetBool());
    }
    // Parse int props
    for (auto it : getIntPropStrToIdx())
    {
        if (json.HasMember(it.first.c_str()))
            setIntProp(it.second, json[it.first.c_str()].GetInt());
    }
    // Parse string props
    for (auto it : getStringPropStrToIdx())
    {
        if (json.HasMember(it.first.c_str()))
            setStringProp(it.second, json[it.first.c_str()].GetString());
    }
};

void KetObjWithProps::saveOptsToKet(JsonWriter& writer) const
{
    // Parse bool props
    std::map<int, std::string> boolPropIdxTostr;
    for (auto it : getBoolPropStrToIdx())
    {
        boolPropIdxTostr.emplace(it.second, it.first);
    }
    for (auto it : boolPropIdxTostr)
    {
        if (hasBoolProp(it.first))
        {
            writer.Key(it.second);
            writer.Bool(getBoolProp(it.second));
        }
    }

    // Parse int props
    std::map<int, std::string> intPropIdxTostr;
    for (auto it : getIntPropStrToIdx())
    {
        intPropIdxTostr.emplace(it.second, it.first);
    }
    for (auto it : intPropIdxTostr)
    {
        if (hasIntProp(it.first))
        {
            writer.Key(it.second);
            writer.Int(getIntProp(it.second));
        }
    }
    // Parse string props
    std::map<int, std::string> strPropIdxTostr;
    for (auto it : getStringPropStrToIdx())
    {
        strPropIdxTostr.emplace(it.second, it.first);
    }
    for (auto it : strPropIdxTostr)
    {
        if (hasStringProp(it.first))
        {
            writer.Key(it.second);
            writer.String(getStringProp(it.second));
        }
    }
};

IMPL_ERROR(KetQueryProperties, "Ket Query Properties")

const std::map<std::string, int>& KetQueryProperties::getIntPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"degree", toUType(IntProps::degree)},
        {"ringMembership", toUType(IntProps::ringMembership)},
        {"ringSize", toUType(IntProps::ringSize)},
        {"connectivity", toUType(IntProps::connectivity)},
        {"ringConnectivity", toUType(IntProps::ringConnectivity)},
        {"atomicMass", toUType(IntProps::atomicMass)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetQueryProperties::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"aromaticity", toUType(StringProps::aromaticity)},
        {"chirality", toUType(StringProps::chirality)},
    };
    return str_to_idx;
};

IMPL_ERROR(KetBaseAtom, "Ket Base Atom Type")

KetBaseAtom::atype KetBaseAtomType::stringToAtype(std::string atom_type)
{
    static std::map<std::string, atype> str_to_atype{{"atom", atype::atom}, {"atom-list", atype::atom_list}, {"rg-label", atype::rg_label}};
    auto it = str_to_atype.find(atom_type);
    if (it == str_to_atype.end())
        throw Error("Unknown atom type %s", atom_type.c_str());
    return it->second;
}

IMPL_ERROR(KetBaseAtomType, "Ket Base Atom")

const std::map<std::string, int>& KetBaseAtom::getIntPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"charge", toUType(IntProps::charge)},
        {"explicitValence", toUType(IntProps::explicitValence)},
        {"isotope", toUType(IntProps::isotope)},
        {"radical", toUType(IntProps::radical)},
        {"stereoParity", toUType(IntProps::stereoParity)},
        {"ringBondCount", toUType(IntProps::ringBondCount)},
        {"substitutionCount ", toUType(IntProps::substitutionCount)},
        {"hCount", toUType(IntProps::hCount)},
        {"implicitHCount", toUType(IntProps::implicitHCount)},
        {"mapping", toUType(IntProps::mapping)},
        {"invRet", toUType(IntProps::invRet)},
    };

    return str_to_idx;
};

const std::map<std::string, int>& KetBaseAtom::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"alias", toUType(StringProps::alias)},
        {"stereoLabel", toUType(StringProps::stereoLabel)},
        {"cip", toUType(StringProps::cip)},
    };

    return str_to_idx;
};

IMPL_ERROR(KetAtom, "Ket Atom")

const std::map<std::string, int>& KetAtom::getBoolPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"unsaturatedAtom", toUType(BoolProps::unsaturatedAtom)},
        {"exactChangeFlag", toUType(BoolProps::exactChangeFlag)},
    };

    if (_custom_query.has_value())
        return str_to_idx;

    return str_to_idx;
};

const std::map<std::string, int>& KetAtom::getIntPropStrToIdx() const
{
    if (_custom_query.has_value())
        return empty_str_to_idx;

    return KetBaseAtom::getIntPropStrToIdx();
};

const std::map<std::string, int>& KetAtom::getStringPropStrToIdx() const
{
    if (_custom_query.has_value())
        return empty_str_to_idx;

    return KetBaseAtom::getStringPropStrToIdx();
};

IMPL_ERROR(KetAtomList, "Ket Atom List")

const std::map<std::string, int>& KetAtomList::getBoolPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"unsaturatedAtom", toUType(BoolProps::unsaturatedAtom)},
        {"exactChangeFlag", toUType(BoolProps::exactChangeFlag)},
        {"notlist", toUType(BoolProps::notlist)},
    };

    return str_to_idx;
};

IMPL_ERROR(KetBond, "Ket Bond")

KetBond::KetBond(int bond_type, int atom1, int atom2) : _atoms(atom1, atom2)
{
    if (bond_type < toUType(bond_types::single) || bond_type > toUType(bond_types::hydrogen))
        throw Error("Invalid bond type %d", bond_type);
    _type = static_cast<bond_types>(bond_type);
}

const std::map<std::string, int>& KetBond::getIntPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"stereo", toUType(IntProps::stereo)},
        {"topology", toUType(IntProps::topology)},
        {"center", toUType(IntProps::center)},
        {"stereobox", toUType(IntProps::stereobox)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetBond::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"cip", toUType(StringProps::cip)},
    };
    return str_to_idx;
};

IMPL_ERROR(KetRUSGroup, "Ket RU SGroup")

const std::map<std::string, int>& KetRUSGroup::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"subscript", toUType(StringProps::subscript)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetSASGroupAttPoint::getIntPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"leavingAtom", toUType(IntProps::leavingAtom)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetSASGroupAttPoint::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"attachmentId", toUType(StringProps::attachmentId)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetSASGroup::getBoolPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"expanded", toUType(BoolProps::expanded)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetDataSGroup::getBoolPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"display", toUType(BoolProps::display)},
        {"placement", toUType(BoolProps::placement)},
    };
    return str_to_idx;
};

const std::map<std::string, int>& KetDataSGroup::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"context", toUType(StringProps::context)},
    };
    return str_to_idx;
};

IMPL_ERROR(KetMolecule, "Ket Molecule")

KetMolecule::atom_ptr& KetMolecule::addAtom(const std::string& label)
{
    _atoms.push_back(std::make_unique<KetAtom>(label));
    return _atoms.back();
};

KetMolecule::atom_ptr& KetMolecule::addAtom(const std::string& label, const std::string& custom_query)
{
    _atoms.push_back(std::make_unique<KetAtom>(label, custom_query));
    return _atoms.back();
};

KetMolecule::atom_ptr& KetMolecule::addAtomList(std::vector<std::string>& atom_list)
{
    _atoms.push_back(std::make_unique<KetAtomList>(atom_list));
    return _atoms.back();
};

KetMolecule::atom_ptr& KetMolecule::addRGLabel()
{
    _atoms.push_back(std::make_unique<KetRgLabel>());
    return _atoms.back();
};

KetMolecule::sgroup_ptr& KetMolecule::addMulSGroup(std::vector<int>& atoms, int mul)
{
    _sgroups.push_back(std::make_unique<KetMulSGroup>(atoms, mul));
    return _sgroups.back();
};

KetMolecule::sgroup_ptr& KetMolecule::addRUSGroup(std::vector<int>& atoms, std::string& connectivity)
{
    _sgroups.push_back(std::make_unique<KetRUSGroup>(atoms, connectivity));
    return _sgroups.back();
};

KetMolecule::sgroup_ptr& KetMolecule::addSASGroup(std::vector<int>& atoms, std::string& name)
{
    _sgroups.push_back(std::make_unique<KetSASGroup>(atoms, name));
    return _sgroups.back();
};

KetMolecule::sgroup_ptr& KetMolecule::addDataSGroup(std::vector<int>& atoms, std::string& name, std::string& data)
{
    _sgroups.push_back(std::make_unique<KetDataSGroup>(atoms, name, data));
    return _sgroups.back();
};

KetMolecule::sgroup_ptr& KetMolecule::addQueryComponentSGroup(std::vector<int>& atoms)
{
    _sgroups.push_back(std::make_unique<KetQueryComponentSGroup>(atoms));
    return _sgroups.back();
};

void KetMolecule::parseKetAtoms(KetMolecule::atoms_type& ket_atoms, const rapidjson::Value& atoms)
{
    for (SizeType i = 0; i < atoms.Size(); i++)
    {
        const Value& atom = atoms[i];

        KetBaseAtomType* atom_ptr;
        std::optional<KetQueryProperties> query_props;
        std::string atom_type = "atom";
        if (atom.HasMember("type"))
            atom_type = atom["type"].GetString();
        if (atom.HasMember("queryProperties"))
        {
            auto qProps = atom["queryProperties"].GetObject();
            if (qProps.HasMember("customQuery"))
            {
                ket_atoms.push_back(std::make_unique<KetAtom>(atom["label"].GetString(), qProps["customQuery"].GetString()));
                continue; // no other options allowed
            }
            else
            {
                KetQueryProperties q_props;
                q_props.parseOptsFromKet(atom["queryProperties"]);
                query_props = q_props;
            }
        }
        if (atom_type == "atom")
        {
            ket_atoms.push_back(std::make_unique<KetAtom>(atom["label"].GetString()));
            auto base_atom = ket_atoms.rbegin();
            if (query_props.has_value())
                static_cast<KetAtom*>(base_atom->get())->setQueryProperties(query_props.value());
            atom_ptr = base_atom->get();
        }
        else if (atom_type == "rg-label")
        {
            ket_atoms.push_back(std::make_unique<KetRgLabel>());
            auto rg_label = ket_atoms.rbegin();
            KetRgLabel* r_ptr = static_cast<KetRgLabel*>(rg_label->get());
            if (atom.HasMember("$refs"))
            {
                auto& refs = atom["$refs"];
                std::vector<std::string> ref_list;
                for (SizeType r = 0; r < refs.Size(); i++)
                {
                    ref_list.emplace_back(refs[r].GetString());
                }
                r_ptr->setRefs(ref_list);
            }
            if (atom.HasMember("attachmentOrder"))
            {
                const auto& rattachments = atom["attachmentOrder"];
                std::vector<std::pair<int, int>> att_order;
                for (SizeType j = 0; j < rattachments.Size(); ++j)
                {
                    att_order.emplace_back(rattachments[j]["attachmentAtom"].GetInt(), rattachments[j]["attachmentId"].GetInt());
                }
                r_ptr->setAttachmentOrder(att_order);
            }
        }
        else if (atom_type == "atom-list")
        {
            const Value& elements = atom["elements"];
            std::vector<std::string> elem_list;
            for (rapidjson::SizeType j = 0; j < elements.Size(); ++j)
            {
                elem_list.emplace_back(elements[j].GetString());
            }
            ket_atoms.push_back(std::make_unique<KetAtomList>(elem_list));
            // auto& base_atom = ket_atoms.rbegin();
            // if (query_props.has_value())
            //     static_cast<KetBaseAtom*>(base_atom->get())->setQueryProperties(query_props.value());
            // atom_ptr = base_atom->get();
        }
        else
            throw Error("invalid atom type: %s", atom_type.c_str());

        if (atom.HasMember("location"))
        {
            Vec3f location;
            const Value& coords = atom["location"];
            if (coords.Size() > 0)
            {
                location.x = coords[0].GetFloat();
                location.y = coords[1].GetFloat();
                location.z = coords[2].GetFloat();
                atom_ptr->setLocation(location);
            }
        }

        atom_ptr->parseOptsFromKet(atom);
    }
}

void KetMolecule::parseKetBonds(std::vector<KetBond>& ket_bonds, const rapidjson::Value& bonds)
{
    for (SizeType i = 0; i < bonds.Size(); i++)
    {
        const Value& bond = bonds[i];

        KetBond ket_bond(bond["type"].GetInt(), bond["atoms"][0].GetInt(), bond["atoms"][1].GetInt());
        ket_bond.parseOptsFromKet(bond);
        ket_bonds.emplace_back(ket_bond);
    }
};

void KetMolecule::parseKetSGroups(rapidjson::Value& sgroups)
{
    for (SizeType i = 0; i < sgroups.Size(); i++)
    {
        // const Value& sgroup = sgroups[i];
    }
}

IMPL_ERROR(KetAttachmentPoint, "Ket Attachment Point");

const std::map<std::string, int>& KetAttachmentPoint::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"label", toUType(StringProps::label)},
        {"type", toUType(StringProps::type)},
    };
    return str_to_idx;
}

IMPL_ERROR(KetBaseMonomer, "Ket Base Monomer")

void KetBaseMonomer::connectAttachmentPointTo(const std::string& ap_id, const std::string& monomer_ref, const std::string& other_ap_id)
{
    if (_attachment_points.find(ap_id) == _attachment_points.end())
        throw Error("Unknown attachment point '%s' in monomer %s", ap_id.c_str(), _alias.c_str());
    auto it = _connections.find(ap_id);
    if (it != _connections.end() && (it->second.first != monomer_ref || it->second.second != other_ap_id))
        throw Error("Monomer '%s' attachment point '%s' already connected to monomer'%s' attachment point '%s'", _alias.c_str(), ap_id.c_str(),
                    it->second.first.c_str(), it->second.second.c_str());
    if (it == _connections.end())
        _connections.try_emplace(ap_id, monomer_ref, other_ap_id);
}

IMPL_ERROR(KetMonomer, "Ket Monomer")

const std::map<std::string, int>& KetMonomer::getBoolPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"expanded", toUType(BoolProps::expanded)},
    };
    return str_to_idx;
}

const std::map<std::string, int>& KetMonomer::getIntPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"seqid", toUType(IntProps::seqid)},
    };
    return str_to_idx;
}

IMPL_ERROR(KetConnectionEndPoint, "Ket Connection End Point")

const std::map<std::string, int>& KetConnectionEndPoint::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"groupId", toUType(StringProps::groupId)},
        {"monomerId", toUType(StringProps::monomerId)},
        {"moleculeId", toUType(StringProps::moleculeId)},
        {"atomId", toUType(StringProps::atomId)},
        {"attachmentPointId", toUType(StringProps::attachmentPointId)},
    };
    return str_to_idx;
}

IMPL_ERROR(KetConnection, "Ket Connection")

KetConnection::KetConnection(KetConnection::TYPE conn_type, KetConnectionEndPoint ep1, KetConnectionEndPoint ep2) : _ep1(ep1), _ep2(ep2)
{
    switch (conn_type)
    {
    case TYPE::SINGLE:
        _connection_type = KetConnectionSingle;
        break;
    case TYPE::HYDROGEN:
        _connection_type = KetConnectionHydro;
        break;
    default:
        throw Error("Unknown connection type %d.", conn_type);
    }
}

const KetConnection::TYPE KetConnection::connType() const
{
    if (_connection_type == KetConnectionSingle)
        return TYPE::SINGLE;
    else if (_connection_type == KetConnectionHydro)
        return TYPE::HYDROGEN;
    else
        throw Error("Unknown connection type '%s'.", _connection_type.c_str());
}

const std::map<std::string, int>& KetConnection::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"label", toUType(StringProps::label)},
    };
    return str_to_idx;
}

IMPL_ERROR(KetAmbiguousMonomer, "Ket Ambiguous Monomer")

const std::map<std::string, int>& KetAmbiguousMonomer::getIntPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"seqid", toUType(IntProps::seqid)},
    };
    return str_to_idx;
}

const std::map<std::string, int>& KetAmbiguousMonomer::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"alias", toUType(StringProps::alias)},
    };
    return str_to_idx;
}

IMPL_ERROR(KetBaseMonomerTemplate, "Ket Base Monomer Template")

bool KetBaseMonomerTemplate::hasIdtAlias(const std::string& alias, IdtModification mod)
{
    if (_idt_alias.hasModification(mod) && (_idt_alias.getModification(mod) == alias))
        return true;
    return false;
}

bool KetBaseMonomerTemplate::hasIdtAliasBase(const std::string& alias_base)
{
    if (_idt_alias.getBase() == alias_base)
        return true;
    return false;
}

IMPL_ERROR(KetMonomerShape, "Monomer Shape")

KetMonomerShape::KetMonomerShape(const std::string& id, bool collapsed, const std::string& shape, Vec2f position, const std::vector<std::string>& monomers)
    : KetObjWithProps(), _id(id), _collapsed(collapsed), _shape(strToShapeType(shape)), _position(position), _monomers(monomers)
{
}

KetMonomerShape::shape_type KetMonomerShape::strToShapeType(std::string shape)
{
    static std::map<std::string, KetMonomerShape::shape_type> str_to_shape{
        {"generic", shape_type::generic},
        {"antibody", shape_type::antibody},
        {"double helix", shape_type::double_helix},
        {"globular protein", shape_type::globular_protein},
    };
    auto it = str_to_shape.find(shape);
    if (it == str_to_shape.end())
        throw Error("Unknown shape type %s", shape.c_str());
    return it->second;
}

std::string KetMonomerShape::shapeTypeToStr(shape_type shape)
{
    static std::map<KetMonomerShape::shape_type, std::string> shape_to_str{
        {shape_type::generic, "generic"},
        {shape_type::antibody, "antibody"},
        {shape_type::double_helix, "double helix"},
        {shape_type::globular_protein, "globular protein"},
    };
    auto it = shape_to_str.find(shape);
    if (it == shape_to_str.end())
        throw Error("Unknown shape type %d", shape);
    return it->second;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif