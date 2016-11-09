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

#include <algorithm>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

#include "elements.h"
#include "base_cpp/trie.h"
#include "base_cpp/non_copyable.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

/*
The base class for NameToStructure feature
Session local instance of this class is used by public API indigoNameToStructure
*/
class DLLEXPORT MoleculeNameParser {
   DECL_ERROR;

   /*
   Enum class of token types
   These types are assigned to tokens during lexical analysis
   */
   enum class TokenType : int {
      END_OF_STREAM = -2,						// a special terminator for lexemes stream
      UNKNOWN,								// a default value

      /*
      multipliers.inc
      */
      FACTOR,									// a factor (x10, x100, etc.)
      BASIC,									// basic [1..9] OR combined final value, e.g. 113
      GROUP,									// group multipliers
      ENDING,									// special multiplier endings
      RING_ASSEMBLY,							// ring multipliers

      /*
      separators.inc
      */
      PUNCTUATION,							// a generic punctuation mark
      OPENING_BRACKET,						// ( [ {
      CLOSING_BRACKET,						// ) ] }
      PRIME,									// '
      LOCANT,									// a locant in basic structure

      /*
      basic_elements.inc
      */
      BASIC_ELEMENT,							// a basic chemical element

      /*
      A generic text fragment
      Will be analyzed and split into smaller lexemes
      */
      TEXT,

      /*
      FIXME
      needs re-naming and re-design
      */
      BASES,									// names of alkanes(-enes, -ynes)
      SUFFIXES,								// suffixes for alkanes(enes, -ynes)

      /*
      Miscelaneous structure flags (cyclo-, cis-, trans-, etc.)
      */
      FLAG,

      SKELETAL_PREFIX
   }; //enum class TokenType

   /*
   A struct denoting a token
   Keeps a name of the token family, associated value, and type
   Due to restrictions of tinyxml library, all types are const char* or strings
   */
   struct Token {
      std::string name;
      std::string value;
      TokenType type = TokenType::UNKNOWN;

      inline Token() { }
      inline Token(const std::string& n, const std::string& v, const TokenType& t) {
         name = n;
         value = v;
         type = t;
      }
   }; // struct Token

   /*
   A lexeme represents a product of parsing
   Each lexeme has a token associated with it
   */
   struct Lexeme {
      std::string lexeme;				            // a lexeme
      Token       token;				            // a token

      inline Lexeme() { }
      inline Lexeme(char ch, const Token& t) {
         lexeme = ch;
         token = t;
      }
      inline Lexeme(const std::string& l, const Token& t) {
         lexeme = l;
         token = t;
      }
   }; // class Lexeme

   // A dictionary of known pre-defined symbols
   typedef std::map<std::string, Token> SymbolDictionary;

   // A trie for known pre-defined lexemes
   typedef Trie<Token> LexemesTrie;

   typedef std::vector<std::string> TokenTypeStrings;

   /*
   A dictionary for managing various global symbol tables
   */
   class DictionaryManager : public NonCopyable {
   public:
      DictionaryManager();

      LexemesTrie lexemesTrie;		            // global trie of pre-defined lexemes
      SymbolDictionary dictionary;		         // global dictionary of pre-defined symbols
      std::string separators;		               // a string of separator characters

   private:
      DECL_ERROR;

      void _readBasicElementsTable();
      void _readSkeletalAtomsTable();
      void _readTable(const char* table, bool useTrie = false);
      void _readTokenTypeStrings();
      void _addLexeme(const std::string& lexeme, const Token& token, bool useTrie);

      TokenTypeStrings _tokenTypeStrings;

      TokenType _tokenTypeFromString(const std::string& s);
   }; // class DictionaryManager

   typedef std::vector<std::string> Failures;
   typedef std::vector<Lexeme> Lexemes;
      
   /*
   A product of parsing process
   Keeps dictionaries of lexemes and tokens
   */
   class Parse : public NonCopyable {
   public:
      inline explicit Parse(const std::string& in) {
         input = in;
      }

      /*
      Performs by-symbol input scan, determines basic tokens
      Text fragments require further processing
      */
      void scan();

      // Retrieves a next lexeme from the stream, incrementing the stream pointer
      const Lexeme& getNextLexeme() const;
      // Returns true if next lexeme's token type equals to input
      bool peekNextToken(TokenType peek) const;

      bool hasFailures = false;		            // failure flag
      mutable size_t currentLexeme = 0;         // A pointer to the current position in the stream of lexemes
      std::string input;					         // an input string as-is
      Lexemes		lexemes;				            // a list of lexemes that form the input
      Failures failures;					         // a list of fragments failed to having being parsed

