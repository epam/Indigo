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

#include "molecule/molecule_name_parser.h"

#include <tinyxml2.h>

using namespace std;
using namespace indigo;
using namespace tinyxml2;

IMPL_ERROR(MoleculeNameParser, "indigo::MoleculeNameParser");
IMPL_ERROR(MoleculeNameParser::DictionaryManager, "MoleculeNameParser::DictionaryManager");
IMPL_ERROR(MoleculeNameParser::Parse, "MoleculeNameParser::Parse");
IMPL_ERROR(MoleculeNameParser::TreeBuilder, "MoleculeNameParser::TreeBuilder");
IMPL_ERROR(MoleculeNameParser::SmilesBuilder, "MoleculeNameParser::SmilesBuilder");

MoleculeNameParser::DictionaryManager::DictionaryManager()
{
    _readTokenTypeStrings();

    _readTable(alkanes_table, true);
    _readTable(multipliers_table, true);
    _readTable(separators_table);
    _readTable(flags_table, true);
    _readTable(trivial_table, true);

    _readSkeletalAtomsTable();
    _readBasicElementsTable();
}

void MoleculeNameParser::DictionaryManager::_readSkeletalAtomsTable()
{
    XMLDocument doc;

    doc.Parse(skeletal_table);
    if (doc.Error())
    {
        throw Error("Cannot parse table %s", skeletal_table);
    }

    XMLHandle hdoc(&doc);
    XMLHandle tokenTables = hdoc.FirstChildElement("tokenTables");
    XMLElement* tokenTable = tokenTables.FirstChildElement("tokenTable").ToElement();
    for (; tokenTable; tokenTable = tokenTable->NextSiblingElement())
    {
        const char* name = tokenTable->Attribute("name");
        const char* type = tokenTable->Attribute("type");
        if (!name || !type)
        {
            throw Error("Cannot parse table");
        }

        TokenType tt = _tokenTypeFromString(type);

        XMLElement* e = tokenTable->FirstChildElement("token");
        for (; e; e = e->NextSiblingElement())
        {
            const char* lexeme = e->GetText();
            const char* bonding = e->Attribute("bonding");
            const char* symbol = e->Attribute("symbol");
            if (!lexeme || !bonding || !symbol)
            {
                throw Error("Cannot parse table %s", name);
            }

            /*
            For skeletal atoms, we combine bonding number and symbol into one value
            Values are separated by underscore _
            */
            string value = bonding;
            value += '_';
            value += symbol;

            _addLexeme(lexeme, {name, value, tt}, true);
        }
    }
}

void MoleculeNameParser::DictionaryManager::_readBasicElementsTable()
{
    XMLDocument doc;

    doc.Parse(basic_elements_table);
    if (doc.Error())
    {
        throw Error("Cannot parse table %s", basic_elements_table);
    }

    XMLHandle hdoc(&doc);
    XMLHandle tokenTables = hdoc.FirstChildElement("tokenTables");
    XMLElement* tokenTable = tokenTables.FirstChildElement("tokenTable").ToElement();
    for (; tokenTable; tokenTable = tokenTable->NextSiblingElement())
    {
        const char* name = tokenTable->Attribute("name");
        const char* type = tokenTable->Attribute("type");
        if (!name || !type)
        {
            throw Error("Cannot parse table");
        }

        TokenType tt = _tokenTypeFromString(type);

        XMLElement* e = tokenTable->FirstChildElement("token");
        for (; e; e = e->NextSiblingElement())
        {
            const char* lexeme = e->GetText();
            const char* number = e->Attribute("number");
            const char* symbol = e->Attribute("symbol");
            if (!lexeme || !number || !symbol)
            {
                throw Error("Cannot parse table %s", name);
            }

            /*
            For basic elements, we combine number and symbol into one value
            Values are separated by underscore _
            */

            string value = number;
            value += '_';
            value += symbol;

            /*
            Symbols might have a separator '|', in which case we need to add
            several symbols with the same token type into the dictionary
            */

            char delim[] = "|";
            char* fragment = ::strtok(const_cast<char*>(lexeme), delim);
            while (fragment)
            {
                _addLexeme(fragment, {name, value, tt}, true);
                fragment = ::strtok(nullptr, delim);
            }
        }
    }
}

void MoleculeNameParser::DictionaryManager::_readTokenTypeStrings()
{
    XMLDocument doc;

    doc.Parse(token_types_table);
    if (doc.Error())
    {
        throw Error("Cannot parse the token types table");
    }

    XMLHandle hdoc(&doc);
    XMLHandle tokenTypes = hdoc.FirstChildElement("tokenTypes");
    XMLElement* e = tokenTypes.FirstChildElement("tokenType").ToElement();
    for (; e; e = e->NextSiblingElement())
    {
        _tokenTypeStrings.push_back(e->GetText());
    }
}

