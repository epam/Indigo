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
#include <cctype>
#include <cstdlib>
#include <iostream>

#include "tinyxml.h"

#include "base_cpp/scanner.h"
#include "molecule/smiles_loader.h"
#include "molecule/molecule_name_parser.h"

#include "molecule/alkanes.inc"
#include "molecule/basic_elements.inc"
#include "molecule/elements.h"
#include "molecule/multipliers.inc"
#include "molecule/separators.inc"
#include "molecule/token_types.inc"

using namespace std;
using namespace indigo;
using namespace name_parsing;

IMPL_ERROR(MoleculeNameParser, "name_parsing::MoleculeNameParser");
IMPL_ERROR(AuxParseTools, "name_parsing::AuxParseTools");
IMPL_ERROR(DictionaryManager, "name_parsing::TableManager");
IMPL_ERROR(Parse, "name_parsing::Parse");
IMPL_ERROR(TreeBuilder, "name_parsing::TreeBuilder");

// Converts a given token type name into enum value, 'unknown' if no name found
TokenType Token::tokenTypeFromString(const std::string& s) {
	const auto& begin = std::begin(TokenTypeStrings);
	const auto& end = std::end(TokenTypeStrings);
	const auto& it = find(begin, end, s);
	if (it != end) {
		return static_cast<TokenType>(std::distance(begin, it));
	}
	
	return TokenType::unknown;
}

DictionaryManager::DictionaryManager() {
	dictionary.clear();
	separators.clear();

	readTokenTypeStrings();

	readTable(alkanes_table, true);
	readTable(multipliers_table, true);
	readTable(separators_table);

	readBasicElementsTable();
}