   private:
      DECL_ERROR;

      /*
      Splits a fragment into smaller lexemes
      Sets up the failure flag if unparsable fragment is encountered
      */
      void _processTextFragment(const std::string& fragment);

      // try to find a lexeme using an elision rule
      bool _tryElision(const std::string& failure);
   }; // class Parse

   enum class FragmentClassType : int {
      INVALID = -1,
      ROOT,
      BASE,
      SUBSTITUENT
   }; // enum class FragmentNodeType

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
   class FragmentNode : NonCopyable {
   public:
      inline FragmentNode() { }
      virtual ~FragmentNode();

      // Inserts a new node before anchor position, returns status
      bool insertBefore(FragmentNode* node, const FragmentNode* anchor);

      // Inserts a new node at the end of the list
      void insert(FragmentNode* node);
#ifdef DEBUG
      virtual void print(std::ostream& out) const;
#endif

      FragmentClassType classType = FragmentClassType::INVALID;
      FragmentNode* parent = nullptr;           // A handle to the parent; must not be freed
      Nodes nodes;                              // A list on nodes
   }; // class FragmentNode

   class FragmentNodeRoot : public FragmentNode {
   public:
      inline FragmentNodeRoot() { classType = FragmentClassType::ROOT; }
      virtual ~FragmentNodeRoot() { }

#ifdef DEBUG
      virtual void print(std::ostream& out) const;
#endif
   }; // class FragmentNodeRoot

   typedef std::pair<int, TokenType> Multiplier;
   typedef std::stack<Multiplier> Multipliers;

   typedef std::vector<int> Locants;

   typedef std::pair<int, std::string> Element;

   enum class NodeType : int {
      INVALID = -1,
      BASE,
      ELEMENT,
      SKELETAL,
      SUFFIX
   }; // enum class NodeType

   enum class Isomerism : int {
      NONE = 0,
      CIS,
      TRANS
   };

   /*
   A node that represents a base structure
   Has multipliers that define how many basic elements or groups this structure has
   Any additional atoms or elements will be stored as substituents
   */
   class FragmentNodeBase : public FragmentNode {
   public:
      FragmentNodeBase();
      virtual ~FragmentNodeBase() { }

      /*
      Returns the sum of multipliers stack
      This is a destructive operation
      */
      int combineMultipliers();

#ifdef DEBUG
      virtual void print(std::ostream& out) const;
#endif

      Multipliers multipliers;
      Element element;
      Locants locants;
      Isomerism isomerism = Isomerism::NONE;

      // A bonding of an element
      int bonding = 0;

      // A bond type
      int bondType = BOND_ZERO;

      /*
      The number of atom with free bond
      Alkanes ending with -ane don't have free bonds
      Alkanes ending with -yl have 1 free bond
      */
      int freeAtoms = 0;

      // true if the structure is a cycle
      bool cycle = false;

      NodeType nodeType = NodeType::INVALID;
   }; // class FragmentNodeBase

   typedef std::vector<int> Positions;

   /*
   A node that represents a substituent
   Has locants that define the positions in the base structure where this
   substituent will be applied
   */
   class FragmentNodeSubstituent : public FragmentNodeBase {
   public:
      inline FragmentNodeSubstituent() { classType = FragmentClassType::SUBSTITUENT; }
      virtual ~FragmentNodeSubstituent() { }

#ifdef DEBUG
      virtual void print(std::ostream& out) const;
#endif

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
   }; // class FragmentNodeSubstituent

   /*
   A build tree
   Is constructed from a Parse object
   Can have multiple roots as a whitespace ' ' in the input denotes separate
   structures that must be handled separately
   */
   class FragmentBuildTree : public NonCopyable {
   public:
      FragmentBuildTree();
      virtual ~FragmentBuildTree();

      void addRoot();

      // A handle to current build tree; must not be freed
      FragmentNode* currentRoot = nullptr;

      Nodes roots;
   }; // class FragmentBuildTree

   /*
   Constructs a build tree from a Parse object
   Builds all trees in one pass, consequently reading the lexemes stream
   */
   class TreeBuilder : public NonCopyable {
   public:
      inline TreeBuilder(const Parse& input) : _parse{ &input } { }

      bool processParse();

      FragmentBuildTree buildTree;

   private:
      DECL_ERROR;

      // Indicates that a next locant will start a new substituent node
      bool _startNewNode = true;

