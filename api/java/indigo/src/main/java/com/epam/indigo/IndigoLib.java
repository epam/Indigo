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

package com.epam.indigo;

import com.sun.jna.Library;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.FloatByReference;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

public interface IndigoLib extends Library {
    String indigoVersion();

    long indigoAllocSessionId();

    void indigoSetSessionId(long id);

    void indigoReleaseSessionId(long id);

    String indigoGetLastError();

    int indigoFree(int handle);

    int indigoClone(int handle);

    int indigoCountReferences();

    int indigoSetOption(String name, String value);

    int indigoSetOptionInt(String name, int value);

    int indigoSetOptionBool(String name, int value);

    int indigoSetOptionFloat(String name, float value);

    int indigoSetOptionColor(String name, float r, float g, float b);

    int indigoSetOptionXY(String name, int x, int y);

    Pointer indigoGetOption(String name);

    int indigoGetOptionInt(String name, IntByReference value);

    int indigoGetOptionBool(String name, IntByReference value);

    int indigoGetOptionFloat(String name, FloatByReference value);

    Pointer indigoGetOptionType(String name);

    int indigoResetOption();

    int indigoReadFile(String filename);

    int indigoLoadString(String str);

    int indigoLoadBuffer(byte[] buf, int size);

    int indigoWriteFile(String filename);

    int indigoWriteBuffer();

    int indigoClose(int handle);

    int indigoNext(int iter);

    int indigoHasNext(int iter);

    int indigoIndex(int item);

    int indigoRemove(int item);

    int indigoCreateMolecule();

    int indigoCreateQueryMolecule();

    int indigoLoadMolecule(int source);

    int indigoLoadMoleculeFromString(String str);

    int indigoLoadMoleculeFromFile(String filename);

    int indigoLoadMoleculeFromBuffer(byte[] buffer, int size);

    int indigoLoadQueryMolecule(int source);

    int indigoLoadQueryMoleculeFromString(String str);

    int indigoLoadQueryMoleculeFromFile(String filename);

    int indigoLoadQueryMoleculeFromBuffer(byte[] buffer, int size);

    int indigoLoadSmarts(int source);

    int indigoLoadSmartsFromString(String str);

    int indigoLoadSmartsFromFile(String filename);

    int indigoLoadSmartsFromBuffer(byte[] buffer, int size);

    int indigoLoadStructureFromString(String str, String params);

    int indigoLoadStructureFromFile(String filename, String params);

    int indigoLoadStructureFromBuffer(byte[] buffer, int size, String params);

    int indigoSaveMolfile(int molecule, int output);

    int indigoSaveMolfileToFile(int molecule, String filename);

    Pointer indigoMolfile(int molecule);

    int indigoSaveCml(int object, int output);

    int indigoSaveCmlToFile(int object, String filename);

    Pointer indigoCml(int object);

    Pointer indigoJson(int object);

    @SuppressWarnings("checkstyle:Indentation")
    int indigoSaveCdxml(int object, int output);

    int indigoSaveCdxmlToFile(int object, String filename);

    Pointer indigoCdxml(int object);

    int indigoSaveMDLCT(int item, int output);

    int indigoLoadReaction(int source);

    int indigoLoadReactionFromString(String string);

    int indigoLoadReactionFromFile(String filename);

    int indigoLoadReactionFromBuffer(byte[] buf, int size);

    int indigoLoadQueryReaction(int source);

    int indigoLoadQueryReactionFromString(String str);

    int indigoLoadQueryReactionFromFile(String filename);

    int indigoLoadQueryReactionFromBuffer(byte[] buf, int size);

    int indigoLoadReactionSmarts(int source);

    int indigoLoadReactionSmartsFromString(String str);

    int indigoLoadReactionSmartsFromFile(String filename);

    int indigoLoadReactionSmartsFromBuffer(byte[] buf, int size);

    int indigoCreateReaction();

    int indigoCreateQueryReaction();

    int indigoAddReactant(int reaction, int molecule);

    int indigoAddProduct(int reaction, int molecule);

    int indigoAddCatalyst(int reaction, int molecule);

    int indigoCountReactants(int reaction);

    int indigoCountProducts(int reaction);

    int indigoCountCatalysts(int reaction);

    int indigoCountMolecules(int reaction);