void DictionaryManager::readBasicElementsTable() {
	TiXmlDocument doc;

	doc.Parse(basic_elements_table);
	if (doc.Error()) {
		throw Error("Cannot parse table %s", basic_elements_table);
	}

	TiXmlHandle hdoc(&doc);
	TiXmlHandle tokenTables = hdoc.FirstChild("tokenTables");
	TiXmlElement* tokenTable = tokenTables.FirstChild("tokenTable").ToElement();
	for (; tokenTable; tokenTable = tokenTable->NextSiblingElement()) {
		const char* name = tokenTable->Attribute("name");
		const char* type = tokenTable->Attribute("type");
		if (!name || !type) {
			throw Error("Cannot parse table");
		}

		TokenType tt = Token::tokenTypeFromString(type);

		TiXmlElement* e = tokenTable->FirstChild("token")->ToElement();
		for (; e; e = e->NextSiblingElement()) {
			const char* lexeme = e->GetText();
			const char* number = e->Attribute("number");
			const char* symbol = e->Attribute("symbol");
			if (!lexeme || !number || !symbol) {
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
			while (fragment) {
				addLexeme(fragment, Token(name, value, tt), true);
				fragment = ::strtok(nullptr, delim);
			}
		}
	}
}

void DictionaryManager::readTokenTypeStrings() {
	TiXmlDocument doc;

	doc.Parse(token_types_table);
	if (doc.Error()) {
		throw Error("Cannot parse the token types table");
	}

	TiXmlHandle hdoc(&doc);
	TiXmlHandle tokenTypes = hdoc.FirstChild("tokenTypes");
	TiXmlElement* e = tokenTypes.FirstChild("tokenType").ToElement();
	for (; e; e = e->NextSiblingElement()) {
		TokenTypeStrings.push_back(e->GetText());
	}
}

void DictionaryManager::readTable(const char* table, bool useTrie /* = false*/) {
	TiXmlDocument doc;

	doc.Parse(table);
	if (doc.Error()) {
		throw Error("Cannot parse table %s", table);
	}

	TiXmlHandle hdoc(&doc);
	TiXmlHandle tokenTables = hdoc.FirstChild("tokenTables");
	TiXmlElement* tokenTable = tokenTables.FirstChild("tokenTable").ToElement();
	for (; tokenTable; tokenTable = tokenTable->NextSiblingElement()) {
		const char* name = tokenTable->Attribute("name");
		const char* type = tokenTable->Attribute("type");
		if (!name || !type) {
			throw Error("Cannot parse table");
		}

		const bool isSeparator = (::strcmp(name, "separator") == 0);
		TokenType tt = Token::tokenTypeFromString(type);

		TiXmlElement* e = tokenTable->FirstChild("token")->ToElement();
		for (; e; e = e->NextSiblingElement()) {
			const char* lexeme = e->GetText();
			const char* value = e->Attribute("value");
			if (!lexeme || !value) {
				throw Error("Cannot parse table %s", name);
			}
			
			// Symbols might have a separator '|', in which case we need to add
			// several symbols with the same token type into the dictionary
			char delim[] = "|";
			char* fragment = ::strtok(const_cast<char*>(lexeme), delim);
			while (fragment) {
				addLexeme(fragment, Token(name, value, tt), useTrie);
				fragment = ::strtok(nullptr, delim);
			}
			// all separators are 1-byte ASCII
			if (isSeparator) {
				separators.push_back(lexeme[0]);
			}
		}
	}
}

void DictionaryManager::addLexeme(const string& lexeme, const Token& token, bool useTrie) {
	dictionary[lexeme] = token;
	if (useTrie) {
		lexemesTrie.addWord(lexeme, token);
	}
}

void Parse::scan() {
	DictionaryManager& dm = getDictionaryManagerInstance();
	const SymbolDictionary& dictionary = dm.getDictionary();
	const string& separators = dm.getSeparators();

	const size_t length = input.length();

	/* If a symbol is a separator, convert it into a lexeme
	 * If not, scan until either a next separator or an end of the string is reached,
	 * then add a lexeme for text fragment
	 *
	 * By this time we're already know that brackets match
	 */
	for (size_t i = 0; i < length; i++) {
		char ch = input.at(i);

		// whitespace is a special case
		if (ch == ' ') {
			Token token;
			token.name = "separator";
			token.type = TokenType::punctuation;
			token.value = ch;
			lexemes.push_back(Lexeme(ch, token));
		}

		size_t pos = separators.find(ch);
		if (pos != separators.npos) {
			const auto& it = dictionary.find({ ch });
			if (it != dictionary.end()) {
				lexemes.push_back(Lexeme(ch, it->second));
			}
			continue;
		}

		size_t next = input.find_first_of(separators, i);
		if (next == input.npos) {
			string fragment = input.substr(i, length - i);
			processTextFragment(fragment);
			break;
		} else {
			string fragment = input.substr(i, next - i);
			processTextFragment(fragment);
			i = next - 1;
			continue;
		}
	}

	Token terminator;
	terminator.type = TokenType::endOfStream;
	lexemes.push_back(Lexeme("", terminator));
}

// Retrieves a next lexeme from the stream, incrementing the stream pointer
const Lexeme& Parse::getNextLexeme() const {
	if (currentLexeme < lexemes.size()) {
		return lexemes[currentLexeme++];
	}

	throw Error("Lexemes stream pointer overflow");
}

// Returns true if next lexeme's token type equals to input
bool Parse::peekNextToken(TokenType peek) const {
	const Lexeme& lexeme = lexemes[currentLexeme];
	return (lexeme.getToken().type == peek);
}

/*
Splits a fragment into smaller lexemes
Sets up the failure flag if unparsable fragment is encountered
*/
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
			failures.push_back(buffer);
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
				// decrement counters as we step back 1 char
				total--;
				buffer = buffer.substr(--current);
				continue;
			}

			failures.push_back(lexeme);
			_hasFailures = true;
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
bool Parse::tryElision(const string& failure) {
	DictionaryManager& dm = getDictionaryManagerInstance();
	const LexemesTrie& root = dm.getLexemesTrie();

	string endings = "aoey";
	string tryout = failure;
	for (char ch : endings) {
		tryout.replace(tryout.length() - 1, 1, { ch });
		if (!root.isWord(tryout)) {
			tryout = failure;
			tryout.insert(0, 1, { ch });
			if (!root.isWord(tryout)) {
				return false;
			}
		}
		processTextFragment(tryout);
		_hasElision = true;
		return true;
	}

	return false;
}

FragmentNode::~FragmentNode() {
	for (FragmentNode* node : nodes) {
		delete node;
	}
}

// Inserts a new node before anchor position, returns status
bool FragmentNode::insertBefore(FragmentNode* node, const FragmentNode* anchor) {
	bool result = false;

	node->setParent(this);
	const auto& position = std::find(nodes.begin(), nodes.end(), anchor);
	if (position != nodes.end()) {
		nodes.insert(position, node);
		result = true;
	}

	return result;
}

