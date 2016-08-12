/****************************************************************************
* Copyright (C) 2009-2016 EPAM Systems
*
* This file is part of Indigo toolkit.
*
* This file may be distributed and/or modified under the terms of the
* GNU General Public License version 3 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in the
* packaging of this file.
*
* This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
* WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
***************************************************************************/

#include <algorithm>
#include <cstdlib>

#include "molecule/molecule_name_parser.h"

#include "molecule/alkanes.inc"
#include "molecule/basic_elements.inc"
#include "molecule/multipliers.inc"
#include "molecule/separators.inc"
#include "molecule/token_types.inc"

using namespace std;
using namespace indigo;
using namespace name_parsing;

IMPL_ERROR(MoleculeNameParser, "name_parsing::MoleculeNameParser");
IMPL_ERROR(AuxParseTools, "name_parsing::AuxParseTools");
IMPL_ERROR(Tokenizer, "name_parsing::Tokenizer");
IMPL_ERROR(TokenizationResult, "name_parsing::TokenizationResult");
IMPL_ERROR(DictionaryManager, "name_parsing::TableManager");
IMPL_ERROR(Parse, "name_parsing::Parse");
IMPL_ERROR(ResultBuilder, "name_parsing::ResultBuilder");
IMPL_ERROR(BuildFragment, "name_parsing::BuildFragment");

// Converts a given token type name into enum value, 'unknown' if no name found
TokenType Token::tokenTypeFromString(const std::string& s) {
	auto begin = std::begin(TokenTypeStrings);
	auto end = std::end(TokenTypeStrings);
	auto it = find(begin, end, s);
	if (it != end)
		return static_cast<TokenType>(distance(begin, it));
	else
		return TokenType::unknown;
}

DictionaryManager::DictionaryManager() {
	_dictionary.clear();
	_separators.clear();

	readTokenTypeStrings();

	readTable(alkanes_table, true);
	readTable(basic_elements_table, true);
	readTable(multipliers_table, true);
	readTable(separators_table);
}

void DictionaryManager::readTokenTypeStrings() {
	TiXmlDocument doc;

	doc.Parse(token_types_table);
	if (doc.Error())
		throw Error("Cannot parse the token types table");

	TiXmlHandle hdoc(&doc);
	TiXmlHandle tokenTypes = hdoc.FirstChild("tokenTypes");
	TiXmlElement* e = tokenTypes.FirstChild("tokenType").ToElement();
	for (; e; e = e->NextSiblingElement())
		TokenTypeStrings.push_back(e->GetText());
}

void DictionaryManager::readTable(const char* table, bool useTrie /* = false*/) {
	TiXmlDocument doc;

	doc.Parse(table);
	if (doc.Error())
		throw Error("Cannot parse table %s", table);

	TiXmlHandle hdoc(&doc);
	TiXmlHandle tokenTables = hdoc.FirstChild("tokenTables");
	TiXmlElement* tokenTable = tokenTables.FirstChild("tokenTable").ToElement();
	for (; tokenTable; tokenTable = tokenTable->NextSiblingElement()) {
		const char* name = tokenTable->Attribute("name");
		const char* type = tokenTable->Attribute("type");
		if (!name || !type)
			throw Error("Cannot parse table");

		const bool isSeparator = (::strcmp(name, "separator") == 0);
		TokenType tt = Token::tokenTypeFromString(type);

		TiXmlElement* e = tokenTable->FirstChild("token")->ToElement();
		for (; e; e = e->NextSiblingElement()) {
			const char* lexeme = e->GetText();
			const char* value = e->Attribute("value");
			if (!lexeme || !value)
				throw Error("Cannot parse table %s", name);
			
			// Symbols might have a separator '|', in which case we need to add
			// several symbols with the same token type into the dictionary
			char delim[] = "|";
			char* fragment = ::strtok(const_cast<char*>(lexeme), delim);
			while (fragment) {
				addLexeme(fragment, Token(name, value, tt), useTrie);
				fragment = ::strtok(nullptr, delim);
			}
			// all separators are 1-byte ASCII
			if (isSeparator)
				_separators.push_back(lexeme[0]);
		}
	}
}

void DictionaryManager::addLexeme(const string& lexeme, const Token& token, bool useTrie) {
	_dictionary[lexeme] = token;
	if (useTrie)
		_lexTrie.addWord(lexeme, token);
}

