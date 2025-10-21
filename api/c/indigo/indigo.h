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

#ifndef __indigo__
#define __indigo__

#include <stdint.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#define qword unsigned __int64
#else
#define qword unsigned long long
#endif

#ifndef EXPORT_SYMBOL
#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#elif (defined __GNUC__ || defined __APPLE__)
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#define EXPORT_SYMBOL
#endif
#endif

#ifndef CEXPORT
#ifndef __cplusplus
#define CEXPORT EXPORT_SYMBOL
#else
#define CEXPORT extern "C" EXPORT_SYMBOL
#endif
#endif

#ifndef __byte_typedef__
#define __byte_typedef__
typedef unsigned char byte;
#endif

/* All integer and float functions return -1 on error. */
/* All string functions return zero pointer on error. */

/* Almost all string functions return the same pointer on success;
   you should not free() it, but rather strdup() it if you want to keep it. */

/* System */

CEXPORT const char* indigoVersion();
CEXPORT const char* indigoVersionInfo();

// Allocate a new session. Each session has its own
// set of objects created and options set up.
CEXPORT qword indigoAllocSessionId();
// Switch to another session. The session, if was not allocated
// previously, is allocated automatically and initialized with
// empty set of objects and default options.
CEXPORT void indigoSetSessionId(qword id);
// Release session. The memory used by the released session
// is not freed, but the number will be reused on
// further allocations.
CEXPORT void indigoReleaseSessionId(qword id);

// Get the last error message
CEXPORT const char* indigoGetLastError(void);

typedef void (*INDIGO_ERROR_HANDLER)(const char* message, void* context);
CEXPORT void indigoSetErrorHandler(INDIGO_ERROR_HANDLER handler, void* context);

// Free an object
CEXPORT int indigoFree(int handle);
// Clone an object
CEXPORT int indigoClone(int object);
// Count object currently allocated
CEXPORT int indigoCountReferences(void);

// Deallocate all the objects in the current session
CEXPORT int indigoFreeAllObjects();

/* Options */

CEXPORT int indigoSetOption(const char* name, const char* value);
CEXPORT int indigoSetOptionInt(const char* name, int value);
CEXPORT int indigoSetOptionBool(const char* name, int value);
CEXPORT int indigoSetOptionFloat(const char* name, float value);
CEXPORT int indigoSetOptionColor(const char* name, float r, float g, float b);
CEXPORT int indigoSetOptionXY(const char* name, int x, int y);
CEXPORT int indigoResetOptions();

CEXPORT const char* indigoGetOption(const char* name);
CEXPORT int indigoGetOptionInt(const char* name, int* value);
CEXPORT int indigoGetOptionBool(const char* name, int* value);
CEXPORT int indigoGetOptionFloat(const char* name, float* value);
CEXPORT int indigoGetOptionColor(const char* name, float* r, float* g, float* b);
CEXPORT int indigoGetOptionXY(const char* name, int* x, int* y);

CEXPORT const char* indigoGetOptionType(const char* name);

/* Basic input-output */

// indigoRead*** return a new reader object.
// indigoLoad*** return a new reader object which already
// contains all the data and does not depend on the given
// string/buffer. All these functions are low-level and
// rarely needed to anyone.

CEXPORT int indigoReadFile(const char* filename);
CEXPORT int indigoReadString(const char* str);
CEXPORT int indigoLoadString(const char* str);
CEXPORT int indigoReadBuffer(const char* buffer, int size);
CEXPORT int indigoLoadBuffer(const char* buffer, int size);

// indigoWrite*** return a new writer object.

CEXPORT int indigoWriteFile(const char* filename);
CEXPORT int indigoWriteBuffer(void);

// Closes the file output stream but does not delete the object
CEXPORT int indigoClose(int output);

/* Iterators */

/* Iterators work in the following way:
 *
 * int item, iter = indigoIterate***(...)
 *
 * if (iter == -1)
 * {
 *    fprintf(stderr, "%s", indigoGetLastError());
 *    return;
 * }
 *
 * while (item = indigoNext(iter))
 * {
 *    if (item == -1)
 *    {
 *       fprintf(stderr, "%s", indigoGetLastError());
 *       break;
 *    }
 *
 *    printf("on item #%d\n", indigoIndex(item));
 *
 *    // do something with item
 *
 *    indigoFree(item);
 * }
 * indigoFree(iter);
 */

// Obtains the next element, returns zero if there is no next element
CEXPORT int indigoNext(int iter);
// Does not obtain the next element, just tells if there is one
CEXPORT int indigoHasNext(int iter);
// Returns the index of the element
CEXPORT int indigoIndex(int item);

// Removes the item from its container (usually a molecule)
CEXPORT int indigoRemove(int item);

/* Molecules, query molecules, SMARTS */

CEXPORT const char* indigoGetOriginalFormat(int item);

CEXPORT int indigoCreateMolecule(void);
CEXPORT int indigoCreateQueryMolecule(void);

CEXPORT int indigoLoadStructureFromString(const char* string, const char* params);
CEXPORT int indigoLoadStructureFromBuffer(const byte* string, int bufferSize, const char* params);
CEXPORT int indigoLoadStructureFromFile(const char* filename, const char* params);

CEXPORT int indigoLoadMoleculeWithLib(int source, int monomer_library);
CEXPORT int indigoLoadMoleculeWithLibFromString(const char* string, int monomer_library);
CEXPORT int indigoLoadMoleculeWithLibFromFile(const char* filename, int monomer_library);
CEXPORT int indigoLoadMoleculeWithLibFromBuffer(const char* buffer, int size, int monomer_library);

CEXPORT int indigoLoadMolecule(int source);
CEXPORT int indigoLoadMoleculeFromString(const char* string);
CEXPORT int indigoLoadMoleculeFromFile(const char* filename);
CEXPORT int indigoLoadMoleculeFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadQueryMoleculeWithLib(int source, int monomer_library);
CEXPORT int indigoLoadQueryMoleculeWithLibFromString(const char* string, int monomer_library);
CEXPORT int indigoLoadQueryMoleculeWithLibFromFile(const char* filename, int monomer_library);
CEXPORT int indigoLoadQueryMoleculeWithLibFromBuffer(const char* buffer, int size, int monomer_library);

