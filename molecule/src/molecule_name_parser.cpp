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

IMPL_ERROR(MoleculeNameParser, "NameToStructure parser");
IMPL_ERROR(AuxParseTools, "Parse tools");
IMPL_ERROR(Tokenizer, "Tokenizer");
IMPL_ERROR(TokenizationResult, "TokenizationResult");
IMPL_ERROR(DictionaryManager, "TableManager");

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

	readTable(basic_elements_table);
	readTable(multipliers_table);
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

void DictionaryManager::readTable(const char* table) {
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

		TiXmlElement* e = tokenTable->FirstChild("token")->ToElement();
		for (; e; e = e->NextSiblingElement()) {
			string symbol = e->GetText();
			
			//string value = e->Attribute("value");

			// Symbols might have a separator, in which case we need to add
			// several symbols with the same token type into the dictionary
			size_t pos = symbol.find('|');
			if (pos != symbol.npos) {
				int i = 0;
				string alias;
				while (pos < symbol.length()) {
					alias = symbol.substr(i, pos);
					_dictionary[alias] = { alias, Token::tokenTypeFromString(type) };
					i += ++pos;
					pos = symbol.find('|', pos);
				}
				alias = symbol.substr(i, pos);
				_dictionary[alias] = { alias, Token::tokenTypeFromString(type) };
			} else {
				_dictionary[symbol] = { name, Token::tokenTypeFromString(type) };

				// all separators are 1-byte ASCII
				if (isSeparator) _separators.push_back(symbol[0]);
			}
		}
	}
}

void Parse::scan() {
	DictionaryManager& dm = getTableManagerInstance();
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
				addLexeme({ch}, it->second);
			continue;
		}

		size_t next = _input.find_first_of(separators, i);
		if (next == _input.npos) {
			string fragment = _input.substr(i, length - i);
			addLexeme(fragment, { fragment, TokenType::text });
			break;
		}
		else {
			string fragment = _input.substr(i, next - i);
			addLexeme(fragment, { fragment, TokenType::text });
			i = next - 1;
			continue;
		}
	}
}

void Parse::addLexeme(const string& s, Token t) {
	_lexems.push_back({s, t});
	_tokens.push_back(t);

	// number of lexems and tokens must match
	assert(_lexems.size() == _tokens.size());
}

TokenizationResult Tokenizer::tokenize(const char* name) {
	string input(name);
	transform(input.begin(), input.end(), input.begin(), tolower);

	AuxParseTools::checkBrackets(input);
	TokenizationResult result(input);
	result.parse();
	if (!result.is_completely_parsed()) {
		// check parse error severity
	}

	return result;
}

void TokenizationResult::parse() {
	_parse.scan();
	const Tokens& tokens = _parse.getTokens();
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
	mol.clear();
	TokenizationResult result = _tokenizer.tokenize(name);
}

void AuxParseTools::checkBrackets(const string& input) {
	int level = 0;
	for (int i = 0; i < input.length(); i++) {
		auto ch = input.at(i);
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

_SessionLocalContainer<MoleculeNameParser> name_parser_self;
_SessionLocalContainer<DictionaryManager> table_manager_self;

MoleculeNameParser& name_parsing::getMoleculeNameParserInstance() {
	return name_parser_self.getLocalCopy();
}

DictionaryManager& name_parsing::getTableManagerInstance() {
	return table_manager_self.getLocalCopy();
}