void FragmentNode::insert(FragmentNode* node) {
	node->setParent(this);
	nodes.push_back(node);
}

#ifdef DEBUG
void FragmentNode::print(ostream& out) const {
	out << "Parent: " << parent << endl;

	if (type == FragmentNodeType::unknown) {
		out << "Type: unknown" << endl;
	}
}

void FragmentNodeRoot::print(ostream& out) const {
	out << "Type: FragmentNodeRoot" << endl;
	FragmentNode::print(out);
}

void FragmentNodeBase::print(ostream& out) const {
	out << "Type: FragmentNodeBase" << endl;

	out << "Multipliers:" << endl;
	const auto& mul_container = multipliers._Get_container();
	std::for_each(mul_container.begin(), mul_container.end(), [&out](const Multiplier& multiplier) {
		out << "\tvalue: " << multiplier.first << endl;
	});

	out << "Locants:" << endl;
	for (int locant : locants) {
		out << "\tvalue: " << locant << endl;
	}

	out << "Element: " << endl;
	out << "\tnumber: " << element.first << endl;
	out << "\tsymbol: " << element.second << endl;
	FragmentNode::print(out);
}

void FragmentNodeSubstituent::print(ostream& out) const {
	out << "Type: FragmentNodeSubstituent" << endl;
	out << "Positions:" << endl;
	for (int pos : positions) {
		out << "\tvalue: " << pos << endl;
	}
	out << "Fragment multiplier: " << fragmentMultiplier << endl;
	FragmentNodeBase::print(out);
}

ostream& operator<<(ostream& out, const FragmentNode& node) {
	node.print(out);
	return out;
}

ostream& operator<<(ostream& out, const FragmentNode* node) {
	node->print(out);
	return out;
}
#endif // DEBUG

FragmentNodeBase::FragmentNodeBase() {
	type = FragmentNodeType::base;
	element.first = indigo::ELEM_MIN;
}

int FragmentNodeBase::combineMultipliers() {
	int result = 0;
	while (!multipliers.empty()) {
		const Multiplier& mul = multipliers.top();
		result += mul.first;
		multipliers.pop();
	}

	assert(multipliers.empty());
	return result;
}

FragmentBuildTree::FragmentBuildTree() {
	addRoot();
}

FragmentBuildTree::~FragmentBuildTree() {
	for (FragmentNode* root : roots) {
		delete root;
	}
}

void FragmentBuildTree::addRoot() {
	FragmentNode* root = new FragmentNodeRoot;
	currentRoot = root;
	roots.push_back(root);
}

bool TreeBuilder::processParse() {
	initBuildTree();
	return processParseImpl();
}

void TreeBuilder::initBuildTree() {
	FragmentNodeBase* node = new FragmentNodeBase;
	FragmentNode* root = buildTree->getCurrentRoot();
	root->insert(node);
	current = node;
}

/*
Returns one level up in the tree, setting current node to the new
level's base fragment
Returns false if operation cannot be performed
*/
bool TreeBuilder::upOneLevel() {
	if (parse->peekNextToken(TokenType::endOfStream)) {
		return true;
	}

	if (current->checkType(FragmentNodeType::base)) {
		if (!current->getParent()) {
			return false;
		}
	}

	startNewNode = true;
	current = getParentBase();
	return (current != nullptr);
}

/*
The implementation of parse processing
Recursively calls itself until EndOfStream is reached or error occured
*/
bool TreeBuilder::processParseImpl() {
	const Lexeme& lexeme = parse->getNextLexeme();
	const Token& token = lexeme.getToken();

	TokenType tt = token.type;
	const string& tname = token.name;
	
	if (tt == TokenType::endOfStream) {
		return true;
	}

	if ((tt == TokenType::unknown) || (tt == TokenType::text)) {
		return false;
	}

	if (tname == "alkanes") {
		if (!processAlkane(lexeme)) {
			return false;
		}
	} else if (tname == "multiplier") {
		if (!processMultiplier(lexeme)) {
			return false;
		}
	} else if (tname == "separator") {
		if (!processSeparator(lexeme)) {
			return false;
		}
	} else if (tname == "basicElement") {
		if (!processBasicElement(lexeme)) {
			return false;
		}
	}

	return processParseImpl();
}