CEXPORT int indigoLoadQueryMolecule(int source);
CEXPORT int indigoLoadQueryMoleculeFromString(const char* string);
CEXPORT int indigoLoadQueryMoleculeFromFile(const char* filename);
CEXPORT int indigoLoadQueryMoleculeFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadSmarts(int source);
CEXPORT int indigoLoadSmartsFromString(const char* string);
CEXPORT int indigoLoadSmartsFromFile(const char* filename);
CEXPORT int indigoLoadSmartsFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadMonomerLibrary(int source);
CEXPORT int indigoLoadMonomerLibraryFromString(const char* string);
CEXPORT int indigoLoadMonomerLibraryFromFile(const char* filename);
CEXPORT int indigoLoadMonomerLibraryFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadKetDocument(int source);
CEXPORT int indigoLoadKetDocumentFromString(const char* string);
CEXPORT int indigoLoadKetDocumentFromFile(const char* filename);
CEXPORT int indigoLoadKetDocumentFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadSequence(int source, const char* seq_type, int library);
CEXPORT int indigoLoadSequenceFromString(const char* string, const char* seq_type, int library);
CEXPORT int indigoLoadSequenceFromFile(const char* filename, const char* seq_type, int library);

CEXPORT int indigoLoadFasta(int source, const char* seq_type, int library);
CEXPORT int indigoLoadFastaFromString(const char* string, const char* seq_type, int library);
CEXPORT int indigoLoadFastaFromFile(const char* filename, const char* seq_type, int library);

CEXPORT int indigoLoadIdt(int source, int library);
CEXPORT int indigoLoadIdtFromString(const char* string, int library);
CEXPORT int indigoLoadIdtFromFile(const char* filename, int library);

CEXPORT int indigoLoadHelm(int source, int library);
CEXPORT int indigoLoadHelmFromString(const char* string, int library);
CEXPORT int indigoLoadHelmFromFile(const char* filename, int library);

CEXPORT int indigoLoadAxoLabs(int source, int library);
CEXPORT int indigoLoadAxoLabsFromString(const char* string, int library);
CEXPORT int indigoLoadAxoLabsFromFile(const char* filename, int library);

CEXPORT int indigoSaveMolfile(int molecule, int output);
CEXPORT int indigoSaveMolfileToFile(int molecule, const char* filename);
CEXPORT const char* indigoMolfile(int molecule);

CEXPORT int indigoSaveSequence(int molecule, int output, int library);
CEXPORT int indigoSaveSequenceToFile(int molecule, const char* filename, int library);
CEXPORT const char* indigoSequence(int molecule, int library);

CEXPORT int indigoSaveSequence3Letter(int molecule, int output, int library);
CEXPORT int indigoSaveSequence3LetterToFile(int molecule, const char* filename, int library);
CEXPORT const char* indigoSequence3Letter(int molecule, int library);

CEXPORT int indigoSaveFasta(int molecule, int output, int library);
CEXPORT int indigoSaveFastaToFile(int molecule, const char* filename, int library);
CEXPORT const char* indigoFasta(int molecule, int library);

CEXPORT int indigoSaveIdt(int molecule, int output, int library);
CEXPORT int indigoSaveIdtToFile(int molecule, const char* filename, int library);
CEXPORT const char* indigoIdt(int molecule, int library);

CEXPORT int indigoSaveHelm(int molecule, int output, int library);
CEXPORT int indigoSaveHelmToFile(int molecule, const char* filename, int library);
CEXPORT const char* indigoHelm(int molecule, int library);

CEXPORT int indigoSaveAxoLabs(int molecule, int output, int library);
CEXPORT int indigoSaveAxoLabsToFile(int molecule, const char* filename, int library);
CEXPORT const char* indigoAxoLabs(int molecule, int library);

CEXPORT int indigoSaveMonomerLibrary(int output, int library);
CEXPORT int indigoSaveMonomerLibraryToFile(const char* filename, int library);
CEXPORT const char* indigoMonomerLibrary(int library);

CEXPORT int indigoSaveJsonToFile(int item, const char* filename);
CEXPORT int indigoSaveJson(int item, int output);

CEXPORT int indigoExpandMonomers(int item);

// accepts molecules and reactions (but not query ones)
CEXPORT int indigoSaveCml(int object, int output);
CEXPORT int indigoSaveCmlToFile(int object, const char* filename);
CEXPORT const char* indigoCml(int object);
CEXPORT const char* indigoCdxBase64(int object);

// accepts molecules and reactions
CEXPORT int indigoSaveCdxml(int object, int output);
CEXPORT int indigoSaveCdx(int item, int output);

CEXPORT const char* indigoCdxml(int item);

CEXPORT int indigoSaveCdxmlToFile(int object, const char* filename);
CEXPORT int indigoSaveCdxToFile(int item, const char* filename);

CEXPORT const char* indigoCdxml(int object);

// the output must be a file or a buffer, but not a string
// (because MDLCT data usually contains zeroes)
CEXPORT int indigoSaveMDLCT(int item, int output);

CEXPORT const char* indigoJson(int object);

/*
Converts a chemical name into a corresponding structure
Returns -1 if parsing fails or no structure is found
Parameters:
   name - a name to parse
   params - a string containing parsing options or nullptr if no options are changed
*/
CEXPORT int indigoNameToStructure(const char* name, const char* params);

/* Reactions, query reactions */
/*
 * Reaction centers
 */
enum
{
    INDIGO_RC_NOT_CENTER = -1,
    INDIGO_RC_UNMARKED = 0,
    INDIGO_RC_CENTER = 1,
    INDIGO_RC_UNCHANGED = 2,
    INDIGO_RC_MADE_OR_BROKEN = 4,
    INDIGO_RC_ORDER_CHANGED = 8
};
CEXPORT int indigoLoadReaction(int source);
CEXPORT int indigoLoadReactionFromString(const char* string);
CEXPORT int indigoLoadReactionFromFile(const char* filename);
CEXPORT int indigoLoadReactionFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadReactionWithLib(int source, int monomer_library);
CEXPORT int indigoLoadReactionWithLibFromString(const char* string, int monomer_library);
CEXPORT int indigoLoadReactionWithLibFromFile(const char* filename, int monomer_library);
CEXPORT int indigoLoadReactionWithLibFromBuffer(const char* buffer, int size, int monomer_library);