    int indigoIterateReactants(int reaction);

    int indigoIterateProducts(int reaction);

    int indigoIterateCatalysts(int reaction);

    int indigoIterateMolecules(int reaction);

    int indigoSaveRxnfile(int reaction, int output);

    int indigoSaveRxnfileToFile(int reaction, String filename);

    Pointer indigoRxnfile(int reaction);

    int indigoOptimize(int query, String options);

    int indigoNormalize(int structure, String options);

    int indigoStandardize(int item);

    int indigoIonize(int item, float pH, float pH_toll);

    Pointer indigoGetAcidPkaValue(int item, int atom, int level, int min_level);

    Pointer indigoGetBasicPkaValue(int item, int atom, int level, int min_level);

    int indigoBuildPkaModel(int level, float theshold, String filename);

    int indigoAutomap(int reaction, String mode);

    int indigoGetAtomMappingNumber(int reaction, int reaction_atom);

    int indigoSetAtomMappingNumber(int reaction, int reaction_atom, int number);

    int indigoClearAAM(int reaction);

    int indigoCorrectReactingCenters(int reaction);

    int indigoIterateAtoms(int molecule);

    int indigoIteratePseudoatoms(int molecule);

    int indigoIterateRSites(int molecule);

    int indigoIterateStereocenters(int molecule);

    int indigoIterateAlleneCenters(int molecule);

    int indigoIterateRGroups(int molecule);

    int indigoIsPseudoatom(int atom);

    int indigoIsRSite(int atom);

    int indigoIsTemplateAtom(int atom);

    int indigoStereocenterType(int atom);

    int indigoStereocenterGroup(int atom);

    Pointer indigoStereocenterPyramid(int atom);

    int indigoSetStereocenterGroup(int atom, int group);

    int indigoChangeStereocenterType(int atom, int type);

    int indigoSingleAllowedRGroup(int rsite);

    int indigoAddStereocenter(int atom, int type, int v1, int v2, int v3, int v4);

    int indigoIterateRGroupFragments(int rgroup);

    int indigoCountRGroups(int molecule);

    int indigoCountAttachmentPoints(int rgroup);

    Pointer indigoSymbol(int atom);

    int indigoDegree(int atom);

    int indigoGetCharge(int atom, IntByReference charge);

    int indigoGetRadical(int atom, IntByReference radical);

    int indigoGetExplicitValence(int atom, IntByReference valence);

    int indigoGetRadicalElectrons(int atom, IntByReference electrons);

    int indigoAtomicNumber(int atom);

    int indigoIsotope(int atom);

    int indigoValence(int atom);

    int indigoCheckValence(int atom);

    int indigoCheckQuery(int item);

    int indigoCheckRGroups(int item);

    int indigoCheckChirality(int item);

    int indigoCheck3DStereo(int item);

    int indigoCheckStereo(int item);

    int indigoCountHydrogens(int atom, IntByReference valence);

    int indigoCountImplicitHydrogens(int item);

    Pointer indigoCheck(String item, String type, String options);

    Pointer indigoCheckObj(int item, String type);

    Pointer indigoCheckStructure(String structure, String type);

    int indigoGetReactingCenter(int reaction, int reaction_bond, IntByReference rc);

    int indigoSetReactingCenter(int reaction, int reaction_bond, int rc);

    Pointer indigoXYZ(int atom);

    int indigoSetXYZ(int atom, float x, float y, float z);

    int indigoCountSuperatoms(int molecule);

    int indigoCountDataSGroups(int molecule);

    int indigoCountRepeatingUnits(int molecule);

    int indigoCountMultipleGroups(int molecule);

    int indigoCountGenericSGroups(int molecule);

    int indigoIterateDataSGroups(int molecule);

    int indigoIterateSuperatoms(int molecule);

    int indigoIterateGenericSGroups(int molecule);

    int indigoIterateRepeatingUnits(int molecule);

    int indigoIterateMultipleGroups(int molecule);

    int indigoIterateSGroups(int molecule);

    int indigoIterateTGroups(int molecule);

    int indigoGetSuperatom(int molecule, int index);

    int indigoGetDataSGroup(int molecule, int index);

    int indigoGetGenericSGroup(int molecule, int index);

    int indigoGetMultipleGroup(int molecule, int index);

    int indigoGetRepeatingUnit(int molecule, int index);