void MoleculeNameParser::DictionaryManager::_readTable(const char* table, bool useTrie /* = false*/)
{
    XMLDocument doc;

    doc.Parse(table);
    if (doc.Error())
    {
        throw Error("Cannot parse table %s", table);
    }

    XMLHandle hdoc(&doc);
    XMLHandle tokenTables = hdoc.FirstChildElement("tokenTables");
    XMLElement* tokenTable = tokenTables.FirstChildElement("tokenTable").ToElement();
    for (; tokenTable; tokenTable = tokenTable->NextSiblingElement())
    {
        const char* name = tokenTable->Attribute("name");
        const char* type = tokenTable->Attribute("type");
        if (!name || !type)
        {
            throw Error("Cannot parse table");
        }

        const bool isSeparator = (::strcmp(name, "separator") == 0);
        TokenType tt = _tokenTypeFromString(type);

        XMLElement* e = tokenTable->FirstChildElement("token");
        for (; e; e = e->NextSiblingElement())
        {
            const char* lexeme = e->GetText();
            const char* value = e->Attribute("value");
            if (!lexeme || !value)
            {
                throw Error("Cannot parse table %s", name);
            }

            // Symbols might have a separator '|', in which case we need to add
            // several symbols with the same token type into the dictionary
            char delim[] = "|";
            char* fragment = ::strtok(const_cast<char*>(lexeme), delim);
            while (fragment)
            {
                _addLexeme(fragment, Token(name, value, tt), useTrie);
                fragment = ::strtok(nullptr, delim);
            }
            // all separators are 1-byte ASCII
            if (isSeparator)
            {
                separators.push_back(lexeme[0]);
            }
        }
    }
}

void MoleculeNameParser::DictionaryManager::_addLexeme(const string& lexeme, const Token& token, bool useTrie)
{
    dictionary[lexeme] = token;
    if (useTrie)
    {
        lexemesTrie.addWord(lexeme, token);
    }
}

/*
Converts a token type name into token type value
Returns TokenType::UNKNOWN if no matching found
*/
MoleculeNameParser::TokenType MoleculeNameParser::DictionaryManager::_tokenTypeFromString(const string& s)
{
    const auto& begin = std::begin(_tokenTypeStrings);
    const auto& end = std::end(_tokenTypeStrings);
    const auto& it = std::find(begin, end, s);
    if (it != end)
    {
        return static_cast<TokenType>(std::distance(begin, it));
    }

    return TokenType::UNKNOWN;
}

/*
Performs by-symbol input scan, determines basic tokens
Text fragments require further processing
*/
void MoleculeNameParser::Parse::scan()
{
    const DictionaryManager& dm = mnp.dictionaryManager;
    const SymbolDictionary& sd = dm.dictionary;
    const string& separators = dm.separators;

    // Check for trivial names
    const string& trivial = input;
    const auto& it = sd.find(trivial);
    if (it != sd.end())
    {
        lexemes.push_back(Lexeme(it->first, it->second));

        Token terminator("", "", TokenType::END_OF_STREAM);
        lexemes.push_back(Lexeme("", terminator));

        return;
    }

    const size_t length = input.length();

    // A buffer for locant symbol(s)
    string locant;

    /*
    If a symbol is a separator, convert it into a lexeme
    If not, scan until either a next separator, flag, or an end of the string
    is reached, then add a lexeme for text fragment

    By this time we already know that brackets do match
    */
    for (size_t i = 0; i < length; i++)
    {
        char ch = input[i];

        // whitespace is a special case
        if (ch == ' ')
        {
            Token token{"separator", {ch}, TokenType::PUNCTUATION};
            lexemes.push_back({ch, token});
        }

        /*
        The input symbol is a separator; add a separator lexeme
        */
        size_t pos = separators.find(ch);
        if (pos != separators.npos)
        {
            const auto& sdit = sd.find({ch});
            if (sdit != sd.end())
            {
                // For locants, we need additional check if the number is multi-digit
                if (std::isdigit(ch))
                {
                    locant += ch;
                    while (std::isdigit(input[i + 1]))
                    {
                        locant += input[i + 1];
                        ++i;
                    }
                    lexemes.push_back(Lexeme(locant, Token("separator", locant, TokenType::LOCANT)));
                    locant.clear();
                    continue;
                }
                lexemes.push_back(Lexeme(ch, sdit->second));
            }
            continue;
        }

        /*
        The current fragment is a text fragment
        Search until a next separator or the end of string, check the dictionary,
        process text fragment
        */
        size_t next = input.find_first_of(separators, i);
        if (next == input.npos)
        {
            string fragment = input.substr(i, length - i);
            _processTextFragment(fragment);
            break;
        }
        else
        {
            string fragment = input.substr(i, next - i);
            _processTextFragment(fragment);
            i = next - 1;
            continue;
        }
    }

    Token terminator("", "", TokenType::END_OF_STREAM);
    lexemes.push_back(Lexeme("", terminator));
}

// Retrieves a next lexeme from the stream, incrementing the stream pointer
const MoleculeNameParser::Lexeme& MoleculeNameParser::Parse::getNextLexeme() const
{
    if (currentLexeme < lexemes.size())
    {
        return lexemes[currentLexeme++];
    }

    throw Error("Lexemes stream pointer overflow");
}

// Returns true if next lexeme's token type equals to input
bool MoleculeNameParser::Parse::peekNextToken(TokenType peek) const
{
    const Lexeme& lexeme = lexemes[currentLexeme];
    return (lexeme.token.type == peek);
}

/*
Splits a fragment into smaller lexemes
Sets up the failure flag if unparsable fragment is encountered
*/
void MoleculeNameParser::Parse::_processTextFragment(const string& fragment)
{
    const DictionaryManager& dm = mnp.dictionaryManager;
    const LexemesTrie& root = dm.lexemesTrie;

    const size_t fLength = fragment.length();

    // global position inside the input string
    size_t total = 0;

    string buffer = fragment;

    /*
    Slow track with elision, see tryElision
    */
    while (total < fLength)
    {
        /*
        Fast track if trie already contains the word
        */
        if (root.isWord(buffer))
        {
            const Trie<Token>* wordNode = root.getNode(buffer);
            const Token& token = wordNode->getData();
            lexemes.push_back(Lexeme(buffer, token));
            return;
        }

        // current position inside a buffer
        int current = 0;

        const Trie<Token>* match = root.getNode({buffer[0]});
        if (!match)
        {
            failures.push_back(buffer);
            hasFailures = true;
            return;
        }

        while (match && !match->isMark())
        {
            match = match->getNode({buffer[++current]});
            total++;
        }

        // need to increment counters here, as we manupulate characters and
        // not buffer positions
        current++;
        total++;

        string lexeme = buffer.substr(0, current);

        if (!match)
        {
            if (_tryElision(lexeme))
            {
                // decrement counters as we step back 1 char
                total--;
                buffer = buffer.substr(--current);
                continue;
            }

            failures.push_back(lexeme);
            hasFailures = true;
            return;
        }

        const Token& token = match->getData();
        lexemes.push_back(Lexeme(lexeme, token));

        buffer = buffer.substr(current);
    }
}