CEXPORT int indigoLoadQueryReaction(int source);
CEXPORT int indigoLoadQueryReactionFromString(const char* string);
CEXPORT int indigoLoadQueryReactionFromFile(const char* filename);
CEXPORT int indigoLoadQueryReactionFromBuffer(const char* buffer, int size);

CEXPORT int indigoLoadQueryReactionWithLib(int source, int monomer_library);
CEXPORT int indigoLoadQueryReactionWithLibFromString(const char* string, int monomer_library);
CEXPORT int indigoLoadQueryReactionWithLibFromFile(const char* filename, int monomer_library);
CEXPORT int indigoLoadQueryReactionWithLibFromBuffer(const char* buffer, int size, int monomer_library);

CEXPORT int indigoLoadReactionSmarts(int source);
CEXPORT int indigoLoadReactionSmartsFromString(const char* string);
CEXPORT int indigoLoadReactionSmartsFromFile(const char* filename);
CEXPORT int indigoLoadReactionSmartsFromBuffer(const char* buffer, int size);

CEXPORT int indigoCreateReaction(void);
CEXPORT int indigoCreateQueryReaction(void);

CEXPORT int indigoAddReactant(int reaction, int molecule);
CEXPORT int indigoAddProduct(int reaction, int molecule);
CEXPORT int indigoAddCatalyst(int reaction, int molecule);

CEXPORT int indigoCountReactants(int reaction);
CEXPORT int indigoCountProducts(int reaction);
CEXPORT int indigoCountCatalysts(int reaction);
// Counts reactants, products, and catalysts.
CEXPORT int indigoCountMolecules(int reaction);
CEXPORT int indigoGetMolecule(int reaction, int index);

CEXPORT int indigoIterateReactants(int reaction);
CEXPORT int indigoIterateProducts(int reaction);
CEXPORT int indigoIterateCatalysts(int reaction);
// Returns an iterator for reactants, products, and catalysts.
CEXPORT int indigoIterateMolecules(int reaction);
CEXPORT int indigoIterateReactions(int reaction);

CEXPORT int indigoSaveRxnfile(int reaction, int output);
CEXPORT int indigoSaveRxnfileToFile(int reaction, const char* filename);
CEXPORT const char* indigoRxnfile(int reaction);

// Method for query optimizations for faster substructure search
// (works for both query molecules and query reactions)
CEXPORT int indigoOptimize(int query, const char* options);

// Methods for structure normalization
// It neutrailzes charges, resolves 5-valence Nitrogen, removes hydrogens and etc.
// Default options is empty.
CEXPORT int indigoNormalize(int structure, const char* options);

// Method for molecule and query standardizing
// It stadrdize charges, stereo and etc.
CEXPORT int indigoStandardize(int item);

// Method for structure ionization at specified pH and pH tollerance
CEXPORT int indigoIonize(int item, float pH, float pH_toll);

// Method for building PKA model
CEXPORT int indigoBuildPkaModel(int max_level, float threshold, const char* filename);

CEXPORT float* indigoGetAcidPkaValue(int item, int atom, int level, int min_level);
CEXPORT float* indigoGetBasicPkaValue(int item, int atom, int level, int min_level);

// Automatic reaction atom-to-atom mapping
// mode is one of the following (separated by a space):
//    "discard" : discards the existing mapping entirely and considers only
//                the existing reaction centers (the default)
//    "keep"    : keeps the existing mapping and maps unmapped atoms
//    "alter"   : alters the existing mapping, and maps the rest of the
//                reaction but may change the existing mapping
//    "clear"   : removes the mapping from the reaction.
//
//    "ignore_charges" : do not consider atom charges while searching
//    "ignore_isotopes" : do not consider atom isotopes while searching
//    "ignore_valence" : do not consider atom valence while searching
//    "ignore_radicals" : do not consider atom radicals while searching
CEXPORT int indigoAutomap(int reaction, const char* mode);

// Returns mapping number. It might appear that there is more them
// one atom with the same number in AAM
// Value 0 means no mapping number has been specified.
CEXPORT int indigoGetAtomMappingNumber(int reaction, int reaction_atom);
CEXPORT int indigoSetAtomMappingNumber(int reaction, int reaction_atom, int number);

// Getters and setters for reacting centers
CEXPORT int indigoGetReactingCenter(int reaction, int reaction_bond, int* rc);
CEXPORT int indigoSetReactingCenter(int reaction, int reaction_bond, int rc);

// Clears all reaction AAM information
CEXPORT int indigoClearAAM(int reaction);

// Corrects reacting centers according to AAM
CEXPORT int indigoCorrectReactingCenters(int reaction);

/* Accessing a molecule */

enum
{
    INDIGO_ABS = 1,
    INDIGO_OR = 2,
    INDIGO_AND = 3,
    INDIGO_EITHER = 4,
    INDIGO_UP = 5,
    INDIGO_DOWN = 6,
    INDIGO_CIS = 7,
    INDIGO_TRANS = 8,
    INDIGO_CHAIN = 9,
    INDIGO_RING = 10,
    INDIGO_ALLENE = 11,

    INDIGO_SINGLET = 101,
    INDIGO_DOUBLET = 102,
    INDIGO_TRIPLET = 103,
};

// Returns an iterator for all atoms of the given
// molecule, including r-sites and pseudoatoms.
CEXPORT int indigoIterateAtoms(int molecule);
CEXPORT int indigoIteratePseudoatoms(int molecule);
CEXPORT int indigoIterateRSites(int molecule);
CEXPORT int indigoIterateStereocenters(int molecule);
CEXPORT int indigoIterateAlleneCenters(int molecule);
CEXPORT int indigoIterateRGroups(int molecule);

CEXPORT int indigoCountRGroups(int molecule);
CEXPORT int indigoCopyRGroups(int molecule_from, int molecule_to);

CEXPORT int indigoIsPseudoatom(int atom);
CEXPORT int indigoIsRSite(int atom);
CEXPORT int indigoIsTemplateAtom(int atom);

// returns INDIGO_{ABS,OR,AND,EITHER}
// or zero if the atom is not a stereoatom
CEXPORT int indigoStereocenterType(int atom);
CEXPORT int indigoChangeStereocenterType(int atom, int type);

CEXPORT int indigoStereocenterGroup(int atom);
CEXPORT int indigoSetStereocenterGroup(int atom, int group);

