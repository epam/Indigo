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
#include <string>
#include <vector>

#include "molecule.h"
#include "tinyxml.h"
#include "base_cpp/trie.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace name_parsing {

	enum class TokenType {
		unknown = -1,

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
		text
	};

	// static array of token type names
	static std::vector<std::string> TokenTypeStrings;

	/*
	* A lexeme represents a product of parsing
	* Each lexeme has a token associated with it
	*/
	class Lexeme {
		std::string _lexeme;					// a lexeme
		TokenType	_token;						// a token

	public:
		inline Lexeme(char ch, TokenType token) : _token{ token } { _lexeme += ch; }
		inline Lexeme(const std::string& lexeme, TokenType token) : _lexeme{ lexeme }, _token{ token } { }

		inline const std::string& getLexeme() const { return _lexeme; }
		inline TokenType getToken() const { return _token; }

		static TokenType tokenTypeFromString(const std::string& s);
	}; // Lexeme

	// A dictionary of known pre-defined symbols
	typedef std::map<std::string, TokenType> SymbolDictionary;

	// A trie for known pre-defined lexems
	typedef indigo::Trie<TokenType> LexemsTrie;

	/*
	 * A singleton for managing various global symbol tables
	 */
	class DictionaryManager {

		DECL_ERROR;

		LexemsTrie		 _lexTrie;				// global trie of pre-defined lexems
		SymbolDictionary _dictionary;			// global dictionary of pre-defined symbols
		std::string		 _separators;			// a string of separator characters

		/*
		 * Helpers
		 */
		void readTable(const char* table, bool useTrie = false);
		void readTokenTypeStrings();
		void addLexeme(const std::string& lexeme, TokenType token, bool useTrie);

	public:
		DictionaryManager();

		inline LexemsTrie& getLexemsTrie() { return _lexTrie; }
		inline const LexemsTrie& getLexemsTrie() const { return _lexTrie; }
		inline const SymbolDictionary& getDictionary() const { return _dictionary; }
		inline const std::string& getSeparators() const { return _separators; }
	};

	typedef std::vector<Lexeme> Lexems;
	typedef std::vector<std::string> Failures;

	/*
	 * A product of parsing process
	 * Keeps dictionaries of lexems and tokens
	 */
	class Parse {
		DECL_ERROR;

		std::string _input;						// an input string as-is
		Lexems		_lexems;					// a list of lexems that form the input

		Failures _failures;						// a list of fragments failed to having being parsed
		bool	 _hasFailures;					// failure flag

		/*
		 * Splits a fragment into smaller lexems
		 * Sets up the failure flag if unparsable fragment is encountered
		 */
		void processTextFragment(const std::string& fragment);

	public:
		inline explicit Parse(const std::string& input) : _input{ input }, _hasFailures{ false } { }

		inline const std::string& getInput() const { return _input; }
		inline const Lexems& getLexemes() const { return _lexems; }
		inline const Failures& getFailures() const { return _failures; }
		inline bool hasFailures() const { return _hasFailures; }

		/*
		 * Performs by-symbol input scan, determines basic tokens
		 * Text fragments require further processing
		 */
		void scan();
	};

	class TokenizationResult {
		DECL_ERROR;

		std::string _input;
		Parse _parse;

		bool _completelyParsed;

	public:
		inline TokenizationResult(const std::string& input) : _input{ input },
			_completelyParsed{ false },
			_parse{ input } { }

		inline const std::string& getInput() const { return _input; }
		inline const Parse& getParse() const { return _parse; }

		inline bool isCompletelyParsed() const { return _completelyParsed; }

		void parse();
	};

	class Tokenizer {
		DECL_ERROR;

	public:
		TokenizationResult tokenize(const char* name);
	};

	/*
	 * A result builder
	 * Builds a resulting molecule from a tokenization result
	 */
	class ResultBuilder {
		DECL_ERROR;

		indigo::Molecule* _mol;

		void processLexeme(const Lexeme& l);

	public:
		inline explicit ResultBuilder(indigo::Molecule& mol) : _mol(&mol) { }

		/*
		 * Builds a result from a parse
		 * Returns true if successful
		 */
		bool build(const Parse& parse);
	};

	/*
	 * The base class for NameToStructure feature
	 * Session local instance of this class is used by public API indigoNameToStructure
	 */
	class DLLEXPORT MoleculeNameParser {
		DECL_ERROR;

		Tokenizer	  _tokenizer;

	public:
		// returns parse result, NULL otherwise
		void parseMolecule(const char *name, indigo::Molecule &mol);
	};

	// Auxillary all-static tools for syntax checks etc.
	class AuxParseTools {
		DECL_ERROR;

	public:
		// checks if allowed opening and closing brackets match
		static void checkBrackets(const std::string& s);
		static int splitMultipliers(const std::string& s);
	};

	MoleculeNameParser& getMoleculeNameParserInstance();
	DictionaryManager& getDictionaryManagerInstance();
} // namespace name_parsing

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_name_parser__
