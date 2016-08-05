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

// Converts a given token type name into enum value, 'unknown' if no name found
TokenType Lexeme::tokenTypeFromString(const std::string& s) {
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
		string name = tokenTable->Attribute("name");
		const bool isSeparator = (name == "separator");

		string type = tokenTable->Attribute("type");
		TokenType token = Lexeme::tokenTypeFromString(type);

		TiXmlElement* e = tokenTable->FirstChild("token")->ToElement();
		for (; e; e = e->NextSiblingElement()) {
			string symbol = e->GetText();
			
			//string value = e->Attribute("value");

			// Symbols might have a separator, in which case we need to add
			// several symbols with the same token type into the dictionary
			size_t pos = symbol.find('|');
			if (pos != symbol.npos) {
				size_t i = 0;
				string alias;
				while (pos < symbol.length()) {
					alias = symbol.substr(i, pos);
					addLexeme(alias, token, useTrie);
					i += ++pos;
					pos = symbol.find('|', pos);
				}
				alias = symbol.substr(i, pos);
				addLexeme(alias, token, useTrie);
			} else {
				addLexeme(symbol, token, useTrie);

				// all separators are 1-byte ASCII
				if (isSeparator)
					_separators.push_back(symbol[0]);
			}
		}
	}
}

void DictionaryManager::addLexeme(const string& lexeme, TokenType token, bool useTrie) {
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
				_lexems.push_back(Lexeme(ch, it->second));
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
}

void Parse::processTextFragment(const string& fragment) {
	DictionaryManager& dm = getDictionaryManagerInstance();
	const LexemsTrie& root = dm.getLexemsTrie();

	const int fLength = fragment.length();

	int total = 0;
	string buffer = fragment;

	while (total <= fLength) {
		int current = 0;

		const Trie<TokenType>* match = root.getNode({ buffer[0] });
		if (!match) {
			_failures.push_back(buffer);
			_hasFailures = true;
			return;
		}

		while (match && !match->isMark()) {
			match = match->getNode({ buffer[++current] });
			total++;
		}

		string lexeme = buffer.substr(0, current + 1);

		if (!match) {
			_failures.push_back(lexeme);
			_hasFailures = false;
			return;
		}

		TokenType token = match->getData();
		_lexems.push_back(Lexeme(lexeme, token));

		buffer = buffer.substr(current + 1);
	}
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

bool ResultBuilder::build(const Parse& parse) {
	_mol->clear();

	const Lexems& lexems = parse.getLexemes();
	for (const Lexeme& l : lexems) {
		processLexeme(l);
	}

	return false;
}

void ResultBuilder::processLexeme(const Lexeme& l) {
	const string lexeme = l.getLexeme();
	const TokenType type = l.getToken();
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