// returns 4 integers with atom indices that defines stereocenter pyramid
CEXPORT const int* indigoStereocenterPyramid(int atom);

CEXPORT int indigoSingleAllowedRGroup(int rsite);

CEXPORT int indigoAddStereocenter(int atom, int type, int v1, int v2, int v3, int v4);

// Applicable to an R-Group, but not to a molecule
CEXPORT int indigoIterateRGroupFragments(int rgroup);
// Applicable to an R-Group and to a molecule
// Returns maximal order of attachment points
CEXPORT int indigoCountAttachmentPoints(int item);
CEXPORT int indigoIterateAttachmentPoints(int item, int order);

CEXPORT const char* indigoSymbol(int atom);
CEXPORT int indigoDegree(int atom);

// Returns zero if the charge is ambiguous
// If the charge is nonambiguous, returns 1 and writes *charge
CEXPORT int indigoGetCharge(int atom, int* charge);
// Same as indigoGetCharge
CEXPORT int indigoGetExplicitValence(int atom, int* valence);

CEXPORT int indigoSetExplicitValence(int atom, int valence);

// Returns a number of element from the periodic table.
// Returns zero on ambiguous atom.
// Can not be applied to pseudo-atoms and R-sites.
CEXPORT int indigoAtomicNumber(int atom);
// Returns zero on unspecified or ambiguous isotope
CEXPORT int indigoIsotope(int atom);
// Not applicable to query molecules.
CEXPORT int indigoValence(int atom);
// Return atom hybridization
// S = 1,
// SP = 2,
// SP2 = 3,
// SP3 = 4,
// SP3D = 5,
// SP3D2 = 6,
// SP3D3 = 7,
// SP3D4 = 8,
// SP2D = 9
CEXPORT int indigoGetHybridization(int atom);
// Returns zero if valence of the atom is wrong
CEXPORT int indigoCheckValence(int atom);

// Returns one if atom or bond belongs Query or has any query feature
CEXPORT int indigoCheckQuery(int item);

// Returns one if structure contains RGroup features (RSites, RGroups or attachment points
CEXPORT int indigoCheckRGroups(int item);

// Return atom index
CEXPORT int indigoAtomIndex(int atom);
// Return bond index
CEXPORT int indigoBondIndex(int bond);
// Return atom index begining a bond
CEXPORT int indigoBondBegin(int bond);
// Return atom index ending a bond
CEXPORT int indigoBondEnd(int bond);

// Returns check result for Indigo object as text file for requested properties as JSON
CEXPORT const char* indigoCheck(const char* item, const char* check_flags, const char* load_params);

// Returns check result for Indigo object for requested properties as JSON
CEXPORT const char* indigoCheckObj(int item, const char* check_flags);

// Returns check result for structure against requested properties
CEXPORT const char* indigoCheckStructure(const char* structure, const char* props);

// Applicable to atoms, query atoms, and molecules. Can fail
// (return zero) on query atoms where the number of hydrogens
// is not definitely known. Otherwise, returns one and writes *hydro.
CEXPORT int indigoCountHydrogens(int item, int* hydro);

// Applicable to non-query molecules and atoms.
CEXPORT int indigoCountImplicitHydrogens(int item);

// Calculate macromolecule properties. Return Json string with properties.
CEXPORT const char* indigoMacroProperties(int object, float upc, float nac);

// On success, returns always the same pointer to a 3-element array;
// you should not free() it, but rather memcpy() it if you want to keep it.
CEXPORT float* indigoXYZ(int atom);

CEXPORT int indigoSetXYZ(int atom, float x, float y, float z);

CEXPORT int indigoClearXYZ(int molecule);
CEXPORT int indigoCountSuperatoms(int molecule);
CEXPORT int indigoCountDataSGroups(int molecule);
CEXPORT int indigoCountRepeatingUnits(int molecule);
CEXPORT int indigoCountMultipleGroups(int molecule);
CEXPORT int indigoCountGenericSGroups(int molecule);
CEXPORT int indigoIterateDataSGroups(int molecule);
CEXPORT int indigoIterateSuperatoms(int molecule);
CEXPORT int indigoIterateGenericSGroups(int molecule);
CEXPORT int indigoIterateRepeatingUnits(int molecule);
CEXPORT int indigoIterateMultipleGroups(int molecule);

CEXPORT int indigoIterateTGroups(int molecule);
CEXPORT int indigoIterateSGroups(int molecule);

CEXPORT int indigoGetSuperatom(int molecule, int index);
CEXPORT int indigoGetDataSGroup(int molecule, int index);
CEXPORT int indigoGetGenericSGroup(int molecule, int index);
CEXPORT int indigoGetMultipleGroup(int molecule, int index);
CEXPORT int indigoGetRepeatingUnit(int molecule, int index);

CEXPORT const char* indigoDescription(int data_sgroup);
CEXPORT const char* indigoData(int data_sgroup);

CEXPORT int indigoAddDataSGroup(int molecule, int natoms, int* atoms, int nbonds, int* bonds, const char* description, const char* data);

CEXPORT int indigoAddSuperatom(int molecule, int natoms, int* atoms, const char* name);

CEXPORT int indigoSetDataSGroupXY(int sgroup, float x, float y, const char* options);

CEXPORT int indigoSetSGroupData(int sgroup, const char* data);
CEXPORT int indigoSetSGroupCoords(int sgroup, float x, float y);
CEXPORT int indigoSetSGroupDescription(int sgroup, const char* description);
CEXPORT int indigoSetSGroupFieldName(int sgroup, const char* name);
CEXPORT int indigoSetSGroupQueryCode(int sgroup, const char* querycode);
CEXPORT int indigoSetSGroupQueryOper(int sgroup, const char* queryoper);
CEXPORT int indigoSetSGroupDisplay(int sgroup, const char* option);
CEXPORT int indigoSetSGroupLocation(int sgroup, const char* option);
CEXPORT int indigoSetSGroupTag(int sgroup, const char* tag);
CEXPORT int indigoSetSGroupTagAlign(int sgroup, int tag_align);
CEXPORT int indigoSetSGroupDataType(int sgroup, const char* type);
CEXPORT int indigoSetSGroupXCoord(int sgroup, float x);
CEXPORT int indigoSetSGroupYCoord(int sgroup, float y);