    Pointer indigoDescription(int data_sgroup);

    Pointer indigoData(int data_sgroup);

    int indigoAddDataSGroup(
            int molecule,
            int natoms,
            int[] atoms,
            int nbonds,
            int[] bonds,
            String description,
            String data);

    int indigoSetDataSGroupXY(int sgroup, float x, float y, String options);

    int indigoCreateSGroup(String type, int mapping, String name);

    int indigoSetSGroupClass(int sgroup, String sgclass);

    int indigoSetSGroupName(int sgroup, String sgname);

    Pointer indigoGetSGroupClass(int sgroup);

    Pointer indigoGetSGroupName(int sgroup);

    int indigoGetSGroupNumCrossBonds(int sgroup);

    int indigoAddSGroupAttachmentPoint(int sgroup, int aidx, int lvidx, String apid);

    int indigoDeleteSGroupAttachmentPoint(int sgroup, int apidx);

    int indigoGetSGroupDisplayOption(int sgroup);

    int indigoSetSGroupDisplayOption(int sgroup, int option);

    int indigoGetSGroupSeqId(int sgroup);

    int indigoGetSGroupMultiplier(int sgroup);

    Pointer indigoGetRepeatingUnitSubscript(int sgroup);

    int indigoGetRepeatingUnitConnectivity(int sgroup);

    int indigoSetSGroupMultiplier(int sgroup, int mult);

    int indigoSetSGroupBrackets(
            int sgroup,
            int brk_style,
            float x1,
            float y1,
            float x2,
            float y2,
            float x3,
            float y3,
            float x4,
            float y4);

    Pointer indigoGetSGroupCoords(int sgroup);

    int indigoSetSGroupData(int sgroup, String data);

    int indigoSetSGroupCoords(int sgroup, float x, float y);

    int indigoSetSGroupDescription(int sgroup, String description);

    int indigoSetSGroupFieldName(int sgroup, String name);

    int indigoSetSGroupQueryCode(int sgroup, String querycode);

    int indigoSetSGroupQueryOper(int sgroup, String queryoper);

    int indigoSetSGroupDisplay(int sgroup, String option);

    int indigoSetSGroupLocation(int sgroup, String option);

    int indigoSetSGroupTag(int sgroup, String tag);

    int indigoSetSGroupTagAlign(int sgroup, int tag_align);

    int indigoSetSGroupDataType(int sgroup, String type);

    int indigoSetSGroupXCoord(int sgroup, float x);

    int indigoSetSGroupYCoord(int sgroup, float y);

    int indigoFindSGroups(int molecule, String property, String value);

    int indigoGetSGroupType(int sgroup);

    int indigoGetSGroupIndex(int sgroup);

    int indigoGetSGroupOriginalId(int sgroup);

    int indigoSetSGroupOriginalId(int sgroup, int original);

    int indigoGetSGroupParentId(int sgroup);

    int indigoSetSGroupParentId(int sgroup, int parent);

    int indigoAddTemplate(int molecule, int templates, String name);

    int indigoRemoveTemplate(int molecule, String name);

    int indigoFindTemplate(int molecule, String name);

    Pointer indigoGetTGroupClass(int tgroup);

    Pointer indigoGetTGroupName(int tgroup);

    Pointer indigoGetTGroupAlias(int tgroup);

    int indigoTransformSCSRtoCTAB(int molecule);

    int indigoTransformCTABtoSCSR(int molecule, int templates);

    Pointer indigoGetTemplateAtomClass(int atom);

    int indigoSetTemplateAtomClass(int atom, String name);

    int indigoResetCharge(int atom);

    int indigoResetExplicitValence(int atom);

    int indigoResetRadical(int atom);

    int indigoResetIsotope(int atom);

    int indigoSetAttachmentPoint(int atom, int order);

    int indigoClearAttachmentPoints(int item);

    int indigoIterateAttachmentPoints(int item, int order);

    int indigoRemoveConstraints(int item, String type);

    int indigoAddConstraint(int item, String type, String value);

    int indigoAddConstraintNot(int item, String type, String value);

    int indigoAddConstraintOr(int item, String type, String value);

    int indigoResetStereo(int item);

    int indigoInvertStereo(int item);

    int indigoCountAtoms(int molecule);