      // A handle to current node being processed; must not be freed
      FragmentNode* _current = nullptr;

      // A handle to Parse object; must not be freed
      const Parse* _parse;

      // Retrieves current level's base; each level has only one base
      FragmentNodeBase* _getCurrentBase();

      // Retrieves upper level's base, if any; each level has only one base
      FragmentNodeBase* _getParentBase();

      /*
      The implementation of parse processing
      Recursively calls itself until EndOfStream is reached or error occured
      */
      bool _processParse();

      void _initBuildTree();

      /*
      Returns one level up in the tree, setting current node to the new
      level's base fragment
      Returns false if operation cannot be performed
      Does not change current when fails
      */
      bool _upOneLevel();

      // Processes alkane lexemes
      bool _processAlkane(const Lexeme& lexeme);

      // Processes multiplier lexemes
      bool _processMultiplier(const Lexeme& lexeme);

      // Processes separator lexemes
      bool _processSeparator(const Lexeme& lexeme);

      // Processes an alkane suffix
      void _processSuffix(const Lexeme& lexeme);

      bool _processBasicElement(const Lexeme& lexeme);
      bool _processAlkaneBase(const Lexeme& lexeme);
      bool _processAlkaneSuffix(const Lexeme& lexeme);
      bool _processBasicMultiplier(const Lexeme& lexeme);
      bool _processFactorMultiplier(const Lexeme& lexeme);
      bool _processLocant(const Lexeme& lexeme);
      bool _processPunctuation(const Lexeme& lexeme);
      bool _processFlags(const Lexeme& lexeme);
      bool _processSkeletal(const Lexeme& lexeme);
      bool _processSkeletalPrefix(const Lexeme& lexeme);

      // Converts std::string to int
      int _strToInt(const std::string& str);
   }; // class TreeBuilder

   /*
   As a single position in SMILES string may contain more than one symbol,
   hence we need a collection of strings rather than a collection of symbols
   */
   typedef std::list<std::string> Fragment;
   typedef std::stack<Fragment> Fragments;

   typedef std::map<int, std::string> Elements;

   /*
   Builds a resulting structure from a build tree
   Uses depth-first traversal
   */
   class SmilesBuilder : public NonCopyable {
   public:
      SmilesBuilder(const Parse& input) : _treeBuilder{ input } { _initOrganicElements(); }

      inline bool buildTree() { return _treeBuilder.processParse(); }

      /*
      Traverses the build tree in post-order depth-first order, creates
      SMILES representation and loads the SMILES into the resulting Molecule
      */
      bool buildResult(Molecule& molecule);

   private:
      DECL_ERROR;

      // The tree builder, which provides the build tree
      TreeBuilder _treeBuilder;

      std::string _SMILES;

      Elements _organicElements;
      void _initOrganicElements();

      Fragments _fragments;

      /*
      Processes a single node in the build tree
      Performs depth-first traversal
      Dispatches further processing depending on node's type
      */
      bool _processNode(FragmentNode* node);

      /*
      Processes a base node. A base node contains information about structure or
      substituent base: number of locants, chemical element info, bonds, etc.
      */
      bool _processBaseNode(FragmentNodeBase* base);

      /*
      Processes a substituent node. Any substituent might also be a base
      */
      bool _processSubstNode(FragmentNodeSubstituent* subst);

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
      bool _combine(FragmentNode* node);

      /*
      Returns an iterator pointing at the last occurence of the given string in the list

      Since we deal with a collection of strings rather than a collection of symbols,
      we can't just use find_last_of() or a similar method to look for the last occurence
      of placeholder symbol, nor can we convert list into string and perform a search (as
      we'll lose match between indeces)
      */
      Fragment::const_iterator find_last(const Fragment& frag, const std::string& what) const {
         Fragment tmp = frag;
         tmp.reverse();
         size_t pos = 1;
         for (const std::string& s : tmp) {
            if (s == what) {
               break;
            }
            ++pos;
         }

         if (pos > frag.size()) {
            return frag.end();
         }

         Fragment::const_iterator result = frag.begin();
         std::advance(result, frag.size() - pos);
         return result;
      }
   }; // class ResultBuilder

   /*
   Checks if allowed opening and closing brackets match
   Doesn't check brackets type matching (e.g. ((]] is valid)
   */
   void _checkBrackets(const std::string& s);

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
   void parseMolecule(const char *name, Molecule &molecule);

   DictionaryManager dictionaryManager;
}; // class MoleculeNameParser

MoleculeNameParser& getMoleculeNameParserInstance();
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_name_parser__
