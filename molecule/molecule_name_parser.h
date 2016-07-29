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

	/* A token represents a unit of chemical name parse
	 * A set of tokens later is checked against grammar rules
	 */
	struct Token {
		inline Token() : _name{ "" }, _type{ TokenType::unknown } { }
		inline Token(const std::string& name, TokenType type) : _name{ name }, _type{ type } { }

		std::string _name;
		TokenType	_type;

		static TokenType tokenTypeFromString(const std::string& s);
	};

	// A dictionary of known pre-defined names and name parts
	typedef std::map<std::string, Token> SymbolDictionary;

	/* A singleton for managing various symbol tables
	 */
	class DictionaryManager {

		DECL_ERROR;

		SymbolDictionary _dictionary;			// global dictionary of pre-defined symbols
		std::string		 _separators;			// a string of separator characters

		void readTable(const char* table);
		void readTokenTypeStrings();

	public:
		DictionaryManager();

		inline const SymbolDictionary& getDictionary() const { return _dictionary; }
		inline const std::string& getSeparators() const { return _separators; }
	};

	/* A lexeme represents a product of parsing
	 * Each lexeme has a token associated with it
	 */
	class Lexeme {
		std::string _lexeme;				// a lexeme
		Token _token;						// a token

	public:
		inline Lexeme(const std::string& lexeme,
					  const Token& token) : _lexeme(lexeme), _token(token) { }

		inline const std::string& get_lexeme() const { return _lexeme; }
		inline const Token& get_token() const { return _token; }
	}; // Lexeme

	typedef std::vector<Lexeme> Lexems;
	typedef std::vector<Token> Tokens;

	/* A product of parsing process
	 * Keeps dictionaries of lexems and tokens
	 */
	class Parse {
		std::string _input;					// an input string as-is
		Lexems _lexems;						// a list of lexems that form the input
		Tokens _tokens;						// a list of tokens

		void addLexeme(const std::string& l, Token t);

	public:
		inline Parse(const std::string& input) : _input(input) { }

		inline const std::string& getInput() const { return _input; }
		inline const Lexems& getLexemes() const { return _lexems; }
		inline const Tokens& getTokens() const { return _tokens; }

		void scan();
	};

	class TokenizationResult {
		// the input string as-is
		std::string _input;
		Parse _parse;

		bool _completely_parsed;

		DECL_ERROR;

	public:
		inline TokenizationResult(const std::string& input) : _input(input),
															  _completely_parsed(false),
															  _parse(input) { }

		inline const std::string& get_input() const { return _input; }
		inline const Parse& get_parse() const { return _parse; }

		inline bool is_completely_parsed() const { return _completely_parsed; }

		void parse();
	};
	
	class Tokenizer {

		DECL_ERROR;

	public:
		TokenizationResult tokenize(const char* name);
	};

	/* The base class for NameToStructure feature
	 * Session local instance of this class is used by public API indigoNameToStructure
	 */
	class DLLEXPORT MoleculeNameParser {

		DECL_ERROR;

		Tokenizer _tokenizer;

	public:
		// returns parse result, NULL otherwise
		void parseMolecule(const char *name, indigo::Molecule &mol);
	};

	// Auxillary all-static tools for syntax checks etc.
	class AuxParseTools {

		DECL_ERROR;

	public:
		// checks if allowed opening and closing brackets match
		static void checkBrackets(const std::string& input);
	};

	MoleculeNameParser& getMoleculeNameParserInstance();
	DictionaryManager& getTableManagerInstance();
} // namespace name_parsing

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //__molecule_name_parser__