    int indigoCountBonds(int molecule);

    int indigoCountPseudoatoms(int molecule);

    int indigoCountRSites(int molecule);

    int indigoIterateBonds(int molecule);

    int indigoBondOrder(int bond);

    int indigoBondStereo(int bond);

    int indigoTopology(int bond);

    int indigoIterateNeighbors(int atom);

    int indigoBond(int nei);

    int indigoGetAtom(int molecule, int idx);

    int indigoGetBond(int molecule, int idx);

    int indigoGetMolecule(int reaction, int idx);

    int indigoSource(int bond);

    int indigoDestination(int bond);

    int indigoClearCisTrans(int handle);

    int indigoClearStereocenters(int handle);

    int indigoClearAlleneCenters(int handle);

    int indigoCountStereocenters(int molecule);

    int indigoCountAlleneCenters(int molecule);

    int indigoResetSymmetricCisTrans(int handle);

    int indigoResetSymmetricStereocenters(int handle);

    int indigoMarkEitherCisTrans(int handle);

    int indigoMarkStereobonds(int handle);

    int indigoValidateChirality(int handle);

    int indigoAddAtom(int molecule, String symbol);

    int indigoResetAtom(int atom, String symbol);

    int indigoAddRSite(int molecule, String name);

    int indigoSetRSite(int atom, String name);

    int indigoSetCharge(int atom, int charge);

    int indigoSetExplicitValence(int atom, int valence);

    int indigoSetIsotope(int atom, int isotope);

    int indigoSetImplicitHCount(int atom, int impl_h);

    int indigoSetRadical(int atom, int radical);

    int indigoAddBond(int source, int destination, int order);

    int indigoSetBondOrder(int bond, int order);

    int indigoMerge(int where_to, int what);

    int indigoHighlight(int atom);

    int indigoUnhighlight(int item);

    int indigoIsHighlighted(int item);

    int indigoCountComponents(int molecule);

    int indigoComponentIndex(int atom);

    int indigoIterateComponents(int molecule);

    int indigoComponent(int molecule, int index);

    int indigoCountSSSR(int molecule);

    int indigoIterateSSSR(int molecule);

    int indigoIterateSubtrees(int molecule, int min_atoms, int max_atoms);

    int indigoIterateRings(int molecule, int min_atoms, int max_atoms);

    int indigoIterateEdgeSubmolecules(int molecule, int min_bonds, int max_bonds);

    int indigoCountHeavyAtoms(int molecule);

    int indigoGrossFormula(int molecule);

    double indigoMolecularWeight(int molecule);

    double indigoMostAbundantMass(int molecule);

    double indigoMonoisotopicMass(int molecule);

    Pointer indigoMassComposition(int molecule);

    Pointer indigoCanonicalSmiles(int molecule);

    Pointer indigoLayeredCode(int molecule);

    Pointer indigoSymmetryClasses(int molecule, IntByReference count);

    int indigoHasCoord(int molecule);

    int indigoHasZCoord(int molecule);

    int indigoIsChiral(int molecule);

    int indigoIsPossibleFischerProjection(int molecule, String options);

    int indigoCreateSubmolecule(int molecule, int nvertices, int[] vertices);

    int indigoGetSubmolecule(int molecule, int nvertices, int[] vertices);

    int indigoCreateEdgeSubmolecule(
            int molecule, int nvertices, int[] vertices, int nedges, int[] edges);

    int indigoRemoveAtoms(int molecule, int nvertices, int[] vertices);

    int indigoRemoveBonds(int molecule, int nbonds, int[] bonds);

    float indigoAlignAtoms(int molecule, int natoms, int[] atom_ids, float[] desired_xyz);

    int indigoAromatize(int item);

    int indigoDearomatize(int item);

    int indigoFoldHydrogens(int item);

    int indigoUnfoldHydrogens(int item);

    int indigoLayout(int object);

    int indigoClean2d(int object);

    Pointer indigoSmiles(int item);

    Pointer indigoSmarts(int item);

    Pointer indigoCanonicalSmarts(int item);

    int indigoExactMatch(int item1, int item2, String flags);

    int indigoSetTautomerRule(int id, String beg, String end);

    int indigoRemoveTautomerRule(int id);

    int indigoClearTautomerRules();

    Pointer indigoName(int handle);

