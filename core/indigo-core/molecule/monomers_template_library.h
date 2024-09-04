#ifndef __monomers_template_library__
#define __monomers_template_library__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/exception.h"
#include "molecule/idt_alias.h"
#include "molecule/ket_objects.h"
#include "molecule/monomers_defs.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace indigo
{
    class MonomerTemplateLibrary;
    class TGroup;

    class DLLEXPORT MonomerTemplate : public KetBaseMonomerTemplate
    {
    public:
        DECL_ERROR;

        MonomerTemplate() = delete;
        MonomerTemplate(const MonomerTemplate& other) = delete;
        MonomerTemplate& operator=(const MonomerTemplate&) = delete;

        MonomerTemplate(const std::string& id, MonomerClass mt_class, IdtAlias idt_alias, bool unresolved)
            : KetBaseMonomerTemplate(TemplateType::MonomerTemplate, id, mt_class, idt_alias), _unresolved(unresolved){};

        MonomerTemplate(const std::string& id, std::string mt_class, IdtAlias idt_alias, bool unresolved)
            : KetBaseMonomerTemplate(TemplateType::MonomerTemplate, id, MonomerTemplate::StrToMonomerClass(mt_class), idt_alias), _unresolved(unresolved){};

        MonomerTemplate(MonomerTemplate&& other) = default;

        static inline const std::string ref_prefix = "monomerTemplate-";

        static const std::string& MonomerClassToStr(MonomerClass monomer_type)
        {
            static const std::map<MonomerClass, std::string> _type_to_str{
                {MonomerClass::AminoAcid, "AminoAcid"},
                {MonomerClass::Sugar, "Sugar"},
                {MonomerClass::Phosphate, "Phosphate"},
                {MonomerClass::Base, "Base"},
                {MonomerClass::Terminator, "Terminator"},
                {MonomerClass::Linker, "Linker"},
                {MonomerClass::Unknown, "Unknown"},
                {MonomerClass::CHEM, "CHEM"},
                {MonomerClass::DNA, "DNA"},
                {MonomerClass::RNA, "RNA"},
            };

            return _type_to_str.at(monomer_type);
        }

        static const MonomerClass StrToMonomerClass(const std::string& monomer_type)
        {
            static const std::map<std::string, MonomerClass> _str_to_type = {
                {"aminoacid", MonomerClass::AminoAcid},
                {"sugar", MonomerClass::Sugar},
                {"phosphate", MonomerClass::Phosphate},
                {"base", MonomerClass::Base},
                {"terminator", MonomerClass::Terminator},
                {"linker", MonomerClass::Linker},
                {"unknown", MonomerClass::Unknown},
                {"chem", MonomerClass::CHEM},
                {"dna", MonomerClass::DNA},
                {"rna", MonomerClass::RNA},
            };
            std::string mt = monomer_type;
            std::transform(mt.begin(), mt.end(), mt.begin(), [](unsigned char c) { return std::tolower(c); });
            if (_str_to_type.count(mt))
                return _str_to_type.at(mt);
            return MonomerClass::Unknown;
        }

        const std::map<std::string, int>& getStringPropStrToIdx() const override;

        KetAttachmentPoint& AddAttachmentPoint(const std::string& label, int att_atom);

        const KetAttachmentPoint& getAttachmenPointById(const std::string& att_point_id);

        std::unique_ptr<TGroup> getTGroup() const;

        inline bool hasAttachmenPointWithId(const std::string& att_point_id)
        {
            return _attachment_points.count(att_point_id) != 0;
        };

        inline const std::string& monomerClassStr() const
        {
            return MonomerClassToStr(_monomer_class);
        }

        inline bool unresolved() const
        {
            return _unresolved;
        };

        using atom_ptr = std::shared_ptr<KetBaseAtomType>;
        using atoms_type = std::vector<atom_ptr>;

        const atoms_type& atoms() const
        {
            return _atoms;
        }

        const std::vector<KetBond> bonds() const
        {
            return _bonds;
        }

        void parseAtoms(const rapidjson::Value& atoms)
        {
            KetMolecule::parseKetAtoms(_atoms, atoms);
        }

        void parseBonds(const rapidjson::Value& bonds)
        {
            KetMolecule::parseKetBonds(_bonds, bonds);
        }

        void copy(const MonomerTemplate& other)
        {
            KetBaseMonomerTemplate::copy(other);
            _unresolved = other._unresolved;
            _atoms = other._atoms;
            _bonds = other._bonds;
        }

        int AddAtom(const std::string& label, Vec3f location);

        int AddBond(int bond_type, int atom1, int atom2);

    private:
        enum class StringProps
        {
            classHELM,
            fullName,
            alias,
            naturalAnalog,
            naturalAnalogShort
        };
        bool _unresolved;
        atoms_type _atoms;
        std::vector<KetBond> _bonds;
    };

    class DLLEXPORT MonomerGroupTemplate
    {
    public:
        DECL_ERROR;
        MonomerGroupTemplate() = delete;
        MonomerGroupTemplate(const MonomerGroupTemplate& other) = delete;
        MonomerGroupTemplate& operator=(const MonomerGroupTemplate&) = delete;

        MonomerGroupTemplate(MonomerGroupTemplate&& other)
            : _id(other._id), _name(other._name), _class(other._class), _idt_alias(other._idt_alias), _monomer_templates(std::move(other._monomer_templates))

        {
            ;
        };

        MonomerGroupTemplate(const std::string& id, const std::string& name, const std::string& template_class)
            : _id(id), _name(name), _class(template_class){};

        MonomerGroupTemplate(const std::string& id, const std::string& name, const std::string& template_class, const std::string& idt_alias_base)
            : _id(id), _name(name), _class(template_class), _idt_alias(idt_alias_base){};

        MonomerGroupTemplate(const std::string& id, const std::string& name, const std::string& template_class, const IdtAlias& idt_alias)
            : _id(id), _name(name), _class(template_class), _idt_alias(idt_alias){};

        void addTemplate(MonomerTemplateLibrary& library, const std::string& template_id);

        bool hasIdtAlias(const std::string& alias, IdtModification mod);

        bool hasIdtAliasBase(const std::string& alias_base);

        inline bool hasTemplateClass(MonomerClass monomer_class)
        {
            for (auto& id_template : _monomer_templates)
            {
                if (id_template.second.get().monomerClass() == monomer_class)
                    return true;
            }
            return false;
        }

        const MonomerTemplate& getTemplateByClass(MonomerClass monomer_class) const;

        bool hasTemplate(MonomerClass monomer_class) const;

        bool hasTemplate(MonomerClass monomer_class, const std::string monomer_id) const;

        inline const std::string& id() const
        {
            return _id;
        }

        inline const IdtAlias& idtAlias() const
        {
            return _idt_alias;
        }

    private:
        std::string _id;
        std::string _name;
        std::string _class;

        IdtAlias _idt_alias;
        std::map<std::string, std::reference_wrapper<const MonomerTemplate>> _monomer_templates;
    };

    class DLLEXPORT MonomerTemplateLibrary
    {
    public:
        DECL_ERROR;

        MonomerTemplateLibrary(){};

        MonomerTemplateLibrary(const MonomerTemplateLibrary&) = delete;
        MonomerTemplateLibrary(MonomerTemplateLibrary&&) = delete;
        MonomerTemplateLibrary& operator=(const MonomerTemplateLibrary&) = delete;
        MonomerTemplateLibrary& operator=(MonomerTemplateLibrary&&) = delete;

        MonomerTemplate& addMonomerTemplate(const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved = false);

        inline void addMonomerGroupTemplate(MonomerGroupTemplate&& monomer_group_template)
        {
            auto res = _monomer_group_templates.emplace(monomer_group_template.id(), std::move(monomer_group_template));
            if (res.second)
                for (auto modification : {IdtModification::FIVE_PRIME_END, IdtModification::INTERNAL, IdtModification::THREE_PRIME_END})
                {
                    if (monomer_group_template.idtAlias().hasModification(modification))
                    {
                        const std::string& alias = monomer_group_template.idtAlias().getModification(modification);
                        _id_alias_to_monomer_group_templates.emplace(alias, std::make_pair(std::ref(res.first->second), modification));
                    }
                }
        }

        const MonomerTemplate& getMonomerTemplateById(const std::string& monomer_template_id);
        const std::string& getMonomerTemplateIdByAlias(MonomerClass monomer_class, const std::string& monomer_template_alias);
        MonomerGroupTemplate& getMonomerGroupTemplateById(const std::string& monomer_template_id);

        const std::string& getMonomerTemplateIdByIdtAliasBase(const std::string& alias_base);
        const std::string& getMGTidByIdtAliasBase(const std::string& alias_base);

        const std::string& getMonomerTemplateIdByIdtAlias(const std::string& alias, IdtModification& mod);
        const std::string& getMGTidByIdtAlias(const std::string& alias, IdtModification& mod);

        const std::string& getIdtAliasByModification(IdtModification modification, const std::string sugar_id, const std::string base_id,
                                                     const std::string phosphate_id);

    private:
        std::map<std::string, MonomerTemplate> _monomer_templates;
        std::map<std::string, MonomerGroupTemplate> _monomer_group_templates;
        std::map<std::string, std::pair<MonomerTemplate&, IdtModification>> _id_alias_to_monomer_templates;
        std::map<std::string, std::pair<MonomerGroupTemplate&, IdtModification>> _id_alias_to_monomer_group_templates;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