void Parse::scan() {
	DictionaryManager& dm = getDictionaryManagerInstance();
	const SymbolDictionary& dictionary = dm.getDictionary();
	const string& separators = dm.getSeparators();

	const size_t length = _input.length();

	/* If a symbol is a separator, convert it into a lexeme
	 * If not, scan until either a next separator or an end of the string is reached,
	 * then add a lexeme for text fragment
	 *
	 * By this time we're already know that brackets match
	 */
	for (size_t i = 0; i < length; i++) {
		char ch = _input.at(i);
		size_t pos = separators.find(ch);
		if (pos != separators.npos) {
			auto it = dictionary.find({ ch });
			if (it != dictionary.end())
				_lexemes.push_back(Lexeme(ch, it->second));
			continue;
		}

		size_t next = _input.find_first_of(separators, i);
		if (next == _input.npos) {
			string fragment = _input.substr(i, length - i);
			processTextFragment(fragment);
			break;
		}
		else {
			string fragment = _input.substr(i, next - i);
			processTextFragment(fragment);
			i = next - 1;
			continue;
		}
	}

	Token terminator;
	terminator._type = TokenType::endOfStream;
	_lexemes.push_back(Lexeme("", terminator));
}

Lexeme& Parse::getNextLexeme() {
	if (_currentLexeme < _lexemes.size())
		return _lexemes[_currentLexeme++];

	throw Error("Lexemes array owerflow");
}

const Lexeme& Parse::getNextLexeme() const {
	if (_currentLexeme < _lexemes.size())
		return _lexemes[_currentLexeme++];

	throw Error("Lexemes array owerflow");
}

void Parse::processTextFragment(const string& fragment) {
	DictionaryManager& dm = getDictionaryManagerInstance();
	const LexemesTrie& root = dm.getLexemesTrie();

	const size_t fLength = fragment.length();

	// global position inside the input string
	int total = 0;
	string buffer = fragment;

	while (total < fLength) {
		// current position inside a buffer
		int current = 0;

		const Trie<Token>* match = root.getNode({ buffer[0] });
		if (!match) {
			_failures.push_back(buffer);
			_hasFailures = true;
			return;
		}

		while (match && !match->isMark()) {
			match = match->getNode({ buffer[++current] });
			total++;
		}

		// need to increment counters here, as we manupulate characters and
		// not buffer positions
		current++;
		total++;

		string lexeme = buffer.substr(0, current);

		if (!match) {
			if (tryElision(lexeme)) {
				continue;
			}

			_failures.push_back(lexeme);
			_hasFailures = true;
			return;
		}

		const Token& token = match->getData();
		_lexemes.push_back(Lexeme(lexeme, token));

		buffer = buffer.substr(current);
	}
}

bool Parse::tryElision(const string& failure) {
	const Lexeme& last = _lexemes.back();
	const string& l = last.getLexeme();
	char ch = l.back();
	if (ch == 'a' || ch == 'e' || ch == 'o') {
		string tryout = failure;
		tryout.insert(0, 1, ch);
		processTextFragment(tryout);
		_elision = true;
		return true;
	}

	return false;
}

TokenizationResult Tokenizer::tokenize(const char* name) {
	string input(name);
	transform(input.begin(), input.end(), input.begin(), tolower);

	AuxParseTools::checkBrackets(input);
	TokenizationResult result(input);
	result.parse();
	if (!result.isCompletelyParsed()) {
		/* TODO
		 * result has some unparsed fragments, display warning
		 */
	}

	return result;
}

void TokenizationResult::parse() {
	_parse.scan();
	_completelyParsed = _parse.hasFailures();
}

/*
* Builds a result from a parse
* Returns true if successful
*/
bool ResultBuilder::build(const Parse& parse) {
	_mol->clear();

	BuildFragment fragment;
	if (!fragment.processLexeme(parse))
		return false;

	Array<int> mapping;
	_mol->mergeWithMolecule(fragment.toMolecule(), &mapping);
	
	Molecule::checkForConsistency(*_mol);
	return true;
}

BuildFragment::BuildFragment() {
	_fragment.clear();

	_hasMultiplier = false;
	_hasLocant = false;
	_multiplier = 0;
}

