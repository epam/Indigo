#include "common.h"
#include "reaction/reaction_cml_saver.h"
#include "reaction/reaction_json_saver.h"
#include "reaction/rxnfile_saver.h"

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/canonical_smiles_saver.h>
#include <molecule/molecule_auto_loader.h>
#include <molecule/molecule_substructure_matcher.h>
#include <reaction/reaction_auto_loader.h>
#include <reaction/rsmiles_saver.h>

using namespace indigo;

namespace
{
    const std::string dataPathPrefix = DATA_PATH;
}

void IndigoCoreTest::loadMolecule(const char* buf, Molecule& molecule)
{
    BufferScanner scanner(buf);
    MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(molecule);
}

void IndigoCoreTest::loadQueryMolecule(const char* buf, QueryMolecule& queryMolecule)
{
    BufferScanner scanner(buf);
    MoleculeAutoLoader loader(scanner);
    loader.loadQueryMolecule(queryMolecule);
}

bool IndigoCoreTest::substructureMatch(const char* targetString, const char* queryString)
{
    Molecule target;
    QueryMolecule query;
    loadMolecule(targetString, target);
    loadQueryMolecule(queryString, query);
    MoleculeSubstructureMatcher matcher(target);
    matcher.setQuery(query);
    return matcher.find();
}

std::string IndigoCoreTest::smilesLoadSaveLoad(const std::string& queryString, bool smarts)
{
    if (smarts)
    {
        QueryMolecule query;
        loadQueryMolecule(queryString.c_str(), query);
        Array<char> buffer;
        ArrayOutput output(buffer);
        CanonicalSmilesSaver smilesSaver(output);
        if (smarts)
        {
            smilesSaver.smarts_mode = true;
            smilesSaver.ignore_hydrogens = false;
        }
        smilesSaver.saveQueryMolecule(query);
        output.writeChar(0);
        std::string result(buffer.ptr());
        return result;
    }
    else
    {
        Molecule molecule;
        loadMolecule(queryString.c_str(), molecule);
        Array<char> buffer;
        ArrayOutput output(buffer);
        CanonicalSmilesSaver smilesSaver(output);
        smilesSaver.saveMolecule(molecule);
        output.writeChar(0);
        std::string result(buffer.ptr());
        return result;
    }
}

std::string IndigoCoreTest::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}

void IndigoCoreTest::loadReaction(const char* buf, Reaction& reaction)
{
    BufferScanner scanner(buf);
    ReactionAutoLoader loader(scanner);
    loader.loadReaction(reaction);
}

void IndigoCoreTest::loadQueryReaction(const char* buf, QueryReaction& queryReaction)
{
    BufferScanner scanner(buf);
    ReactionAutoLoader loader(scanner);
    loader.loadQueryReaction(queryReaction);
}

std::string IndigoCoreTest::saveReactionSmiles(Reaction& reaction)
{
    Array<char> buffer;
    ArrayOutput output(buffer);
    RSmilesSaver smilesSaver(output);
    smilesSaver.saveReaction(reaction);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}

std::string IndigoCoreTest::saveReactionSmiles(QueryReaction& queryReaction, const bool smarts)
{
    Array<char> buffer;
    ArrayOutput output(buffer);
    RSmilesSaver smilesSaver(output);
    smilesSaver.smarts_mode = smarts;
    smilesSaver.saveQueryReaction(queryReaction);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}

std::string IndigoCoreTest::saveRxnfle(Reaction& reaction)
{
    Array<char> buffer;
    ArrayOutput output(buffer);
    RxnfileSaver rxnfileSaver(output);
    rxnfileSaver.skip_date = true;
    rxnfileSaver.saveReaction(reaction);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}

std::string IndigoCoreTest::saveRxnfle(QueryReaction& queryReaction)
{
    Array<char> buffer;
    ArrayOutput output(buffer);
    RxnfileSaver rxnfileSaver(output);
    rxnfileSaver.skip_date = true;
    rxnfileSaver.saveQueryReaction(queryReaction);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}

std::string IndigoCoreTest::saveReactionCML(Reaction& reaction)
{
    Array<char> buffer;
    ArrayOutput output(buffer);
    ReactionCmlSaver saver(output);
    saver.saveReaction(reaction);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}

std::string IndigoCoreTest::saverReactionJson(BaseReaction& reaction)
{
    Array<char> buffer;
    ArrayOutput output(buffer);
    ReactionJsonSaver saver(output);
    saver.saveReaction(reaction);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}
