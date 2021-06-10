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

import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

import java.util.Collection;
import java.util.Iterator;
import java.util.NoSuchElementException;

public class IndigoObject implements Iterator<IndigoObject>, Iterable<IndigoObject> {
    protected final Indigo dispatcher;
    protected final IndigoLib lib;
    public int self;
    private Object parent; // should keep the parent so that GC will not remove it

    public IndigoObject(Indigo dispatcher, int id) {
        this.dispatcher = dispatcher;
        this.self = id;
        lib = Indigo.getLibrary();
    }

    public IndigoObject(Indigo dispatcher, int id, Object parent) {
        this.dispatcher = dispatcher;
        this.self = id;
        this.parent = parent;
        lib = Indigo.getLibrary();
    }

    public Indigo getIndigo() {
        return dispatcher;
    }

    public void dispose() {
        if (self >= 0) {
            dispatcher.setSessionID();
            lib.indigoFree(self);
            self = -1;
        }
    }

    @Override
    @SuppressWarnings("FinalizeDeclaration")
    protected void finalize() throws Throwable {
        if (!dispatcher.sessionReleased()) {
            dispose();
        }
        super.finalize();
    }

    @Override
    public IndigoObject clone() {
        dispatcher.setSessionID();
        return new IndigoObject(dispatcher, Indigo.checkResult(this, lib.indigoClone(self)));
    }

