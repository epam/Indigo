#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>

#define RAPIDJSON_HAS_STDSTRING 1

#include <cppcodec/base64_rfc4648.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "indigo-inchi.h"
#include "indigo.h"

#ifndef INDIGO_NO_RENDER
#include "indigo-renderer.h"
#endif

namespace indigo
{
    using cstring = const char*;

    EM_JS(void, jsThrow, (cstring str), { throw UTF8ToString(str); });

    EM_JS(void, print_jsn, (cstring str, int n), { console.log(UTF8ToString(str) + n); });
#ifdef _DEBUG
    EM_JS(void, print_js, (cstring str), { console.log(UTF8ToString(str)); });
#else
    void print_js(const cstring)
    {
    }
#endif

    int _checkResult(int result)
    {
        if (result < 0)
        {
            jsThrow(indigoGetLastError());
        }
        return result;
    }

    double _checkResultFloat(double result)
    {
        if (result < 0.5)
        {
            jsThrow(indigoGetLastError());
        }
        return result;
    }

    cstring _checkResultString(cstring result)
    {
        if (result == nullptr)
        {
            jsThrow(indigoGetLastError());
        }
        return result;
    }

    struct IndigoObject
    {
        const int id;

        IndigoObject(const IndigoObject&) = delete;
        IndigoObject(IndigoObject&&) = default;
        IndigoObject operator=(const IndigoObject&) = delete;
        IndigoObject& operator=(IndigoObject&&) = delete;

        explicit IndigoObject(const int id) : id(id)
        {
        }

        ~IndigoObject()
        {
            indigoFree(id);
        }
    };

    struct IndigoKetcherObject
    {
        enum KOType
        {
            EKETMolecule,
            EKETMoleculeQuery,
            EKETReaction,
            EKETReactionQuery,
            EKETDocument,
        };
        KOType objtype;

    private:
        std::shared_ptr<const IndigoObject> indigo_object;
        std::shared_ptr<const IndigoObject> parent = nullptr;

    public:
        explicit IndigoKetcherObject(const int id, const KOType type, std::shared_ptr<const IndigoObject> parent = nullptr)
            : indigo_object(std::make_shared<IndigoObject>(id)), objtype(type), parent(std::move(parent))
        {
        }

        void set(const int id, const KOType type, std::shared_ptr<const IndigoObject> parent = nullptr)
        {
            indigo_object = std::make_shared<IndigoObject>(id);
            objtype = type;
        }