CEXPORT int indigoCreateSGroup(const char* type, int mapping, const char* name);
CEXPORT const char* indigoGetSGroupClass(int sgroup);
CEXPORT const char* indigoGetSGroupName(int sgroup);
CEXPORT int indigoSetSGroupClass(int sgroup, const char* sgclass);
CEXPORT int indigoSetSGroupName(int sgroup, const char* sgname);
CEXPORT int indigoGetSGroupNumCrossBonds(int sgroup);

CEXPORT int indigoAddSGroupAttachmentPoint(int sgroup, int aidx, int lvidx, const char* apid);
CEXPORT int indigoDeleteSGroupAttachmentPoint(int sgroup, int index);
CEXPORT int indigoGetSGroupDisplayOption(int sgroup);
CEXPORT int indigoSetSGroupDisplayOption(int sgroup, int option);
CEXPORT int indigoGetSGroupSeqId(int sgroup);
CEXPORT float* indigoGetSGroupCoords(int sgroup);

CEXPORT int indigoGetSGroupMultiplier(int sgroup);
CEXPORT int indigoSetSGroupMultiplier(int sgroup, int multiplier);

CEXPORT const char* indigoGetRepeatingUnitSubscript(int sgroup);
CEXPORT int indigoGetRepeatingUnitConnectivity(int sgroup);