bool BuildFragment::processLexeme(const Parse& parse) {
	const Lexeme& l = parse.getNextLexeme();
	const Token& token = l.getToken();

	TokenType tt = token._type;
	const string tname = token._name;
	if (tt == TokenType::endOfStream)
		return true;
	if (tt == TokenType::unknown)
		throw Error("Unknown token encountered: %s-%s", tname, token._value);
	if (tt == TokenType::text)
		throw Error("Unparsed text fragment encountered: %s", l.getLexeme());

	if (tname == "alkanes") {
		processAlkane(l);
	}
	
	if (tname == "multiplier") {
		processMultiplier(l);
	}

	if (tname == "separator") {
		processSeparator(l);
	}

	return processLexeme(parse);
}

void BuildFragment::processAlkane(const Lexeme& l) {
	const string& text = l.getLexeme();
	const Token& token = l.getToken();

	switch (token._type) {

	case TokenType::bases: {
	} break;

	case TokenType::suffixes: {
		if (text == "ane")
			finalizeAcyclic();
	} break;

	default:
		break;
	}
}

void BuildFragment::processMultiplier(const Lexeme& l) {
	const string& text = l.getLexeme();
	const Token& token = l.getToken();

	char* end;
	switch (token._type) {

	case TokenType::basic: {
		int number = std::strtol(token._value.c_str(), &end, 10);
		_multipliers.push({ number, token._type });
		_hasMultiplier = true;
	} break;

	case TokenType::factor: {
		int number = std::strtol(token._value.c_str(), &end, 10);

		if (!_multipliers.empty()) {
			Multiplier prev = _multipliers.top();
			if (prev.second != TokenType::basic)
				throw Error("Inconsistent token sequence");
			else {
				number *= prev.first;
				_multipliers.pop();
				_multipliers.push({ number, TokenType::basic });
				_hasMultiplier = true;
			}
		} else {
			_multipliers.push({ number, TokenType::basic });
			_hasMultiplier = true;
		}
	} break;

	default:
		break;
	}
}

void BuildFragment::processSeparator(const Lexeme& l) {
	const string& text = l.getLexeme();
	const Token& token = l.getToken();

	switch (token._type) {
	case TokenType::bracket:
	case TokenType::locant:
	case TokenType::prime:
	case TokenType::punctuation:
		break;

	default:
		break;
	}
}

void BuildFragment::combineMultipliers() {
	while (!_multipliers.empty()) {
		_multiplier += _multipliers.top().first;
		_multipliers.pop();
	}
}

void BuildFragment::finalizeAcyclic() {
	if (_hasMultiplier)
		combineMultipliers();

	for (int i = 0; i < _multiplier; i++)
		_fragment.addAtom(6);

	for (int i = 0; i < _multiplier - 1; i++)
		_fragment.addBond(i, i + 1, 1);
}

/* Main method for convertion from a chemical name into a Molecule object
 * A given name undergoes several transformations
 *		phase 1: lexical analysis
 *		phase 2: tokenization
 *		phase 3: grammatical rules check
 *		phase 4: construction of a Moleclule object from parsed fragments
 * No param check - did that on caller side
 */
void MoleculeNameParser::parseMolecule(const char *name, Molecule &mol) {
	TokenizationResult result = _tokenizer.tokenize(name);
	const Parse& parse = result.getParse();
	if (parse.hasFailures()) {
		const Failures& failures = parse.getFailures();
		string result;
		for (const string& f : failures)
			result += f + " ";
		throw Error("Cannot parse input %s due to errors: %s", name, result);
	}

	ResultBuilder builder(mol);
	if (!builder.build(parse)) {
		mol.clear();
		throw Error("Unable to parse name: %s", name);
	}
}

void AuxParseTools::checkBrackets(const string& s) {
	int level = 0;
	for (char ch : s) {
		if (ch == '(' || ch == '[' || ch == '{') {
			level++;
		}
		else if (ch == ')' || ch == ']' || ch == '}') {
			level--;
		}
	}

	if (level > 0) {
		throw Error("Wrong number of opening brackets: %d", level);
	}
	else if (level < 0) {
		throw Error("Wrong number of closing brackets: %d", -level);
	}
}

int AuxParseTools::splitMultipliers(const string& s) {
	return -1;
}

_SessionLocalContainer<MoleculeNameParser> name_parser_self;
_SessionLocalContainer<DictionaryManager> dictionary_manager_self;

MoleculeNameParser& name_parsing::getMoleculeNameParserInstance() {
	return name_parser_self.getLocalCopy();
}

DictionaryManager& name_parsing::getDictionaryManagerInstance() {
	return dictionary_manager_self.getLocalCopy();
}