/*
Tries to apply different elision rules to a lexeme
For instance, 'pentyl' consists of 2 lexemes, 'penta' and 'yl',
with 'a' being elided. Hence, the first lexeme will be 'penty',
and parsing will fail as there's no known word 'penty'
The idea is to try several different word endidngs and see if we actually
know the word in question
*/
bool MoleculeNameParser::Parse::_tryElision(const string& failure)
{
    const DictionaryManager& dm = mnp.dictionaryManager;
    const LexemesTrie& root = dm.lexemesTrie;

    string endings = "aoey";
    string tryout = failure;
    for (char ch : endings)
    {
        tryout.replace(tryout.length() - 1, 1, {ch});
        if (!root.isWord(tryout))
        {
            tryout = failure;
            tryout.insert(0, 1, ch);
            if (!root.isWord(tryout))
            {
                tryout = failure;
                tryout += ch;
                if (!root.isWord(tryout))
                {
                    return false;
                }
            }
        }
        _processTextFragment(tryout);
        return true;
    }

    return false;
}

MoleculeNameParser::FragmentNode::~FragmentNode()
{
    for (FragmentNode* node : nodes)
    {
        delete node;
    }
}

// Inserts a new node before anchor position, returns status
bool MoleculeNameParser::FragmentNode::insertBefore(FragmentNode* node, const FragmentNode* anchor)
{
    node->parent = this;
    const auto& position = std::find(nodes.begin(), nodes.end(), anchor);
    if (position != nodes.end())
    {
        nodes.insert(position, node);
        return true;
    }

    return false;
}

void MoleculeNameParser::FragmentNode::insert(FragmentNode* node)
{
    node->parent = this;
    nodes.push_back(node);
}

#ifdef DEBUG
void MoleculeNameParser::FragmentNode::print(ostream& out) const
{
    out << "Parent: " << parent << endl;

    if (classType == FragmentClassType::INVALID)
    {
        out << "Type: INVALID" << endl;
    }
}

void MoleculeNameParser::FragmentNodeRoot::print(ostream& out) const
{
    out << "Type: FragmentNodeRoot" << endl;
    FragmentNode::print(out);
}

void MoleculeNameParser::FragmentNodeBase::print(ostream& out) const
{
    out << "Type: FragmentNodeBase" << endl;

    out << "Multipliers:" << endl;
    auto muls = multipliers;
    Multipliers muls_rev;
    while (!muls.empty())
    {
        muls_rev.push(muls.top());
        muls.pop();
    }
    while (!muls_rev.empty())
    {
        out << "\tvalue: " << muls_rev.top().first << endl;
        muls_rev.pop();
    }

    out << "Locants:" << endl;
    for (int locant : locants)
    {
        out << "\tvalue: " << locant << endl;
    }

    out << "Element: " << endl;
    out << "\tnumber: " << element.first << endl;
    out << "\tsymbol: " << element.second << endl;
    FragmentNode::print(out);
}

void MoleculeNameParser::FragmentNodeSubstituent::print(ostream& out) const
{
    out << "Type: FragmentNodeSubstituent" << endl;
    out << "Positions:" << endl;
    for (int pos : positions)
    {
        out << "\tvalue: " << pos << endl;
    }
    out << "Fragment multiplier: " << fragmentMultiplier << endl;
    FragmentNodeBase::print(out);
}

#if 0
ostream& operator<<(ostream& out, const MoleculeNameParser::FragmentNode& node) {
   node.print(out);
   return out;
}

ostream& operator<<(ostream& out, const MoleculeNameParser::FragmentNode* node) {
   node->print(out);
   return out;
}
#endif
#endif // DEBUG

MoleculeNameParser::FragmentNodeBase::FragmentNodeBase()
{
    classType = FragmentClassType::BASE;
    element.first = ELEM_MIN;
}

/*
Returns the sum of multipliers stack
This is a destructive operation
*/
int MoleculeNameParser::FragmentNodeBase::combineMultipliers()
{
    int result = 0;
    while (!multipliers.empty())
    {
        const Multiplier& mul = multipliers.top();
        result += mul.first;
        multipliers.pop();
    }

    assert(multipliers.empty());
    return result;
}

MoleculeNameParser::FragmentBuildTree::FragmentBuildTree()
{
    addRoot();
}

MoleculeNameParser::FragmentBuildTree::~FragmentBuildTree()
{
    for (FragmentNode* root : roots)
    {
        delete root;
    }
}

void MoleculeNameParser::FragmentBuildTree::addRoot()
{
    FragmentNode* root = new FragmentNodeRoot;
    currentRoot = root;
    roots.push_back(root);
}

bool MoleculeNameParser::TreeBuilder::processParse()
{
    _initBuildTree();
    return _processParse();
}

