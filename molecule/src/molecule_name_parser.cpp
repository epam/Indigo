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
#include "molecule/flags.inc"
#include "molecule/multipliers.inc"
#include "molecule/separators.inc"
#include "molecule/token_types.inc"

using namespace std;
using namespace indigo;

IMPL_ERROR(MoleculeNameParser, "indigo::MoleculeNameParser");
IMPL_ERROR(MoleculeNameParser::DictionaryManager, "MoleculeNameParser::DictionaryManager");
IMPL_ERROR(MoleculeNameParser::Parse, "MoleculeNameParser::Parse");
IMPL_ERROR(MoleculeNameParser::TreeBuilder, "MoleculeNameParser::TreeBuilder");
IMPL_ERROR(MoleculeNameParser::ResultBuilder, "MoleculeNameParser::ResultBuilder");

MoleculeNameParser::DictionaryManager::DictionaryManager() {
   _readTokenTypeStrings();

   _readTable(alkanes_table, true);
   _readTable(multipliers_table, true);
   _readTable(separators_table);
   _readTable(flags_table, true);

   _readBasicElementsTable();
}

void MoleculeNameParser::DictionaryManager::_readBasicElementsTable() {
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

      TokenType tt = _tokenTypeFromString(type);

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
            _addLexeme(fragment, { name, value, tt }, true);
            fragment = ::strtok(nullptr, delim);
         }
      }
   }
}

void MoleculeNameParser::DictionaryManager::_readTokenTypeStrings() {
   TiXmlDocument doc;

   doc.Parse(token_types_table);
   if (doc.Error()) {
      throw Error("Cannot parse the token types table");
   }

   TiXmlHandle hdoc(&doc);
   TiXmlHandle tokenTypes = hdoc.FirstChild("tokenTypes");
   TiXmlElement* e = tokenTypes.FirstChild("tokenType").ToElement();
   for (; e; e = e->NextSiblingElement()) {
      _tokenTypeStrings.push_back(e->GetText());
   }
}

void MoleculeNameParser::DictionaryManager::_readTable(const char* table, bool useTrie /* = false*/) {
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
      TokenType tt = _tokenTypeFromString(type);

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
            _addLexeme(fragment, Token(name, value, tt), useTrie);
            fragment = ::strtok(nullptr, delim);
         }
         // all separators are 1-byte ASCII
         if (isSeparator) {
            separators.push_back(lexeme[0]);
         }
      }
   }
}

void MoleculeNameParser::DictionaryManager::_addLexeme(const string& lexeme, const Token& token, bool useTrie) {
   dictionary[lexeme] = token;
   if (useTrie) {
      lexemesTrie.addWord(lexeme, token);
   }
}

/*
Converts a token type name into token type value
Returns TokenType::unknow if no matching found
*/
MoleculeNameParser::TokenType MoleculeNameParser::DictionaryManager::_tokenTypeFromString(const string& s) {
   const auto& begin = std::begin(_tokenTypeStrings);
   const auto& end = std::end(_tokenTypeStrings);
   const auto& it = std::find(begin, end, s);
   if (it != end) {
      return static_cast<TokenType>(std::distance(begin, it));
   }

   return TokenType::UNKNOWN;
}

