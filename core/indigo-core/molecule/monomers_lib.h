#ifndef __monomers_lib__
#define __monomers_lib__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/exception.h"
#include "molecule/molecule_tgroups.h"
#include "molecule/monomers_defs.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace indigo
{
    class BaseMolecule;

    using MonomerKey = std::pair<MonomerClass, std::string>;
    using NucleotideKey = std::pair<NucleotideType, std::string>;
    using MonomersLib = std::unordered_map<MonomerKey, std::reference_wrapper<TGroup>, pair_hash>;
    using GranularNucleotide = std::unordered_map<MonomerClass, std::reference_wrapper<MonomersLib::value_type>>;

    class MonomerTemplateLibrary;

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
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
