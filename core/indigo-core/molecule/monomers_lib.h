#ifndef __monomers_lib__
#define __monomers_lib__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/exception.h"
#include "molecule/idt_alias.h"
#include "molecule/molecule_tgroups.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace indigo
{
    // composite unordered_map key for pair
    const auto kCompositeKeyHashMagicNumber = 0x9e3779b9;
    template <class T>
    inline void hash_combine(size_t& seed, T const& v)
    {
        seed ^= std::hash<T>()(v) + kCompositeKeyHashMagicNumber + (seed << 6) + (seed >> 2);
    }

    // universal fuctor for pair hash calculation
    struct pair_hash
    {
        template <class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const
        {
            size_t seed = 0;
            hash_combine(seed, p.first);
            hash_combine(seed, p.second);
            return seed;
        }
    };

    class BaseMolecule;

    enum class MonomerClass
    {
        AminoAcid,
        Sugar,
        Phosphate,
        Base,
        Terminator,
        Linker,
        Unknown,
        CHEM,
        DNA,
        RNA
    };

    enum class NucleotideType
    {
        RNA,
        DNA
    };

    using MonomerKey = std::pair<MonomerClass, std::string>;
    using NucleotideKey = std::pair<NucleotideType, std::string>;
    using MonomersLib = std::unordered_map<MonomerKey, std::reference_wrapper<TGroup>, pair_hash>;
    using GranularNucleotide = std::unordered_map<MonomerClass, std::reference_wrapper<MonomersLib::value_type>>;

    // singleton MonomerTemplates class

    class DLLEXPORT MonomerTemplates
    {
        DECL_ERROR;

    public:
        MonomerTemplates(const MonomerTemplates&) = delete;
        MonomerTemplates(MonomerTemplates&&) = delete;
        MonomerTemplates& operator=(const MonomerTemplates&) = delete;
        MonomerTemplates& operator=(MonomerTemplates&&) = delete;
        static const MonomerTemplates& _instance();
        static bool getMonomerTemplate(MonomerClass mon_type, std::string alias, TGroup& tgroup);
        static bool getMonomerTemplate(std::string mon_type, std::string alias, TGroup& tgroup);
        static bool splitNucleotide(NucleotideType nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide);
        static bool splitNucleotide(std::string nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide);
        static const std::string& classToStr(MonomerClass mon_type);
        static const std::unordered_map<std::string, MonomerClass>& getStrToMonomerType();

    private:
        MonomerTemplates();
        void initializeMonomers();
        ~MonomerTemplates() = default;
        MonomersLib _monomers_lib;
        std::unique_ptr<BaseMolecule> _templates_mol;
        std::unordered_map<NucleotideKey, GranularNucleotide, pair_hash> _nucleotides_lib;
        std::unordered_map<std::string, std::shared_ptr<BaseMolecule>> _aminoacids_lib;
    };

    enum class AttachmentPointType
    {
        LEFT,
        RIGHT,
        SIDE
    };

    class DLLEXPORT AttachmentPoint
    {
    public:
        DECL_ERROR;

        AttachmentPoint() = delete;

        AttachmentPoint(const std::string& label, AttachmentPointType ap_type, int att_atom, std::vector<int>& leaving_group)
            : _label(label), _type(ap_type), _attachment_atom(att_atom), _leaving_group(leaving_group){};

        AttachmentPoint(const AttachmentPoint& second)
        {
            _label = second._label;
            _type = second._type;
            _attachment_atom = second._attachment_atom;
            _leaving_group = second._leaving_group;
        }

        inline const std::string& label() const
        {
            return _label;
        };

    private:
        std::string _label;
        AttachmentPointType _type;
        int _attachment_atom;
        std::vector<int> _leaving_group;
    };

    class DLLEXPORT MonomerTemplate
    {
    public:
        DECL_ERROR;

        MonomerTemplate() = delete;

        MonomerTemplate(const std::string& id, MonomerClass mt_class, const std::string& class_HELM, const std::string& full_name, const std::string& alias,
                        const std::string& natural_analog, bool unresolved, const TGroup& tgroup)
            : _id(id), _class(mt_class), _class_HELM(class_HELM), _full_name(full_name), _alias(alias), _natural_analog(natural_analog), _unresolved(unresolved)
        {
            _tgroup.copy(tgroup);
        }

        MonomerTemplate(const MonomerTemplate& other)
            : _id(other._id), _class(other._class), _class_HELM(other._class_HELM), _full_name(other._full_name), _alias(other._alias),
              _natural_analog(other._natural_analog), _idt_alias(other._idt_alias), _unresolved(other._unresolved)
        {
            _tgroup.copy(other._tgroup);
        }

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
                {MonomerClass::CHEM, "Chem"},
                {MonomerClass::DNA, "DNA"},
                {MonomerClass::RNA, "RNA"},
            };

            return _type_to_str.at(monomer_type);
        }

        static const MonomerClass StrToMonomerClass(const std::string& monomer_type)
        {
            static const std::map<std::string, MonomerClass> _str_to_type = {
                {"AminoAcid", MonomerClass::AminoAcid},
                {"Sugar", MonomerClass::Sugar},
                {"Phosphate", MonomerClass::Phosphate},
                {"Base", MonomerClass::Base},
                {"Terminator", MonomerClass::Terminator},
                {"Linker", MonomerClass::Linker},
                {"Unknown", MonomerClass::Unknown},
                {"Chem", MonomerClass::CHEM},
                {"DNA", MonomerClass::DNA},
                {"RNA", MonomerClass::RNA},
            };
            if (_str_to_type.count(monomer_type))
                return _str_to_type.at(monomer_type);
            return MonomerClass::Unknown;
        }

        inline void AddAttachmentPoint(const AttachmentPoint& att_point)
        {
            _attachment_points.insert(std::pair<std::string, AttachmentPoint>(att_point.label(), att_point));
        };

        void AddAttachmentPoint(const std::string& id, const std::string& ap_type, int att_atom, std::vector<int>& leaving_group);

        const AttachmentPoint& getAttachmenPointById(const std::string& att_point_id);

        const TGroup& getTGroup() const;

        inline bool hasAttachmenPointWithId(const std::string& att_point_id)
        {
            return _attachment_points.count(att_point_id) != 0;
        };

        inline const std::string& id() const
        {
            return _id;
        };

        inline MonomerClass monomerClass() const
        {
            return _class;
        }

        inline const std::string& monomerClassStr() const
        {
            return MonomerClassToStr(_class);
        }

        inline const std::string& classHELM() const
        {
            return _class_HELM;
        }

        inline const std::string& fullName() const
        {
            return _full_name;
        }

        inline const std::string& alias() const
        {
            return _alias;
        }

        inline const std::string& naturalAnalog() const
        {
            return _natural_analog;
        }

        inline const IdtAlias& idtAlias() const
        {
            return _idt_alias;
        }

        inline void setIdtAlias(const IdtAlias& idt_alias)
        {
            _idt_alias = idt_alias;
        }

        bool hasIdtAlias(const std::string& alias, IdtModification mod);

        bool hasIdtAliasBase(const std::string& alias_base);

    private:
        std::string _id;
        MonomerClass _class;
        std::string _class_HELM;
        std::string _full_name;
        std::string _alias;
        std::string _natural_analog;
        std::map<std::string, AttachmentPoint> _attachment_points;
        std::string molecule;
        IdtAlias _idt_alias;
        bool _unresolved;
        TGroup _tgroup;
    };

    class DLLEXPORT MonomerGroupTemplate
    {

    public:
        DECL_ERROR;
        MonomerGroupTemplate() = delete;

        MonomerGroupTemplate(const std::string& id, const std::string& name, const std::string& template_class)
            : _id(id), _name(name), _class(template_class){};

        MonomerGroupTemplate(const std::string& id, const std::string& name, const std::string& template_class, const std::string& idt_alias_base)
            : _id(id), _name(name), _class(template_class), _idt_alias(idt_alias_base){};

        MonomerGroupTemplate(const std::string& id, const std::string& name, const std::string& template_class, const IdtAlias& idt_alias)
            : _id(id), _name(name), _class(template_class), _idt_alias(idt_alias){};

        void addTemplate(const std::string& template_id);

        bool hasIdtAlias(const std::string& alias, IdtModification mod);

        bool hasIdtAliasBase(const std::string& alias_base);

        inline bool hasTemplateClass(MonomerClass monomer_class)
        {
            for (auto& id_template : _monomer_templates)
            {
                if (id_template.second.monomerClass() == monomer_class)
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
        std::map<std::string, MonomerTemplate> _monomer_templates;
    };

    class DLLEXPORT MonomerTemplateLibrary
    {
    public:
        DECL_ERROR;

        MonomerTemplateLibrary(const MonomerTemplateLibrary&) = delete;
        MonomerTemplateLibrary(MonomerTemplateLibrary&&) = delete;
        MonomerTemplateLibrary& operator=(const MonomerTemplateLibrary&) = delete;
        MonomerTemplateLibrary& operator=(MonomerTemplateLibrary&&) = delete;
        static MonomerTemplateLibrary& instance();

        inline void addMonomerTemplate(MonomerTemplate& monomer_template)
        {
            _monomer_templates.erase(monomer_template.id());
            auto res = _monomer_templates.emplace(monomer_template.id(), monomer_template);
            if (res.second)
                for (auto modification : {IdtModification::FIVE_PRIME_END, IdtModification::INTERNAL, IdtModification::THREE_PRIME_END})
                {
                    if (monomer_template.idtAlias().hasModification(modification))
                    {
                        const std::string& alias = monomer_template.idtAlias().getModification(modification);
                        MonomerTemplate& templ_ref = res.first->second;
                        _id_alias_to_monomer_templates.emplace(alias, std::make_pair(std::ref(templ_ref), modification));
                    }
                }
        }

        inline void addMonomerGroupTemplate(const MonomerGroupTemplate& monomer_group_template)
        {
            _monomer_group_templates.erase(monomer_group_template.id());
            auto res = _monomer_group_templates.emplace(monomer_group_template.id(), monomer_group_template);
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

    protected:
        MonomerTemplateLibrary(){};

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
