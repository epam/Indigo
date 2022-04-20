/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __molecule_name_parser__
#define __molecule_name_parser__

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iterator>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "molecule/alkanes.inc"
#include "molecule/basic_elements.inc"
#include "molecule/elements.h"
#include "molecule/flags.inc"
#include "molecule/multipliers.inc"
#include "molecule/separators.inc"
#include "molecule/skeletal.inc"
#include "molecule/token_types.inc"
#include "molecule/trivial.inc"

#include "base_cpp/non_copyable.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tree.h"
#include "base_cpp/trie.h"
#include "elements.h"
#include "molecule/smiles_loader.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    /*
    The base class for NameToStructure feature
    Session local instance of this class is used by public API indigoNameToStructure
    */
    class DLLEXPORT MoleculeNameParser
    {
    public:
        DECL_ERROR;

        typedef unsigned long long int ParserOptionsType;

        /*
        Parsing options
        Options might be listed in any order
        Usage:
        space ' ' is the separator
        +OPTION to turn OPTION on
        -OPTION to turn OPTION off
        */
        enum ParserOptions : ParserOptionsType
        {
            /*
            Follow strict IUPAC rules when parsing names
            When strict rules are ON, names must comply with IUPAC recommendations,
            i.e. names like 1,3-hexadiene will be rejected
            Default: OFF
            */
            IUPAC_STRICT = 1ULL
        };

        ParserOptionsType _options = 0ULL;

        /*
        Enum class of token types
        These types are assigned to tokens during lexical analysis
        */
        enum class TokenType : int
        {
            END_OF_STREAM = -2, // a special terminator for lexemes stream
            UNKNOWN,            // a default value

            /*
            multipliers.inc
            */
            FACTOR,        // a factor (x10, x100, etc.)
            BASIC,         // basic [1..9] OR combined final value, e.g. 113
            GROUP,         // group multipliers
            ENDING,        // special multiplier endings
            RING_ASSEMBLY, // ring multipliers

            /*
            separators.inc
            */
            PUNCTUATION,     // a generic punctuation mark
            OPENING_BRACKET, // ( [ {
            CLOSING_BRACKET, // ) ] }
            PRIME,           // '
            LOCANT,          // a locant in basic structure

            /*
            basic_elements.inc
            */
            BASIC_ELEMENT, // a basic chemical element

            /*
            A generic text fragment
            Will be analyzed and split into smaller lexemes
            */
            TEXT,

            /*
            FIXME
            needs re-naming and re-design
            */
            BASES,    // names of alkanes(-enes, -ynes)
            SUFFIXES, // suffixes for alkanes(enes, -ynes)

            /*
            Miscelaneous structure flags (cyclo-, cis-, trans-, etc.)
            */
            FLAG,

            SKELETAL_PREFIX,

            TRIVIAL
        }; // enum class TokenType

        /*
        A struct denoting a token
        Keeps a name of the token family, associated value, and type
        Due to restrictions of tinyxml library, all types are const char* or strings
        */
        struct Token
        {
            std::string name;
            std::string value;
            TokenType type = TokenType::UNKNOWN;

            inline Token() = default;

            inline Token(const std::string& n, const std::string& v, const TokenType& t)
            {
                name = n;
                value = v;
                type = t;
            }
        }; // struct Token

        /*
        A lexeme represents a product of parsing
        Each lexeme has a token associated with it
        */
        struct Lexeme
        {
            std::string lexeme; // a lexeme
            Token token;        // a token

            mutable bool processed = false; // true if a lexeme was processed

            inline Lexeme() = default;

            inline Lexeme(char ch, const Token& t)
            {
                lexeme = ch;
                token = t;
            }
            inline Lexeme(const std::string& l, const Token& t)
            {
                lexeme = l;
                token = t;
            }
        }; // class Lexeme

        // A dictionary of known pre-defined symbols
        typedef std::map<std::string, Token> SymbolDictionary;

        // A trie for known pre-defined lexemes
        typedef Trie<Token> LexemesTrie;

        /*
        A dictionary for managing various global symbol tables
        */
        class DictionaryManager : public NonCopyable
        {
        public:
            DictionaryManager();

            LexemesTrie lexemesTrie;     // global trie of pre-defined lexemes
            SymbolDictionary dictionary; // global dictionary of pre-defined symbols
            std::string separators;      // a string of separator characters

        private:
            DECL_ERROR;

            void _readBasicElementsTable();
            void _readSkeletalAtomsTable();
            void _readTable(const char* table, bool useTrie = false);
            void _readTokenTypeStrings();
            void _addLexeme(const std::string& lexeme, const Token& token, bool useTrie);

            typedef std::vector<std::string> TokenTypeStrings;
            TokenTypeStrings _tokenTypeStrings;

            TokenType _tokenTypeFromString(const std::string& s);
        }; // class DictionaryManager

        typedef std::vector<std::string> Failures;
        typedef std::vector<Lexeme> Lexemes;

        /*
        A product of parsing process
        Keeps dictionaries of lexemes and tokens
        */
        class Parse : public NonCopyable
        {
        public:
            inline explicit Parse(const std::string& in, const MoleculeNameParser& mnp) : input(in), mnp(mnp){};

            /*
            Performs by-symbol input scan, determines basic tokens
            Text fragments require further processing
            */
            void scan();

            // Retrieves a next lexeme from the stream, incrementing the stream pointer
            const Lexeme& getNextLexeme() const;
            // Returns true if next lexeme's token type equals to input
            bool peekNextToken(TokenType peek) const;

            bool hasFailures = false;         // failure flag
            mutable size_t currentLexeme = 0; // A pointer to the current position in the stream of lexemes
            std::string input;                // an input string as-is
            Lexemes lexemes;                  // a list of lexemes that form the input
            Failures failures;                // a list of fragments failed to having being parsed

            const MoleculeNameParser& mnp;

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

        enum class FragmentClassType : int
        {
            INVALID = -1,
            ROOT,
            BASE,
            SUBSTITUENT
        }; // enum class FragmentNodeType

        class FragmentNode;
        class FragmentNodeBase;
        class FragmentNodeSubstituent;
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
        class FragmentNode : NonCopyable
        {
        public:
            FragmentNode() = default;
            virtual ~FragmentNode();

            // Inserts a new node before anchor position, returns status
            bool insertBefore(FragmentNode* node, const FragmentNode* anchor);

            // Inserts a new node at the end of the list
            void insert(FragmentNode* node);
#ifdef DEBUG
            virtual void print(std::ostream& out) const;
#endif

            FragmentClassType classType = FragmentClassType::INVALID;
            FragmentNode* parent = nullptr; // A handle to the parent; must not be freed
            Nodes nodes;                    // A list on nodes
        };                                  // class FragmentNode

        class FragmentNodeRoot : public FragmentNode
        {
        public:
            inline FragmentNodeRoot()
            {
                classType = FragmentClassType::ROOT;
            }
            ~FragmentNodeRoot() override = default;

#ifdef DEBUG
            void print(std::ostream& out) const override;
#endif
        }; // class FragmentNodeRoot

        typedef std::pair<int, TokenType> Multiplier;
        typedef std::stack<Multiplier> Multipliers;

        typedef std::vector<int> Locants;
        typedef std::vector<int> Skeletals;

        typedef std::pair<int, std::string> Element;

        enum class NodeType : int
        {
            INVALID = -1,
            BASE,
            ELEMENT,
            SKELETAL,
            SUFFIX
        }; // enum class NodeType

        enum class Isomerism : int
        {
            NONE = 0,
            CIS,
            TRANS
        };

        /*
        A node that represents a base structure
        Has multipliers that define how many basic elements or groups this structure has
        Any additional atoms or elements will be stored as substituents
        */
        class FragmentNodeBase : public FragmentNode
        {
        public:
            FragmentNodeBase();
            ~FragmentNodeBase() override = default;

            /*
            Returns the sum of multipliers stack
            This is a destructive operation
            */
            int combineMultipliers();

#ifdef DEBUG
            void print(std::ostream& out) const override;
#endif

            Element element;
            Isomerism isomerism = Isomerism::NONE;
            Multipliers multipliers;
            Locants locants;
            Skeletals skeletals;

            // A bonding of an element
            int bonding = 0;

            // A bond type
            int bondType = BOND_SINGLE;

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
        class FragmentNodeSubstituent : public FragmentNodeBase
        {
        public:
            inline FragmentNodeSubstituent()
            {
                classType = FragmentClassType::SUBSTITUENT;
            }
            ~FragmentNodeSubstituent() override = default;

            inline operator const FragmentNodeBase*() const
            {
                return dynamic_cast<const FragmentNodeBase*>(this);
            }
            inline operator FragmentNodeBase*()
            {
                return dynamic_cast<FragmentNodeBase*>(this);
            }

#ifdef DEBUG
            void print(std::ostream& out) const override;
#endif

            // Positions of this substituent inside its base
            Positions positions;

            /*
            With IUPAC_STRICT:
               First multiplier in a substituent must match the number of locant positions
               in base structure
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

            int expectedMultiplierCount = 1;
        }; // class FragmentNodeSubstituent

        /*
        A build tree
        Is constructed from a Parse object
        Can have multiple roots as a whitespace ' ' in the input denotes separate
        structures that must be handled separately
        */
        class FragmentBuildTree : public NonCopyable
        {
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
        class TreeBuilder : public NonCopyable
        {
        public:
            inline TreeBuilder(const Parse& input) : _parse{&input}
            {
            }

            bool processParse();

            FragmentBuildTree buildTree;

        private:
            DECL_ERROR;

            /*
            Checks if certain option(s) are set
            Returns true if condition matches
            */
            bool _checkParserOption(ParserOptionsType options);

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

        typedef std::map<int, std::string> Elements;

        struct SmilesRoot;
        struct SmilesNode : public NonCopyable
        {
            std::vector<SmilesRoot> roots;
            SmilesRoot* parent = nullptr;

            std::string str;
            int bondType = BOND_ZERO;

            SmilesNode() = default;

            inline SmilesNode(SmilesNode&& rhs)
            {
                roots = std::move(rhs.roots);
                parent = std::move(rhs.parent);
                str = std::move(rhs.str);
                bondType = std::move(rhs.bondType);
            }

            inline SmilesNode& operator=(SmilesNode&& rhs)
            {
                roots = std::move(rhs.roots);
                parent = std::move(rhs.parent);
                str = std::move(rhs.str);
                bondType = std::move(rhs.bondType);

                return *this;
            }

            inline SmilesNode(const std::string& s, int bond, SmilesRoot* p)
            {
                str = s;
                bondType = bond;
                parent = p;
            }
        };

        struct SmilesRoot : public NonCopyable
        {
            std::vector<SmilesNode> nodes;
            SmilesNode* parent = nullptr;

            SmilesRoot() = default;

            inline SmilesRoot(SmilesRoot&& rhs)
            {
                nodes = std::move(rhs.nodes);
                parent = std::move(rhs.parent);
            }

            inline SmilesRoot& operator=(SmilesRoot&& rhs)
            {
                nodes = std::move(rhs.nodes);
                parent = std::move(rhs.parent);

                return *this;
            }

            inline explicit SmilesRoot(SmilesNode* p)
            {
                parent = p;
            }
        };

        /*
        Builds a resulting structure from a build tree
        Uses depth-first traversal
        */
        class SmilesBuilder : public NonCopyable
        {
        public:
            SmilesBuilder(const Parse& input) : _treeBuilder{input}
            {
                _parse = &input;
                _initOrganicElements();
            }

            inline bool buildTree()
            {
                return _treeBuilder.processParse();
            }

            /*
            Traverses the build tree in post-order depth-first order, creates
            SMILES representation and loads the SMILES into the resulting Molecule
            */
            bool buildResult(Molecule& molecule);

            bool checkTrivial();
            void buildTrivial(Molecule& molecule);

        private:
            DECL_ERROR;

            // The tree builder, which provides the build tree
            TreeBuilder _treeBuilder;

            const Parse* _parse;

            std::string _SMILES;

            SmilesRoot _smilesTree;

            void _buildSmiles(SmilesRoot& root);

            void _calcHydrogens(const Element& element, int pos, SmilesRoot& root);

            Elements _organicElements;
            void _initOrganicElements();

            bool _processNodes(const Nodes& nodes, SmilesRoot& root);

            /*
            Processes a base node. A base node contains information about structure or
            substituent base: number of locants, chemical element info, bonds, etc.
            */
            bool _processBaseNode(FragmentNodeBase* base, SmilesRoot& root);

            /*
            Processes a substituent node. Any substituent might also be a base
            */
            bool _processSubstNode(FragmentNodeSubstituent* subst, SmilesRoot& root);
        }; // class SmilesBuilder

        // Turns a certain option on or off depending on input flag
        void _setOption(const char* option);

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
        void parseMolecule(const char* name, Molecule& molecule);

        const ParserOptionsType& getOptions() const
        {
            return _options;
        }

        /*
        Sets parsing options
        Changes the 'options' parameter (via strtok())
        */
        void setOptions(char* options);

        DictionaryManager dictionaryManager;
    }; // class MoleculeNameParser
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_name_parser__