void MoleculeNameParser::TreeBuilder::_initBuildTree()
{
    FragmentNodeBase* node = new FragmentNodeBase;
    FragmentNode* root = buildTree.currentRoot;
    root->insert(node);
    _current = node;
}

/*
Checks if certain options are (un)set
Returns true if condition matches
*/
bool MoleculeNameParser::TreeBuilder::_checkParserOption(ParserOptionsType options)
{
    return (_parse->mnp.getOptions() & options) == options;
}

/*
Returns one level up in the tree, setting current node to the new
level's base fragment
Returns false if operation cannot be performed
*/
bool MoleculeNameParser::TreeBuilder::_upOneLevel()
{
    if (_parse->peekNextToken(TokenType::END_OF_STREAM))
    {
        return true;
    }

    if (_current->classType == FragmentClassType::BASE)
    {
        if (!_current->parent)
        {
            return false;
        }
    }

    _startNewNode = true;
    _current = _getParentBase();
    return (_current != nullptr);
}

/*
The implementation of parse processing
Recursively calls itself until EndOfStream is reached or error occured
*/
bool MoleculeNameParser::TreeBuilder::_processParse()
{
    const Lexeme& lexeme = _parse->getNextLexeme();

    const TokenType& tt = lexeme.token.type;
    if (lexeme.processed)
    {
        if (tt == TokenType::SUFFIXES && lexeme.lexeme == "yl")
        {
            _current = _getCurrentBase();
        }
        return _processParse();
    }

    if (tt == TokenType::END_OF_STREAM)
    {
        return true;
    }

    if ((tt == TokenType::UNKNOWN) || (tt == TokenType::TEXT))
    {
        return false;
    }

    const string& tname = lexeme.token.name;
    if (tname == "alkanes")
    {
        if (!_processAlkane(lexeme))
        {
            return false;
        }
    }
    else if (tname == "multiplier")
    {
        if (!_processMultiplier(lexeme))
        {
            return false;
        }
    }
    else if (tname == "separator")
    {
        if (!_processSeparator(lexeme))
        {
            return false;
        }
    }
    else if (tname == "basicElement")
    {
        if (!_processBasicElement(lexeme))
        {
            return false;
        }
    }
    else if (tname == "flags")
    {
        if (!_processFlags(lexeme))
        {
            return false;
        }
    }
    else if (tname == "skeletal")
    {
        if (!_processSkeletal(lexeme))
        {
            return false;
        }
    }

    return _processParse();
}

bool MoleculeNameParser::TreeBuilder::_processSkeletalPrefix(const Lexeme& lexeme)
{
    if (_current->classType != FragmentClassType::SUBSTITUENT)
    {
        FragmentNodeSubstituent* subst = new FragmentNodeSubstituent;
        if (!_current->parent->insertBefore(subst, _getCurrentBase()))
        {
            return false;
        }
        subst->positions.push_back(1);

        FragmentNodeBase* levelBase = _getCurrentBase();
        if (levelBase->locants.empty())
        {
            levelBase->locants.push_back(1);
        }

        _current = subst;
    }

    const Token& token = lexeme.token;
    const string& value = token.value;
    const size_t pos = value.find('_');
    if (pos == string::npos)
    {
        return false;
    }

    const string bonding = value.substr(0, pos);
    const string symbol = value.substr(pos + 1);

    FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
    base->element.first = indigo::Element::fromString(symbol.c_str());
    base->element.second = symbol;
    base->nodeType = NodeType::SKELETAL;
    base->bondType = BOND_SINGLE;
    base->bonding = _strToInt(bonding);
    base->multipliers.push({1, TokenType::BASIC});

    /*
    We erase skeletal positions from base's locants list, as skeletals are not
    substituents, and move it into skeletals
    */
    FragmentNodeBase* levelBase = _getCurrentBase();
    Locants& locants = levelBase->locants;
    Skeletals& skeletals = levelBase->skeletals;
    FragmentNodeSubstituent* subst = dynamic_cast<FragmentNodeSubstituent*>(_current);
    for (int n : subst->positions)
    {
        const auto& it = std::find(locants.begin(), locants.end(), n);
        if (it != locants.end())
        {
            int val = *it;
            locants.erase(it);
            skeletals.push_back(val);
        }
    }

    _current = _getCurrentBase();
    _startNewNode = true;
    lexeme.processed = true;
    return true;
}

bool MoleculeNameParser::TreeBuilder::_processSkeletal(const Lexeme& lexeme)
{
    bool result = true;

    switch (lexeme.token.type)
    {
    case TokenType::SKELETAL_PREFIX: {
        result = _processSkeletalPrefix(lexeme);
    }
    break;

    default:
        break;
    }

    return result;
}

bool MoleculeNameParser::TreeBuilder::_processFlags(const Lexeme& lexeme)
{
    const string& name = lexeme.lexeme;

    if (name == "cyclo")
    {
        FragmentNodeBase* base = _getCurrentBase();
        if (base == nullptr)
        {
            return false;
        }

        if (base->cycle == true)
        {
            return false;
        }

        base->cycle = true;
        lexeme.processed = true;
        return true;
    }

    if (name == "cis" || name == "trans")
    {
        if ((_current->classType == FragmentClassType::SUBSTITUENT) || (_current->classType == FragmentClassType::BASE))
        {
            FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
            if (name == "cis")
            {
                base->isomerism = Isomerism::CIS;
            }
            else
            {
                base->isomerism = Isomerism::TRANS;
            }
            lexeme.processed = true;
            return true;
        }
    }

    return false;
}