bool TreeBuilder::processBasicElement(const Lexeme& lexeme) {
	// FIXME
	// currently inserting basic element into base node
	assert(current->checkType(FragmentNodeType::base));

	const Token& token = lexeme.getToken();
	const string& value = token.value;
	const size_t pos = value.find('_');
	if (pos == string::npos) {
		return false;
	}

	const string number = value.substr(0, pos);
	const string symbol = value.substr(pos + 1);
	const int element = AuxParseTools::strToInt(number);

	FragmentNodeBase* base = static_cast<FragmentNodeBase*>(current);
	base->setElementNumber(element);
	base->setElementSymbol(symbol);

	return true;
}

void TreeBuilder::processSuffix(const Lexeme& lexeme) {
	const string& text = lexeme.getLexeme();
	const Token& token = lexeme.getToken();

	FragmentNodeBase* node = static_cast<FragmentNodeBase*>(current);
	node->setElementNumber(ELEM_C);

	if (text == "ane") {
		node->setBondOrder(BOND_SINGLE);
		node->setValencyDiff(0);
		node->setFreeAtomOrder(0);
	}
	else if (text == "yl") {
		node->setBondOrder(BOND_SINGLE);
		node->setValencyDiff(1);
		node->setFreeAtomOrder(1);
	}
}

// Processes alkane lexemes
bool TreeBuilder::processAlkane(const Lexeme& lexeme) {
	const string& text = lexeme.getLexeme();
	const Token& token = lexeme.getToken();

	switch (token.type) {
	case TokenType::bases: {
		int value = AuxParseTools::strToInt(token.value);
		Multipliers& multipliers = static_cast<FragmentNodeBase*>(current)->getMultipliers();
		multipliers.push({ value, TokenType::basic });
	} break;

	case TokenType::suffixes: {
		processSuffix(lexeme);

		if (parse->peekNextToken(TokenType::closingBracket)) {
			break;
		}

		// FIXME: move nodes
		if (current->checkType(FragmentNodeType::substituent)) {
			current = getCurrentBase();
			if (!current) {
				return false;
			}
		} else if (current->checkType(FragmentNodeType::base)) {
			if (!upOneLevel()) {
			}
		}
	} break;

	default:
		break;
	}

	return true;
}

// Processes multiplier lexemes
bool TreeBuilder::processMultiplier(const Lexeme& lexeme) {
	const Token& token = lexeme.getToken();

	switch (token.type) {
	case TokenType::basic: {
		int value = AuxParseTools::strToInt(token.value);

		if (current->checkType(FragmentNodeType::substituent)) {
			FragmentNodeSubstituent* node = static_cast<FragmentNodeSubstituent*>(current);
			if (node->getExpectFragMultiplier()) {
				node->setFragmentMultiplier(value);
				bool flag = parse->peekNextToken(TokenType::factor);
				node->setExpectFragMultiplier(flag);
				break;
			}
		}

		Multipliers& multipliers = static_cast<FragmentNodeBase*>(current)->getMultipliers();
		multipliers.push({ value, token.type });
	} break;

	case TokenType::factor: {
		int value = AuxParseTools::strToInt(token.value);

		if (current->checkType(FragmentNodeType::substituent)) {
			FragmentNodeSubstituent* node = static_cast<FragmentNodeSubstituent*>(current);
			if (node->getExpectFragMultiplier()) {
				int fragMultiplier = node->getFragmentMultiplier();
				if (fragMultiplier != 1) {
					fragMultiplier *= value;
				}
				node->setFragmentMultiplier(fragMultiplier);
				node->setExpectFragMultiplier(false);
				break;
			}
		}

		Multipliers& multipliers = static_cast<FragmentNodeBase*>(current)->getMultipliers();
		if (multipliers.empty()) {
			multipliers.push({ value, TokenType::basic });
		} else {
			const Multiplier& prev = multipliers.top();
			int value = AuxParseTools::strToInt(token.value);
			value *= prev.first;
			multipliers.pop();
			multipliers.push({ value, TokenType::basic });
		}
	} break;

	default:
		break;
	}

	return true;
}