    public String molfile() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoMolfile(self));
    }

    public void saveMolfile(String filename) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSaveMolfileToFile(self, filename));
    }

    public String cml() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCml(self));
    }

    public String json() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoJson(self));
    }

    public void saveCml(String filename) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSaveCmlToFile(self, filename));
    }

    public String cdxml() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCdxml(self));
    }

    public void saveCdxml(String filename) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSaveCdxmlToFile(self, filename));
    }

    public byte[] mdlct() {
        IndigoObject buf = dispatcher.writeBuffer();
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSaveMDLCT(self, buf.self));
        return buf.toBuffer();
    }

    public void addReactant(IndigoObject molecule) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, molecule, lib.indigoAddReactant(self, molecule.self));
    }

    public void addProduct(IndigoObject molecule) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, molecule, lib.indigoAddProduct(self, molecule.self));
    }

    public void addCatalyst(IndigoObject molecule) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, molecule, lib.indigoAddCatalyst(self, molecule.self));
    }

    public int countReactants() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountReactants(self));
    }

    public int countProducts() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountProducts(self));
    }

    public int countCatalysts() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountCatalysts(self));
    }

    public int countMolecules() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountMolecules(self));
    }

    public IndigoObject iterateAttachmentPoints(int order) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoIterateAttachmentPoints(self, order)),
                this);
    }

    public IndigoObject iterateReactants() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateReactants(self)), this);
    }

    public IndigoObject iterateProducts() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateProducts(self)), this);
    }

    public IndigoObject iterateCatalysts() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateCatalysts(self)), this);
    }

    public IndigoObject iterateMolecules() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateMolecules(self)), this);
    }

    public String rxnfile() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoRxnfile(self));
    }

    public void saveRxnfile(String filename) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSaveRxnfileToFile(self, filename));
    }

    public void automap() {
        automap("");
    }

    public void automap(String mode) {
        if (mode == null) mode = "";
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAutomap(self, mode));
    }

    public int atomMappingNumber(IndigoObject reaction_atom) {
        dispatcher.setSessionID();
        return Indigo.checkResult(
                this, reaction_atom, lib.indigoGetAtomMappingNumber(self, reaction_atom.self));
    }

    public void setAtomMappingNumber(IndigoObject reaction_atom, int number) {
        dispatcher.setSessionID();
        Indigo.checkResult(
                this,
                reaction_atom,
                lib.indigoSetAtomMappingNumber(self, reaction_atom.self, number));
    }

    public void clearAAM() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClearAAM(self));
    }

    public void correctReactingCenters() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoCorrectReactingCenters(self));
    }

    public IndigoObject iterateAtoms() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateAtoms(self)), this);
    }

    public IndigoObject iteratePseudoatoms() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIteratePseudoatoms(self)), this);
    }

    public IndigoObject iterateRSites() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateRSites(self)), this);
    }

    public IndigoObject iterateStereocenters() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateStereocenters(self)), this);
    }

    public IndigoObject iterateAlleneCenters() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateAlleneCenters(self)), this);
    }

    public IndigoObject iterateRGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateRGroups(self)), this);
    }

    public IndigoObject iterateRGroupFragments() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateRGroupFragments(self)), this);
    }

    public int countRGroups() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountRGroups(self));
    }

    public int countAttachmentPoints() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountAttachmentPoints(self));
    }

    public boolean isPseudoatom() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsPseudoatom(self)) == 1;
    }

    public boolean isRSite() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsRSite(self)) == 1;
    }

    public IndigoObject setRSite(String name) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoSetRSite(self, name)), this);
    }

    public boolean isTemplateAtom() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsTemplateAtom(self)) == 1;
    }

    public int stereocenterType() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoStereocenterType(self));
    }

    public int stereocenterGroup() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoStereocenterGroup(self));
    }

    public int[] stereocenterPyramid() {
        dispatcher.setSessionID();
        Pointer ptr = Indigo.checkResultPointer(this, lib.indigoStereocenterPyramid(self));
        return ptr.getIntArray(0, 4);
    }

    public void changeStereocenterType(int type) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoChangeStereocenterType(self, type));
    }

    public void setStereocenterGroup(int group) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetStereocenterGroup(self, group));
    }

    public int singleAllowedRGroup() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSingleAllowedRGroup(self));
    }

    public String symbol() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoSymbol(self));
    }

    public int degree() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoDegree(self));
    }

    public Integer charge() {
        IntByReference res = new IntByReference();
        dispatcher.setSessionID();
        if (Indigo.checkResult(this, lib.indigoGetCharge(self, res)) == 1) return res.getValue();
        return null;
    }

    public int reactingCenter(IndigoObject bond) {
        IntByReference res = new IntByReference();
        dispatcher.setSessionID();
        if (Indigo.checkResult(this, bond, lib.indigoGetReactingCenter(self, bond.self, res)) == 1)
            return res.getValue();
        throw new IndigoException(this, "reactingCenter(): unexpected result");
    }

    public void setReactingCenter(IndigoObject bond, int type) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, bond, lib.indigoSetReactingCenter(self, bond.self, type));
    }

    public Integer explicitValence() {
        IntByReference res = new IntByReference();
        dispatcher.setSessionID();
        if (Indigo.checkResult(this, lib.indigoGetExplicitValence(self, res)) == 1)
            return res.getValue();
        return null;
    }

    public Integer radicalElectrons() {
        IntByReference res = new IntByReference();
        dispatcher.setSessionID();
        if (Indigo.checkResult(this, lib.indigoGetRadicalElectrons(self, res)) == 1)
            return res.getValue();
        return null;
    }

    public Integer radical() {
        IntByReference res = new IntByReference();
        dispatcher.setSessionID();
        if (Indigo.checkResult(this, lib.indigoGetRadical(self, res)) == 1) return res.getValue();
        return null;
    }

    public int atomicNumber() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoAtomicNumber(self));
    }

    public int isotope() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsotope(self));
    }

    public int valence() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoValence(self));
    }

    public int checkValence() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCheckValence(self));
    }

    public int checkQuery() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCheckQuery(self));
    }

    public int checkRGroups() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCheckRGroups(self));
    }

    public int checkChirality() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCheckChirality(self));
    }

    public int check3DStereo() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCheck3DStereo(self));
    }

    public int checkStereo() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCheckStereo(self));
    }

    public String check() {
        String type = "";
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCheckObj(self, type));
    }

    public String check(String type) {
        if (type == null) type = "";
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCheckObj(self, type));
    }

    public Integer countHydrogens() {
        IntByReference res = new IntByReference();
        dispatcher.setSessionID();
        if (Indigo.checkResult(this, lib.indigoCountHydrogens(self, res)) == 1)
            return res.getValue();
        return null;
    }

    public int countImplicitHydrogens() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountImplicitHydrogens(self));
    }

    public void resetCharge() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoResetCharge(self));
    }

    public void resetExplicitValence() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoResetExplicitValence(self));
    }

    public void resetRadical() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoResetRadical(self));
    }

    public void resetIsotope() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoResetIsotope(self));
    }

    public void setAttachmentPoint(int order) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetAttachmentPoint(self, order));
    }

    public void clearAttachmentPoints() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClearAttachmentPoints(self));
    }

    public void removeConstraints(String type) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoRemoveConstraints(self, type));
    }

    public void addConstraint(String type, String value) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAddConstraint(self, type, value));
    }

    public void addConstraintNot(String type, String value) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAddConstraintNot(self, type, value));
    }

    public void addConstraintOr(String type, String value) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAddConstraintOr(self, type, value));
    }

    public void resetStereo() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoResetStereo(self));
    }

    public void invertStereo() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoInvertStereo(self));
    }

    public int countAtoms() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountAtoms(self));
    }

    public int countBonds() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountBonds(self));
    }

    public int countPseudoatoms() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountPseudoatoms(self));
    }

    public int countRSites() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountRSites(self));
    }

    public IndigoObject iterateBonds() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateBonds(self)), this);
    }

    public int bondOrder() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoBondOrder(self));
    }

    public int bondStereo() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoBondStereo(self));
    }

    public int topology() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoTopology(self));
    }

    public IndigoObject iterateNeighbors() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateNeighbors(self)), this);
    }

    public IndigoObject bond() {
        dispatcher.setSessionID();
        return new IndigoObject(dispatcher, Indigo.checkResult(this, lib.indigoBond(self)));
    }

    public IndigoObject getAtom(int idx) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoGetAtom(self, idx)), this);
    }

    public IndigoObject getMolecule(int idx) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoGetMolecule(self, idx)), this);
    }

    public IndigoObject getBond(int idx) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoGetBond(self, idx)), this);
    }

    public IndigoObject source() {
        dispatcher.setSessionID();
        return new IndigoObject(dispatcher, Indigo.checkResult(this, lib.indigoSource(self)));
    }

    public IndigoObject destination() {
        dispatcher.setSessionID();
        return new IndigoObject(dispatcher, Indigo.checkResult(this, lib.indigoDestination(self)));
    }

    public void clearCisTrans() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClearCisTrans(self));
    }

    public void clearStereocenters() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClearStereocenters(self));
    }

    public void clearAlleneCenters() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClearAlleneCenters(self));
    }

    public int countStereocenters() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountStereocenters(self));
    }

    public int countAlleneCenters() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountAlleneCenters(self));
    }

    public int resetSymmetricCisTrans() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoResetSymmetricCisTrans(self));
    }

    public int resetSymmetricStereocenters() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoResetSymmetricStereocenters(self));
    }

    public int markEitherCisTrans() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoMarkEitherCisTrans(self));
    }

    public int markStereobonds() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoMarkStereobonds(self));
    }

    public int validateChirality() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoValidateChirality(self));
    }

    public IndigoObject addAtom(String symbol) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoAddAtom(self, symbol)), this);
    }

    public IndigoObject resetAtom(String symbol) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoResetAtom(self, symbol)), this);
    }

    public IndigoObject addRSite(String name) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoAddRSite(self, name)), this);
    }

    public void setCharge(int charge) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetCharge(self, charge));
    }

    public void setRadical(int radical) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetRadical(self, radical));
    }

    public void setExplicitValence(int valence) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetExplicitValence(self, valence));
    }

    public void setIsotope(int isotope) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetIsotope(self, isotope));
    }

    public void setImplicitHCount(int hcount) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetImplicitHCount(self, hcount));
    }

    public IndigoObject addBond(IndigoObject atom, int order) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoAddBond(self, atom.self, order)),
                atom);
    }

    public void setBondOrder(int order) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetBondOrder(self, order));
    }

    public IndigoObject merge(IndigoObject other) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, other, lib.indigoMerge(self, other.self)),
                this);
    }

    public void highlight() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoHighlight(self));
    }

    public void unhighlight() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoUnhighlight(self));
    }

    public boolean isHighlighted() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsHighlighted(self)) == 1;
    }

    public int countComponents() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountComponents(self));
    }

    public int componentIndex() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoComponentIndex(self));
    }

    public IndigoObject iterateComponents() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateComponents(self)), this);
    }

    public IndigoObject component(int index) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoComponent(self, index)), this);
    }

    public int countSSSR() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountSSSR(self));
    }

    public IndigoObject iterateSSSR() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateSSSR(self)), this);
    }

    public IndigoObject iterateSubtrees(int min_vertices, int max_vertices) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(
                        this, lib.indigoIterateSubtrees(self, min_vertices, max_vertices)),
                this);
    }

    public IndigoObject iterateRings(int min_vertices, int max_vertices) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoIterateRings(self, min_vertices, max_vertices)),
                this);
    }

    public IndigoObject iterateEdgeSubmolecules(int min_edges, int max_edges) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(
                        this, lib.indigoIterateEdgeSubmolecules(self, min_edges, max_edges)),
                this);
    }

    public int countHeavyAtoms() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountHeavyAtoms(self));
    }

    public String grossFormula() {
        int gf = -1;
        try {
            dispatcher.setSessionID();
            gf = Indigo.checkResult(this, lib.indigoGrossFormula(self));
            return Indigo.checkResultString(this, lib.indigoToString(gf));
        } finally {
            lib.indigoFree(gf);
        }
    }

    public double molecularWeight() {
        dispatcher.setSessionID();
        return Indigo.checkResultDouble(this, lib.indigoMolecularWeight(self));
    }

    public double mostAbundantMass() {
        dispatcher.setSessionID();
        return Indigo.checkResultDouble(this, lib.indigoMostAbundantMass(self));
    }

    public double monoisotopicMass() {
        dispatcher.setSessionID();
        return Indigo.checkResultDouble(this, lib.indigoMonoisotopicMass(self));
    }

    public String massComposition() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoMassComposition(self));
    }

    public String canonicalSmiles() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCanonicalSmiles(self));
    }

    public String layeredCode() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoLayeredCode(self));
    }

    public boolean hasCoord() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoHasCoord(self)) == 1;
    }

    public boolean hasZCoord() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoHasZCoord(self)) == 1;
    }

    public boolean isChiral() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsChiral(self)) == 1;
    }

    public boolean isPossibleFischerProjection(String options) {
        if (options == null) options = "";
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIsPossibleFischerProjection(self, options)) == 1;
    }

    public float[] xyz() {
        dispatcher.setSessionID();
        Pointer ptr = Indigo.checkResultPointer(this, lib.indigoXYZ(self));
        return ptr.getFloatArray(0, 3);
    }

    public void setXYZ(float x, float y, float z) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetXYZ(self, x, y, z));
    }

    public void setXYZ(float[] xyz) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetXYZ(self, xyz[0], xyz[1], xyz[2]));
    }

    public int countSuperatoms() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountSuperatoms(self));
    }

    public int countDataSGroups() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountDataSGroups(self));
    }

    public int countRepeatingUnits() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountRepeatingUnits(self));
    }

    public int countMultipleGroups() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountMultipleGroups(self));
    }

    public int countGenericSGroups() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountGenericSGroups(self));
    }

    public IndigoObject iterateSuperatoms() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateSuperatoms(self)), this);
    }

    public IndigoObject iterateDataSGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateDataSGroups(self)), this);
    }

    public IndigoObject iterateRepeatingUnits() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateRepeatingUnits(self)), this);
    }

    public IndigoObject iterateMultipleGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateMultipleGroups(self)), this);
    }

    public IndigoObject iterateGenericSGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateGenericSGroups(self)), this);
    }

    public IndigoObject iterateSGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateSGroups(self)), this);
    }

    public IndigoObject iterateTGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateTGroups(self)), this);
    }

    public IndigoObject getDataSGroup(int index) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoGetDataSGroup(self, index)), this);
    }

    public IndigoObject getSuperatom(int index) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoGetSuperatom(self, index)), this);
    }

    public IndigoObject getGenericSGroup(int index) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoGetGenericSGroup(self, index)),
                this);
    }

    public IndigoObject getMultipleGroup(int index) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoGetMultipleGroup(self, index)),
                this);
    }

    public IndigoObject getRepeatingUnit(int index) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoGetRepeatingUnit(self, index)),
                this);
    }

    public String description() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoDescription(self));
    }

    public String data() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoData(self));
    }

    public IndigoObject addDataSGroup(int[] atoms, int[] bonds, String description, String data) {
        if (description == null) description = "";
        if (data == null) data = "";
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(
                        this,
                        lib.indigoAddDataSGroup(
                                self, atoms.length, atoms, bonds.length, bonds, description, data)),
                this);
    }

    public IndigoObject addDataSGroup(
            Collection<Integer> atoms, Collection<Integer> bonds, String description, String data) {
        return addDataSGroup(Indigo.toIntArray(atoms), Indigo.toIntArray(bonds), description, data);
    }

    public IndigoObject createSGroup(String type, IndigoObject mapping, String name) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoCreateSGroup(type, mapping.self, name)),
                this);
    }

    public String getSGroupClass() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetSGroupClass(self));
    }

    public void setSGroupClass(String sgclass) {
        if (sgclass == null) sgclass = "";

        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetSGroupClass(self, sgclass));
    }

    public String getSGroupName() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetSGroupName(self));
    }

    public void setSGroupName(String sgname) {
        if (sgname == null) sgname = "";

        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetSGroupName(self, sgname));
    }

    public int getSGroupNumCrossBonds() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupNumCrossBonds(self));
    }

    public int addSGroupAttachmentPoint(int aidx, int lvidx, String apid) {
        dispatcher.setSessionID();
        return Indigo.checkResult(
                this, lib.indigoAddSGroupAttachmentPoint(self, aidx, lvidx, apid));
    }

    public int deleteSGroupAttachmentPoint(int apidx) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoDeleteSGroupAttachmentPoint(self, apidx));
    }

    public int getSGroupDisplayOption() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupDisplayOption(self));
    }

    public int getSGroupSeqId() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupSeqId(self));
    }

    public float[] getSGroupCoords() {
        dispatcher.setSessionID();
        Pointer ptr = Indigo.checkResultPointer(this, lib.indigoGetSGroupCoords(self));
        return ptr.getFloatArray(0, 2);
    }

    public int setSGroupDisplayOption(int option) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupDisplayOption(self, option));
    }

    public String getRepeatingUnitSubscript() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetRepeatingUnitSubscript(self));
    }

    public int getRepeatingUnitConnectivity() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetRepeatingUnitConnectivity(self));
    }

    public int getSGroupMultiplier() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupMultiplier(self));
    }

    public int setSGroupMultiplier(int mult) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupMultiplier(self, mult));
    }

    public int setSGroupData(String data) {
        dispatcher.setSessionID();

        if (data == null) data = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupData(self, data));
    }

    public int setSGroupCoords(float x, float y) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupCoords(self, x, y));
    }

    public int setSGroupDescription(String description) {
        dispatcher.setSessionID();

        if (description == null) description = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupDescription(self, description));
    }

    public int setSGroupFieldName(String name) {
        dispatcher.setSessionID();

        if (name == null) name = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupFieldName(self, name));
    }

    public int setSGroupQueryCode(String querycode) {
        dispatcher.setSessionID();

        if (querycode == null) querycode = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupQueryCode(self, querycode));
    }

    public int setSGroupQueryOper(String queryoper) {
        dispatcher.setSessionID();

        if (queryoper == null) queryoper = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupQueryOper(self, queryoper));
    }

    public int setSGroupDisplay(String option) {
        dispatcher.setSessionID();

        if (option == null) option = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupDisplay(self, option));
    }

    public int setSGroupLocation(String option) {
        dispatcher.setSessionID();

        if (option == null) option = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupLocation(self, option));
    }

    public int setSGroupTag(String tag) {
        dispatcher.setSessionID();

        if (tag == null) tag = "";
        return Indigo.checkResult(this, lib.indigoSetSGroupTag(self, tag));
    }

    public int setSGroupTagAlign(int tag_align) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupTagAlign(self, tag_align));
    }

    public int setSGroupDataType(String data_type) {
        if (data_type == null) data_type = "";
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupDataType(self, data_type));
    }

    public int setSGroupXCoord(float x) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupXCoord(self, x));
    }

    public int setSGroupYCoord(float y) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupYCoord(self, y));
    }

    public IndigoObject findSGroups(String property, String value) {
        if (property == null) property = "";
        if (value == null) value = "";
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoFindSGroups(self, property, value)),
                this);
    }

    public int getSGroupType() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupType(self));
    }

    public int getSGroupIndex() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupIndex(self));
    }

    public int getSGroupOriginalId() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupOriginalId(self));
    }

    public int setSGroupOriginalId(int original) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupOriginalId(self, original));
    }

    public int getSGroupParentId() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoGetSGroupParentId(self));
    }

    public int setSGroupParentId(int parent) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetSGroupParentId(self, parent));
    }

    public int addTemplate(IndigoObject templates, String name) {
        if (name == null) name = "";
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoAddTemplate(self, templates.self, name));
    }

    public int removeTemplate(String name) {
        if (name == null) name = "";
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoRemoveTemplate(self, name));
    }

    public int findTemplate(String name) {
        if (name == null) name = "";
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoFindTemplate(self, name));
    }

    public String getTGroupClass() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetTGroupClass(self));
    }

    public String getTGroupName() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetTGroupName(self));
    }

    public String getTGroupAlias() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetTGroupAlias(self));
    }

    public int transformSCSRtoCTAB() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoTransformSCSRtoCTAB(self));
    }

    public int transformCTABtoSCSR(IndigoObject templates) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoTransformCTABtoSCSR(self, templates.self));
    }

    public String getTemplateAtomClass() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetTemplateAtomClass(self));
    }

    public int setTemplateAtomClass(String name) {
        if (name == null) name = "";
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoSetTemplateAtomClass(self, name));
    }

    public int setSGroupBrackets(
            int brk_style,
            float x1,
            float y1,
            float x2,
            float y2,
            float x3,
            float y3,
            float x4,
            float y4) {
        dispatcher.setSessionID();
        return Indigo.checkResult(
                this, lib.indigoSetSGroupBrackets(self, brk_style, x1, y1, x2, y2, x3, y3, x4, y4));
    }

    public void setDataSGroupXY(float x, float y) {
        setDataSGroupXY(x, y, "");
    }

    public void setDataSGroupXY(float x, float y, String options) {
        dispatcher.setSessionID();

        if (options == null) options = "";

        Indigo.checkResult(this, lib.indigoSetDataSGroupXY(self, x, y, options));
    }

    public void addStereocenter(int type, int v1, int v2, int v3) {
        addStereocenter(type, v1, v2, v3, -1);
    }

    public void addStereocenter(int type, int v1, int v2, int v3, int v4) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAddStereocenter(self, type, v1, v2, v3, v4));
    }

    public IndigoObject createSubmolecule(int[] vertices) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(
                        this, lib.indigoCreateSubmolecule(self, vertices.length, vertices)));
    }

    public IndigoObject createSubmolecule(Collection<Integer> vertices) {
        return createSubmolecule(Indigo.toIntArray(vertices));
    }

    public IndigoObject getSubmolecule(int[] vertices) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(
                        this, lib.indigoGetSubmolecule(self, vertices.length, vertices)));
    }

    public IndigoObject getSubmolecule(Collection<Integer> vertices) {
        return getSubmolecule(Indigo.toIntArray(vertices));
    }

    public IndigoObject createEdgeSubmolecule(int[] vertices, int[] edges) {
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(
                        this,
                        lib.indigoCreateEdgeSubmolecule(
                                self, vertices.length, vertices, edges.length, edges)));
    }

    public IndigoObject createEdgeSubmolecule(
            Collection<Integer> vertices, Collection<Integer> edges) {
        return createEdgeSubmolecule(Indigo.toIntArray(vertices), Indigo.toIntArray(edges));
    }

    public void removeAtoms(int[] vertices) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoRemoveAtoms(self, vertices.length, vertices));
    }

    public void removeAtoms(Collection<Integer> vertices) {
        removeAtoms(Indigo.toIntArray(vertices));
    }

    public void removeBonds(int[] bonds) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoRemoveBonds(self, bonds.length, bonds));
    }

    public void removeBonds(Collection<Integer> bonds) {
        removeBonds(Indigo.toIntArray(bonds));
    }

    public float alignAtoms(int[] atom_ids, float[] desired_xyz) {
        if (atom_ids.length * 3 != desired_xyz.length)
            throw new IndigoException(
                    this, "desired_xyz[] must be exactly 3 times bigger than atom_ids[]");
        dispatcher.setSessionID();
        return Indigo.checkResultFloat(
                this, lib.indigoAlignAtoms(self, atom_ids.length, atom_ids, desired_xyz));
    }

    public float alignAtoms(Collection<Integer> atom_ids, Collection<Float> desired_xyz) {
        return alignAtoms(Indigo.toIntArray(atom_ids), Indigo.toFloatArray(desired_xyz));
    }

    public void aromatize() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAromatize(self));
    }

    public void dearomatize() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoDearomatize(self));
    }

    public void foldHydrogens() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoFoldHydrogens(self));
    }

    public void unfoldHydrogens() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoUnfoldHydrogens(self));
    }

    public void layout() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoLayout(self));
    }

    public void clean2d() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClean2d(self));
    }

    public String smiles() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoSmiles(self));
    }

    public String smarts() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoSmarts(self));
    }

    public String canonicalSmarts() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCanonicalSmarts(self));
    }

    public String name() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoName(self));
    }

    public void setName(String name) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetName(self, name));
    }

    public byte[] serialize() {
        PointerByReference ptr = new PointerByReference();
        IntByReference size = new IntByReference();

        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSerialize(self, ptr, size));
        Pointer p = ptr.getValue();
        return p.getByteArray(0, size.getValue());
    }

    public boolean hasProperty(String prop) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoHasProperty(self, prop)) == 1;
    }

    public String getProperty(String prop) {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetProperty(self, prop));
    }

    public void setProperty(String prop, String value) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoSetProperty(self, prop, value));
    }

    public void removeProperty(String prop) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoRemoveProperty(self, prop));
    }

    public IndigoObject iterateProperties() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateProperties(self)), this);
    }

    public void clearProperties() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClearProperties(self));
    }

    public String checkBadValence() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCheckBadValence(self));
    }

    public String checkAmbiguousH() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoCheckAmbiguousH(self));
    }

    public IndigoObject fingerprint() {
        return fingerprint("");
    }

    public IndigoObject fingerprint(String type) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoFingerprint(self, type)));
    }

    public String oneBitsList() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoOneBitsList(self));
    }

    public int countBits() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCountBits(self));
    }

    public String rawData() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoRawData(self));
    }

    public int tell() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoTell(self));
    }

    public void sdfAppend(IndigoObject item) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, item, lib.indigoSdfAppend(self, item.self));
    }

    public void smilesAppend(IndigoObject item) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, item, lib.indigoSmilesAppend(self, item.self));
    }

    public void rdfHeader() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoRdfHeader(self));
    }

    public void rdfAppend(IndigoObject item) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, item, lib.indigoRdfAppend(self, item.self));
    }

    public void cmlHeader() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoCmlHeader(self));
    }

    public void cmlAppend(IndigoObject item) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, item, lib.indigoCmlAppend(self, item.self));
    }

    public void cmlFooter() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoCmlFooter(self));
    }

    public IndigoObject iterateArray() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoIterateArray(self)), this);
    }

    public int count() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoCount(self));
    }

    public void clear() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClear(self));
    }

    public int arrayAdd(IndigoObject other) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, other, lib.indigoArrayAdd(self, other.self));
    }

    public IndigoObject at(int idx) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoAt(self, idx)), this);
    }

    public void ignoreAtom(IndigoObject atom) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, atom, lib.indigoIgnoreAtom(self, atom.self));
    }

    public void unignoreAtom(IndigoObject atom) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, atom, lib.indigoUnignoreAtom(self, atom.self));
    }

    public void unignoreAllAtoms() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoUnignoreAllAtoms(self));
    }

    public IndigoObject match(IndigoObject query) {
        dispatcher.setSessionID();
        int res = Indigo.checkResult(this, query, lib.indigoMatch(self, query.self));

        if (res == 0) return null;

        return new IndigoObject(dispatcher, res, this);
    }

    public int countMatches(IndigoObject query) {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, query, lib.indigoCountMatches(self, query.self));
    }

    public int countMatchesWithLimit(IndigoObject query, int embeddings_limit) {
        dispatcher.setSessionID();
        return Indigo.checkResult(
                this, query, lib.indigoCountMatchesWithLimit(self, query.self, embeddings_limit));
    }

    public IndigoObject iterateMatches(IndigoObject query) {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, query, lib.indigoIterateMatches(self, query.self)),
                this);
    }

    public IndigoObject highlightedTarget() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoHighlightedTarget(self)));
    }

    public IndigoObject mapAtom(IndigoObject query_atom) {
        dispatcher.setSessionID();
        int res = Indigo.checkResult(this, query_atom, lib.indigoMapAtom(self, query_atom.self));
        if (res == 0) return null;
        return new IndigoObject(dispatcher, res, this);
    }

    public IndigoObject mapMolecule(IndigoObject query_reaction_molecule) {
        dispatcher.setSessionID();
        int res =
                Indigo.checkResult(
                        this,
                        query_reaction_molecule,
                        lib.indigoMapMolecule(self, query_reaction_molecule.self));
        if (res == 0) return null;
        return new IndigoObject(dispatcher, res, this);
    }

    public IndigoObject mapBond(IndigoObject query_bond) {
        Object[] guard = {this, query_bond};
        dispatcher.setSessionID();
        int res = Indigo.checkResult(this, query_bond, lib.indigoMapBond(self, query_bond.self));
        if (res == 0) return null;
        return new IndigoObject(dispatcher, res, this);
    }

    public IndigoObject allScaffolds() {
        dispatcher.setSessionID();
        return new IndigoObject(dispatcher, Indigo.checkResult(this, lib.indigoAllScaffolds(self)));
    }

    public IndigoObject decomposedMoleculeScaffold() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher, Indigo.checkResult(this, lib.indigoDecomposedMoleculeScaffold(self)));
    }

    @Deprecated
    public IndigoObject iterateDecomposedMolecules() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoIterateDecomposedMolecules(self)),
                this);
    }

    public IndigoObject decomposedMoleculeHighlighted() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoDecomposedMoleculeHighlighted(self)));
    }

    public IndigoObject decomposedMoleculeWithRGroups() {
        dispatcher.setSessionID();
        return new IndigoObject(
                dispatcher,
                Indigo.checkResult(this, lib.indigoDecomposedMoleculeWithRGroups(self)));
    }

    public IndigoObject decomposeMolecule(IndigoObject mol) {
        dispatcher.setSessionID();
        int res = Indigo.checkResult(this, lib.indigoDecomposeMolecule(self, mol.self));
        if (res == 0) return null;
        return new IndigoObject(dispatcher, res, this);
    }

    public IndigoObject iterateDecompositions() {
        dispatcher.setSessionID();
        int res = Indigo.checkResult(this, lib.indigoIterateDecompositions(self));
        if (res == 0) return null;
        return new IndigoObject(dispatcher, res, this);
    }

    public void addDecomposition(IndigoObject q_match) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoAddDecomposition(self, q_match.self));
    }

    public Iterator<IndigoObject> iterator() {
        return this;
    }

    public void remove() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoRemove(self));
    }

    public void close() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoClose(self));
    }

    public IndigoObject next() throws NoSuchElementException {
        dispatcher.setSessionID();
        int next = Indigo.checkResult(this, lib.indigoNext(self));

        if (next == 0) throw new NoSuchElementException("iterator has ended");

        return new IndigoObject(dispatcher, next, this);
    }

    public boolean hasNext() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoHasNext(self)) == 1;
    }

    public int index() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoIndex(self));
    }

    @Override
    public String toString() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoToString(self));
    }

    public byte[] toBuffer() {
        PointerByReference ptr = new PointerByReference();
        IntByReference size = new IntByReference();

        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoToBuffer(self, ptr, size));
        Pointer p = ptr.getValue();
        return p.getByteArray(0, size.getValue());
    }

    public int[] symmetryClasses() {
        IntByReference count = new IntByReference();
        dispatcher.setSessionID();
        Pointer p = Indigo.checkResultPointer(this, lib.indigoSymmetryClasses(self, count));
        return p.getIntArray(0, count.getValue());
    }

    public void append(IndigoObject obj) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, obj, lib.indigoAppend(self, obj.self));
    }

    public void optimize() {
        optimize(null);
    }

    public void optimize(String options) {
        if (options == null) options = "";
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoOptimize(self, options));
    }

    public boolean normalize() {
        return normalize(null);
    }

    public boolean normalize(String options) {
        if (options == null) options = "";
        dispatcher.setSessionID();
        return (Indigo.checkResult(this, lib.indigoNormalize(self, options)) == 1);
    }

    public void standardize() {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoStandardize(self));
    }

    public void ionize(float pH, float pHToll) {
        dispatcher.setSessionID();
        Indigo.checkResult(this, lib.indigoIonize(self, pH, pHToll));
    }

    public float getAcidPkaValue(IndigoObject atom, int level, int min_level) {
        dispatcher.setSessionID();
        Pointer ptr =
                Indigo.checkResultPointer(
                        this, lib.indigoGetAcidPkaValue(self, atom.self, level, min_level));
        return ptr.getFloat(0);
    }

    public float getBasicPkaValue(IndigoObject atom, int level, int min_level) {
        dispatcher.setSessionID();
        Pointer ptr =
                Indigo.checkResultPointer(
                        this, lib.indigoGetBasicPkaValue(self, atom.self, level, min_level));
        return ptr.getFloat(0);
    }

    public int expandAbbreviations() {
        dispatcher.setSessionID();
        return Indigo.checkResult(this, lib.indigoExpandAbbreviations(self));
    }

    public String dbgInternalType() {
        dispatcher.setSessionID();
        return Indigo.checkResultString(this, lib.indigoDbgInternalType(self));
    }
}