bool MoleculeNameParser::TreeBuilder::_processBasicElement(const Lexeme& lexeme)
{
    if (_current->classType != FragmentClassType::BASE)
    {
        return false;
    }

    const string& value = lexeme.token.value;
    const size_t pos = value.find('_');
    if (pos == string::npos)
    {
        return false;
    }

    const string number = value.substr(0, pos);
    const string symbol = value.substr(pos + 1);
    const int element = _strToInt(number);

    FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
    base->element.first = element;
    base->element.second = symbol;
    base->nodeType = NodeType::ELEMENT;
    base->multipliers.push({1, TokenType::BASIC});

    lexeme.processed = true;
    return true;
}

/*
Processes suffixes
Suffixes are -ane, -ene, -yne|-yn, -yl
-ane means that all bonds are single, e.g. hexane CCCCCC
-ene means that at least the first bond is double
locants signify multiple atoms with double bonds, e.g. 2,4-hexadiene CC=CC=CC
-yne|-yn means that at least the first bond is triple
locants signify multiple atoms with triple bonds, e.g. 2,4-hexadiyne CC#CC#CC
*/
void MoleculeNameParser::TreeBuilder::_processSuffix(const Lexeme& lexeme)
{
    FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
    if (base->nodeType == NodeType::INVALID)
    {
        base->nodeType = NodeType::SUFFIX;
    }

    base->element.first = ELEM_C;
    base->element.second = "C";

    if (base->multipliers.empty())
    {
        base->multipliers.push({1, TokenType::BASIC});
    }

    // A stardard carbon bonding
    const int carbonBonding = 4;

    if (lexeme.lexeme == "ane")
    {
        base->bondType = BOND_SINGLE;
        base->bonding = carbonBonding - BOND_SINGLE;
        base->freeAtoms = 0;
    }
    else if (lexeme.lexeme == "yl")
    {
        base->bondType = BOND_SINGLE;
        base->bonding = carbonBonding - BOND_SINGLE - 1;
        base->freeAtoms = 1;
    }
    else if (lexeme.lexeme == "ene")
    {
        base->bondType = BOND_DOUBLE;
        base->bonding = carbonBonding - BOND_DOUBLE;
        base->freeAtoms = 0;
    }
    else if (lexeme.lexeme == "yne" || lexeme.lexeme == "yn")
    {
        base->bondType = BOND_TRIPLE;
        base->bonding = carbonBonding - BOND_TRIPLE;
        base->freeAtoms = 0;
    }

    if (_current->classType == FragmentClassType::SUBSTITUENT)
    {
        FragmentNodeBase* currentBase = _getCurrentBase();
        if (!currentBase)
        {
            throw Error("Can't get current level base node");
        }
        currentBase->element.first = ELEM_C;
        currentBase->element.second = "C";

        _startNewNode = true;
    }
}

bool MoleculeNameParser::TreeBuilder::_processAlkaneBase(const Lexeme& lexeme)
{
    FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
    base->nodeType = NodeType::BASE;

    int value = _strToInt(lexeme.token.value);
    base->multipliers.push({value, TokenType::BASIC});

    return true;
}

bool MoleculeNameParser::TreeBuilder::_processAlkaneSuffix(const Lexeme& lexeme)
{
    _processSuffix(lexeme);

    if (_parse->peekNextToken(TokenType::CLOSING_BRACKET))
    {
        lexeme.processed = true;
        return true;
    }

    if (_current->classType == FragmentClassType::SUBSTITUENT)
    {
        _current = _getCurrentBase();
        if (!_current)
        {
            return false;
        }
    }
    else if (_current->classType == FragmentClassType::BASE)
    {
        if (!_upOneLevel())
        {
            return false;
        }
    }

    lexeme.processed = true;
    return true;
}

// Processes alkane lexemes
bool MoleculeNameParser::TreeBuilder::_processAlkane(const Lexeme& lexeme)
{
    bool result = true;

    switch (lexeme.token.type)
    {
    case TokenType::BASES: {
        result = _processAlkaneBase(lexeme);
    }
    break;

    case TokenType::SUFFIXES: {
        result = _processAlkaneSuffix(lexeme);
    }
    break;

    default:
        break;
    }

    return result;
}

bool MoleculeNameParser::TreeBuilder::_processBasicMultiplier(const Lexeme& lexeme)
{
    const int value = _strToInt(lexeme.token.value);

    if (_current->classType == FragmentClassType::SUBSTITUENT)
    {
        FragmentNodeSubstituent* node = dynamic_cast<FragmentNodeSubstituent*>(_current);
        if (node->expectFragMultiplier)
        {
            if (value != (int)node->positions.size())
            {
                throw Error("Locants and fragment multiplier don't match");
            }

            node->fragmentMultiplier = value;
            bool flag = _parse->peekNextToken(TokenType::FACTOR);
            node->expectFragMultiplier = flag;
            lexeme.processed = true;
            return true;
        }
    }

    FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
    base->multipliers.push({value, lexeme.token.type});
    base->nodeType = NodeType::BASE;

    lexeme.processed = true;
    return true;
}

bool MoleculeNameParser::TreeBuilder::_processFactorMultiplier(const Lexeme& lexeme)
{
    int value = _strToInt(lexeme.token.value);

    if (_current->classType == FragmentClassType::SUBSTITUENT)
    {
        FragmentNodeSubstituent* node = dynamic_cast<FragmentNodeSubstituent*>(_current);
        if (node->expectFragMultiplier)
        {
            if (node->fragmentMultiplier != 1)
            {
                node->fragmentMultiplier *= value;
            }
            node->expectFragMultiplier = false;
            lexeme.processed = true;
            return true;
        }
    }

    FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
    Multipliers& multipliers = base->multipliers;
    if (multipliers.empty())
    {
        multipliers.push({value, TokenType::BASIC});
    }
    else
    {
        const Multiplier& prev = multipliers.top();
        value = _strToInt(lexeme.token.value);
        value *= prev.first;
        multipliers.pop();
        multipliers.push({value, TokenType::BASIC});
    }
    base->nodeType = NodeType::BASE;

    lexeme.processed = true;
    return true;
}

