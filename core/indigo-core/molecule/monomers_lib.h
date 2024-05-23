#ifndef __monomers_lib__
#define __monomers_lib__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/exception.h"
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
        CHEM
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

    enum class IdtModification
    {
        FIVE_PRIME_END,
        INTERNAL,
        THREE_PRIME_END,
    };

    class DLLEXPORT IdtAlias
    {
    public:
        DECL_ERROR;
        IdtAlias(){};
        IdtAlias(const std::string& base) : _base(base), _five_prime_end("5" + base), _internal("i" + base), _three_prime_end("3" + base){};
        IdtAlias(const std::string& base, const std::string& five_prime_end, const std::string& internal, const std::string& three_prime_end)
            : _base(base), _five_prime_end(five_prime_end), _internal(internal), _three_prime_end(three_prime_end){};

        inline void setModifications(const std::string& five_prime_end, const std::string& internal, const std::string& three_prime_end)
        {
            _five_prime_end = five_prime_end;
            _internal = internal;
            _three_prime_end = three_prime_end;
        };

        inline bool hasModification(IdtModification modification) const
        {
            switch (modification)
            {
            case IdtModification::FIVE_PRIME_END:
                return hasFivePrimeEnd();
            case IdtModification::INTERNAL:
                return hasInternal();
            case IdtModification::THREE_PRIME_END:
                return hasThreePrimeEnd();
            };
            return false;
        }

        inline bool hasFivePrimeEnd() const
        {
            return _five_prime_end.size() != 0;
        }

        inline bool hasInternal() const
        {
            return _internal.size() != 0;
        }

        inline bool hasThreePrimeEnd() const
        {
            return _three_prime_end.size() != 0;
        }

        const std::string& getModification(IdtModification modification) const;
        const std::string& getFivePrimeEnd() const;
        const std::string& getInternal() const;
        const std::string& getThreePrimeEnd() const;

        inline static std::string IdtModificationToString(IdtModification mod)
        {
            switch (mod)
            {
            case IdtModification::FIVE_PRIME_END:
                return "five-prime end";
            case IdtModification::INTERNAL:
                return "internal";
            case IdtModification::THREE_PRIME_END:
                return "three-prime end";
            };
            return "unknown modification";
        };

    private:
        std::string _base;
        std::string _five_prime_end;
        std::string _internal;
        std::string _three_prime_end;
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

        MonomerTemplate(const MonomerTemplate& other);

        MonomerTemplate(const std::string& id, MonomerClass mt_class, const std::string& class_HELM, const std::string& full_name, const std::string& alias,
                        const std::string& natural_analog, int tgroup_id, BaseMolecule& mol);

        MonomerTemplate(const std::string& id, const std::string& mt_class, const std::string& class_HELM, const std::string& full_name,
                        const std::string& alias, const std::string& natural_analog, int tgroup_id, BaseMolecule& mol);

        static const std::string& MonomerClassToStr(MonomerClass monomer_type);
        static const MonomerClass StrToMonomerClass(const std::string& monomer_type);

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

        inline void setIdtAlias(const IdtAlias& idt_alias)
        {
            _idt_alias = idt_alias;
        }

        bool hasIdtAlias(const std::string& alias, IdtModification mod);

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

        inline const IdtAlias& idt_alias() const
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
            _monomer_templates.emplace(monomer_template.id(), monomer_template);
        };
        inline void addMonomerGroupTemplate(const MonomerGroupTemplate& monomer_group_template)
        {
            _monomer_group_templates.emplace(monomer_group_template.id(), (monomer_group_template));
        };

        const MonomerTemplate& getMonomerTemplateById(const std::string& monomer_template_id);
        const std::string& getMonomerTemplateIdByAlias(MonomerClass monomer_class, const std::string& monomer_template_alias);
        MonomerGroupTemplate& getMonomerGroupTemplateById(const std::string& monomer_template_id);

        const std::string& getMonomerTemplateIdByIdtAliasAndMod(const std::string& alias, IdtModification mod);
        const std::string& getMGTidByIdtAliasAndMod(const std::string& alias, IdtModification mod);

        const std::string& getIdtAliasByModification(IdtModification modification, const std::string sugar_id, const std::string base_id,
                                                     const std::string phosphate_id);

    protected:
        MonomerTemplateLibrary(){};

    private:
        std::map<std::string, MonomerTemplate> _monomer_templates;
        std::map<std::string, MonomerGroupTemplate> _monomer_group_templates;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