        std::string toString(const std::map<std::string, std::string>& options, const std::string& outputFormat, int library = -1) const
        {
            print_js("toString:");
            std::string result;
            if (outputFormat == "molfile" || outputFormat == "rxnfile" || outputFormat == "chemical/x-mdl-molfile" || outputFormat == "chemical/x-mdl-rxnfile")
            {
                if (is_reaction())
                    result = _checkResultString(indigoRxnfile(id()));
                else
                    result = _checkResultString(indigoMolfile(id()));
            }
            else if (outputFormat == "smiles" || outputFormat == "chemical/x-daylight-smiles" || outputFormat == "chemical/x-chemaxon-cxsmiles")
            {
                if (options.count("smiles") > 0 && options.at("smiles") == "canonical")
                {
                    result = _checkResultString(indigoCanonicalSmiles(id()));
                }
                else
                {
                    if (outputFormat == "chemical/x-chemaxon-cxsmiles")
                        indigoSetOption("smiles-saving-format", "chemaxon");
                    else if (outputFormat == "chemical/x-daylight-smiles")
                        indigoSetOption("smiles-saving-format", "daylight");

                    result = _checkResultString(indigoSmiles(id()));
                }
            }
            else if (outputFormat == "sequence" || outputFormat == "chemical/x-sequence")
            {
                result = _checkResultString(indigoSequence(id(), library));
            }
            else if (outputFormat == "peptide-sequence-3-letter" || outputFormat == "chemical/x-peptide-sequence-3-letter")
            {
                result = _checkResultString(indigoSequence3Letter(id(), library));
            }
            else if (outputFormat == "fasta" || outputFormat == "chemical/x-fasta")
            {
                result = _checkResultString(indigoFasta(id(), library));
            }
            else if (outputFormat == "idt" || outputFormat == "chemical/x-idt")
            {
                result = _checkResultString(indigoIdt(id(), library));
            }
            else if (outputFormat == "helm" || outputFormat == "chemical/x-helm")
            {
                result = _checkResultString(indigoHelm(id(), library));
            }
            else if (outputFormat == "smarts" || outputFormat == "chemical/x-daylight-smarts")
            {
                if (options.count("smarts") > 0 && options.at("smarts") == "canonical")
                    result = _checkResultString(indigoCanonicalSmarts(id()));
                else
                    result = _checkResultString(indigoSmarts(id()));
            }
            else if (outputFormat == "cml" || outputFormat == "chemical/x-cml")
            {
                result = _checkResultString(indigoCml(id()));
            }
            else if (outputFormat == "cdxml" || outputFormat == "chemical/x-cdxml")
            {
                result = _checkResultString(indigoCdxml(id()));
            }
            else if (outputFormat == "cdx" || outputFormat == "chemical/x-cdx")
            {
                result = _checkResultString(indigoCdxBase64(id()));
            }
            else if (outputFormat == "inchi" || outputFormat == "chemical/x-inchi")
            {
                result = _checkResultString(indigoInchiGetInchi(id()));
            }
            else if (outputFormat == "inchi-key" || outputFormat == "chemical/x-inchi-key")
            {
                std::string inchi_str = _checkResultString(indigoInchiGetInchi(id()));
                result = _checkResultString(indigoInchiGetInchiKey(inchi_str.c_str()));
            }
            else if (outputFormat == "inchi-aux" || outputFormat == "chemical/x-inchi-aux")
            {
                std::stringstream ss;
                ss << _checkResultString(indigoInchiGetInchi(id())) << '\n' << _checkResultString(indigoInchiGetAuxInfo());
                result = ss.str();
            }
            else if (outputFormat == "ket" || outputFormat == "chemical/x-indigo-ket")
            {
                print_js(outputFormat.c_str());
                result = _checkResultString(indigoJson(id()));
            }
            else if (outputFormat == "sdf" || outputFormat == "chemical/x-sdf")
            {
                auto buffer = IndigoObject(_checkResult(indigoWriteBuffer()));
                auto comp_it = (objtype == EKETMolecule || objtype == EKETMoleculeQuery) ? IndigoObject(_checkResult(indigoIterateComponents(id())))
                                                                                         : IndigoObject(_checkResult(indigoIterateMolecules(id())));
                while (indigoHasNext(comp_it.id))
                {
                    const auto frag = IndigoObject(_checkResult(indigoNext(comp_it.id)));
                    const auto mol = IndigoObject(_checkResult(indigoClone(frag.id)));
                    indigoSdfAppend(buffer.id, mol.id);
                }
                print_js(outputFormat.c_str());
                result = _checkResultString(indigoToString(buffer.id));
            }
            else if (outputFormat == "rdf" || outputFormat == "chemical/x-rdf")
            {
                auto buffer = IndigoObject(_checkResult(indigoWriteBuffer()));
                auto reac_it = IndigoObject(_checkResult(indigoIterateReactions(id())));
                indigoRdfHeader(buffer.id);
                while (indigoHasNext(reac_it.id))
                {
                    const auto reac_obj = IndigoObject(_checkResult(indigoNext(reac_it.id)));
                    const auto reac = IndigoObject(_checkResult(indigoClone(reac_obj.id)));
                    indigoRdfAppend(buffer.id, reac.id);
                }
                print_js(outputFormat.c_str());
                result = _checkResultString(indigoToString(buffer.id));
            }
            else
            {
                std::stringstream ss;
                ss << "Unknown output format: " << outputFormat;
                jsThrow(ss.str().c_str());
                return ""; // suppress warning
            }
            auto out = options.find("output-content-type");
            if (out != options.end() && out->second == "application/json")
            {
                std::string originalFormat = _checkResultString(indigoGetOriginalFormat(id()));
                rapidjson::Document resultJson;
                auto& allocator = resultJson.GetAllocator();
                resultJson.SetObject();
                resultJson.AddMember("struct", result, allocator);
                resultJson.AddMember("format", outputFormat, allocator);
                resultJson.AddMember("original_format", originalFormat, allocator);
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                resultJson.Accept(writer);
                return buffer.GetString();
            }
            else
            {
                return result;
            }
        }

        IndigoKetcherObject substructure(const std::vector<int>& selected_atoms) const
        {
            std::vector<int> mutable_selected_atoms(selected_atoms);
            int* selected_atoms_array = &mutable_selected_atoms[0];
            return IndigoKetcherObject(_checkResult(indigoGetSubmolecule(id(), static_cast<int>(selected_atoms.size()), selected_atoms_array)), objtype,
                                       indigo_object);
        }

        int id() const
        {
            return indigo_object->id;
        }

        bool is_reaction() const
        {
            return objtype == EKETReaction || objtype == EKETReactionQuery;
        }
    };

    void indigoSetOptions(const std::map<std::string, std::string>& options)
    {
        std::set<std::string> to_skip{"smiles", "smarts", "input-format", "output-content-type", "monomerLibrary", "sequence-type", "upc", "nac"};
        for (const auto& option : options)
        {
            if (to_skip.count(option.first) < 1)
            {
                _checkResult(indigoSetOption(option.first.c_str(), option.second.c_str()));
            }
        }
    }

    class IndigoSession
    {
    private:
        const qword id;

    public:
        IndigoSession() : id(indigoAllocSessionId())
        {
            indigoSetSessionId(id);
            _checkResult(indigoInchiInit(id));
        }

        ~IndigoSession()
        {
            indigoInchiDispose(id);
            indigoReleaseSessionId(id);
        }

        IndigoSession(const IndigoSession&) = delete;
        IndigoSession& operator=(const IndigoSession&) = delete;
        IndigoSession(IndigoSession&&) = delete;
        IndigoSession& operator=(IndigoSession&&) = delete;

        qword getSessionId() const
        {
            return id;
        }
    };

    class IndigoRendererSession
    {
    private:
        const qword _sessionId;

    public:
        explicit IndigoRendererSession(const qword sessionId) : _sessionId(sessionId)
        {
#ifndef INDIGO_NO_RENDER
            indigoRendererInit(_sessionId);
#else
            jsThrow("No renderer included in this wasm bundle!");
#endif
        }