// Processes multiplier lexemes
bool MoleculeNameParser::TreeBuilder::_processMultiplier(const Lexeme& lexeme)
{
    bool result = true;

    switch (lexeme.token.type)
    {
    case TokenType::BASIC: {
        result = _processBasicMultiplier(lexeme);
    }
    break;

    case TokenType::FACTOR: {
        result = _processFactorMultiplier(lexeme);
    }
    break;

    default:
        break;
    }

    return result;
}

bool MoleculeNameParser::TreeBuilder::_processLocant(const Lexeme& lexeme)
{
    int value = _strToInt(lexeme.token.value);
    if (value == 0)
    {
        return false;
    }

    if (_startNewNode)
    {
        FragmentNodeSubstituent* subst = new FragmentNodeSubstituent;
        if (!_current->parent->insertBefore(subst, _getCurrentBase()))
        {
            return false;
        }
        _current = subst;
        _startNewNode = false;
    }

    FragmentNodeSubstituent* subst = dynamic_cast<FragmentNodeSubstituent*>(_current);
    subst->positions.push_back(value);

    FragmentNodeBase* base = _getCurrentBase();
    base->locants.push_back(value);

    if (!_checkParserOption(IUPAC_STRICT))
    {
        auto it = _parse->lexemes.begin();
        std::advance(it, _parse->currentLexeme);

        if ((it->token.type == TokenType::PUNCTUATION) && (it->lexeme == ","))
        {
            lexeme.processed = true;
            return true;
        }

        /*
        Look for nearest suffix
        Then, check previous multiplier. If it's not equal to expected multiplier,
        continue to parse lexemes in order
        */
        while (!(it->token.type == TokenType::LOCANT || it->token.type == TokenType::SUFFIXES || it->token.type == TokenType::SKELETAL_PREFIX ||
                 it->token.type == TokenType::END_OF_STREAM))
        {
            ++it;
        }

        switch (it->token.type)
        {
        // shouldn't be here
        case TokenType::END_OF_STREAM: {
            return false;
        }
        break;

        // mark lexeme as processed, continue with processing
        case TokenType::SKELETAL_PREFIX:
        case TokenType::LOCANT: {
            lexeme.processed = true;
            return true;
        }
        break;

        case TokenType::SUFFIXES: {
            if (it->lexeme == "yl")
            {
                lexeme.processed = true;
                return true;
            }

            if (subst->positions.size() == 1)
            {
                return _processAlkaneSuffix(*it);
            }

            auto prev = std::prev(it);
            if (prev->token.type == TokenType::BASIC)
            {
                if (!_processMultiplier(*prev))
                {
                    lexeme.processed = false;
                    return true;
                }
                return _processAlkaneSuffix(*it);
            }
        }
        break;

        default:
            return false;
        }
    }

    lexeme.processed = true;
    return true;
}

bool MoleculeNameParser::TreeBuilder::_processPunctuation(const Lexeme& lexeme)
{
    if (lexeme.lexeme == ",")
    {
        if (_current->classType != FragmentClassType::SUBSTITUENT)
        {
            return false;
        }
        dynamic_cast<FragmentNodeSubstituent*>(_current)->expectFragMultiplier = true;
        lexeme.processed = true;
        return true;
    }

    /*
    Whitespace is a special case when we actually add a new build tree
    for a new structure
    */
    // FIXME - process acids (acid names have whitespace)
    if (lexeme.lexeme == " ")
    {
        buildTree.addRoot();
        _initBuildTree();
        lexeme.processed = true;
        return true;
    }

    lexeme.processed = true;
    return true;
}

// Processes separator lexemes
bool MoleculeNameParser::TreeBuilder::_processSeparator(const Lexeme& lexeme)
{
    switch (lexeme.token.type)
    {
    case TokenType::OPENING_BRACKET: {
        // empty brackets aren't allowed
        if (_parse->peekNextToken(TokenType::CLOSING_BRACKET))
        {
            return false;
        }

        FragmentNodeBase* base = new FragmentNodeBase;
        base->nodeType = NodeType::BASE;
        _current->insert(base);
        _current = base;
        _startNewNode = true;
    }
    break;

    case TokenType::CLOSING_BRACKET: {
        if (!_upOneLevel())
        {
            return false;
        }
    }
    break;

    case TokenType::LOCANT: {
        return _processLocant(lexeme);
    }
    break;

    // currently no-op
    case TokenType::PRIME:
        break;

    case TokenType::PUNCTUATION: {
        return _processPunctuation(lexeme);
    }
    break;

    default:
        break;
    }

    lexeme.processed = true;
    return true;
}

// Converts std::string to int
int MoleculeNameParser::TreeBuilder::_strToInt(const string& str)
{
    char* ch = nullptr;
    return std::strtol(str.c_str(), &ch, 10);
}

// Retrieves current level's base; each level has only one base
MoleculeNameParser::FragmentNodeBase* MoleculeNameParser::TreeBuilder::_getCurrentBase()
{
    if (_current->classType == FragmentClassType::BASE)
    {
        return dynamic_cast<FragmentNodeBase*>(_current);
    }

    FragmentNode* parent = _current->parent;
    if (!parent)
    {
        return nullptr;
    }

    // Base is always the rightmost node
    return dynamic_cast<FragmentNodeBase*>(parent->nodes.back());
}

