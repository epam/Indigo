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

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

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
		bracket,
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
	* A lexeme represents a product of parsing
	* Each lexeme has a token associated with it
	*/
	class Lexeme {
		std::string lexeme;						// a lexeme
		Token		token;						// a token

		Lexeme() = delete;

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

		/*
		 * Helpers
		 */
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
	class Parse {
		DECL_ERROR;

		std::string input;						// an input string as-is
		Lexemes		lexemes;					// a list of lexemes that form the input

		Failures failures;						// a list of fragments failed to having being parsed
		bool	 _hasFailures = false;			// failure flag

		/*
		 * Splits a fragment into smaller lexemes
		 * Sets up the failure flag if unparsable fragment is encountered
		 */
		void processTextFragment(const std::string& fragment);

		bool _hasElision = false;				// there was an elision during a parse

		// try to find a lexeme using an elision rule
		bool tryElision(const std::string& failure);

		mutable size_t currentLexeme = 0;

		Parse() = delete;

	public:
		inline explicit Parse(const std::string& input) : input{ input } { }

		inline const std::string& getInput() const { return input; }
		inline const Lexemes& getLexemes() const { return lexemes; }
		inline const Failures& getFailures() const { return failures; }
		inline bool hasFailures() const { return _hasFailures; }
		inline bool hasElision() const { return _hasElision; }

		inline const size_t getCurrentLexeme() const { return currentLexeme; }

		/*
		 * Performs by-symbol input scan, determines basic tokens
		 * Text fragments require further processing
		 */
		void scan();

		Lexeme& getNextLexeme();
		const Lexeme& getNextLexeme() const;
	};

	class TokenizationResult {
		DECL_ERROR;

		std::string input;						// input string as-is
		Parse		_parse;						// parse result

		bool _isCompletelyParsed = false;

		TokenizationResult() = delete;

	public:
		explicit TokenizationResult(const std::string& input) : input{ input }, _parse{ input } { }

		inline Parse& getParse() { return _parse; }
		inline const Parse& getParse() const { return _parse; }

		inline bool isCompletelyParsed() const { return _isCompletelyParsed; }

		void parse();
	};

	class BuildFragment;
	typedef std::pair<int, BuildFragment> Substituent;
	typedef std::vector<Substituent> Substituents;

	class BuildFragment {

		DECL_ERROR;

		Parse* parse = nullptr;
		std::unique_ptr<BuildFragment> next;
		std::unique_ptr<BuildFragment> prev;
		std::unique_ptr<indigo::Molecule> fragment;

		bool hasMultiplier = false;
		int multiplierFactor = 0;

		typedef std::pair<int, TokenType> Multiplier;
		std::stack<Multiplier> multipliers;
		void combineMultipliers();

		bool hasPendingLocant = false;
		Substituents substituents;

		bool processAlkane(const Lexeme& l);
		bool processMultiplier(const Lexeme& l);
		bool processSeparator(const Lexeme& l);

		bool processSuffix(const Lexeme& l);

		BuildFragment() = delete;

	public:
		BuildFragment(Parse& p);

		inline indigo::Molecule& toMolecule() { return *fragment; }
		inline const indigo::Molecule& toMolecule() const { return *fragment; }

		bool processLexeme();
	};

	class ResultBuilder {
		DECL_ERROR;

		std::unique_ptr<BuildFragment> base;

		ResultBuilder() = delete;

	public:
		explicit ResultBuilder(Parse& parse);

		bool build(indigo::Molecule& molecule);
	};

	/*
	 * The base class for NameToStructure feature
	 * Session local instance of this class is used by public API indigoNameToStructure
	 */
	class DLLEXPORT MoleculeNameParser {
		DECL_ERROR;

	public:
		static void parseMolecule(const char *name, indigo::Molecule &molecule);
	};

	// Auxillary all-static tools for syntax checks etc.
	class AuxParseTools {
		DECL_ERROR;

	public:
		// checks if allowed opening and closing brackets match
		static void checkBrackets(const std::string& s);
	};

	MoleculeNameParser& getMoleculeNameParserInstance();
	DictionaryManager& getDictionaryManagerInstance();
} // namespace name_parsing

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_name_parser__
