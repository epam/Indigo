#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include <iomanip>

#include <emscripten.h>
#include <emscripten/bind.h>

#define RAPIDJSON_HAS_STDSTRING 1

#include <cppcodec/base64_rfc4648.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "indigo.h"
#include "indigo-inchi.h"
#include "indigo-renderer.h"

namespace indigo
{
    using cstring = const char*;

    EM_JS(void, jsThrow, (cstring str), {
        throw UTF8ToString(str);
    });

    EM_JS(void, print_jsn, (cstring str, int n), {
        console.log( UTF8ToString(str) + n );
    });


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
	    enum  KOType { EKETMolecule, EKETMoleculeQuery, EKETReaction, EKETReactionQuery };	
        const KOType objtype;
    private:
        const std::shared_ptr<const IndigoObject> indigo_object;
        const std::shared_ptr<const IndigoObject> parent = nullptr;

    public:
        explicit IndigoKetcherObject(const int id, const KOType type, std::shared_ptr<const IndigoObject>  parent = nullptr) :
              indigo_object(std::make_shared<IndigoObject>(id)), objtype(type), parent(std::move(parent))
        {}

        std::string toString() const
        {
            if( is_reaction() )
            {
                return _checkResultString(indigoRxnfile(id()));
            }
            return _checkResultString(indigoMolfile(id()));
        }

        int id() const
        {
            return indigo_object->id;
        }

        IndigoKetcherObject substructure(const std::vector<int>& selected_atoms) const
        {
            const int* selected_atoms_array = &selected_atoms[0];
            return IndigoKetcherObject(_checkResult(indigoGetSubmolecule(id(), static_cast<int>(selected_atoms.size()), selected_atoms_array)), objtype, indigo_object);
        }
		