// Retrieves upper level's base, if any; each level has only one base
MoleculeNameParser::FragmentNodeBase* MoleculeNameParser::TreeBuilder::_getParentBase()
{
    const FragmentNode* parent = _current->parent;
    if (!parent)
    {
        return nullptr;
    }

    const FragmentNode* grand = parent->parent;
    if (!grand)
    {
        return nullptr;
    }

    // Base is always the rightmost node
    return dynamic_cast<FragmentNodeBase*>(grand->nodes.back());
}

void MoleculeNameParser::SmilesBuilder::_initOrganicElements()
{
    _organicElements[ELEM_B] = "B";
    _organicElements[ELEM_C] = "C";
    _organicElements[ELEM_N] = "N";
    _organicElements[ELEM_O] = "O";
    _organicElements[ELEM_P] = "P";
    _organicElements[ELEM_S] = "S";
    _organicElements[ELEM_F] = "F";
    _organicElements[ELEM_Cl] = "Cl";
    _organicElements[ELEM_Br] = "Br";
}

void MoleculeNameParser::SmilesBuilder::_buildSmiles(SmilesRoot& root)
{
    for (SmilesNode& node : root.nodes)
    {
        _SMILES += node.str;

        if (node.bondType == BOND_DOUBLE)
        {
            _SMILES += "=";
        }

        if (node.bondType == BOND_TRIPLE)
        {
            _SMILES += "#";
        }

        if (!node.roots.empty())
        {
            for (SmilesRoot& r : node.roots)
            {
                _SMILES += "(";
                _buildSmiles(r);
                _SMILES += ")";
            }
        }
    }
}

/*
Traverses the build tree in post-order depth-first order, creates
SMILES representation and loads the SMILES into the resulting Molecule
*/
bool MoleculeNameParser::SmilesBuilder::buildResult(Molecule& molecule)
{
    molecule.clear();

    const FragmentBuildTree& buildTree = _treeBuilder.buildTree;
    const Nodes& roots = buildTree.roots;
    if (roots.empty())
    {
        return false;
    }

    // most time we'll have a sigle root
    for (FragmentNode* root : roots)
    {
        const Nodes& nodes = root->nodes;
        if (!_processNodes(nodes, _smilesTree))
        {
            return false;
        }
    }

    _buildSmiles(_smilesTree);

    BufferScanner scanner(_SMILES.c_str());
    SmilesLoader loader(scanner);
    loader.loadMolecule(molecule);

    return true;
}

bool MoleculeNameParser::SmilesBuilder::_processNodes(const Nodes& nodes, SmilesRoot& root)
{
    auto node = nodes.rbegin();
    if (!_processBaseNode(dynamic_cast<FragmentNodeBase*>(*node), root))
    {
        return false;
    }

    ++node;
    while (node != nodes.rend())
    {
        if (!_processSubstNode(dynamic_cast<FragmentNodeSubstituent*>(*node), root))
        {
            return false;
        }

        ++node;
    }

    return true;
}

/*
Processes a base node. A base node contains information about structure or
substituent base: number of locants, chemical element info, bonds, etc.
*/
bool MoleculeNameParser::SmilesBuilder::_processBaseNode(FragmentNodeBase* base, SmilesRoot& root)
{
    const NodeType& nt = base->nodeType;
    const Element& element = base->element;
    const int number = element.first;

    const int multipliers = base->combineMultipliers();
    if (multipliers >= 1)
    {
        bool organicElement = (_organicElements.find(number) != _organicElements.end());
        if (nt == NodeType::ELEMENT)
        {
            organicElement = false;
        }

        const string& symbol = organicElement ? _organicElements[number] : "[" + element.second + "]";
        SmilesNode node(symbol, BOND_SINGLE, &root);
        root.nodes.push_back(std::move(node));

        for (int i = 1; i < multipliers; i++)
        {
            SmilesNode node1(symbol, BOND_SINGLE, &root);
            root.nodes.push_back(std::move(node1));
        }
    }

    if (base->cycle)
    {
        if (root.nodes.size())
        {
            SmilesNode& first = root.nodes.front();
            first.str += "1";

            SmilesNode& last = root.nodes.back();
            last.str += "1";
        }
        else
            throw Exception("Error at _processBaseNode. Bad structure name.");
    }

    if (base->bondType != BOND_SINGLE)
    {
        SmilesNode& sn = root.nodes.front();
        sn.bondType = base->bondType;
    }

    return true;
}

void MoleculeNameParser::SmilesBuilder::_calcHydrogens(const Element& element, int pos, SmilesRoot& root)
{
    int number = indigo::Element::fromString(element.second.c_str());
    if (number == ELEM_C)
    {
        return;
    }

    bool organicElement = (_organicElements.find(number) != _organicElements.end());

    int connections = indigo::Element::getMaximumConnectivity(number, 0, 0, false);
    int valence = indigo::Element::calcValenceMinusHyd(number, 0, 0, connections);

    if ((pos - 1) < static_cast<long long>(root.nodes.size()))
    {
        SmilesNode& sn = root.nodes.at(pos - 1);

        string buffer;
        if (!organicElement)
        {
            int hydrogens = 0;

            if (root.nodes.size() == 1)
            {
                hydrogens = valence;
            }
            else
            {
                if (pos > 1)
                {
                    const SmilesNode& prev = root.nodes.at(pos - 2);

                    int prevBond = prev.bondType;
                    hydrogens = valence - prevBond - sn.bondType;
                }
                else
                {
                    hydrogens = valence - sn.bondType;
                }
            }

            if (hydrogens > 0)
            {
                char buff[12];
                ::sprintf(buff, "%d", hydrogens);
                buffer += "[" + element.second + "H" + buff + "]";
            }
            else
            {
                buffer += "[" + element.second + "]";
            }
        }
        else
        {
            buffer = _organicElements[number];
        }

        sn.str = buffer;
    }
    else
        throw Exception("Error at calcHydrogen. Bad structure name.");
}

