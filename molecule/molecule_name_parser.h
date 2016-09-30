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

#ifndef __molecule_name_parser__
#define __molecule_name_parser__

#include <list>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

#include "elements.h"
#include "molecule.h"
#include "base_cpp/trie.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace name_parsing {

	enum class TokenType : int {
		endOfStream = -2,
		unknown,

		// multipliers.inc
		factor,
		basic,
		group,
		ending,
		ringAssembly,

		// separators.inc
		punctuation,
		openingBracket,
		closingBracket,
		prime,
		locant,

		// basic_elements.inc
		basicElement,

		// text fragment
		text,

		// alkanes
		bases,
		suffixes
	};

	// static array of token type names
	static std::vector<std::string> TokenTypeStrings;

	struct Token {
		std::string name;
		std::string value;
		TokenType type = TokenType::unknown;

		inline Token() { }
		inline Token(const std::string& name, const std::string& value, TokenType type) :
			name{ name }, value{ value }, type{ type } { }

		static TokenType tokenTypeFromString(const std::string& s);
	};

	/*
	A lexeme represents a product of parsing
	Each lexeme has a token associated with it
	*/
	class Lexeme {
		std::string lexeme;						// a lexeme
		Token		token;						// a token

	public:
		inline Lexeme(char ch, const Token& token) : token{ token } { lexeme += ch; }
		inline Lexeme(const std::string& lexeme, const Token& token) : lexeme{ lexeme }, token{ token } { }

		inline const std::string& getLexeme() const { return lexeme; }
		inline const Token& getToken() const { return token; }
	}; // Lexeme

	// A dictionary of known pre-defined symbols
	typedef std::map<std::string, Token> SymbolDictionary;

	// A trie for known pre-defined lexemes
	typedef indigo::Trie<Token> LexemesTrie;

	/*
	 * A singleton for managing various global symbol tables
	 */
	class DictionaryManager {

		DECL_ERROR;

		LexemesTrie		 lexemesTrie;			// global trie of pre-defined lexemes
		SymbolDictionary dictionary;			// global dictionary of pre-defined symbols
		std::string		 separators;			// a string of separator characters

		void readBasicElementsTable();
		void readTable(const char* table, bool useTrie = false);
		void readTokenTypeStrings();
		void addLexeme(const std::string& lexeme, const Token& token, bool useTrie);

	public:
		DictionaryManager();

		inline LexemesTrie& getLexemesTrie() { return lexemesTrie; }
		inline const LexemesTrie& getLexemesTrie() const { return lexemesTrie; }
		inline const SymbolDictionary& getDictionary() const { return dictionary; }
		inline const std::string& getSeparators() const { return separators; }
	};

	typedef std::vector<Lexeme> Lexemes;
	typedef std::vector<std::string> Failures;

	/*
	 * A product of parsing process
	 * Keeps dictionaries of lexemes and tokens
	 */
	class Parse : public indigo::NonCopyable {
		DECL_ERROR;

		std::string input;						// an input string as-is
		Lexemes		lexemes;					// a list of lexemes that form the input

		Failures failures;						// a list of fragments failed to having being parsed
		bool	 _hasFailures = false;			// failure flag

		/*
		 Splits a fragment into smaller lexemes
		 Sets up the failure flag if unparsable fragment is encountered
		 */
		void processTextFragment(const std::string& fragment);

		bool _hasElision = false;				// there was an elision during a parse

		// try to find a lexeme using an elision rule
		bool tryElision(const std::string& failure);

		// A pointer to the current position in the stream of lexemes
		mutable size_t currentLexeme = 0;

	public:
		inline explicit Parse(const std::string& input) : input{ input } { }

		inline const std::string& getInput() const { return input; }
		inline const Lexemes& getLexemes() const { return lexemes; }
		inline const Failures& getFailures() const { return failures; }
		inline bool hasFailures() const { return _hasFailures; }
		inline bool hasElision() const { return _hasElision; }

		inline const size_t getCurrentLexeme() const { return currentLexeme; }

		/*
		 Performs by-symbol input scan, determines basic tokens
		 Text fragments require further processing
		 */
		void scan();

		// Retrieves a next lexeme from the stream, incrementing the stream pointer
		const Lexeme& getNextLexeme() const;
		// Returns true if next lexeme's token type equals to input
		bool peekNextToken(TokenType peek) const;
	};

	enum class FragmentNodeType : int {
		unknown = -1,
		root,
		base,
		substituent
	};

	class FragmentNode;
	typedef std::list<FragmentNode*> Nodes;

	/*
	A node in a build tree
	Can be either a base or a substituent. The base node represents a base structure.
	The substituent node represents a substituent in a base
	Each base can have an arbitrary number of substituents; each level in the
	tree can have exactly one base
	A base is always inserted first, then any substituents are insreted in their
	respective order before base node. Later, depth-first traversal is used
	to create Molecule objects
	*/
	class FragmentNode {

		// A handle to the parent; must not be freed
		FragmentNode* parent = nullptr;

		// A list on nodes
		Nodes nodes;

	protected:
		FragmentNodeType type = FragmentNodeType::unknown;

	public:
		inline FragmentNode() { }
		virtual ~FragmentNode();

		inline FragmentNodeType getType() const { return type; }
		inline bool checkType(FragmentNodeType type) { return (this->type == type); }
		inline void setType(FragmentNodeType type) { this->type = type; }

		inline FragmentNode* getParent() { return parent; }
		inline const FragmentNode* getParent() const { return parent; }
		inline void setParent(FragmentNode* parent) { this->parent = parent; }

		inline Nodes& getNodes() { return nodes; }
		inline const Nodes& getNodes() const { return nodes; }

		// Inserts a new node before anchor position, returns status
		bool insertBefore(FragmentNode* node, const FragmentNode* anchor);

		// Inserts a new node at the end of the list
		void insert(FragmentNode* node);

#ifdef DEBUG
		virtual void print(std::ostream& out) const;
#endif
	};

	class FragmentNodeRoot : public FragmentNode {
	public:
		inline FragmentNodeRoot() { type = FragmentNodeType::root; }

#ifdef DEBUG
		virtual void print(std::ostream& out) const;
#endif
	};

	typedef std::pair<int, TokenType> Multiplier;
	typedef std::stack<Multiplier> Multipliers;

	typedef std::vector<int> Locants;

	typedef std::pair<int, std::string> Element;

	/*
	A node that represents a base structure
	Has multipliers that define how many basic elements or groups this structure has
	Any additional atoms or elements will be stored as substituents
	*/
	class FragmentNodeBase : public FragmentNode {

		Locants locants;

	protected:
		Multipliers multipliers;

		Element element;

		/*
		A diff in total valency of the (sub)stucture
		Must correspond to the name grammar and syntax, and bonds count
		*/
		int valencyDiff = 0;

		/*
		The number of atom with free bond
		Alkanes ending with -ane don't have free bonds
		Alkanes ending with -yl have 1 free bond
		*/
		int freeAtomOrder = 0;

		int bondOrder = indigo::BOND_ZERO;		// from base_molecule.h via molecule.h

	public:
		FragmentNodeBase();

		inline Multipliers& getMultipliers() { return multipliers; }
		inline const Multipliers& getMultipliers() const { return multipliers; }

		inline Element& getElement() { return element; }
		inline const Element& getElement() const { return element; }
		inline void setElementNumber(int number) { element.first = number; }
		inline void setElementSymbol(const std::string& symbol) { element.second = symbol; }

		inline int getValenceDiff() const { return valencyDiff; }
		inline void setValencyDiff(int diff) { valencyDiff = diff; }

		inline int getBondOrder() const { return bondOrder; }
		inline void setBondOrder(int order) { bondOrder = order; }

		inline int getFreeAtomOrder() const { return freeAtomOrder; }
		inline void setFreeAtomOrder(int order) { freeAtomOrder = order; }

		inline Locants& getLocants() { return locants; }
		inline const Locants& getLocants() const { return locants; }

		/*
		Returns the sum of multipliers stack
		This is a destructive operation
		*/
		int combineMultipliers();

#ifdef DEBUG
		virtual void print(std::ostream& out) const;
#endif
	};

	typedef std::vector<int> Positions;

	/*
	A node that represents a substituent
	Has locants that define the positions in the base structure where this
	substituent will be applied
	*/
	class FragmentNodeSubstituent : public FragmentNodeBase {

		// Positions of this substituent inside its base
		Positions positions;

		/*
		First multiplier in a substituent must match the number of locant positions
		in base structure. Next two fields control this behavior
		Example:
		2,3,3-trimethyl-octane (correct)
			locants:		  2 3 3 (total 3)
			first multiplier: tri (3)

		2,4-ethyl-hexane (incorrect, should be: 2,4-diethyl-hexane)
			locants:		  2 4 (total 2)
			first multiplier: none (default 1)
		*/

		int fragmentMultiplier = 1;
		// If true, next multiplier will be treated as fragment multiplier
		bool expectFragMultiplier = false;

	public:
		inline FragmentNodeSubstituent() { type = FragmentNodeType::substituent; }

		inline Positions& getPositions() { return positions; }
		inline const Positions& getPositions() const { return positions; }

		inline int getFragmentMultiplier() const { return fragmentMultiplier; }
		inline void setFragmentMultiplier(int multiplier) { fragmentMultiplier = multiplier; }

		inline bool getExpectFragMultiplier() const { return expectFragMultiplier; }
		inline void setExpectFragMultiplier(bool flag) { expectFragMultiplier = flag; }

#ifdef DEBUG
		virtual void print(std::ostream& out) const;
#endif
	};

	/*
	A build tree
	Is constructed from a Parse object
	Can have multiple roots as a whitespace ' ' in the input denotes separate
	structures that must be handled separately
	*/
	class FragmentBuildTree : public indigo::NonCopyable {

		Nodes roots;

		// A handle to current build tree; must not be freed
		FragmentNode* currentRoot = nullptr;

	public:
		FragmentBuildTree();
		~FragmentBuildTree();

		inline FragmentNode* getCurrentRoot() { return currentRoot; }
		inline const FragmentNode* getCurrentRoot() const { return currentRoot; }

		inline Nodes& getRoots() { return roots; }
		inline const Nodes& getRoots() const { return roots; }

		void addRoot();
	};

	/*
	Constructs a build tree from a Parse object
	Builds all trees in one pass, consequently reading the lexemes stream
	*/
	class TreeBuilder : public indigo::NonCopyable {
		DECL_ERROR;

		std::unique_ptr<FragmentBuildTree> buildTree;

		// Indicates that a next locant will start a new substituent node
		bool startNewNode = true;

		// A handle to current node being processed; must not be freed
		FragmentNode* current = nullptr;

		// Retrieves current level's base; each level has only one base
		FragmentNodeBase* getCurrentBase();

		// Retrieves upper level's base, if any; each level has only one base
		FragmentNodeBase* getParentBase();

		// A handle to Parse object; must not be freed
		const Parse* parse;

		/*
		The implementation of parse processing
		Recursively calls itself until EndOfStream is reached or error occured
		*/
		bool processParseImpl();

		void initBuildTree();

		/*
		Returns one level up in the tree, setting current node to the new
		level's base fragment
		Returns false if operation cannot be performed
		Does not change current when fails
		*/
		bool upOneLevel();

		// Processes alkane lexemes
		bool processAlkane(const Lexeme& lexeme);
		// Processes multiplier lexemes
		bool processMultiplier(const Lexeme& lexeme);
		// Processes separator lexemes
		bool processSeparator(const Lexeme& lexeme);

		void processSuffix(const Lexeme& lexeme);
		bool processBasicElement(const Lexeme& lexeme);

	public:
		inline TreeBuilder(const Parse& input) : parse{ &input } { buildTree.reset(new FragmentBuildTree); }

		inline std::unique_ptr<FragmentBuildTree>& getBuildTree() { return buildTree; }
		inline const std::unique_ptr<FragmentBuildTree>& getBuildTree() const { return buildTree; }

		bool processParse();
	};

	/*
	Builds a resulting structure from a build tree
	Uses depth-first traversal
	*/
	class ResultBuilder : public indigo::NonCopyable {
		DECL_ERROR;

		// A pointer to the tree builder, which provides the build tree
		std::unique_ptr<TreeBuilder> treeBuilder;

		std::string SMILES;

		typedef std::map<int, std::string> Elements;
		Elements organicElements;
		void initOrganicElements();

		typedef std::stack<std::string> Fragments;
		Fragments fragments;

		void processNode(FragmentNode* node);
		void processBaseNode(FragmentNodeBase* base);
		void processSubstNode(FragmentNodeSubstituent* subst);
		void combine(FragmentNode* node);

	public:
		ResultBuilder(const Parse& input);

		inline bool buildTree() const { return treeBuilder->processParse(); }

		/*
		Traverses the build tree in post-order depth-first order, creates
		SMILES representation and loads the SMILES into the resulting Molecule
		*/
		bool buildResult(indigo::Molecule& molecule);
	};

	/*
	The base class for NameToStructure feature
	Session local instance of this class is used by public API indigoNameToStructure
	*/
	class DLLEXPORT MoleculeNameParser {
		DECL_ERROR;

	public:
		/*
		Main method for convertion from a chemical name into a Molecule object
		A given name undergoes several transformations:
		phase 1: lexical analysis
		phase 2: tokenization
		phase 3: grammatical rules check
		phase 4: construction of a Moleclule object from parsed fragments
		No param check - did that on caller side
		*/
		static void parseMolecule(const char *name, indigo::Molecule &molecule);
	};

	// Auxillary all-static tools for syntax checks etc.
	class AuxParseTools {
		DECL_ERROR;

	public:
		/*
		Checks if allowed opening and closing brackets match
		Doesn't check brackets type matching (e.g. ((]] is valid)
		*/
		static void checkBrackets(const std::string& s);

		// Converts std::string to int
		static int strToInt(const std::string& str);
	};

	MoleculeNameParser& getMoleculeNameParserInstance();
	DictionaryManager& getDictionaryManagerInstance();
} // namespace name_parsing

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_name_parser__