// Processes separator lexemes
bool TreeBuilder::processSeparator(const Lexeme& lexeme) {
	const string& text = lexeme.getLexeme();
	const Token& token = lexeme.getToken();

	switch (token.type) {
	case TokenType::openingBracket: {
		// empty brackets aren't allowed
		if (parse->peekNextToken(TokenType::closingBracket)) {
			return false;
		}

		FragmentNodeBase* base = new FragmentNodeBase;
		current->insert(base);
		current = base;
		startNewNode = true;
	} break;

	case TokenType::closingBracket:
		if (!upOneLevel()) {
			return false;
		}
		break;

	case TokenType::locant: {
		if (startNewNode) {
			FragmentNode* parent = current->getParent();
			FragmentNodeSubstituent* subst = new FragmentNodeSubstituent;
			if (!parent->insertBefore(subst, getCurrentBase())) {
				return false;
			}
			current = subst;
			startNewNode = false;
		}
		int value = AuxParseTools::strToInt(token.value);
		Positions& positions = static_cast<FragmentNodeSubstituent*>(current)->getPositions();
		positions.push_back(value);
		FragmentNodeBase* base = getCurrentBase();
		Locants& locants = base->getLocants();
		locants.push_back(value);
	} break;

	// currently no-op
	case TokenType::prime:
		break;

	case TokenType::punctuation: {
		if (text == ",") {
			if (!current->checkType(FragmentNodeType::substituent)) {
				return false;
			}
			static_cast<FragmentNodeSubstituent*>(current)->setExpectFragMultiplier(true);
			break;
		}

		/*
		Whitespace is a special case when we actually add a new build tree
		for a new structure
		*/
		// FIXME - process acids (acid names have whitespace)
		if (text == " ") {
			buildTree->addRoot();
			initBuildTree();
			break;
		}
	} break;

	default:
		break;
	}

	return true;
}

// Retrieves current level's base; each level has only one base
FragmentNodeBase* TreeBuilder::getCurrentBase() {
	if (current->checkType(FragmentNodeType::base)) {
		return static_cast<FragmentNodeBase*>(current);
	}

	FragmentNode* parent = current->getParent();
	if (!parent) {
		return nullptr;
	}

	// Base is always the rightmost node
	const Nodes& nodes = parent->getNodes();
	return static_cast<FragmentNodeBase*>(nodes.back());
}

// Retrieves upper level's base, if any; each level has only one base
FragmentNodeBase* TreeBuilder::getParentBase() {
	const FragmentNode* parent = current->getParent();
	if (!parent) {
		return nullptr;
	}

	const FragmentNode* grand = parent->getParent();
	if (!grand) {
		return nullptr;
	}

	// Base is always the rightmost node
	const Nodes& nodes = grand->getNodes();
	return static_cast<FragmentNodeBase*>(nodes.back());
}

ResultBuilder::ResultBuilder(const Parse& input) {
	treeBuilder.reset(new TreeBuilder(input));
	initOrganicElements();
}

void ResultBuilder::initOrganicElements() {
	organicElements[ELEM_B] = "B";
	organicElements[ELEM_C] = "C";
	organicElements[ELEM_N] = "N";
	organicElements[ELEM_O] = "O";
	organicElements[ELEM_P] = "P";
	organicElements[ELEM_S] = "S";
	organicElements[ELEM_F] = "F";
	organicElements[ELEM_Cl] = "Cl";
	organicElements[ELEM_Br] = "Br";
}

/*
Traverses the build tree in post-order depth-first order, creates
SMILES representation and loads the SMILES into the resulting Molecule
*/
bool ResultBuilder::buildResult(Molecule& molecule) {
	molecule.clear();

	const auto& buildTree = treeBuilder->getBuildTree();
	const Nodes& roots = buildTree->getRoots();
	if (roots.empty()) {
		return false;
	}

	// most time we'll have a sigle root
	for (FragmentNode* root : roots) {
		const Nodes& nodes = root->getNodes();
		for (FragmentNode* node : nodes) {
			processNode(node);
		}
		combine(root);
	}

	SMILES += std::move(fragments.top());
	fragments.pop();
	while (!fragments.empty()) {
		SMILES += ".";
		SMILES += std::move(fragments.top());
		fragments.pop();
	}

	BufferScanner scanner(SMILES.c_str());
	SmilesLoader loader(scanner);
	loader.loadMolecule(molecule);

	SMILES.clear();
	auto containter = fragments._Get_container();
	containter.clear();

	return true;
}