/*
Processes a substituent node. Any substituent might also be a base
*/
bool MoleculeNameParser::SmilesBuilder::_processSubstNode(FragmentNodeSubstituent* subst, SmilesRoot& root)
{
    const Nodes& nodes = subst->nodes;
    const Positions& positions = subst->positions;

    if (!nodes.empty())
    {
        for (int pos : positions)
        {
            SmilesNode& sn = root.nodes.at(pos - 1);
            sn.roots.push_back(SmilesRoot{&sn});
            if (!_processNodes(nodes, sn.roots.back()))
            {
                return false;
            }
        }

        return true;
    }

    FragmentNodeBase* as_base = subst;
    const Element& element = as_base->element;
    const int number = element.first;
    bool organicElement = (_organicElements.find(number) != _organicElements.end());

    switch (subst->nodeType)
    {
    case NodeType::SUFFIX: {
        for (int pos : positions)
        {
            SmilesNode& sn = root.nodes.at(pos - 1);
            sn.bondType = as_base->bondType;

            _calcHydrogens(element, pos, root);
        }
    }
    break;

    case NodeType::SKELETAL: {
        for (int pos : positions)
        {
            _calcHydrogens(element, pos, root);
        }
    }
    break;

    case NodeType::BASE: {
        int multiplier = as_base->combineMultipliers();

        for (int pos : positions)
        {
            SmilesNode& sn = root.nodes.at(pos - 1);
            SmilesRoot r(&sn);

            string symbol = organicElement ? _organicElements[number] : "[" + element.second + "]";
            for (int i = 0; i < multiplier; i++)
            {
                r.nodes.push_back({symbol, BOND_SINGLE, &r});
            }

            sn.roots.push_back(std::move(r));
        }
    }
    break;

    default:
        break;
    }

    return true;
}

bool MoleculeNameParser::SmilesBuilder::checkTrivial()
{
    bool good = true;
    for (const Lexeme& l : _parse->lexemes)
    {
        if (!(l.token.type == TokenType::TRIVIAL || l.token.type == TokenType::END_OF_STREAM))
        {
            good = false;
            break;
        }
    }

    return good;
}

void MoleculeNameParser::SmilesBuilder::buildTrivial(Molecule& molecule)
{
    molecule.clear();

    // currently, build the first trivial name
    const Lexeme& l = _parse->getNextLexeme();

    BufferScanner scanner(l.token.value.c_str());
    SmilesLoader loader(scanner);
    loader.loadMolecule(molecule);
}

/*
Main method for convertion from a chemical name into a Molecule object
A given name undergoes several transformations:
   phase 1: lexical analysis
   phase 2: tokenization
   phase 3: grammatical rules check
   phase 4: construction of a SMILES representation from parsed fragments
No param check - did that on caller side
*/
void MoleculeNameParser::parseMolecule(const char* name, Molecule& molecule)
{
    string input(name);
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned long c) { return std::tolower(c); });

    _checkBrackets(input);
    Parse parse(input, *this);
    parse.scan();

    if (parse.hasFailures)
    {
        string message;
        for (const string& f : parse.failures)
        {
            message += f + " ";
        }
        throw Error("Cannot parse input %s due to errors: %s", name, message.c_str());
    }

    SmilesBuilder builder(parse);
    if (builder.checkTrivial())
    {
        builder.buildTrivial(molecule);
        return;
    }

    if (!builder.buildTree())
    {
        molecule.clear();
        throw Error("Cannot construct the build tree for name %s", name);
    }

    if (!builder.buildResult(molecule))
    {
        molecule.clear();
        throw Error("Cannot build a resulting structure for name %s", name);
    }
}

/*
Checks if allowed opening and closing brackets match
Doesn't check brackets type matching (e.g. ((]] is valid)
*/
void MoleculeNameParser::_checkBrackets(const string& s)
{
    int level = 0;
    for (char ch : s)
    {
        if (ch == '(' || ch == '[' || ch == '{')
        {
            level++;
            continue;
        }

        if (ch == ')' || ch == ']' || ch == '}')
        {
            level--;
            continue;
        }
    }

    if (level != 0)
    {
        throw Error("Opening and closing brackets don't match: %d", level);
    }
}

/*
Sets parsing options
Changes the 'options' parameter (via strtok())
*/
void MoleculeNameParser::setOptions(char* options)
{
    // null check is on caller side

    if (::strlen(options) == 0)
    {
        return;
    }

    char delim[] = " ";
    char* option = ::strtok(options, delim);
    while (option)
    {
        _setOption(option);
        option = ::strtok(nullptr, delim);
    }
}

// Turns a certain option on or off depending on input flag
void MoleculeNameParser::_setOption(const char* option)
{
    string buffer = option;
    if ((buffer[0] != '+') && (buffer[0] != '-'))
    {
        throw Error("Invalid option notation: %s", option);
    }

    bool on = (buffer[0] == '+') ? true : false;

    if (std::move(buffer.substr(1)) == "IUPAC_STRICT")
    {
        on ? _options |= ParserOptions::IUPAC_STRICT : _options &= ~ParserOptions::IUPAC_STRICT;
    }
}