/*
Performs by-symbol input scan, determines basic tokens
Text fragments require further processing
*/
void MoleculeNameParser::Parse::scan() {
   const DictionaryManager& dm = getMoleculeNameParserInstance().dictionaryManager;
   const SymbolDictionary& sd = dm.dictionary;
   const string& separators = dm.separators;

   const size_t length = input.length();

   // A buffer for locant symbol(s)
   string locant;

   /*
   If a symbol is a separator, convert it into a lexeme
   If not, scan until either a next separator, flag, or an end of the string
   is reached, then add a lexeme for text fragment
   
   By this time we already know that brackets do match
   */
   for (size_t i = 0; i < length; i++) {
      char ch = input[i];

      // whitespace is a special case
      if (ch == ' ') {
         Token token{ "separator", { ch }, TokenType::PUNCTUATION };
         lexemes.push_back({ ch, token });
      }

      /*
      The input symbol is a separator; add a separator lexeme
      */
      size_t pos = separators.find(ch);
      if (pos != separators.npos) {
         const auto& it = sd.find({ ch });
         if (it != sd.end()) {
            // For locants, we need additional check if the number is multi-digit
            if (std::isdigit(ch)) {
               locant += ch;
               while (std::isdigit(input[i + 1])) {
                  locant += input[i + 1];
                  ++i;
               }
               lexemes.push_back(Lexeme(locant, Token("separator", locant, TokenType::LOCANT)));
               locant.clear();
               continue;
            }
            lexemes.push_back(Lexeme(ch, it->second));
         }
         continue;
      }

      /*
      The current fragment is a text fragment
      Search until a next separator or the end of string, check the dictionary,
      process text fragment
      */
      size_t next = input.find_first_of(separators, i);
      if (next == input.npos) {
         string fragment = input.substr(i, length - i);
         _processTextFragment(fragment);
         break;
      } else {
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
const MoleculeNameParser::Lexeme& MoleculeNameParser::Parse::getNextLexeme() const {
   if (currentLexeme < lexemes.size()) {
      return lexemes[currentLexeme++];
   }

   throw Error("Lexemes stream pointer overflow");
}

// Returns true if next lexeme's token type equals to input
bool MoleculeNameParser::Parse::peekNextToken(TokenType peek) const {
   const Lexeme& lexeme = lexemes[currentLexeme];
   return (lexeme.token.type == peek);
}

/*
Splits a fragment into smaller lexemes
Sets up the failure flag if unparsable fragment is encountered
*/
void MoleculeNameParser::Parse::_processTextFragment(const string& fragment) {
   const DictionaryManager& dm = getMoleculeNameParserInstance().dictionaryManager;
   const LexemesTrie& root = dm.lexemesTrie;

   const size_t fLength = fragment.length();

   // global position inside the input string
   int total = 0;

   string buffer = fragment;

   /*
   Slow track with elision, see tryElision
   */
   while (total < fLength) {
      /*
      Fast track if trie already contains the word
      */
      if (root.isWord(buffer)) {
         const Trie<Token>* wordNode = root.getNode(buffer);
         const Token& token = wordNode->getData();
         lexemes.push_back(Lexeme(buffer, token));
         return;
      }

      // current position inside a buffer
      int current = 0;

      const Trie<Token>* match = root.getNode({ buffer[0] });
      if (!match) {
         failures.push_back(buffer);
         hasFailures = true;
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
         if (_tryElision(lexeme)) {
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
bool MoleculeNameParser::Parse::_tryElision(const string& failure) {
   const DictionaryManager& dm = getMoleculeNameParserInstance().dictionaryManager;
   const LexemesTrie& root = dm.lexemesTrie;

   string endings = "aoey";
   string tryout = failure;
   for (char ch : endings) {
      tryout.replace(tryout.length() - 1, 1, { ch });
      if (!root.isWord(tryout)) {
         tryout = failure;
         tryout.insert(0, 1, { ch });
         if (!root.isWord(tryout)) {
            tryout = failure;
            tryout += ch;
            if (!root.isWord(tryout)) {
               return false;
            }
         }
      }
      _processTextFragment(tryout);
      return true;
   }

   return false;
}

MoleculeNameParser::FragmentNode::~FragmentNode() {
   for (FragmentNode* node : nodes) {
      delete node;
   }
}

// Inserts a new node before anchor position, returns status
bool MoleculeNameParser::FragmentNode::insertBefore(FragmentNode* node, const FragmentNode* anchor) {
   node->parent = this;
   const auto& position = std::find(nodes.begin(), nodes.end(), anchor);
   if (position != nodes.end()) {
      nodes.insert(position, node);
      return true;
   }

   return false;
}

void MoleculeNameParser::FragmentNode::insert(FragmentNode* node) {
   node->parent = this;
   nodes.push_back(node);
}

#ifdef DEBUG
void MoleculeNameParser::FragmentNode::print(ostream& out) const {
   out << "Parent: " << parent << endl;

   if (type == FragmentNodeType::UNKNOWN) {
      out << "Type: UNKNOWN" << endl;
   }
}

void MoleculeNameParser::FragmentNodeRoot::print(ostream& out) const {
   out << "Type: FragmentNodeRoot" << endl;
   FragmentNode::print(out);
}

void MoleculeNameParser::FragmentNodeBase::print(ostream& out) const {
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

void MoleculeNameParser::FragmentNodeSubstituent::print(ostream& out) const {
   out << "Type: FragmentNodeSubstituent" << endl;
   out << "Positions:" << endl;
   for (int pos : positions) {
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

MoleculeNameParser::FragmentNodeBase::FragmentNodeBase() {
   type = FragmentNodeType::BASE;
   element.first = ELEM_MIN;
}

int MoleculeNameParser::FragmentNodeBase::combineMultipliers() {
   int result = 0;
   while (!multipliers.empty()) {
      const Multiplier& mul = multipliers.top();
      result += mul.first;
      multipliers.pop();
   }

   assert(multipliers.empty());
   return result;
}

MoleculeNameParser::FragmentBuildTree::FragmentBuildTree() {
   addRoot();
}

MoleculeNameParser::FragmentBuildTree::~FragmentBuildTree() {
   for (FragmentNode* root : roots) {
      delete root;
   }
}

void MoleculeNameParser::FragmentBuildTree::addRoot() {
   FragmentNode* root = new FragmentNodeRoot;
   currentRoot = root;
   roots.push_back(root);
}

bool MoleculeNameParser::TreeBuilder::processParse() {
   _initBuildTree();
   return _processParse();
}

void MoleculeNameParser::TreeBuilder::_initBuildTree() {
   FragmentNodeBase* node = new FragmentNodeBase;
   FragmentNode* root = buildTree.currentRoot;
   root->insert(node);
   _current = node;
}

/*
Returns one level up in the tree, setting current node to the new
level's base fragment
Returns false if operation cannot be performed
*/
bool MoleculeNameParser::TreeBuilder::_upOneLevel() {
   if (_parse->peekNextToken(TokenType::END_OF_STREAM)) {
      return true;
   }

   if (_current->type == FragmentNodeType::BASE) {
      if (!_current->parent) {
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
bool MoleculeNameParser::TreeBuilder::_processParse() {
   const Lexeme& lexeme = _parse->getNextLexeme();
   
   if (lexeme.token.type == TokenType::END_OF_STREAM) {
      return true;
   }

   if ((lexeme.token.type == TokenType::UNKNOWN) ||
       (lexeme.token.type == TokenType::TEXT)) {
      return false;
   }

   const string& tname = lexeme.token.name;
   if (tname == "alkanes") {
      if (!_processAlkane(lexeme)) {
         return false;
      }
   } else if (tname == "multiplier") {
      if (!_processMultiplier(lexeme)) {
         return false;
      }
   } else if (tname == "separator") {
      if (!_processSeparator(lexeme)) {
         return false;
      }
   } else if (tname == "basicElement") {
      if (!_processBasicElement(lexeme)) {
         return false;
      }
   } else if (tname == "flags") {
      if (!_processFlags(lexeme)) {
         return false;
      }
   }

   return _processParse();
}

bool MoleculeNameParser::TreeBuilder::_processFlags(const Lexeme& lexeme) {
   const string& name = lexeme.lexeme;

   if (name == "cyclo") {
      FragmentNodeBase* base = _getCurrentBase();
      if (base == nullptr) {
         return false;
      }

      if (base->cycle == true) {
         return false;
      }

      base->cycle = true;
      return true;
   }

   if (name == "cis" || name == "trans") {
      if ((_current->type == FragmentNodeType::SUBSTITUENT) || (_current->type == FragmentNodeType::BASE)) {
         FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
         if (name == "cis") {
            base->isomerism = Isomerism::CIS;
         } else {
            base->isomerism = Isomerism::TRANS;
         }
         return true;
      }
   }

   return false;
}

bool MoleculeNameParser::TreeBuilder::_processBasicElement(const Lexeme& lexeme) {
   // FIXME
   // currently inserting basic element into base node
   assert(_current->type == FragmentNodeType::BASE);

   const string& value = lexeme.token.value;
   const size_t pos = value.find('_');
   if (pos == string::npos) {
      return false;
   }

   const string number = value.substr(0, pos);
   const string symbol = value.substr(pos + 1);
   const int element = _strToInt(number);

   FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
   base->element.first = element;
   base->element.second = symbol;
   base->tokenType = NodeType::ELEMENT;
   base->multipliers.push({ 1, TokenType::BASIC});

   return true;
}

/*
Processes suffixes
Suffixes are -ane, -ene, -yne|-yn, -yl
-ane means that all bonds are single, e.g. hexane CCCCCC
-ene means that at least the first bond is double
locants signify multiple atoms with double bonds, e.g. 2,4-hexadiene CC=CC=CC
-yne|-yl means that at least the first bond is triple
locants signify multiple atoms with triple bonds, e.g. 2,4-hexadiyne CC#CC#CC
*/
void MoleculeNameParser::TreeBuilder::_processSuffix(const Lexeme& lexeme) {
   FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
   if (base->tokenType == NodeType::UNKNOWN) {
      base->tokenType = NodeType::SUFFIX;
   }

   base->element.first = ELEM_C;
   base->element.second = "C";

   if (lexeme.lexeme == "ane") {
      base->bondOrder = BOND_SINGLE;
      base->valencyDiff = 0;
      base->freeAtomOrder = 0;
   }
   else if (lexeme.lexeme == "yl") {
      base->bondOrder = BOND_SINGLE;
      base->valencyDiff = 1;
      base->freeAtomOrder = 1;
   } else if (lexeme.lexeme == "ene") {
      base->bondOrder = BOND_DOUBLE;
      base->valencyDiff = 0;
      base->freeAtomOrder = 0;
   } else if (lexeme.lexeme == "yne" || lexeme.lexeme == "yn") {
      base->bondOrder = BOND_TRIPLE;
      base->valencyDiff = 0;
      base->freeAtomOrder = 0;
   }

   if (_current->type == FragmentNodeType::SUBSTITUENT) {
      FragmentNodeBase* currentBase = _getCurrentBase();
      if (!currentBase) {
         throw Error("Can't get current level base node");
      }
      currentBase->element.first = ELEM_C;
      currentBase->element.second = "C";

      _startNewNode = true;
   }
}

bool MoleculeNameParser::TreeBuilder::_processAlkaneBase(const Lexeme& lexeme) {
   FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
   base->tokenType = NodeType::BASE;

   int value = _strToInt(lexeme.token.value);
   base->multipliers.push({ value, TokenType::BASIC });

   return true;
}

bool MoleculeNameParser::TreeBuilder::_processAlkaneSuffix(const Lexeme& lexeme) {
   _processSuffix(lexeme);

   if (_parse->peekNextToken(TokenType::CLOSING_BRACKET)) {
      return true;
   }

   if (_current->type == FragmentNodeType::SUBSTITUENT) {
      _current = _getCurrentBase();
      if (!_current) {
         return false;
      }
   }
   else if (_current->type == FragmentNodeType::BASE) {
      if (!_upOneLevel()) {
         return false;
      }
   }

   return true;
}

// Processes alkane lexemes
bool MoleculeNameParser::TreeBuilder::_processAlkane(const Lexeme& lexeme) {
   bool result = true;

   switch (lexeme.token.type) {
   case TokenType::BASES: {
      result = _processAlkaneBase(lexeme);
   } break;

   case TokenType::SUFFIXES: {
      result = _processAlkaneSuffix(lexeme);
   } break;

   default:
      break;
   }

   return result;
}

bool MoleculeNameParser::TreeBuilder::_processBasicMultiplier(const Lexeme& lexeme) {
   const int value = _strToInt(lexeme.token.value);

   if (_current->type == FragmentNodeType::SUBSTITUENT) {
      FragmentNodeSubstituent* node = dynamic_cast<FragmentNodeSubstituent*>(_current);
      if (node->expectFragMultiplier) {
         if (value != node->positions.size()) {
            throw Error("Locants and fragment multiplier don't match");
         }

         node->fragmentMultiplier = value;
         bool flag = _parse->peekNextToken(TokenType::FACTOR);
         node->expectFragMultiplier = flag;
         return true;
      }
   }

   FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
   base->multipliers.push({ value, lexeme.token.type });
   base->tokenType = NodeType::BASE;

   return true;
}

bool MoleculeNameParser::TreeBuilder::_processFactorMultiplier(const Lexeme& lexeme) {
   const int value = _strToInt(lexeme.token.value);

   if (_current->type == FragmentNodeType::SUBSTITUENT) {
      FragmentNodeSubstituent* node = dynamic_cast<FragmentNodeSubstituent*>(_current);
      if (node->expectFragMultiplier) {
         if (node->fragmentMultiplier != 1) {
            node->fragmentMultiplier *= value;
         }
         node->expectFragMultiplier = false;
         return true;
      }
   }

   FragmentNodeBase* base = dynamic_cast<FragmentNodeBase*>(_current);
   Multipliers& multipliers = base->multipliers;
   if (multipliers.empty()) {
      multipliers.push({ value, TokenType::BASIC });
   }
   else {
      const Multiplier& prev = multipliers.top();
      int value = _strToInt(lexeme.token.value);
      value *= prev.first;
      multipliers.pop();
      multipliers.push({ value, TokenType::BASIC });
   }
   base->tokenType = NodeType::BASE;

   return true;
}

// Processes multiplier lexemes
bool MoleculeNameParser::TreeBuilder::_processMultiplier(const Lexeme& lexeme) {
   bool result = true;

   switch (lexeme.token.type) {
   case TokenType::BASIC: {
      result = _processBasicMultiplier(lexeme);
   } break;

   case TokenType::FACTOR: {
      result = _processFactorMultiplier(lexeme);
   } break;

   default:
      break;
   }

   return result;
}

bool MoleculeNameParser::TreeBuilder::_processLocant(const Lexeme& lexeme) {
   if (_startNewNode) {
      FragmentNodeSubstituent* subst = new FragmentNodeSubstituent;
      if (!_current->parent->insertBefore(subst, _getCurrentBase())) {
         return false;
      }
      _current = subst;
      _startNewNode = false;
   }
   int value = _strToInt(lexeme.token.value);

   FragmentNodeSubstituent* subst = dynamic_cast<FragmentNodeSubstituent*>(_current);
   subst->positions.push_back(value);

   FragmentNodeBase* base = _getCurrentBase();
   base->locants.push_back(value);

   return true;
}

bool MoleculeNameParser::TreeBuilder::_processPunctuation(const Lexeme& lexeme) {
   if (lexeme.lexeme == ",") {
      if (_current->type != FragmentNodeType::SUBSTITUENT) {
         return false;
      }
      dynamic_cast<FragmentNodeSubstituent*>(_current)->expectFragMultiplier = true;
      return true;
   }

   /*
   Whitespace is a special case when we actually add a new build tree
   for a new structure
   */
   // FIXME - process acids (acid names have whitespace)
   if (lexeme.lexeme == " ") {
      buildTree.addRoot();
      _initBuildTree();
      return true;
   }

   return true;
}

// Processes separator lexemes
bool MoleculeNameParser::TreeBuilder::_processSeparator(const Lexeme& lexeme) {
   bool result = true;

   switch (lexeme.token.type) {
   case TokenType::OPENING_BRACKET: {
      // empty brackets aren't allowed
      if (_parse->peekNextToken(TokenType::CLOSING_BRACKET)) {
         return false;
      }

      FragmentNodeBase* base = new FragmentNodeBase;
      base->tokenType = NodeType::BASE;
      _current->insert(base);
      _current = base;
      _startNewNode = true;
   } break;

   case TokenType::CLOSING_BRACKET:
      if (!_upOneLevel()) {
         return false;
      }
      break;

   case TokenType::LOCANT: {
      result = _processLocant(lexeme);
   } break;

   // currently no-op
   case TokenType::PRIME: {
   } break;

   case TokenType::PUNCTUATION: {
      result = _processPunctuation(lexeme);
   } break;

   default:
      break;
   }

   return result;
}

// Converts std::string to int
int MoleculeNameParser::TreeBuilder::_strToInt(const string& str) {
   char* ch = nullptr;
   return std::strtol(str.c_str(), &ch, 10);
}

// Retrieves current level's base; each level has only one base
MoleculeNameParser::FragmentNodeBase* MoleculeNameParser::TreeBuilder::_getCurrentBase() {
   if (_current->type == FragmentNodeType::BASE) {
      return dynamic_cast<FragmentNodeBase*>(_current);
   }

   FragmentNode* parent = _current->parent;
   if (!parent) {
      return nullptr;
   }

   // Base is always the rightmost node
   return dynamic_cast<FragmentNodeBase*>(parent->nodes.back());
}

// Retrieves upper level's base, if any; each level has only one base
MoleculeNameParser::FragmentNodeBase* MoleculeNameParser::TreeBuilder::_getParentBase() {
   const FragmentNode* parent = _current->parent;
   if (!parent) {
      return nullptr;
   }

   const FragmentNode* grand = parent->parent;
   if (!grand) {
      return nullptr;
   }

   // Base is always the rightmost node
   return dynamic_cast<FragmentNodeBase*>(grand->nodes.back());
}

void MoleculeNameParser::ResultBuilder::_initOrganicElements() {
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

/*
Traverses the build tree in post-order depth-first order, creates
SMILES representation and loads the SMILES into the resulting Molecule
*/
bool MoleculeNameParser::ResultBuilder::buildResult(Molecule& molecule) {
   molecule.clear();

   const FragmentBuildTree& buildTree = _treeBuilder.buildTree;
   const Nodes& roots = buildTree.roots;
   if (roots.empty()) {
      return false;
   }

   // most time we'll have a sigle root
   for (FragmentNode* root : roots) {
      const Nodes& nodes = root->nodes;
      for (FragmentNode* node : nodes) {
         if (!_processNode(node)) {
            return false;
         }
      }
      if (!_combine(root)) {
         return false;
      }
   }

   if (!_fragments.empty()) {
      const Fragment& frag = _fragments.top();
      std::for_each(frag.begin(), frag.end(), [=](const string& it) {_SMILES += it;});
      _fragments.pop();
      while (!_fragments.empty()) {
         _SMILES += ".";
         const Fragment& frag = _fragments.top();
         std::for_each(frag.begin(), frag.end(), [=](const string& it) {_SMILES += it;});
         _fragments.pop();
      }
   } else {
      return false;
   }

   BufferScanner scanner(_SMILES.c_str());
   SmilesLoader loader(scanner);
   loader.loadMolecule(molecule);

   return true;
}

/*
Processes a single node in the build tree
Performs depth-first traversal
Dispatches further processing depending on node's type
*/
bool MoleculeNameParser::ResultBuilder::_processNode(FragmentNode* node) {
   const Nodes& nodes = node->nodes;
   for (FragmentNode* frag : nodes) {
      if (!_processNode(frag)) {
         return false;
      }
   }

   FragmentNodeType type = node->type;
   if (type == FragmentNodeType::BASE) {
      if (!_processBaseNode(dynamic_cast<FragmentNodeBase*>(node))) {
         return false;
      }
   } else if (type == FragmentNodeType::SUBSTITUENT) {
      if (!_processSubstNode(dynamic_cast<FragmentNodeSubstituent*>(node))) {
         return false;
      }
   }

   return true;
}

/*
Processes a base node. A base node contains information about structure or
substituent base: number of locants, chemical element info, bonds, etc.
*/
bool MoleculeNameParser::ResultBuilder::_processBaseNode(FragmentNodeBase* base) {
   const Element& element = base->element;
   const int number = element.first;

   string bond_sym;
   if (base->bondOrder == BOND_DOUBLE) {
      bond_sym = "=";
   } else if (base->bondOrder == BOND_TRIPLE) {
      bond_sym = "#";
   }

   Fragment fragment;
   const NodeType& nt = base->tokenType;
   if (nt == NodeType::SUFFIX) {
      fragment.push_back(bond_sym);
      _fragments.push(fragment);
      return true;
   }

   const int multipliers = base->combineMultipliers();
   if (multipliers >= 1) {
      bool organicElement = (_organicElements.find(number) != _organicElements.end());
      if (nt == NodeType::ELEMENT) {
         organicElement = false;
      }

      const string& symbol = organicElement ? _organicElements[number] : "[" + element.second + "]";
      fragment.push_back(symbol);

      for (int i = 1; i < multipliers; i++) {
         fragment.push_back(symbol);
      }
   }

   if (base->cycle) {
      string& first = fragment.front();
      first.insert(1, "1");
      string& last = fragment.back();
      last.insert(1, "1");
   }

   const Locants& locants = base->locants;
   if (!locants.empty()) {
      // locant positions are marked with pipe | for easier search and substitution
      const string placeholder = "|";
      int inserted{ 0 };
      auto it = fragment.begin();
      for (int loc : locants) {
         std::advance(it, loc + inserted);
         fragment.insert(it, placeholder);
         it = fragment.begin();
         ++inserted;
      }
   }

   /*
   If there are no locants and the structure has double or triple bond, insert
   it immediately after the first atom
   */
   if (nt == NodeType::BASE) {
      if (locants.empty() && (base->bondOrder != BOND_SINGLE)) {
         auto it = fragment.begin();
         std::advance(it, 1);
         fragment.insert(it, bond_sym);
      }
   }

   _fragments.push(fragment);
   return true;
}

/*
Processes a substituent node. Any substituent might also be a base
*/
bool MoleculeNameParser::ResultBuilder::_processSubstNode(FragmentNodeSubstituent* subst) {
   if (subst->nodes.empty()) {
      return _processBaseNode(subst);
   } else {
      return _combine(subst);
   }

   // shouldn't reach here
   return false;
}

/*
Combines bases and substituents
1. The current base being processed is the top element on the _fragments stack
Pop it off the stack
Every substituent position is marked by | symbol
2. Reverse iterate through child nodes in node's collection; substituents are
pushed onto _fragments stack in reverse order
Replace corresponding placeholders in base fragment by current substituent
3. When all children/substituents for the current base are processed, push the
result back onto stack; this will either be a complete result or a new substituent
for further combine cycles
*/
bool MoleculeNameParser::ResultBuilder::_combine(FragmentNode* node) {
   Fragment base = _fragments.top();
   if (base.empty()) {
      return false;
   }

   _fragments.pop();

   const Nodes& nodes = node->nodes;
   auto it = nodes.rbegin();
   ++it;

   Fragment frag;
   while (it != nodes.rend()) {
      Fragment f = _fragments.top();
      _fragments.pop();

      const NodeType& nt = dynamic_cast<FragmentNodeBase*>(*it)->tokenType;
      if (nt == NodeType::BASE) {
         frag.push_back("(");
         frag.splice(frag.end(), f);
         frag.push_back(")");
      } else {
         frag = f;
      }

      const FragmentNodeSubstituent* subst = dynamic_cast<FragmentNodeSubstituent*>(*it);
      for (int i = 0; i < subst->fragmentMultiplier; i++) {
         auto placeholder = this->find_last(base, "|");
         placeholder = base.erase(placeholder);
         base.insert(placeholder, frag.begin(), frag.end());
      }

      ++it;
      frag.clear();
   }

   _fragments.push(base);
   if (node->type != FragmentNodeType::ROOT) {
      dynamic_cast<FragmentNodeBase*>(node)->tokenType = NodeType::BASE;
   }
   return true;
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

   _checkBrackets(input);
   Parse parse(input);
   parse.scan();

   if (parse.hasFailures) {
      const Failures& failures = parse.failures;
      string message;
      for (const string& f : failures) {
         message += f + " ";
      }
      throw Error("Cannot parse input %s due to errors: %s", name, message.c_str());
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
void MoleculeNameParser::_checkBrackets(const string& s) {
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

_SessionLocalContainer<MoleculeNameParser> MoleculeNameParser_self;

MoleculeNameParser& indigo::getMoleculeNameParserInstance() {
   return MoleculeNameParser_self.getLocalCopy();
}