CEXPORT int indigoSetSGroupBrackets(int sgroup, int brk_style, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

CEXPORT int indigoFindSGroups(int item, const char* property, const char* value);

CEXPORT int indigoGetSGroupType(int item);
CEXPORT int indigoGetSGroupIndex(int item);

CEXPORT int indigoGetSGroupOriginalId(int sgroup);
CEXPORT int indigoSetSGroupOriginalId(int sgroup, int original);
CEXPORT int indigoGetSGroupParentId(int sgroup);
CEXPORT int indigoSetSGroupParentId(int sgroup, int parent);

CEXPORT int indigoAddTemplate(int molecule, int templates, const char* tname);
CEXPORT int indigoRemoveTemplate(int molecule, const char* tname);
CEXPORT int indigoFindTemplate(int molecule, const char* tname);

CEXPORT const char* indigoGetTGroupClass(int tgroup);
CEXPORT const char* indigoGetTGroupName(int tgroup);
CEXPORT const char* indigoGetTGroupAlias(int tgroup);

CEXPORT int indigoTransformSCSRtoCTAB(int item);
CEXPORT int indigoTransformCTABtoSCSR(int molecule, int templates);

CEXPORT int indigoResetCharge(int atom);
CEXPORT int indigoResetExplicitValence(int atom);
CEXPORT int indigoResetIsotope(int atom);

CEXPORT int indigoSetAttachmentPoint(int atom, int order);
CEXPORT int indigoClearAttachmentPoints(int item);

CEXPORT int indigoRemoveConstraints(int item, const char* type);
CEXPORT int indigoAddConstraint(int item, const char* type, const char* value);
CEXPORT int indigoAddConstraintNot(int item, const char* type, const char* value);
CEXPORT int indigoAddConstraintOr(int atom, const char* type, const char* value);

CEXPORT int indigoResetStereo(int item);
CEXPORT int indigoInvertStereo(int item);

CEXPORT int indigoCountAtoms(int molecule);
CEXPORT int indigoCountBonds(int molecule);
CEXPORT int indigoCountPseudoatoms(int molecule);
CEXPORT int indigoCountRSites(int molecule);

CEXPORT int indigoIterateBonds(int molecule);
// Returns 1/2/3 if the bond is a single/double/triple bond
// Returns 4 if the bond is an aromatic bond
// Returns zero if the bond is ambiguous (query bond)
CEXPORT int indigoBondOrder(int bond);

// Returns INDIGO_{UP/DOWN/EITHER/CIS/TRANS},
// or zero if the bond is not a stereobond
CEXPORT int indigoBondStereo(int bond);

// Returns INDIGO_{CHAIN/RING},
CEXPORT int indigoTopology(int bond);

// Returns an iterator whose elements can be treated as atoms.
// At the same time, they support indigoBond() call.
CEXPORT int indigoIterateNeighbors(int atom);

// Applicable exclusively to the "atom neighbors iterator".
// Returns a bond to the neighbor atom.
CEXPORT int indigoBond(int nei);

// Accessing atoms and bonds by index
CEXPORT int indigoGetAtom(int molecule, int idx);
CEXPORT int indigoGetBond(int molecule, int idx);

CEXPORT int indigoSource(int bond);
CEXPORT int indigoDestination(int bond);

CEXPORT int indigoClearCisTrans(int handle);
CEXPORT int indigoClearStereocenters(int handle);
CEXPORT int indigoCountStereocenters(int molecule);
CEXPORT int indigoClearAlleneCenters(int molecule);
CEXPORT int indigoCountAlleneCenters(int molecule);

CEXPORT int indigoResetSymmetricCisTrans(int handle);
CEXPORT int indigoResetSymmetricStereocenters(int handle);
CEXPORT int indigoMarkEitherCisTrans(int handle);
CEXPORT int indigoMarkStereobonds(int handle);

CEXPORT int indigoValidateChirality(int handle);

// Return CIP descriptor of an atom
CEXPORT int indigoStereocenterCIPDescriptor(int atom);
// Adds CIP descriptors to a molecule
CEXPORT int indigoAddCIPStereoDescriptors(int molecule);

// Accepts a symbol from the periodic table (like "C" or "Br"),
// or a pseudoatom symbol, like "Pol". Returns the added atom.
CEXPORT int indigoAddAtom(int molecule, const char* symbol);
// Set a new atom instead of specified
CEXPORT int indigoResetAtom(int atom, const char* symbol);

CEXPORT const char* indigoGetTemplateAtomClass(int atom);
CEXPORT int indigoSetTemplateAtomClass(int atom, const char* name);

// Accepts Rsite name "R" (or just ""), "R1", "R2" or list with names "R1 R3"
CEXPORT int indigoAddRSite(int molecule, const char* name);
CEXPORT int indigoSetRSite(int atom, const char* name);

CEXPORT int indigoSetCharge(int atom, int charge);
CEXPORT int indigoSetIsotope(int atom, int isotope);

// If the radical is nonambiguous, returns 1 and writes *electrons
CEXPORT int indigoGetRadicalElectrons(int atom, int* electrons);
// If the radical is nonambiguous, returns 1 and writes *radical
CEXPORT int indigoGetRadical(int atom, int* radical);
CEXPORT int indigoSetRadical(int atom, int radical);
CEXPORT int indigoResetRadical(int atom);

// Used for hacks with aromatic molecules; not recommended to use
// in other situations
CEXPORT int indigoSetImplicitHCount(int atom, int impl_h);

// Accepts two atoms (source and destination) and the order of the new bond
// (1/2/3/4 = single/double/triple/aromatic). Returns the added bond.
CEXPORT int indigoAddBond(int source, int destination, int order);

CEXPORT int indigoSetBondOrder(int bond, int order);

CEXPORT int indigoMerge(int where_to, int what);

/* Highlighting */

// Access atoms and bonds
CEXPORT int indigoHighlight(int item);

// Access atoms, bonds, molecules, and reactions
CEXPORT int indigoUnhighlight(int item);

// Access atoms and bonds
CEXPORT int indigoIsHighlighted(int item);

/* Selection */

// Access atoms and bonds
CEXPORT int indigoSelect(int item);

// Access atoms, bonds, molecules, and reactions
CEXPORT int indigoUnselect(int item);

// Access atoms and bonds
CEXPORT int indigoIsSelected(int item);

// Molecule or reaction
CEXPORT int indigoHasSelection(int item);

/* Connected components of molecules */

CEXPORT int indigoCountComponents(int molecule);
CEXPORT int indigoComponentIndex(int atom);
CEXPORT int indigoIterateComponents(int molecule);

// Returns a 'molecule component' object, which can not be used as a
// [query] molecule, but supports the indigo{Count,Iterate}{Atoms,Bonds} calls,
// and also the indigoClone() call, which returns a [query] molecule.
CEXPORT int indigoComponent(int molecule, int index);

/* Smallest Set of Smallest Rings */

CEXPORT int indigoCountSSSR(int molecule);
CEXPORT int indigoIterateSSSR(int molecule);

CEXPORT int indigoIterateSubtrees(int molecule, int min_atoms, int max_atoms);
CEXPORT int indigoIterateRings(int molecule, int min_atoms, int max_atoms);
CEXPORT int indigoIterateEdgeSubmolecules(int molecule, int min_bonds, int max_bonds);

/* Calculation on molecules */

CEXPORT int indigoCountHeavyAtoms(int molecule);
CEXPORT int indigoGrossFormula(int molecule);
CEXPORT int indigoMolecularFormula(int molecule);
CEXPORT double indigoMolecularWeight(int molecule);
CEXPORT double indigoMostAbundantMass(int molecule);
CEXPORT double indigoMonoisotopicMass(int molecule);
CEXPORT const char* indigoMassComposition(int molecule);
CEXPORT double indigoTPSA(int molecule, int includeSP);
CEXPORT int indigoNumRotatableBonds(int molecule);
CEXPORT int indigoNumHydrogenBondAcceptors(int molecule);
CEXPORT int indigoNumHydrogenBondDonors(int molecule);
CEXPORT double indigoLogP(int molecule);
CEXPORT double indigoMolarRefractivity(int molecule);
CEXPORT double indigoPka(int molecule);
CEXPORT const char* indigoPkaValues(int molecule);

CEXPORT const char* indigoCanonicalSmiles(int molecule);
CEXPORT const char* indigoLayeredCode(int molecule);

CEXPORT int64_t indigoHash(int chemicalObject);

CEXPORT const int* indigoSymmetryClasses(int molecule, int* count_out);

CEXPORT int indigoHasCoord(int molecule);
CEXPORT int indigoHasZCoord(int molecule);
CEXPORT int indigoIsChiral(int molecule);
CEXPORT int indigoCheckChirality(int molecule);
CEXPORT int indigoCheck3DStereo(int molecule);
CEXPORT int indigoCheckStereo(int molecule);

CEXPORT int indigoIsPossibleFischerProjection(int molecule, const char* options);

CEXPORT int indigoCreateSubmolecule(int molecule, int nvertices, int* vertices);
CEXPORT int indigoCreateEdgeSubmolecule(int molecule, int nvertices, int* vertices, int nedges, int* edges);

CEXPORT int indigoGetSubmolecule(int molecule, int nvertices, int* vertices);

CEXPORT int indigoRemoveAtoms(int molecule, int nvertices, int* vertices);
CEXPORT int indigoRemoveBonds(int molecule, int nbonds, int* bonds);

// Determines and applies the best transformation to the given molecule
// so that the specified atoms move as close as possible to the desired
// positions. The size of desired_xyz is equal to 3 * natoms.
// The return value is the root-mean-square measure of the difference
// between the desired and obtained positions.
CEXPORT float indigoAlignAtoms(int molecule, int natoms, int* atom_ids, float* desired_xyz);

/* Things that work for both molecules and reactions */

CEXPORT int indigoAromatize(int item);
CEXPORT int indigoDearomatize(int item);

CEXPORT int indigoFoldHydrogens(int item);
CEXPORT int indigoUnfoldHydrogens(int item);
CEXPORT int indigoFoldUnfoldHydrogens(int item);

CEXPORT int indigoLayout(int object);
CEXPORT int indigoLayoutSelected(int object);
CEXPORT int indigoClean2d(int object);

CEXPORT const char* indigoSmiles(int item);
CEXPORT const char* indigoSmarts(int item);
CEXPORT const char* indigoCanonicalSmarts(int item);

// Returns a "mapping" if there is an exact match, zero otherwise
// The flags string consists of space-separated flags.
// The more flags, the more restrictive matching is done.
// "ELE": Distribution of electrons: bond types, atom charges, radicals, valences
// "MAS": Atom isotopes
// "STE": Stereochemistry: chiral centers, stereogroups, and cis-trans bonds
// "FRA": Connected fragments: disallows match of separate ions in salts
// "ALL": All of the above
// By default (with null or empty flags string) all flags are on.
CEXPORT int indigoExactMatch(int item1, int item2, const char* flags);

// "beg" and "end" refer to the two ends of the tautomeric chain. Allowed
// elements are separated by commas. '1' at the beginning means an aromatic
// atom, while '0' means an aliphatic atom.
CEXPORT int indigoSetTautomerRule(int id, const char* beg, const char* end);

CEXPORT int indigoRemoveTautomerRule(int id);

CEXPORT int indigoClearTautomerRules();

CEXPORT const char* indigoName(int handle);
CEXPORT int indigoSetName(int handle, const char* name);

// You should not free() the obtained buffer, but rather memcpy() it if you want to keep it
CEXPORT int indigoSerialize(int handle, byte** buf, int* size);

CEXPORT int indigoUnserialize(const byte* buf, int size);

// Applicable to molecules/reactions obtained from SDF or RDF files,
// and to their clones, and to their R-Group deconvolutions.
CEXPORT int indigoHasProperty(int handle, const char* prop);
CEXPORT const char* indigoGetProperty(int handle, const char* prop);

// Applicable to newly created or cloned molecules/reactions,
// and also to molecules/reactions obtained from SDF or RDF files.
// If the property with the given name does not exist, it is created automatically.
CEXPORT int indigoSetProperty(int item, const char* prop, const char* value);

// Does not raise an error if the given property does not exist
CEXPORT int indigoRemoveProperty(int item, const char* prop);

// Returns an iterator that one can pass to indigoName() to
// know the name of the property. The value of the property can be
// obtained via indigoGetProperty() call to the object
CEXPORT int indigoIterateProperties(int handle);

// Clears all properties of the molecule
CEXPORT int indigoClearProperties(int handle);

// Accepts a molecule or reaction (but not query molecule or query reaction).
// Returns a string describing the first encountered mistake with valence.
// Returns an empty string if the input molecule/reaction is fine.
CEXPORT const char* indigoCheckBadValence(int handle);

// Accepts a molecule or reaction (but not query molecule or query reaction).
// Returns a string describing the first encountered mistake with ambiguous H counter.
// Returns an empty string if the input molecule/reaction is fine.
CEXPORT const char* indigoCheckAmbiguousH(int handle);

/* Fingerprints */

// Returns a 'fingerprint' object, which can then be passed to:
//   indigoToString() -- to get hexadecimal representation
//   indigoToBuffer() -- to get raw byte data
//   indigoSimilarity() -- to calculate similarity with another fingerprint
// The following fingerprint types are available:
//   "sim"     -- "Similarity fingerprint", useful for calculating
//                 similarity measures (the default)
//   "sub"     -- "Substructure fingerprint", useful for substructure screening
//   "sub-res" -- "Resonance substructure fingerprint", useful for resonance
//                 substructure screening
//   "sub-tau" -- "Tautomer substructure fingerprint", useful for tautomer
//                 substructure screening
//   "full"    -- "Full fingerprint", which has all the mentioned
//                 fingerprint types included
CEXPORT int indigoFingerprint(int item, const char* type);

// Counts the nonzero (i.e. one) bits in a fingerprint
CEXPORT int indigoCountBits(int fingerprint);

// Counts the number of the coinincident in two fingerprints
CEXPORT int indigoCommonBits(int fingerprint1, int fingerprint2);

// Return one bits string for the fingerprint object
CEXPORT const char* indigoOneBitsList(int fingerprint);

// Returns a 'fingerprint' object with data from 'buffer'
CEXPORT int indigoLoadFingerprintFromBuffer(const byte* buffer, int size);

// Constructs a 'fingerprint' object from a normalized array of double descriptors
CEXPORT int indigoLoadFingerprintFromDescriptors(const double* arr, int arr_len, int size, double density);

// Accepts two molecules, two reactions, or two fingerprints.
// Returns the similarity measure between them.
// Metrics: "tanimoto", "tversky", "tversky <alpha> <beta>", "euclid-sub" or "normalized-edit"
// Zero pointer or empty string defaults to "tanimoto".
// "tversky" without numbers defaults to alpha = beta = 0.5
CEXPORT float indigoSimilarity(int item1, int item2, const char* metrics);

/* Working with SDF/RDF/SMILES/CML/CDX files  */

CEXPORT int indigoIterateSDF(int reader);
CEXPORT int indigoIterateRDF(int reader);
CEXPORT int indigoIterateSmiles(int reader);
CEXPORT int indigoIterateCML(int reader);
CEXPORT int indigoIterateCDX(int reader);

CEXPORT int indigoIterateSDFile(const char* filename);
CEXPORT int indigoIterateRDFile(const char* filename);
CEXPORT int indigoIterateSmilesFile(const char* filename);
CEXPORT int indigoIterateCMLFile(const char* filename);
CEXPORT int indigoIterateCDXFile(const char* filename);

// Applicable to items returned by SDF/RDF iterators.
// Returns the content of SDF/RDF item.
CEXPORT const char* indigoRawData(int item);

// Applicable to items returned by SDF/RDF iterators.
// Returns the offset in the SDF/RDF file.
CEXPORT int indigoTell(int handle);
CEXPORT long long indigoTell64(int handle);

// Saves the molecule to an SDF output stream
CEXPORT int indigoSdfAppend(int output, int item);
// Saves the molecule to a multiline SMILES output stream
CEXPORT int indigoSmilesAppend(int output, int item);

// Similarly for RDF files, except that the header should be written first
CEXPORT int indigoRdfHeader(int output);
CEXPORT int indigoRdfAppend(int output, int item);

// Similarly for CML files, except that they have both header and footer
CEXPORT int indigoCmlHeader(int output);
CEXPORT int indigoCmlAppend(int output, int item);
CEXPORT int indigoCmlFooter(int output);

// Create saver objects that can be used to save molecules or reactions
// Supported formats: 'sdf', 'smi' or 'smiles', 'cml', 'rdf'
// Format argument is case-insensitive
// Saver should be closed with indigoClose function
CEXPORT int indigoCreateSaver(int output, const char* format);
CEXPORT int indigoCreateFileSaver(const char* filename, const char* format);

// Append object to a specified saver stream
CEXPORT int indigoAppend(int saver, int object);

/* Arrays */

CEXPORT int indigoCreateArray();
// Note: a clone of the object is added, not the object itself
CEXPORT int indigoArrayAdd(int arr, int object);
CEXPORT int indigoAt(int item, int index);
CEXPORT int indigoCount(int item);
CEXPORT int indigoClear(int arr);
CEXPORT int indigoIterateArray(int arr);

/* Substructure matching */

// Returns a new 'matcher' object
// 'mode' is reserved for future use; currently its value is ignored
CEXPORT int indigoSubstructureMatcher(int target, const char* mode);

// Ignore target atom in the substructure matcher
CEXPORT int indigoIgnoreAtom(int matcher, int atom_object);

// Ignore target atom in the substructure matcher
CEXPORT int indigoUnignoreAtom(int matcher, int atom_object);

// Clear list of ignored target atoms in the substructure matcher
CEXPORT int indigoUnignoreAllAtoms(int matcher);

// Returns a new 'match' object on success, zero on fail
//    matcher is an matcher object returned by indigoSubstructureMatcher
CEXPORT int indigoMatch(int matcher, int query);

// Counts the number of embeddings of the query structure into the target
CEXPORT int indigoCountMatches(int matcher, int query);

// Counts the number of embeddings of the query structure into the target
// If number of embeddings is more then limit then limit is returned
CEXPORT int indigoCountMatchesWithLimit(int matcher, int query, int embeddings_limit);

// Returns substructure matches iterator
CEXPORT int indigoIterateMatches(int matcher, int query);

// Accepts a 'match' object obtained from indigoMatchSubstructure.
// Returns a new molecule which has the query highlighted.
CEXPORT int indigoHighlightedTarget(int match);

// Accepts an atom from the query, not an atom index.
//   You can use indigoGetAtom() to obtain the atom by its index.
// Returns the corresponding target atom, not an atom index. If query
// atom doesn't match particular atom in the target (R-group or explicit
// hydrogen) then return value is zero.
//   You can use indigoIndex() to obtain the index of the returned atom.
CEXPORT int indigoMapAtom(int handle, int atom);

// Accepts a bond from the query, not a bond index.
//   You can use indigoGetBond() to obtain the bond by its index.
// Returns the corresponding target bond, not a bond index. If query
// bond doesn't match particular bond in the target (R-group or explicit
// hydrogen) then return value is zero.
//   You can use indigoIndex() to obtain the index of the returned bond.
CEXPORT int indigoMapBond(int handle, int bond);

// Accepts a molecule from the query reaction, not a molecule index.
//   You can use indigoGetMolecule() to obtain the bond by its index.
// Returns the corresponding target molecule, not a reaction index. If query
// molecule doesn't match particular molecule in the target then return
// value is zero.
//   You can use indigoIndex() to obtain the index of the returned molecule.
CEXPORT int indigoMapMolecule(int handle, int molecule);

// Accepts a molecule and options for tautomer enumeration algorithms
// Returns an iterator object over the molecules that are tautomers of this molecule.
CEXPORT int indigoIterateTautomers(int molecule, const char* options);

/* Scaffold detection */

// Returns zero if no common substructure is found.
// Otherwise, it returns a new object, which can be
//   (i) treated as a structure: the maximum (by the number of rings) common
//       substructure of the given structures.
//  (ii) passed to indigoAllScaffolds()
CEXPORT int indigoExtractCommonScaffold(int structures, const char* options);

// Returns an array of all possible scaffolds.
// The input parameter is the value returned by indigoExtractCommonScaffold().
CEXPORT int indigoAllScaffolds(int extracted);

/* R-Group deconvolution */

// Returns a ``decomposition'' object that can be passed to
// indigoDecomposedMoleculeScaffold() and
// indigoIterateDecomposedMolecules()
CEXPORT int indigoDecomposeMolecules(int scaffold, int structures);

// Returns a scaffold molecule with r-sites marking the place
// for substituents to add to form the structures given above.
CEXPORT int indigoDecomposedMoleculeScaffold(int decomp);

// Returns an iterator which corresponds to the given collection of structures.
// indigoDecomposedMoleculeHighlighted() and
// indigoDecomposedMoleculeWithRGroups() are applicable to the
// values returned by the iterator.
CEXPORT int indigoIterateDecomposedMolecules(int decomp);

// Returns a molecule with highlighted scaffold
CEXPORT int indigoDecomposedMoleculeHighlighted(int decomp);

// Returns a query molecule with r-sites and "R1=...", "R2=..."
// substituents defined. The 'scaffold' part of the molecule
// is identical to the indigoDecomposedMoleculeScaffold()
CEXPORT int indigoDecomposedMoleculeWithRGroups(int decomp);

/*
 * Decomposition Iteration API
 */
// Returns a 'decomposition' object
CEXPORT int indigoCreateDecomposer(int scaffold);
// Returns a 'decomposition' item
CEXPORT int indigoDecomposeMolecule(int decomp, int mol);
// Returns decomposition iterator
CEXPORT int indigoIterateDecompositions(int deco_item);
// Adds the input decomposition to a full scaffold
CEXPORT int indigoAddDecomposition(int decomp, int q_match);

/* R-Group convolution */

CEXPORT int indigoGetFragmentedMolecule(int elem, const char* options);
CEXPORT int indigoRGroupComposition(int molecule, const char* options);

/*
 * Abbreviations
 */
CEXPORT int indigoExpandAbbreviations(int molecule);

/* Other */

CEXPORT const char* indigoToString(int handle);
CEXPORT const char* indigoToBase64String(int handle);
CEXPORT int indigoToBuffer(int handle, char** buf, int* size);

/* Reaction products enumeration */

// Accepts a query reaction with markd R-sites, and array of arrays
// of substituents corresponding to the R-Sites. Returns an array of
// reactions with R-Sites replaced by the actual substituents.
CEXPORT int indigoReactionProductEnumerate(int reaction, int monomers);

CEXPORT int indigoTransform(int reaction, int monomers);

CEXPORT int indigoTransformHELMtoSCSR(int monomer);

/* Debug functionality */

// Returns internal type of an object
CEXPORT const char* indigoDbgInternalType(int object);

// Internal breakpoint
CEXPORT void indigoDbgBreakpoint(void);

// Methods that returns profiling infromation in a human readable format
CEXPORT const char* indigoDbgProfiling(int /*bool*/ whole_session);

// Reset profiling counters either for the current state or for the whole session
CEXPORT int indigoDbgResetProfiling(int /*bool*/ whole_session);

// Methods that returns profiling counter value for a particular counter
CEXPORT qword indigoDbgProfilingGetCounter(const char* name, int /*bool*/ whole_session);

#endif