		bool is_reaction() const
		{
			return objtype == EKETReaction || objtype == EKETReactionQuery;
		}
    };

    void indigoSetOptions(const std::map<std::string, std::string>& options)
    {
        for (const auto& option : options)
        {
            if (option.first != "smiles" && option.first != "smarts")
            {
                _checkResult(indigoSetOption(option.first.c_str(), option.second.c_str()));
            }
        }
    }

    IndigoKetcherObject loadMoleculeOrReaction(cstring data)
    {
        std::vector<std::string> exceptionMessages;
        exceptionMessages.reserve(4);
        // Let's try a simple molecule
        int objectId = indigoLoadMoleculeFromString(data);
        if (objectId >= 0)
        {
            return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMolecule);
        }
        exceptionMessages.emplace_back(indigoGetLastError());
        // Let's try query molecule
        objectId = indigoLoadQueryMoleculeFromString(data);
        if (objectId >= 0)
        {
            return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETMoleculeQuery);
        }
        exceptionMessages.emplace_back(indigoGetLastError());
        // Let's try simple reaction
        objectId = indigoLoadReactionFromString(data);
        if (objectId >= 0)
        {
            return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETReaction);
        }
        exceptionMessages.emplace_back(indigoGetLastError());
        // Let's try query reaction
        objectId = indigoLoadQueryReactionFromString(data);
        if (objectId >= 0)
        {
            return IndigoKetcherObject(objectId, IndigoKetcherObject::EKETReactionQuery);
        }
        exceptionMessages.emplace_back(indigoGetLastError());
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

    std::string convert(const std::string& data, const std::string& outputFormat, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        IndigoKetcherObject iko = loadMoleculeOrReaction(data.c_str());
        if (outputFormat == "molfile" || outputFormat == "rxnfile" || outputFormat == "chemical/x-mdl-molfile" || outputFormat == "chemical/x-mdl-rxnfile")
        {
            if( iko.is_reaction() )
            {
                return _checkResultString(indigoRxnfile(iko.id()));
            }
            return _checkResultString(indigoMolfile(iko.id()));
        }
        if (outputFormat == "smiles" or outputFormat == "chemical/x-daylight-smiles" || outputFormat == "chemical/x-chemaxon-cxsmiles")
        {
            if (options.count("smiles") && options.at("smiles") == "canonical")
            {
                return _checkResultString(indigoCanonicalSmiles(iko.id()));
            }
            return _checkResultString(indigoSmiles(iko.id()));
        }
        if (outputFormat == "smarts" || outputFormat == "chemical/x-daylight-smarts")
        {
            if (options.count("smarts") && options.at("smarts") == "canonical")
            {
                return _checkResultString(indigoCanonicalSmarts(iko.id()));
            }
            return _checkResultString(indigoSmarts(iko.id()));
        }
        if (outputFormat == "cml" || outputFormat == "chemical/x-cml")
        {
            return _checkResultString(indigoCml(iko.id()));
        }
        if (outputFormat == "inchi" || outputFormat == "chemical/x-inchi")
        {
            return _checkResultString(indigoInchiGetInchi(iko.id()));
        }
        if (outputFormat == "inchi-aux" || outputFormat == "chemical/x-inchi-aux")
        {
            std::stringstream ss;
            ss << _checkResultString(indigoInchiGetInchi(iko.id())) << '\n' << _checkResultString(indigoInchiGetAuxInfo());
            return ss.str();
        }
        std::stringstream ss;
        ss << "Unknown output format: " << outputFormat;
        jsThrow(ss.str().c_str());
    }

    std::string aromatize(const std::string& data, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str());
        _checkResult(indigoAromatize(iko.id()));
        return iko.toString();
    }

    std::string dearomatize(const std::string& data, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str());
        _checkResult(indigoDearomatize(iko.id()));
        return iko.toString();
    }

    std::string layout(const std::string& data, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str());
        _checkResult(indigoLayout(iko.id()));
        return iko.toString();
    }

    std::string clean2d(const std::string& data, const std::map<std::string, std::string>& options, const std::vector<int>& selected_atoms)
    {
        indigoSetOptions(options);
        auto iko = loadMoleculeOrReaction(data.c_str());
        const auto& subiko = (selected_atoms.empty()) ? iko : iko.substructure(selected_atoms);
        _checkResult(indigoClean2d(subiko.id()));
        return iko.toString();
    }

    std::string automap(const std::string& data, const std::string& mode, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str());
        _checkResult(indigoAutomap(iko.id(), mode.c_str()));
        return iko.toString();
    }

    std::string check(const std::string& data, const std::string& properties, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str());
        return _checkResultString(indigoCheckObj2(iko.id(), properties.c_str()));
    }

    std::string calculateCip(const std::string& data, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        indigoSetOption("molfile-saving-add-stereo-desc", "true");
        const auto iko = loadMoleculeOrReaction(data.c_str());
        return iko.toString();
    }

	void calculate_molecule( const IndigoKetcherObject& iko, std::stringstream& molecularWeightStream, std::stringstream& mostAbundantMassStream, std::stringstream& monoisotopicMassStream,  std::stringstream& massCompositionStream, std::stringstream& grossFormulaStream, const std::vector<int>& selected_atoms )
	{
        const std::set<int> selected_set( selected_atoms.begin(), selected_atoms.end() );
        if( indigoCountRGroups(iko.id()) || indigoCountAttachmentPoints( iko.id() ) )
            jsThrow("Cannot calculate properties for RGroups");
		
	    const auto componentsCount = _checkResult(indigoCountComponents( iko.id()));
        for (auto i = 0; i < componentsCount; i++)
        {
            const auto component = IndigoObject(_checkResult(indigoComponent(iko.id(), i)));
            std::vector<int> component_atoms;
            const auto component_atoms_iterator = IndigoObject(_checkResult(indigoIterateAtoms(component.id)));
            while (const auto atom_id = _checkResult(indigoNext(component_atoms_iterator.id)))
            {
                const auto atom = IndigoObject(atom_id);
                int aix = _checkResult(indigoIndex(atom.id));
                if( selected_set.size() && selected_set.find(aix) == selected_set.end() )
                    continue;
                component_atoms.push_back( aix );
            }

            if( component_atoms.size() )
            {
                const auto molecularWeight = indigoMolecularWeightWithSelection(iko.id(), component_atoms.size(), component_atoms.data());
                if (molecularWeight < 0.5)
                {
                    molecularWeightStream << indigoGetLastError();
                }
                else
                {
                    molecularWeightStream << std::fixed << std::setprecision(7) << molecularWeight;
                }

                const auto mostAbundantMass = indigoMostAbundantMassWithSelection(iko.id(), component_atoms.size(), component_atoms.data());
                if (mostAbundantMass < 0.5)
                {
                    mostAbundantMassStream << indigoGetLastError();
                }
                else
                {
                    mostAbundantMassStream << std::fixed << std::setprecision(7) << mostAbundantMass;
                }

                const auto monoisotopicMass = indigoMonoisotopicMassWithSelection(iko.id(), component_atoms.size(), component_atoms.data());
                if (monoisotopicMass < 0.5)
                {
                    monoisotopicMassStream << indigoGetLastError();
                }
                else
                {
                    monoisotopicMassStream << std::fixed << std::setprecision(7) << monoisotopicMass;
                }

                const auto* massComposition = indigoMassCompositionWithSelection(iko.id(), component_atoms.size(), component_atoms.data() );
                if (massComposition == nullptr)
                {
                    massCompositionStream << indigoGetLastError();
                }
                else
                {
                    massCompositionStream << massComposition;
                }

                const auto grossFormulaObject = IndigoObject(_checkResult(indigoGrossFormulaWithSelection(iko.id(), component_atoms.size(), component_atoms.data() )));
                const auto* grossFormula = indigoToString(grossFormulaObject.id);
                if (grossFormula == nullptr)
                {
                    grossFormulaStream << indigoGetLastError();
                }
                else
                {
                    grossFormulaStream << grossFormula;
                }
                
                if( i < componentsCount - 1 )
                {
                    molecularWeightStream << "; ";
                    mostAbundantMassStream << "; ";
                    monoisotopicMassStream << "; ";
                    massCompositionStream << "; ";
                    grossFormulaStream << "; ";
                }
            }
        }
	}

    void calculate_iteration_object( const IndigoObject& iterator, std::stringstream& molecularWeightStream, std::stringstream& mostAbundantMassStream, std::stringstream& monoisotopicMassStream,  std::stringstream& massCompositionStream, std::stringstream& grossFormulaStream, const std::vector<int>& selected_atoms )
	{
		bool is_not_first = false;
        while (const auto id = _checkResult(indigoNext(iterator.id)))
        {
			if( is_not_first )
			{
                molecularWeightStream << "+";
                mostAbundantMassStream << "+";
                monoisotopicMassStream << "+";
                massCompositionStream << "+";
                grossFormulaStream << "+";
			}
            molecularWeightStream << "[";
            mostAbundantMassStream << "[";
            monoisotopicMassStream << "[";
            massCompositionStream << "[";
            grossFormulaStream << "[";
		    calculate_molecule( IndigoKetcherObject(id, IndigoKetcherObject::EKETMolecule), molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream, selected_atoms );
            molecularWeightStream << "]";
            mostAbundantMassStream << "]";
            monoisotopicMassStream << "]";
            massCompositionStream << "]";
            grossFormulaStream << "]";
			is_not_first = true;
		}
	}
	
	void calculate_reaction( const IndigoKetcherObject& iko, std::stringstream& molecularWeightStream, std::stringstream& mostAbundantMassStream, std::stringstream& monoisotopicMassStream,  std::stringstream& massCompositionStream, std::stringstream& grossFormulaStream, const std::vector<int>& selected_atoms )
	{
		const auto mol_iterator = IndigoObject(_checkResult(indigoIterateMolecules(iko.id())));
        while (const auto mol_id = _checkResult(indigoNext(mol_iterator.id)))
        {
            if( _checkResult(indigoCountRGroups(mol_id)) || _checkResult(indigoCountAttachmentPoints( mol_id )) )
                jsThrow("Cannot calculate properties for RGroups");
        }
		calculate_iteration_object( IndigoObject(_checkResult(indigoIterateReactants(iko.id()))), molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream, selected_atoms );
        molecularWeightStream << " > ";
        mostAbundantMassStream << " > ";
        monoisotopicMassStream << " > ";
        massCompositionStream << " > ";
        grossFormulaStream << " > ";
		calculate_iteration_object( IndigoObject(_checkResult(indigoIterateProducts(iko.id()))), molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream, selected_atoms );
	}

    std::string calculate(const std::string& data, const std::map<std::string, std::string>& options, const std::vector<int>& selected_atoms)
    {
        indigoSetOptions(options);
        auto iko = loadMoleculeOrReaction(data.c_str());
        rapidjson::Document result;
        auto& allocator = result.GetAllocator();
        result.SetObject();
        std::stringstream molecularWeightStream;
        std::stringstream mostAbundantMassStream;
        std::stringstream monoisotopicMassStream;
        std::stringstream massCompositionStream;
        std::stringstream grossFormulaStream;

        switch( iko.objtype )
		{
			case IndigoKetcherObject::EKETMolecule:
  			case IndigoKetcherObject::EKETMoleculeQuery:
			    calculate_molecule(iko, molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream, selected_atoms );
			break;
			case IndigoKetcherObject::EKETReaction:
			    calculate_reaction(iko, molecularWeightStream, mostAbundantMassStream, monoisotopicMassStream, massCompositionStream, grossFormulaStream, selected_atoms );
			break;
			case IndigoKetcherObject::EKETReactionQuery:
			    jsThrow("Cannot calculate properties for structures with query features");
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

    std::string render(const std::string& data, const std::map<std::string, std::string>& options)
    {
        indigoSetOptions(options);
        const auto iko = loadMoleculeOrReaction(data.c_str());
        auto buffer_object = IndigoObject(_checkResult(indigoWriteBuffer()));
        char* raw_ptr = nullptr;
        int size = 0;
        _checkResult(indigoRender(iko.id(), buffer_object.id));
        _checkResult(indigoToBuffer(buffer_object.id, &raw_ptr, &size));
        return cppcodec::base64_rfc4648::encode(raw_ptr, size);
    }

#ifdef __EMSCRIPTEN__

    EMSCRIPTEN_BINDINGS(module)
    {
        emscripten::function("version", &version);
        emscripten::function("convert", &convert);
        emscripten::function("aromatize", &aromatize);
        emscripten::function("dearomatize", &dearomatize);
        emscripten::function("layout", &layout);
        emscripten::function("clean2d", &clean2d);
        emscripten::function("automap", &automap);
        emscripten::function("check", &check);
        emscripten::function("calculateCip", &calculateCip);
        emscripten::function("calculate", &calculate);
        emscripten::function("render", &render);

        emscripten::register_vector<int>("VectorInt");
        emscripten::register_map<std::string, std::string>("MapStringString");
    }

#endif

} // namespace indigo