        ~IndigoRendererSession()
        {
#ifndef INDIGO_NO_RENDER
            indigoRendererDispose(_sessionId);
#else
            jsThrow("No renderer included in this wasm bundle!");
#endif
        }

        IndigoRendererSession(const IndigoRendererSession&) = delete;
        IndigoRendererSession& operator=(const IndigoRendererSession&) = delete;
        IndigoRendererSession(IndigoRendererSession&&) = delete;
        IndigoRendererSession& operator=(IndigoRendererSession&&) = delete;
    };

    IndigoKetcherObject loadMoleculeOrReaction(const std::string& data, const std::map<std::string, std::string>& options, int library = -1,
                                               bool use_document = false)
    {
        constexpr auto PEPTIDE = "PEPTIDE";
        constexpr auto PEPTIDE_3_LETTER = "PEPTIDE-3-LETTER";
        constexpr auto DNA = "DNA";
        constexpr auto RNA = "RNA";
        static std::unordered_map<std::string, std::string> seq_formats = {{"chemical/x-peptide-sequence", PEPTIDE},
                                                                           {"chemical/x-peptide-sequence-3-letter", PEPTIDE_3_LETTER},
                                                                           {"chemical/x-rna-sequence", RNA},
                                                                           {"chemical/x-dna-sequence", DNA}};

        static std::unordered_map<std::string, std::string> fasta_formats = {
            {"chemical/x-peptide-fasta", PEPTIDE}, {"chemical/x-rna-fasta", RNA}, {"chemical/x-dna-fasta", DNA}};

        print_js("loadMoleculeOrReaction:");
        std::vector<std::string> exceptionMessages;
        exceptionMessages.reserve(4);

        int objectId = -1;
        auto input_format = options.find("input-format");
        if (input_format != options.end() && (input_format->second == "smarts" || input_format->second == "chemical/x-daylight-smarts"))
        {
            print_js("load as smarts");
            objectId = indigoLoadSmartsFromBuffer(data.c_str(), data.size());
            if (objectId >= 0)
            {
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMoleculeQuery);
            }
            exceptionMessages.emplace_back(indigoGetLastError());
            // Let's try reaction
            print_js("try as reaction");
            objectId = indigoLoadReactionSmartsFromBuffer(data.c_str(), data.size());
            if (objectId >= 0)
            {
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETReaction);
            }
            exceptionMessages.emplace_back(indigoGetLastError());
        }
        else if (input_format != options.end() && seq_formats.count(input_format->second))
        {
            auto seq_it = seq_formats.find(input_format->second);
            objectId = indigoLoadSequenceFromString(data.c_str(), seq_it->second.c_str(), library);
            if (objectId >= 0)
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
            exceptionMessages.emplace_back(indigoGetLastError());
        }
        else if (input_format != options.end() && fasta_formats.count(input_format->second))
        {
            auto fasta_it = fasta_formats.find(input_format->second);
            objectId = indigoLoadFastaFromString(data.c_str(), fasta_it->second.c_str(), library);
            if (objectId >= 0)
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
            exceptionMessages.emplace_back(indigoGetLastError());
        }
        else if (input_format != options.end() && input_format->second == "chemical/x-idt")
        {
            objectId = indigoLoadIdtFromString(data.c_str(), library);
            if (objectId >= 0)
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
            exceptionMessages.emplace_back(indigoGetLastError());
        }
        else if (input_format != options.end() && input_format->second == "chemical/x-helm")
        {
            objectId = indigoLoadHelmFromString(data.c_str(), library);
            if (objectId >= 0)
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
            exceptionMessages.emplace_back(indigoGetLastError());
        }
        else
        {
            if (data.find("InChI") == 0)
            {
                objectId = indigoInchiLoadMolecule(data.c_str());
                if (objectId >= 0)
                    return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
            }
            if (use_document)
            {
                print_js("try as document");
                objectId = indigoLoadKetDocumentFromBuffer(data.c_str(), data.size());
                if (objectId >= 0)
                {
                    return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                }
            }
            auto i = options.find("query");
            if (i == options.end() || i->second == "false")
            {
                // Let's try a simple molecule
                print_js("try as molecule:1");
                objectId = indigoLoadMoleculeWithLibFromBuffer(data.c_str(), data.size(), library);
                if (objectId >= 0)
                {
                    return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
                }
                exceptionMessages.emplace_back(indigoGetLastError());
                print_js(indigoGetLastError());

                // Let's try reaction
                print_js("try as reaction");
                objectId = indigoLoadReactionWithLibFromBuffer(data.c_str(), data.size(), library);
                if (objectId >= 0)
                {
                    return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETReaction);
                }
                exceptionMessages.emplace_back(indigoGetLastError());

                if (library >= 0)
                {
                    auto sequence_type = options.find("sequence-type");
                    print_js("try as PEPTIDE-3-LETTER");
                    objectId = indigoLoadSequenceFromString(data.c_str(), PEPTIDE_3_LETTER, library);
                    if (objectId >= 0)
                    {
                        return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                    }
                    auto is_upper = std::all_of(data.begin(), data.end(), [](int ch) { return !std::isalpha(ch) || std::isupper(ch); });
                    auto is_lower = std::all_of(data.begin(), data.end(), [](int ch) { return !std::isalpha(ch) || std::islower(ch); });
                    if (is_upper || is_lower)
                    {
                        if (sequence_type != options.end()) // try according to selector
                        {
                            std::string msg = "try as " + sequence_type->second;
                            print_js(msg.c_str());
                            objectId = indigoLoadSequenceFromString(data.c_str(), sequence_type->second.c_str(), library);
                            if (objectId >= 0)
                            {
                                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                            }
                        }
                        print_js("try as PEPTIDE");
                        objectId = indigoLoadSequenceFromString(data.c_str(), PEPTIDE, library);
                        if (objectId >= 0)
                        {
                            return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                        }
                    }
                    print_js("try as IDT");
                    objectId = indigoLoadIdtFromString(data.c_str(), library);
                    if (objectId >= 0)
                    {
                        return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                    }
                    if (sequence_type != options.end())
                    {
                        std::string msg = "try as " + sequence_type->second;
                        print_js(msg.c_str());
                        objectId = indigoLoadSequenceFromString(data.c_str(), sequence_type->second.c_str(), library);
                        if (objectId >= 0)
                        {
                            return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                        }
                    }
                    print_js("try as PEPTIDE");
                    objectId = indigoLoadSequenceFromString(data.c_str(), PEPTIDE, library);
                    if (objectId >= 0)
                    {
                        return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                    }
                    print_js("try as HELM");
                    objectId = indigoLoadHelmFromString(data.c_str(), library);
                    if (objectId >= 0)
                    {
                        return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETDocument);
                    }
                }
            }
            exceptionMessages.emplace_back(indigoGetLastError());
            // Let's try query molecule
            print_js("try as query molecule:2");
            objectId = indigoLoadQueryMoleculeWithLibFromBuffer(data.c_str(), data.size(), library);
            if (objectId >= 0)
            {
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMoleculeQuery);
            }
            print_js(indigoGetLastError());
            exceptionMessages.emplace_back(indigoGetLastError());
            // Let's try query reaction
            print_js("try as query reaction");
            objectId = indigoLoadQueryReactionWithLibFromBuffer(data.c_str(), data.size(), library);
            if (objectId >= 0)
            {
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETReactionQuery);
            }

            // Let's try a simple molecule
            print_js("try as molecule");
            objectId = indigoLoadMoleculeWithLibFromBuffer(data.c_str(), data.size(), library);
            if (objectId >= 0)
            {
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
            }
            exceptionMessages.emplace_back(indigoGetLastError());

            // Let's try reaction
            print_js("try as reaction");
            objectId = indigoLoadReactionWithLibFromBuffer(data.c_str(), data.size(), library);
            if (objectId >= 0)
            {
                return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETReaction);
            }
            exceptionMessages.emplace_back(indigoGetLastError());
        }
        // It's not anything we can load, let's throw an exception
        std::stringstream ss;
        ss << "Given string could not be loaded as (query or plain) molecule or reaction, see the error messages: ";
        for (const auto& exceptionMessage : exceptionMessages)
        {
            ss << "'" << exceptionMessage << "', ";
        }
        std::string exceptionText = ss.str();
        exceptionText.pop_back();
        exceptionText.pop_back();
        jsThrow(exceptionText.c_str());
    }

    std::string version()
    {
        return _checkResultString(indigoVersion());
    }

    std::string versionInfo()
    {
        return _checkResultString(indigoVersionInfo());
    }

    std::string convert(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        std::map<std::string, std::string> options_copy;
        for (const auto& option : options)
        {
            if (option.first != "monomerLibrary")
            {
                options_copy[option.first] = option.second;
            }
        }

        int library = -1;
        auto monomerLibrary = options.find("monomerLibrary");
        if (monomerLibrary != options.end() && monomerLibrary->second.size())
        {
            library = indigoLoadMonomerLibraryFromString(monomerLibrary->second.c_str());
        }
        else
        {
            library = indigoLoadMonomerLibraryFromString("{\"root\":{}}");
        }

        if (outputFormat.find("smarts") != std::string::npos)
        {
            options_copy["query"] = "true";
        }
        indigoSetOptions(options);
        std::string input_format = "ket";
        if (const auto& it = options.find("input-format"); it != options.end())
            input_format = it->second;

        bool use_document = false;
        static const std::set<std::string> document_formats{"sequence",
                                                            "chemical/x-sequence",
                                                            "peptide-sequence-3-letter",
                                                            "chemical/x-peptide-sequence-3-letter",
                                                            "fasta",
                                                            "chemical/x-fasta",
                                                            "idt",
                                                            "chemical/x-idt",
                                                            "helm",
                                                            "chemical/x-helm"};
        if ((input_format == "ket" || input_format == "application/json") && outputFormat.size() > 0 && document_formats.count(outputFormat) > 0)
            use_document = true;
        IndigoKetcherObject iko = loadMoleculeOrReaction(data, options_copy, library, use_document);

        return iko.toString(options, outputFormat.size() ? outputFormat : "ket", library);
    }

    std::string convert_explicit_hydrogens(const std::string& data, const std::string& mode, const std::string& outputFormat,
                                           const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        std::map<std::string, std::string> options_copy = options;
        if (outputFormat.find("smarts") != std::string::npos)
        {
            options_copy["query"] = "true";
        }
        IndigoKetcherObject iko = loadMoleculeOrReaction(data, options_copy);
        if (mode == "fold")
        {
            _checkResult(indigoFoldHydrogens(iko.id()));
        }
        else if (mode == "unfold")
        {
            _checkResult(indigoUnfoldHydrogens(iko.id()));
        }
        else if (mode == "auto")
        {
            _checkResult(indigoFoldUnfoldHydrogens(iko.id()));
        }
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    std::string aromatize(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        _checkResult(indigoAromatize(iko.id()));
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    std::string dearomatize(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        _checkResult(indigoDearomatize(iko.id()));
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    std::string layout(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        std::map<std::string, std::string> options_copy = options;
        if (outputFormat.find("smarts") != std::string::npos)
        {
            options_copy["query"] = "true";
        }
        const auto iko = loadMoleculeOrReaction(data.c_str(), options_copy);
        _checkResult(indigoLayout(iko.id()));
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    std::string clean2d(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options,
                        const std::vector<int>& selected_atoms)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        auto iko = loadMoleculeOrReaction(data.c_str(), options);
        const auto& subiko = (selected_atoms.empty()) ? iko : iko.substructure(selected_atoms);
        _checkResult(indigoClean2d(subiko.id()));
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    std::string automap(const std::string& data, const std::string& mode, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        _checkResult(indigoAutomap(iko.id(), mode.c_str()));
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    std::string check(const std::string& data, const std::string& properties, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        return _checkResultString(indigoCheckObj(iko.id(), properties.c_str()));
    }

    std::string calculateCip(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        indigoSetOption("json-saving-add-stereo-desc", "true");
        indigoSetOption("molfile-saving-add-stereo-desc", "true");
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    void qmol2mol(IndigoKetcherObject& iko, const std::set<int>& selected_set)
    {
        IndigoObject qc(_checkResult(indigoClone(iko.id()))); // create query copy
        const auto atoms_iterator = IndigoObject(_checkResult(indigoIterateAtoms(qc.id)));
        while (const auto atom_id = _checkResult(indigoNext(atoms_iterator.id)))
        {
            int aix = _checkResult(indigoIndex(atom_id));
            if (selected_set.size() && selected_set.find(aix) == selected_set.end())
                _checkResult(indigoResetAtom(atom_id, "C")); // replace not selected atoms with C
        }

        const auto bonds_iterator = IndigoObject(_checkResult(indigoIterateBonds(qc.id)));
        while (const auto bond_id = _checkResult(indigoNext(bonds_iterator.id)))
        {
            int bix = _checkResult(indigoIndex(bond_id));
            const auto beg = _checkResult(indigoIndex(_checkResult(indigoSource(bond_id))));
            const auto end = _checkResult(indigoIndex(_checkResult(indigoDestination(bond_id))));
            if (selected_set.size() && selected_set.find(beg) == selected_set.end() && selected_set.find(end) == selected_set.end())
                _checkResult(indigoRemoveBonds(qc.id, 1, &bix));
        }

        auto mid = indigoLoadMoleculeFromString(indigoMolfile(qc.id));
        if (mid < 0)
            jsThrow("Cannot calculate properties for structures with query features!");
        iko.set(mid, IndigoKetcherObject::EKETMolecule);
    }

    class VectorStringSemicoloned
    {
        const std::vector<std::string>& vec;

    public:
        explicit VectorStringSemicoloned(const std::vector<std::string>& v) : vec(v)
        {
        }
        friend std::stringstream& operator<<(std::stringstream& ss, const VectorStringSemicoloned& vs);
    };

    std::stringstream& operator<<(std::stringstream& ss, const VectorStringSemicoloned& vs)
    {
        bool isFirst = true;
        for (const auto& str : vs.vec)
        {
            if (isFirst)
                isFirst = false;
            else
                ss << "; ";
            ss << str;
        }
        return ss;
    }

    class VectorStringBracketed
    {
        const std::vector<std::string>& vec1;
        const std::vector<std::string>& vec2;

    public:
        explicit VectorStringBracketed(const std::vector<std::string>& v1, const std::vector<std::string>& v2) : vec1(v1), vec2(v2)
        {
        }

        static void dumpVector(const std::vector<std::string>& v, std::stringstream& ss, bool both)
        {
            bool isFirst = true;
            for (const auto& str : v)
            {
                if (v.size() == 1 && !both)
                {
                    ss << str;
                    break;
                }

                if (isFirst)
                    isFirst = false;
                else
                    ss << "+";
                ss << "[" << str << "]";
            }
        }

        friend std::stringstream& operator<<(std::stringstream& ss, const VectorStringBracketed& vs);
    };

    std::stringstream& operator<<(std::stringstream& ss, const VectorStringBracketed& vs)
    {
        bool both = vs.vec1.size() && vs.vec2.size();
        VectorStringBracketed::dumpVector(vs.vec1, ss, both);
        if (both)
            ss << " > ";
        VectorStringBracketed::dumpVector(vs.vec2, ss, both);
        return ss;
    }

    void pushDoubleAsStr(double arg, std::vector<std::string>& vec)
    {
        std::stringstream ss;

        if (arg < 0.5)
            ss << indigoGetLastError();
        else
            ss << std::fixed << std::setprecision(7) << arg;

        std::string str = ss.str();

        if (str.size())
            vec.push_back(str);
        else
            vec.push_back("Unknown error");
    }

    void calculate_molecule(IndigoKetcherObject iko, std::stringstream& molecularWeightStream, std::stringstream& mostAbundantMassStream,
                            std::stringstream& monoisotopicMassStream, std::stringstream& massCompositionStream, std::stringstream& grossFormulaStream,
                            const std::vector<int>& selected_atoms)
    {

        const std::set<int> selected_set(selected_atoms.begin(), selected_atoms.end());

        if (iko.objtype == IndigoKetcherObject::EKETMoleculeQuery)
            qmol2mol(iko, selected_set);

        const auto componentsCount = _checkResult(indigoCountComponents(iko.id()));

        if (indigoCountRGroups(iko.id()) || indigoCountAttachmentPoints(iko.id()))
            jsThrow("Cannot calculate properties for RGroups");

        std::vector<std::string> molWeights, mamMasses, misoMasses, massCompositions, grossFormulas;

        for (auto i = 0; i < componentsCount; i++)
        {
            const auto component = IndigoObject(_checkResult(indigoComponent(iko.id(), i)));
            std::vector<int> component_atoms;
            const auto component_atoms_iterator = IndigoObject(_checkResult(indigoIterateAtoms(component.id)));
            indigoUnselect(iko.id());
            while (const auto atom_id = _checkResult(indigoNext(component_atoms_iterator.id)))
            {
                const auto atom = IndigoObject(atom_id);
                int aix = _checkResult(indigoIndex(atom.id));
                if (selected_set.size() && selected_set.find(aix) == selected_set.end())
                    continue;
                component_atoms.push_back(aix);
                indigoSelect(atom_id);
            }

            if (component_atoms.size())
            {

                pushDoubleAsStr(indigoMolecularWeight(iko.id()), molWeights);
                pushDoubleAsStr(indigoMostAbundantMass(iko.id()), mamMasses);
                pushDoubleAsStr(indigoMonoisotopicMass(iko.id()), misoMasses);

                const auto* massComposition = indigoMassComposition(iko.id());
                if (massComposition == nullptr)
                    massCompositions.push_back(indigoGetLastError());
                else
                    massCompositions.push_back(std::string(massComposition));

                const auto grossFormulaObject = IndigoObject(_checkResult(indigoGrossFormula(iko.id())));
                const auto* grossFormula = indigoToString(grossFormulaObject.id);
                if (grossFormula == nullptr)
                    grossFormulas.push_back(indigoGetLastError());
                else
                    grossFormulas.push_back(std::string(grossFormula));
            }
        }

        molecularWeightStream << VectorStringSemicoloned(molWeights);
        mostAbundantMassStream << VectorStringSemicoloned(mamMasses);
        monoisotopicMassStream << VectorStringSemicoloned(misoMasses);
        massCompositionStream << VectorStringSemicoloned(massCompositions);
        grossFormulaStream << VectorStringSemicoloned(grossFormulas);
    }

    void calculate_iteration_object(const IndigoObject& iterator, std::vector<std::string>& molWeights, std::vector<std::string>& mamMasses,
                                    std::vector<std::string>& misoMasses, std::vector<std::string>& massCompositions, std::vector<std::string>& grossFormulas,
                                    const std::vector<int>& selected_atoms, int& base)
    {
        while (const auto id = _checkResult(indigoNext(iterator.id)))
        {
            auto mol = IndigoKetcherObject(id, _checkResult(indigoCheckQuery(id)) ? IndigoKetcherObject::EKETMoleculeQuery : IndigoKetcherObject::EKETMolecule);
            std::vector<int> subselect;
            if (selected_atoms.size())
            {
                for (int i = 0; i < selected_atoms.size(); ++i)
                {
                    int atom_id = selected_atoms[i] - base;
                    if (atom_id >= 0)
                        subselect.push_back(atom_id);
                }
            }

            if (!selected_atoms.size() || subselect.size())
            {
                std::stringstream mws, mams, misos, mcs, gfs;
                calculate_molecule(mol, mws, mams, misos, mcs, gfs, subselect);
                if (gfs.str().size()) // If we have a gross formula, we also have everything else
                {
                    molWeights.push_back(mws.str());
                    mamMasses.push_back(mams.str());
                    misoMasses.push_back(misos.str());
                    massCompositions.push_back(mcs.str());
                    grossFormulas.push_back(gfs.str());
                }
            }
            if (selected_atoms.size())
                base += indigoCountAtoms(id);
        }
    }

    void calculate_reaction(const IndigoKetcherObject& iko, std::stringstream& molecularWeightStream, std::stringstream& mostAbundantMassStream,
                            std::stringstream& monoisotopicMassStream, std::stringstream& massCompositionStream, std::stringstream& grossFormulaStream,
                            const std::vector<int>& selected_atoms)
    {
        enum
        {
            IDX_REACTANTS = 0,
            IDX_PRODUCTS = 1,
            IDX_UNDEFINED = 2
        };
        std::vector<std::string> molWeights[3], mamMasses[3], misoMasses[3], massCompositions[3], grossFormulas[3];

        const auto mol_iterator = IndigoObject(_checkResult(indigoIterateMolecules(iko.id())));
        while (const auto mol_id = _checkResult(indigoNext(mol_iterator.id)))
        {
            if (_checkResult(indigoCountRGroups(mol_id)) || _checkResult(indigoCountAttachmentPoints(mol_id)))
                jsThrow("Cannot calculate properties for RGroups");
        }
        int base = 0;
        if (_checkResult(indigoCountReactants(iko.id())) || _checkResult(indigoCountProducts(iko.id())))
        {
            calculate_iteration_object(IndigoObject(_checkResult(indigoIterateReactants(iko.id()))), molWeights[IDX_REACTANTS], mamMasses[IDX_REACTANTS],
                                       misoMasses[IDX_REACTANTS], massCompositions[IDX_REACTANTS], grossFormulas[IDX_REACTANTS], selected_atoms, base);

            calculate_iteration_object(IndigoObject(_checkResult(indigoIterateProducts(iko.id()))), molWeights[IDX_PRODUCTS], mamMasses[IDX_PRODUCTS],
                                       misoMasses[IDX_PRODUCTS], massCompositions[IDX_PRODUCTS], grossFormulas[IDX_PRODUCTS], selected_atoms, base);

            molecularWeightStream << VectorStringBracketed(molWeights[IDX_REACTANTS], molWeights[IDX_PRODUCTS]);
            mostAbundantMassStream << VectorStringBracketed(mamMasses[IDX_REACTANTS], mamMasses[IDX_PRODUCTS]);
            monoisotopicMassStream << VectorStringBracketed(misoMasses[IDX_REACTANTS], misoMasses[IDX_PRODUCTS]);
            massCompositionStream << VectorStringBracketed(massCompositions[IDX_REACTANTS], massCompositions[IDX_PRODUCTS]);
            grossFormulaStream << VectorStringBracketed(grossFormulas[IDX_REACTANTS], grossFormulas[IDX_PRODUCTS]);
        }
        else
        {
            calculate_iteration_object(IndigoObject(_checkResult(indigoIterateMolecules(iko.id()))), molWeights[IDX_UNDEFINED], mamMasses[IDX_UNDEFINED],
                                       misoMasses[IDX_UNDEFINED], massCompositions[IDX_UNDEFINED], grossFormulas[IDX_UNDEFINED], selected_atoms, base);

            molecularWeightStream << VectorStringSemicoloned(molWeights[IDX_UNDEFINED]);
            mostAbundantMassStream << VectorStringSemicoloned(mamMasses[IDX_UNDEFINED]);
            monoisotopicMassStream << VectorStringSemicoloned(misoMasses[IDX_UNDEFINED]);
            massCompositionStream << VectorStringSemicoloned(massCompositions[IDX_UNDEFINED]);
            grossFormulaStream << VectorStringSemicoloned(grossFormulas[IDX_UNDEFINED]);
        }
    }

    std::string pka(const std::string& data, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        return std::to_string(indigoPka(iko.id()));
    }

    std::string pkaValues(const std::string& data, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        return indigoPkaValues(iko.id());
    }

    std::string logp(const std::string& data, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        return std::to_string(indigoLogP(iko.id()));
    }

    std::string molarRefractivity(const std::string& data, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        return std::to_string(indigoMolarRefractivity(iko.id()));
    }

    std::string calculate(const std::string& data, const std::map<std::string, std::string>& options, const std::vector<int>& selected_atoms)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        auto iko = loadMoleculeOrReaction(data.c_str(), options);
        rapidjson::Document result;
        auto& allocator = result.GetAllocator();
        result.SetObject();
        std::stringstream molecularWeightStream;
        std::stringstream mostAbundantMassStream;
        std::stringstream monoisotopicMassStream;
        std::stringstream massCompositionStream;
        std::stringstream grossFormulaStream;

        switch (iko.objtype)
        {
        case IndigoKetcherObject::EKETMoleculeQuery:
        case IndigoKetcherObject::EKETMolecule:
            calculate_molecule(iko, molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream,
                               selected_atoms);
            break;
        case IndigoKetcherObject::EKETReactionQuery:
        case IndigoKetcherObject::EKETReaction:
            calculate_reaction(iko, molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream,
                               selected_atoms);
            break;
        }
        result.AddMember("molecular-weight", molecularWeightStream.str(), allocator);
        result.AddMember("most-abundant-mass", mostAbundantMassStream.str(), allocator);
        result.AddMember("monoisotopic-mass", monoisotopicMassStream.str(), allocator);
        result.AddMember("mass-composition", massCompositionStream.str(), allocator);
        result.AddMember("gross-formula", grossFormulaStream.str(), allocator);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        result.Accept(writer);
        return buffer.GetString();
    }

    std::string calculateMacroProperties(const std::string& data, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        // UPC is molar concentration of unipositive cations, NAC is molar concentration of the nucleotide strands, these options need to calculate melting
        // temperature
        float upc = 0.14f; // default value is the average physiological - 140 mM
        char* str_end = nullptr;
        auto it = options.find("upc");
        if (it != options.end())
        {
            upc = std::strtof(it->second.c_str(), &str_end);
            if (str_end != nullptr && *str_end != 0)
                jsThrow("Wrong value for UPC");
        }
        float nac = 0;
        it = options.find("nac");
        if (it != options.end())
        {
            nac = std::strtof(it->second.c_str(), &str_end);
            if (str_end != nullptr && *str_end != 0)
                jsThrow("Wrong value for NAC");
        }
        else
        {
            jsThrow("NAC option is mandatory");
        }
        auto iko = indigoLoadKetDocumentFromString(data.c_str());
        rapidjson::Document result;
        auto& allocator = result.GetAllocator();
        result.SetObject();
        std::string props{indigoMacroProperties(iko, upc, nac)};
        result.AddMember("properties", props, allocator);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        result.Accept(writer);
        return buffer.GetString();
    }

    std::string render(const std::string& data, const std::map<std::string, std::string>& options)
    {
#ifndef INDIGO_NO_RENDER
        const IndigoSession session;
        const IndigoRendererSession indigoRendererSession(session.getSessionId());

        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        auto buffer_object = IndigoObject(_checkResult(indigoWriteBuffer()));
        char* raw_ptr = nullptr;
        int size = 0;
        _checkResult(indigoRender(iko.id(), buffer_object.id));
        _checkResult(indigoToBuffer(buffer_object.id, &raw_ptr, &size));
        return cppcodec::base64_rfc4648::encode(raw_ptr, size);
#else
        jsThrow("No renderer included in this wasm bundle!");
        return "";
#endif
    }

    std::string reactionComponents(const std::string& data, const std::map<std::string, std::string>& options)
    {
        using namespace rapidjson;
        Document result;
        result.SetObject();
        {
            const IndigoSession session;
            indigoSetOptions(options);
            const auto reactionId = _checkResult(indigoLoadQueryReactionFromString(data.c_str()));
            // reactants
            {
                Value reactants(kArrayType);
                reactants.SetArray();
                const auto reactantsIteratorId = _checkResult(indigoIterateReactants(reactionId));
                while (const auto reactantId = _checkResult(indigoNext(reactantsIteratorId)))
                {
                    Value reactant(kStringType);
                    reactant.SetString(_checkResultString(indigoSmiles(reactantId)), result.GetAllocator());
                    reactants.PushBack(reactant, result.GetAllocator());
                    _checkResult(indigoFree(reactantId));
                }
                result.AddMember("reactants", reactants, result.GetAllocator());
                _checkResult(indigoFree(reactantsIteratorId));
            }
            // catalysts
            {
                Value catalysts(kArrayType);
                catalysts.SetArray();
                const auto catalystsIteratorId = _checkResult(indigoIterateCatalysts(reactionId));
                while (const auto catalystId = _checkResult(indigoNext(catalystsIteratorId)))
                {
                    Value catalyst(kStringType);
                    catalyst.SetString(_checkResultString(indigoSmiles(catalystId)), result.GetAllocator());
                    catalysts.PushBack(catalyst, result.GetAllocator());
                    _checkResult(indigoFree(catalystId));
                }
                result.AddMember("catalysts", catalysts, result.GetAllocator());
                _checkResult(indigoFree(catalystsIteratorId));
            }
            // products
            {
                Value products(kArrayType);
                products.SetArray();
                const auto productsIteratorId = _checkResult(indigoIterateProducts(reactionId));
                while (const auto productId = _checkResult(indigoNext(productsIteratorId)))
                {
                    Value product(kStringType);
                    product.SetString(_checkResultString(indigoSmiles(productId)), result.GetAllocator());
                    products.PushBack(product, result.GetAllocator());
                    _checkResult(indigoFree(productId));
                }
                result.AddMember("products", products, result.GetAllocator());
                _checkResult(indigoFree(productsIteratorId));
            }
            _checkResult(indigoFree(reactionId));
        }
        // Serialize results
        StringBuffer buffer;
        Writer<rapidjson::StringBuffer> writer(buffer);
        result.Accept(writer);
        return buffer.GetString();
    }

    std::string expand(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        const IndigoSession session;
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str(), options);
        _checkResult(indigoExpandMonomers(iko.id()));
        return iko.toString(options, outputFormat.size() ? outputFormat : "ket");
    }

    EMSCRIPTEN_BINDINGS(module)
    {
        emscripten::function("version", &version);
        emscripten::function("versionInfo", &versionInfo);
        emscripten::function("convert", &convert);
        emscripten::function("convert_explicit_hydrogens", &convert_explicit_hydrogens);
        emscripten::function("aromatize", &aromatize);
        emscripten::function("dearomatize", &dearomatize);
        emscripten::function("layout", &layout);
        emscripten::function("clean2d", &clean2d);
        emscripten::function("automap", &automap);
        emscripten::function("check", &check);
        emscripten::function("calculateCip", &calculateCip);
        emscripten::function("calculate", &calculate);
        emscripten::function("pka", &pka);
        emscripten::function("pkaValues", &pkaValues);
        emscripten::function("logp", &logp);
        emscripten::function("molarRefractivity", &molarRefractivity);
        emscripten::function("calculateMacroProperties", &calculateMacroProperties);
        emscripten::function("render", &render);
        emscripten::function("reactionComponents", &reactionComponents);
        emscripten::function("expand", &expand);

        emscripten::register_vector<int>("VectorInt");
        emscripten::register_map<std::string, std::string>("MapStringString");
    };
}