void ResultBuilder::processNode(FragmentNode* node) {
	const Nodes& nodes = node->getNodes();
	for (FragmentNode* node : nodes) {
		processNode(node);
	}

	FragmentNodeType type = node->getType();
	if (type == FragmentNodeType::base) {
		processBaseNode(static_cast<FragmentNodeBase*>(node));
	} else if (type == FragmentNodeType::substituent) {
		processSubstNode(static_cast<FragmentNodeSubstituent*>(node));
	}
}

void ResultBuilder::processBaseNode(FragmentNodeBase* base) {
	const int multipliers = base->combineMultipliers();
	const name_parsing::Element& element = base->getElement();
	const int number = element.first;
	const string& symbol = element.second;

	bool organicElement = (organicElements.find(number) != organicElements.end());

	string fragment;
	fragment += organicElement ? organicElements[number] : symbol;

	if (multipliers > 1) {
		for (int i = 1; i < multipliers; i++) {
			fragment += organicElement ? organicElements[number] : symbol;
		}
	}

	// locant positions are marked with pipe | for easier search and substitution
	const char separator{ '|' };
	const Locants& locants = base->getLocants();
	if (!locants.empty()) {
		int inserted{ 0 };
		for (int loc : locants) {
			fragment.insert(loc + inserted, 1, separator);
			inserted++;
		}
	}

	fragments.push(fragment);
}

void ResultBuilder::processSubstNode(FragmentNodeSubstituent* subst) {
	const Nodes& nodes = subst->getNodes();
	if (nodes.empty()) {
		processBaseNode(subst);
	} else {
		combine(subst);
	}
}

void ResultBuilder::combine(FragmentNode* node) {
	string base = std::move(fragments.top());
	fragments.pop();

	const Nodes& nodes = node->getNodes();
	auto it = nodes.rbegin();
	++it;

	for (; it != nodes.rend(); it++) {
		string fragment = "(";
		fragment += std::move(fragments.top());
		fragment += ")";
		fragments.pop();

		FragmentNodeSubstituent* subst = static_cast<FragmentNodeSubstituent*>(*it);
		const int fragMul = subst->getFragmentMultiplier();
		for (int i = 0; i < fragMul; i++) {
			size_t pos = base.find_last_of('|');
			base.replace(pos, 1, fragment);
		}
	}

	fragments.push(base);
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
void MoleculeNameParser::parseMolecule(const char *name, Molecule &molecule) {
	string input(name);
	std::transform(input.begin(), input.end(), input.begin(), [](unsigned long c){ return std::tolower(c); });

	AuxParseTools::checkBrackets(input);
	Parse parse(input);
	parse.scan();

	if (parse.hasFailures()) {
		const Failures& failures = parse.getFailures();
		string message;
		for (const string& f : failures) {
			message += f + " ";
		}
		throw Error("Cannot parse input %s due to errors: %s", name, message);
	}

	ResultBuilder builder(parse);
	if (!builder.buildTree()) {
		molecule.clear();
		throw Error("Cannot construct the build tree for name %s", name);
	}

	if (!builder.buildResult(molecule)) {
		molecule.clear();
		throw Error("Cannot build a resulting structure for name %s", name);
	}
}

/*
Checks if allowed opening and closing brackets match
Doesn't check brackets type matching (e.g. ((]] is valid)
*/
void AuxParseTools::checkBrackets(const string& s) {
	int level = 0;
	for (char ch : s) {
		if (ch == '(' || ch == '[' || ch == '{') {
			level++;
			continue;
		}

		if (ch == ')' || ch == ']' || ch == '}') {
			level--;
			continue;
		}
	}

	if (level != 0) {
		throw Error("Opening and closing brackets don't match: %d", level);
	}
}

// Converts std::string to int
int AuxParseTools::strToInt(const string& str) {
	char* ch = nullptr;
	return std::strtol(str.c_str(), &ch, 10);
}

_SessionLocalContainer<MoleculeNameParser> name_parser_self;
_SessionLocalContainer<DictionaryManager> dictionary_manager_self;

MoleculeNameParser& name_parsing::getMoleculeNameParserInstance() {
	return name_parser_self.getLocalCopy();
}

DictionaryManager& name_parsing::getDictionaryManagerInstance() {
	return dictionary_manager_self.getLocalCopy();
}
