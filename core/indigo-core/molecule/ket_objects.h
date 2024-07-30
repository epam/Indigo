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

#ifndef __ket_objects__
#define __ket_objects__

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <rapidjson/document.h>

#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class MonomerTemplate;
    class JsonWriter;

    template <typename T>
    constexpr auto toUType(T enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(enumerator);
    }

    class DLLEXPORT KetObjWithProps
    {
    public:
        DECL_ERROR;

        inline void setBoolProp(int idx, bool value)
        {
            _bool_props[idx] = value;
        };

        inline void setIntProp(int idx, int value)
        {
            _int_props[idx] = value;
        };

        inline void setIntProp(int idx, std::size_t value)
        {
            _int_props[idx] = static_cast<int>(value);
        };

        inline void setStringProp(int idx, std::string value)
        {
            _string_props[idx] = value;
        };

        void setBoolProp(std::string name, bool value);
        void setIntProp(std::string name, int value);
        void setStringProp(std::string name, std::string value);

        virtual const std::map<std::string, int>& getBoolPropStrToIdx() const;
        virtual const std::map<std::string, int>& getIntPropStrToIdx() const;
        virtual const std::map<std::string, int>& getStringPropStrToIdx() const;

        inline bool hasBoolProp(int idx) const
        {
            return _bool_props.count(idx) > 0;
        };
        inline bool hasIntProp(int idx) const
        {
            return _int_props.count(idx) > 0;
        };
        inline bool hasStringProp(int idx) const
        {
            return _string_props.count(idx) > 0;
        };

        bool getBoolProp(int idx) const;
        int getIntProp(int idx) const;
        const std::string& getStringProp(int idx) const;

        std::pair<bool, int> getBoolPropIdx(const std::string& name) const;
        std::pair<bool, int> getIntPropIdx(const std::string& name) const;
        std::pair<bool, int> getStringPropIdx(const std::string& name) const;

        bool hasBoolProp(const std::string& name) const
        {
            auto res = getBoolPropIdx(name);
            if (res.first)
                return hasBoolProp(res.second);
            return false;
        }
        bool hasIntProp(const std::string& name) const
        {
            auto res = getIntPropIdx(name);
            if (res.first)
                return hasIntProp(res.second);
            return false;
        };
        bool hasStringProp(const std::string& name) const
        {
            auto res = getStringPropIdx(name);
            if (res.first)
                return hasStringProp(res.second);
            return false;
        };

        bool getBoolProp(const std::string& name) const;
        int getIntProp(const std::string& name) const;
        const std::string& getStringProp(const std::string& name) const;

        void parseOptsFromKet(const rapidjson::Value& json);
        void saveOptsToKet(JsonWriter& writer) const;

        void copy(const KetObjWithProps& other)
        {
            _bool_props = other._bool_props;
            _int_props = other._int_props;
            _string_props = other._string_props;
        }

    private:
        std::map<int, bool> _bool_props;
        std::map<int, int> _int_props;
        std::map<int, std::string> _string_props;
    };

    class KetQueryProperties : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        const std::map<std::string, int>& getIntPropStrToIdx() const override;
        const std::map<std::string, int>& getStringPropStrToIdx() const override;

    private:
        enum class IntProps
        {
            degree,
            ringMembership,
            ringSize,
            connectivity,
            ringConnectivity,
            atomicMass,
        };
        enum class StringProps
        {
            aromaticity,
            chirality,
        };
    };

    class DLLEXPORT KetBaseAtomType : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        enum class atype
        {
            atom,
            atom_list,
            rg_label,
        };

        explicit KetBaseAtomType() = delete;

        static atype stringToAtype(std::string atom_type);

        inline atype getType() const
        {
            return _type;
        };

        inline void setLocation(Vec3f location)
        {
            _location = location;
        }

        inline const std::optional<Vec3f>& location() const
        {
            return _location;
        };

    protected:
        KetBaseAtomType(atype atype) : _type(atype){};
        atype _type;
        std::optional<Vec3f> _location;
    };

    class DLLEXPORT KetBaseAtom : public KetBaseAtomType
    {
    public:
        DECL_ERROR;

        explicit KetBaseAtom() = delete;

        const std::map<std::string, int>& getIntPropStrToIdx() const override;
        const std::map<std::string, int>& getStringPropStrToIdx() const override;

        void setQueryProperties(KetQueryProperties& query_properties)
        {
            _query_properties = query_properties;
        };

        const std::optional<KetQueryProperties>& queryProperties() const
        {
            return _query_properties;
        };

    protected:
        KetBaseAtom(atype atype) : KetBaseAtomType(atype){};

    private:
        enum class IntProps
        {
            charge,
            explicitValence,
            isotope,
            radical,
            attachmentPoints,
            stereoParity,
            ringBondCount,
            substitutionCount,
            hCount,
            implicitHCount,
            mapping,
            invRet
        };
        enum class StringProps
        {
            alias,
            stereoLabel,
            cip
        };
        std::optional<KetQueryProperties> _query_properties;
    };

    class DLLEXPORT KetAtom : public KetBaseAtom
    {
    public:
        DECL_ERROR;

        KetAtom(const std::string& label) : KetBaseAtom(atype::atom), _label(label){};

        KetAtom(const std::string& label, const std::string& custom_query) : KetBaseAtom(atype::atom), _label(label), _custom_query(custom_query){};

        const std::map<std::string, int>& getBoolPropStrToIdx() const override;
        const std::map<std::string, int>& getIntPropStrToIdx() const override;
        const std::map<std::string, int>& getStringPropStrToIdx() const override;

        const std::string& label() const
        {
            return _label;
        };

        const std::optional<std::string>& customQuery() const
        {
            return _custom_query;
        };

    private:
        enum class BoolProps
        {
            unsaturatedAtom,
            exactChangeFlag
        };
        std::string _label;
        std::optional<std::string> _custom_query;
    };

    class DLLEXPORT KetAtomList : public KetBaseAtom
    {
    public:
        DECL_ERROR;

        KetAtomList(std::vector<std::string> atom_list) : KetBaseAtom(atype::atom), _atom_list(atom_list){};

        inline const std::vector<std::string>& atomList() const
        {
            return _atom_list;
        }

        const std::map<std::string, int>& getBoolPropStrToIdx() const override;

    private:
        enum class BoolProps
        {
            unsaturatedAtom,
            exactChangeFlag,
            notlist
        };
        std::vector<std::string> _atom_list;
    };

    class DLLEXPORT KetRgLabel : public KetBaseAtomType
    {
    public:
        using AttachemntOrder = std::vector<std::pair<int, int>>;
        KetRgLabel() : KetBaseAtomType(atype::atom), _attachmentOrder(), _refs(){};
        inline void setAttachmentOrder(AttachemntOrder& attOrder)
        {
            _attachmentOrder = attOrder;
        };
        inline const std::optional<AttachemntOrder>& attachmentOrder() const
        {
            return _attachmentOrder;
        };
        inline void setRefs(std::vector<std::string>& refs)
        {
            _refs = refs;
        };
        inline const std::optional<std::vector<std::string>>& refs() const
        {
            return _refs;
        };

    private:
        std::optional<AttachemntOrder> _attachmentOrder;
        std::optional<std::vector<std::string>> _refs;
    };

    class DLLEXPORT KetBond : public KetObjWithProps
    {
    public:
        DECL_ERROR;
        enum class bond_types
        {
            single = 1,
            double_bond,
            triple,
            aromatic,
            single_or_double,
            single_or_aromatic,
            double_or_aromatic,
            any,
            coordination,
            hydrogen
        };

        KetBond(int bond_type, int atom1, int atom2);
        KetBond(const KetBond& other) : KetObjWithProps(other), _type(other._type), _atoms(other._atoms){};

        const std::map<std::string, int>& getIntPropStrToIdx() const override;
        const std::map<std::string, int>& getStringPropStrToIdx() const override;

        inline bond_types getType() const
        {
            return _type;
        };

        inline const std::pair<int, int>& atoms() const
        {
            return _atoms;
        };

        inline void setStereoFlagPosition(float x, float y, float z = 0)
        {
            _stereo_flag_position.emplace(x, y, z);
        };

        inline bool hasStereoFlagPosition()
        {
            return _stereo_flag_position.has_value();
        };

        inline const Vec3f& stereoFlagPosition()
        {
            return _stereo_flag_position.value();
        }

    private:
        enum class IntProps
        {
            stereo,
            topology,
            center,
            stereobox
        };

        enum class StringProps
        {
            cip
        };

        bond_types _type;
        std::pair<int, int> _atoms;
        std::optional<Vec3f> _stereo_flag_position;
    };

    class DLLEXPORT KetBaseSGroup : public KetObjWithProps
    {
    public:
        DECL_ERROR;
        enum class SGroupType
        {
            multiple,
            repetition_unit,
            superatom,
            data,
            queryComponent
        };

    protected:
        KetBaseSGroup(SGroupType sg_type, std::vector<int>& atoms) : _type(sg_type), _atoms(atoms){};

    private:
        SGroupType _type;
        std::vector<int> _atoms;
    };

    class DLLEXPORT KetMulSGroup : public KetBaseSGroup
    {
    public:
        DECL_ERROR;
        KetMulSGroup(std::vector<int>& atoms, int mul) : KetBaseSGroup(SGroupType::multiple, atoms), _mul(mul){};

    private:
        int _mul;
    };

    class DLLEXPORT KetRUSGroup : public KetBaseSGroup
    {
    public:
        DECL_ERROR;
        KetRUSGroup(std::vector<int>& atoms, std::string& connectivity) : KetBaseSGroup(SGroupType::repetition_unit, atoms), _connectivity(connectivity){};

        const std::map<std::string, int>& getStringPropStrToIdx() const override;

    private:
        enum class StringProps
        {
            subscript
        };
        std::string _connectivity;
    };

    class DLLEXPORT KetSASGroupAttPoint : public KetObjWithProps
    {
    public:
        KetSASGroupAttPoint(int attachment_atom) : _attachment_atom(attachment_atom){};

        const std::map<std::string, int>& getIntPropStrToIdx() const override;
        const std::map<std::string, int>& getStringPropStrToIdx() const override;

    private:
        enum class IntProps
        {
            leavingAtom
        };
        enum class StringProps
        {
            attachmentId
        };
        int _attachment_atom;
    };

    class DLLEXPORT KetSASGroup : public KetBaseSGroup
    {
    public:
        DECL_ERROR;
        KetSASGroup(std::vector<int>& atoms, std::string& name) : KetBaseSGroup(SGroupType::superatom, atoms), _name(name){};

        const std::map<std::string, int>& getBoolPropStrToIdx() const override;

    private:
        enum class BoolProps
        {
            expanded
        };
        std::string _name;
    };

    class DLLEXPORT KetDataSGroup : public KetBaseSGroup
    {
    public:
        DECL_ERROR;
        KetDataSGroup(std::vector<int>& atoms, std::string& name, std::string& data) : KetBaseSGroup(SGroupType::data, atoms), _name(name), _data(data){};

        inline void setBonds(std::vector<int> bonds)
        {
            _bonds = bonds;
        };
        inline bool hasBonds()
        {
            return _bonds.has_value();
        };
        inline const std::vector<int>& bonds()
        {
            return _bonds.value();
        };

        const std::map<std::string, int>& getBoolPropStrToIdx() const override;
        const std::map<std::string, int>& getStringPropStrToIdx() const override;

    private:
        enum class BoolProps
        {
            display,
            placement
        };
        enum class StringProps
        {
            context
        };
        std::string _name;
        std::string _data;
        std::optional<std::vector<int>> _bonds;
    };

    class DLLEXPORT KetQueryComponentSGroup : public KetBaseSGroup
    {
    public:
        KetQueryComponentSGroup(std::vector<int>& atoms) : KetBaseSGroup(SGroupType::queryComponent, atoms){};
    };

    class DLLEXPORT KetMolecule
    {
    public:
        DECL_ERROR;
        KetMolecule() : _atoms(){};

        KetMolecule(const KetMolecule& other) = delete;
        KetMolecule& operator=(const KetMolecule&) = delete;

        using atom_ptr = std::shared_ptr<KetBaseAtomType>;
        using atoms_type = std::vector<atom_ptr>;

        using sgroup_ptr = std::unique_ptr<KetBaseSGroup>;
        using sgroups_type = std::vector<sgroup_ptr>;

        KetMolecule(KetMolecule&& other) : _atoms(std::move(other._atoms)), _bonds(std::move(other._bonds)), _sgroups(std::move(other._sgroups)){};

        atom_ptr& addAtom(const std::string& label);
        atom_ptr& addAtom(const std::string& label, const std::string& custom_query);
        atom_ptr& addAtomList(std::vector<std::string>& atom_list);
        atom_ptr& addRGLabel();

        sgroup_ptr& addMulSGroup(std::vector<int>& atoms, int mul);
        sgroup_ptr& addRUSGroup(std::vector<int>& atoms, std::string& connectivity);
        sgroup_ptr& addSASGroup(std::vector<int>& atoms, std::string& name);
        sgroup_ptr& addDataSGroup(std::vector<int>& atoms, std::string& name, std::string& data);
        sgroup_ptr& addQueryComponentSGroup(std::vector<int>& atoms);

        inline void setAtoms(atoms_type& atoms)
        {
            _atoms = std::move(atoms);
        };

        inline void addBond(KetBond& bond)
        {
            _bonds.emplace_back(bond);
        };

        inline const atoms_type& atoms() const
        {
            return _atoms;
        };

        inline atoms_type& writableAtoms()
        {
            return _atoms;
        };

        inline const std::vector<KetBond>& bonds() const
        {
            return _bonds;
        };

        inline std::vector<KetBond>& writableBonds()
        {
            return _bonds;
        };

        static void parseKetAtoms(atoms_type& ket_atoms, const rapidjson::Value& atoms);
        static void parseKetBonds(std::vector<KetBond>& ket_bonds, const rapidjson::Value& bonds);

        void parseKetAtoms(const rapidjson::Value& atoms)
        {
            parseKetAtoms(_atoms, atoms);
        }
        void parseKetBonds(const rapidjson::Value& bonds)
        {
            parseKetBonds(_bonds, bonds);
        }

        void parseKetSGroups(rapidjson::Value& sgroups);

    private:
        atoms_type _atoms;
        std::vector<KetBond> _bonds;
        sgroups_type _sgroups;
    };

    class DLLEXPORT KetMonomer : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        static inline const std::string ref_prefix = "monomer";
        KetMonomer(const std::string& id, const std::string& alias, const std::string& template_id) : _id(id), _alias(alias), _template_id(template_id){};

        inline void setPosition(const Vec2f& position)
        {
            _position = position;
        };

        inline void setPosition(const Vec3f& position)
        {
            _position = Vec2f(position.x, position.y);
        };

        const std::optional<Vec2f>& position() const
        {
            return _position;
        };

        const std::map<std::string, int>& getIntPropStrToIdx() const override;

        const std::string& id() const
        {
            return _id;
        };

        const std::string& alias() const
        {
            return _alias;
        };

        const std::string& templateId() const
        {
            return _template_id;
        };

    private:
        enum class IntProps
        {
            seqid
        };
        std::string _id;
        std::string _alias;
        std::string _template_id;
        std::optional<Vec2f> _position;
    };

    class DLLEXPORT KetConnectionEndPoint : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        const std::map<std::string, int>& getStringPropStrToIdx() const override;

    private:
        enum class StringProps
        {
            groupId,
            monomerId,
            attachmentPointId
        };
    };

    class DLLEXPORT KetConnection : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        KetConnection(KetConnectionEndPoint ep1, KetConnectionEndPoint ep2) : _connection_type("single"), _ep1(ep1), _ep2(ep2){};

        const std::map<std::string, int>& getStringPropStrToIdx() const override;

        inline const std::string connectionType() const
        {
            return _connection_type;
        };

        const KetConnectionEndPoint& ep1() const
        {
            return _ep1;
        };

        const KetConnectionEndPoint& ep2() const
        {
            return _ep2;
        };

    private:
        enum class StringProps
        {
            label,
        };
        std::string _connection_type;
        KetConnectionEndPoint _ep1;
        KetConnectionEndPoint _ep2;
    };

    class DLLEXPORT KetVariantMonomerOption : public KetObjWithProps
    {
    public:
        DECL_ERROR;
        KetVariantMonomerOption(std::string templateId) : _templateId(templateId){};

        const std::string& templateId() const
        {
            return _templateId;
        };

        void setProbability(float probability)
        {
            _probability = probability;
        };

        void setRatio(float ratio)
        {
            _ratio = ratio;
        };

        const std::optional<float> probability() const
        {
            return _probability;
        };

        const std::optional<float> ratio() const
        {
            return _ratio;
        };

    private:
        std::string _templateId;
        std::optional<float> _probability;
        std::optional<float> _ratio;
    };

    class DLLEXPORT KetVariantMonomerTemplate : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        inline static std::string ref_prefix = "variantMonomerTemplate-";

        KetVariantMonomerTemplate(const std::string& subtype, const std::string& id, const std::string& name,
                                  const std::vector<KetVariantMonomerOption>& options)
            : _subtype(subtype), _id(id), _name(name), _options(options){};

        const std::string& subtype() const
        {
            return _subtype;
        };

        const std::string& id() const
        {
            return _id;
        };

        const std::string& name() const
        {
            return _name;
        };

        const std::vector<KetVariantMonomerOption>& options() const
        {
            return _options;
        };

    private:
        std::string _subtype;
        std::string _id;
        std::string _name;
        std::vector<KetVariantMonomerOption> _options;
    };

    class DLLEXPORT KetVariantMonomer : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        inline static std::string ref_prefix = "variantMonomer-";

        KetVariantMonomer(std::string id, std::string template_id) : _id(id), _template_id(template_id){};

        inline void setPosition(const Vec3f& position)
        {
            _position = Vec2f(position.x, position.y);
        };

        const std::optional<Vec2f>& position() const
        {
            return _position;
        };

        const std::string& id() const
        {
            return _id;
        };

        const std::string& templateId() const
        {
            return _template_id;
        };

        const std::map<std::string, int>& getStringPropStrToIdx() const override;

    private:
        enum class StringProps
        {
            alias
        };
        std::string _id;
        std::string _template_id;
        std::optional<Vec2f> _position;
    };
}

#endif