    int indigoSetName(int handle, String name);

    int indigoSerialize(int handle, PointerByReference buf, IntByReference size);

    int indigoUnserialize(byte[] buf, int size);

    int indigoHasProperty(int handle, String prop);

    Pointer indigoGetProperty(int handle, String prop);

    int indigoSetProperty(int item, String prop, String value);

    int indigoRemoveProperty(int item, String prop);

    int indigoIterateProperties(int handle);

    int indigoClearProperties(int handle);

    Pointer indigoCheckBadValence(int handle);

    Pointer indigoCheckAmbiguousH(int handle);

    int indigoFingerprint(int item, String type);

    Pointer indigoOneBitsList(int fingerprint);

    int indigoLoadFingerprintFromBuffer(byte[] buffer, int size);

    int indigoLoadFingerprintFromDescriptors(double[] arr, int arr_len, int size, double density);

    int indigoCountBits(int fingerprint);

    int indigoCommonBits(int fingerprint1, int fingerprint2);

    float indigoSimilarity(int item1, int item2, String metrics);

    int indigoIterateSDF(int reader);

    int indigoIterateRDF(int reader);

    int indigoIterateSmiles(int reader);

    int indigoIterateCML(int reader);

    int indigoIterateCDX(int reader);

    int indigoIterateSDFile(String filename);

    int indigoIterateRDFile(String filename);

    int indigoIterateSmilesFile(String filename);

    int indigoIterateCMLFile(String filename);

    int indigoIterateCDXFile(String filename);

    Pointer indigoRawData(int item);

    int indigoTell(int handle);

    int indigoSdfAppend(int output, int item);

    int indigoSmilesAppend(int output, int item);

    int indigoRdfHeader(int output);

    int indigoRdfAppend(int output, int item);

    int indigoCmlHeader(int output);

    int indigoCmlAppend(int output, int item);

    int indigoCmlFooter(int output);

    int indigoCreateSaver(int output, String format);

    int indigoCreateFileSaver(String filename, String format);

    int indigoAppend(int saver, int object);

    int indigoCreateArray();

    int indigoArrayAdd(int arr, int object);

    int indigoAt(int item, int index);

    int indigoCount(int item);

    int indigoClear(int arr);

    int indigoIterateArray(int arr);

    int indigoSubstructureMatcher(int target, String mode);

    int indigoIgnoreAtom(int matcher, int atom);

    int indigoUnignoreAtom(int matcher, int atom);

    int indigoUnignoreAllAtoms(int matcher);

    int indigoMatch(int matcher, int query);

    int indigoCountMatches(int matcher, int query);

    int indigoCountMatchesWithLimit(int matcher, int query, int embeddings_limit);

    int indigoIterateMatches(int matcher, int query);

    int indigoHighlightedTarget(int match);

    int indigoMapAtom(int match, int query_atom);

    int indigoMapBond(int match, int query_bond);

    int indigoMapMolecule(int match, int query_reaction_molecule);

    int indigoExtractCommonScaffold(int structures, String options);

    int indigoRGroupComposition(int molecule, String options);

    int indigoGetFragmentedMolecule(int molecule, String options);

    int indigoAllScaffolds(int extracted);

    int indigoDecomposeMolecules(int scaffold, int structures);

    int indigoDecomposedMoleculeScaffold(int decomp);

    int indigoIterateDecomposedMolecules(int decomp);

    int indigoDecomposedMoleculeHighlighted(int decomp);

    int indigoDecomposedMoleculeWithRGroups(int decomp);

    int indigoCreateDecomposer(int scaffold);

    int indigoDecomposeMolecule(int decomp, int mol);

    int indigoIterateDecompositions(int deco_item);

    int indigoAddDecomposition(int decomp, int q_match);

    Pointer indigoToString(int handle);

    int indigoToBuffer(int handle, PointerByReference buf, IntByReference size);

    int indigoReactionProductEnumerate(int reaction, int monomers);

    int indigoTransform(int reaction, int monomers);

    int indigoExpandAbbreviations(int structure);

    int indigoIterateTautomers(int structure, String params);

    int indigoNameToStructure(String name, String params);

    int indigoTransformHELMtoSCSR(int item);

    int indigoResetOptions();

    int indigoDbgBreakpoint();

    Pointer indigoDbgInternalType(int object);
}
