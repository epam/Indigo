/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

package com.ggasoftware.indigo;
import java.util.*;

public class IndigoObject implements Iterator<IndigoObject>, Iterable<IndigoObject>
{
   public int self;
   private Indigo dispatcher;

   public IndigoObject (Indigo dispatcher, int id)
   {
      this.dispatcher = dispatcher;
      this.self = id;
   }

   @Override
   @SuppressWarnings("FinalizeDeclaration")
   protected void finalize () throws Throwable
   {
      dispatcher.indigoFree(self);
      super.finalize();
   }

   @Override
   public IndigoObject clone ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoClone(self));
   }

   public String molfile ()
   {
      return dispatcher.indigoMolfile(self);
   }

   public void saveMolfile (String filename)
   {
      dispatcher.indigoSaveMolfileToFile(self, filename);
   }

   public String cml ()
   {
      return dispatcher.indigoCml(self);
   }

   public void saveCml (String filename)
   {
      dispatcher.indigoSaveCmlToFile(self, filename);
   }

   public byte[] mdlct ()
   {
      IndigoObject buf = dispatcher.writeBuffer();
      dispatcher.indigoSaveMDLCT(self, buf.self);
      return buf.toBuffer();
   }

   public void addReactant (IndigoObject molecule)
   {
      dispatcher.indigoAddReactant(self, molecule.self);
   }

   public void addProduct (IndigoObject molecule)
   {
      dispatcher.indigoAddProduct(self, molecule.self);
   }

   public int countReactants ()
   {
      return dispatcher.indigoCountReactants(self);
   }

   public int countProducts ()
   {
      return dispatcher.indigoCountProducts(self);
   }

   public int countMolecules ()
   {
      return dispatcher.indigoCountMolecules(self);
   }

   public IndigoObject iterateReactants ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateReactants(self));
   }

   public IndigoObject iterateProducts ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateProducts(self));
   }

   public IndigoObject iterateMolecules ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateMolecules(self));
   }

   public String rxnfile ()
   {
      return dispatcher.indigoRxnfile(self);
   }

   public void saveRxnfile (String filename)
   {
      dispatcher.indigoSaveRxnfileToFile(self, filename);
   }

   public void automap (String mode)
   {
      if (mode == null)
         mode = "";
      dispatcher.indigoAutomap(self, mode);
   }

   public IndigoObject iterateAtoms ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateAtoms(self));
   }

   public IndigoObject iteratePseudoatoms ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIteratePseudoatoms(self));
   }

   public IndigoObject iterateRSites ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateRSites(self));
   }

   public IndigoObject iterateStereocenters ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateStereocenters(self));
   }

   public IndigoObject iterateRGroups ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateRGroups(self));
   }

   public IndigoObject iterateRGroupFragments ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateRGroupFragments(self));
   }

   public int countAttachmentPoints ()
   {
      return dispatcher.indigoCountAttachmentPoints(self);
   }

   public boolean isPseudoatom ()
   {
      return dispatcher.indigoIsPseudoatom(self) == 1;
   }

   public boolean isRSite ()
   {
      return dispatcher.indigoIsRSite(self) == 1;
   }

   public int stereocenterType ()
   {
      return dispatcher.indigoStereocenterType(self);
   }

   public int singleAllowedRGroup ()
   {
      return dispatcher.indigoSingleAllowedRGroup(self);
   }

   public String symbol ()
   {
      return dispatcher.indigoSymbol(self);
   }

   public int degree ()
   {
      return dispatcher.indigoDegree(self);
   }

   public Integer charge () {return dispatcher.indigoGetCharge(self); }
   public Integer explicitValence () { return dispatcher.indigoGetExplicitValence(self); }
   public Integer radicalElectrons () { return dispatcher.indigoGetRadicalElectrons(self); }

   public int  atomicNumber ()   { return dispatcher.indigoAtomicNumber(self);  }
   public int  isotope ()  { return dispatcher.indigoIsotope(self); }
   public void resetCharge () { dispatcher.indigoResetCharge(self); }
   public void resetExplicitValence () { dispatcher.indigoResetExplicitValence(self); }
   public void resetRadical () { dispatcher.indigoResetRadical(self); }
   public void resetIsotope () { dispatcher.indigoResetIsotope(self); }

   public void setAttachmentPoint (int order) { dispatcher.indigoSetAttachmentPoint(self, order); }
   public void removeConstraints (String type) { dispatcher.indigoRemoveConstraints(self, type); }
   public void addConstraint (String type, String value) { dispatcher.indigoAddConstraint(self, type, value);}
   public void addConstraintNot (String type, String value) { dispatcher.indigoAddConstraintNot(self, type, value);}

   public void resetStereo () { dispatcher.indigoResetStereo(self); }
   public void invertStereo () { dispatcher.indigoInvertStereo(self); }
   public int  countAtoms ()  { return dispatcher.indigoCountAtoms(self); }
   public int  countBonds () { return dispatcher.indigoCountBonds(self); }
   public int  countPseudoatoms () { return dispatcher.indigoCountPseudoatoms(self); }
   public int  countRSites () { return dispatcher.indigoCountRSites(self); }

   public IndigoObject iterateBonds () { return new IndigoObject(dispatcher, dispatcher.indigoIterateBonds(self)); }
   public int bondOrder () { return dispatcher.indigoBondOrder(self); }
   public int bondStereo () { return dispatcher.indigoBondStereo(self); }
   public int topology () { return dispatcher.indigoTopology(self); }

   public IndigoObject iterateNeighbors ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateNeighbors(self));
   }

   public IndigoObject bond () {return new IndigoObject(dispatcher, dispatcher.indigoBond(self)); }

   public IndigoObject getAtom (int idx) { return new IndigoObject(dispatcher, dispatcher.indigoGetAtom(self, idx)); }
   public IndigoObject getBond (int idx) { return new IndigoObject(dispatcher, dispatcher.indigoGetBond(self, idx)); }
   public IndigoObject source () { return new IndigoObject(dispatcher, dispatcher.indigoSource(self)); }
   public IndigoObject destination () { return new IndigoObject(dispatcher, dispatcher.indigoDestination(self)); }

   public void clearCisTrans () { dispatcher.indigoClearCisTrans(self); }
   public void clearStereocenters () { dispatcher.indigoClearStereocenters(self); }
   public int countStereocenters () { return dispatcher.indigoCountStereocenters(self); }
   public int resetSymmetricCisTrans () { return dispatcher.indigoResetSymmetricCisTrans(self); }

   public IndigoObject addAtom (String symbol) { return new IndigoObject(dispatcher, dispatcher.indigoAddAtom(self, symbol)); }
   public void setCharge (int charge) { dispatcher.indigoSetCharge(self, charge); }
   public void setIsotope (int isotope) { dispatcher.indigoSetIsotope(self, isotope); }
   public IndigoObject addBond (IndigoObject atom, int order)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoAddBond(self, atom.self, order));
   }
   public void setBondOrder (int order) { dispatcher.indigoSetBondOrder(self, order); };

   public IndigoObject merge (IndigoObject other) { return new IndigoObject(dispatcher, dispatcher.indigoMerge(self, other.self)); }

   public int countComponents () { return dispatcher.indigoCountComponents(self); }
   public int componentIndex () { return dispatcher.indigoComponentIndex(self); }
   public IndigoObject iterateComponents () {return new IndigoObject(dispatcher, dispatcher.indigoIterateComponents(self));}
   public IndigoObject component (int index) { return new IndigoObject(dispatcher, dispatcher.indigoComponent(self, index)); }

   public int countHeavyAtoms ()
   {
      return dispatcher.indigoCountHeavyAtoms(self);
   }

   public String grossFormula ()
   {
      int gf = dispatcher.indigoGrossFormula(self);
      String result = dispatcher.indigoToString(gf);
      dispatcher.indigoFree(gf);
      return result;
   }
   
   public float molecularWeight () { return dispatcher.indigoMolecularWeight(self); }
   public float mostAbundantMass () { return dispatcher.indigoMostAbundantMass(self); }
   public float monoisotopicMass () { return dispatcher.indigoMonoisotopicMass(self); }
   public String canonicalSmiles () { return dispatcher.indigoCanonicalSmiles(self); }
   public String layeredCode () { return dispatcher.indigoLayeredCode(self); }
   public boolean hasZCoord () { return dispatcher.indigoHasZCoord(self) == 1; }
   public boolean isChiral () { return dispatcher.indigoIsChiral(self) == 1; }

   public float[] xyz () { return dispatcher.indigoXYZ(self); }

   public int countSuperatoms () { return dispatcher.indigoCountSuperatoms(self); }
   public int countDataSGgroups () { return dispatcher.indigoCountDataSGroups(self); }
   public int countRepeatingUnits () { return dispatcher.indigoCountRepeatingUnits(self); }
   public int countMultipleGroups () { return dispatcher.indigoCountMultipleGroups(self); }
   public int countGenericSGroups () { return dispatcher.indigoCountGenericSGroups(self); }

   public IndigoObject iterateSuperatoms () { return new IndigoObject(dispatcher, dispatcher.indigoIterateSuperatoms(self)); }
   public IndigoObject iterateDataSGroups () { return new IndigoObject(dispatcher, dispatcher.indigoIterateDataSGroups(self)); }
   public IndigoObject iterateRepeatingUnits () { return new IndigoObject(dispatcher, dispatcher.indigoIterateRepeatingUnits(self)); }
   public IndigoObject iterateMultipleGroups () { return new IndigoObject(dispatcher, dispatcher.indigoIterateMultipleGroups(self)); }
   public IndigoObject iterateGenericSGroups () { return new IndigoObject(dispatcher, dispatcher.indigoIterateGenericSGroups(self)); }

   public String description () { return dispatcher.indigoDescription(self); }
   public IndigoObject addDataSGroup (int[] atoms, int[] bonds, String description, String data)
   { return new IndigoObject (dispatcher, dispatcher.indigoAddDataSGroup(self, atoms, bonds, description, data)); }
   public void setDataSGroupXY (float x, float y, String options) { dispatcher.indigoSetDataSGroupXY(self, x, y, options); }

   public IndigoObject createSubmolecule (int[] vertices)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoCreateSubmolecule(self, vertices));
   }

   public IndigoObject createEdgeSubmolecule (int[] vertices, int[] edges)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoCreateEdgeSubmolecule(self, vertices, edges));
   }

   public int removeAtoms (int[] vertices) { return dispatcher.indigoRemoveAtoms(self, vertices); }

   public float alignAtoms (int[] atom_ids, float[] desired_xyz)
   {
      return dispatcher.indigoAlignAtoms(self, atom_ids, desired_xyz);
   }

   public void aromatize () { dispatcher.indigoAromatize(self); }
   public void dearomatize () { dispatcher.indigoDearomatize(self); }
   public void foldHydrogens () { dispatcher.indigoFoldHydrogens(self); }
   public void unfoldHydrogens () { dispatcher.indigoUnfoldHydrogens(self); }
   public void layout () { dispatcher.indigoLayout(self); }
   public String smiles () { return dispatcher.indigoSmiles(self); }
   public String name () { return dispatcher.indigoName(self); }
   public void setName (String name) { dispatcher.indigoSetName(self, name); }
   
   public boolean hasProperty (String prop) { return dispatcher.indigoHasProperty(self, prop) == 1; }
   public String getProperty (String prop) { return dispatcher.indigoGetProperty(self, prop); }
   public void setProperty (String prop, String value) { dispatcher.indigoSetProperty(self, prop, value); }
   public void removeProperty (String prop) { dispatcher.indigoRemoveProperty(self, prop); }

   public IndigoObject iterateProperties ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateProperties(self));
   }

   public String checkBadValence ()
   {
      return dispatcher.indigoCheckBadValence(self);
   }

   public String checkAmbiguousH ()
   {
      return dispatcher.indigoCheckAmbiguousH(self);
   }

   public IndigoObject fingerprint ()
   {
      return fingerprint("");
   }

   public IndigoObject fingerprint (String type)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoFingerprint(self, type));
   }

   public int countBits ()
   {
      return dispatcher.indigoCountBits(self);
   }

   public String rawData ()
   {
      return dispatcher.indigoRawData(self);
   }

   public int tell ()
   {
      return dispatcher.indigoTell(self);
   }

   public void sdfAppend (IndigoObject item)
   {
      dispatcher.indigoSdfAppend(self, item.self);
   }

   public void smilesAppend (IndigoObject item)
   {
      dispatcher.indigoSmilesAppend(self, item.self);
   }

   public void rdfHeader ()
   {
      dispatcher.indigoRdfHeader(self);
   }

   public void rdfAppend (IndigoObject item)
   {
      dispatcher.indigoRdfAppend(self, item.self);
   }

   public IndigoObject iterateArray ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateArray(self));
   }

   public int count ()
   {
      return dispatcher.indigoCount(self);
   }

   public void clear ()
   {
      dispatcher.indigoClear(self);
   }

   public IndigoObject arrayAdd (IndigoObject other)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoArrayAdd(self, other.self));
   }

   public IndigoObject at (int idx)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoAt(self, idx));
   }

   public IndigoObject match (IndigoObject query)
   {
      int res = dispatcher.indigoMatch(self, query.self);

      if (res == 0)
         return null;
      
      return new IndigoObject(dispatcher, res);
   }

   public int countMatches (IndigoObject query)
   {
      return dispatcher.indigoCountMatches(self, query.self);
   }
   
   public IndigoObject iterateMatches (IndigoObject query)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateMatches(self, query.self));
   }

   public IndigoObject highlightedTarget ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoHighlightedTarget(self));
   }

   public IndigoObject mapAtom (IndigoObject query_atom)
   {
      int res = dispatcher.indigoMapAtom(self, query_atom.self);
      if (res == 0)
         return null;
      return new IndigoObject(dispatcher, res);
   }

   public IndigoObject mapBond (IndigoObject query_bond)
   {
      int res = dispatcher.indigoMapBond(self, query_bond.self);
      if (res == 0)
         return null;
      return new IndigoObject(dispatcher, res);
   }

   public IndigoObject allScaffolds ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoAllScaffolds(self));
   }

   public IndigoObject decomposedMoleculeScaffold ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoDecomposedMoleculeScaffold(self));
   }

   public IndigoObject iterateDecomposedMolecules ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateDecomposedMolecules(self));
   }

   public IndigoObject decomposedMoleculeHighlighted ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoDecomposedMoleculeHighlighted(self));
   }

   public IndigoObject decomposedMoleculeWithRGroups ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoDecomposedMoleculeWithRGroups(self));
   }

   public Iterator<IndigoObject> iterator ()
   {
      return this;
   }

   public void remove ()
   {
      dispatcher.indigoRemove(self);
   }

   public void close ()
   {
      dispatcher.indigoClose(self);
   }

   public IndigoObject next () throws NoSuchElementException
   {
      int next = dispatcher.indigoNext(self);

      if (next == 0)
         throw new NoSuchElementException("iterator has ended");
      
      return new IndigoObject(dispatcher, next);
   }

   public boolean hasNext ()
   {
      return dispatcher.indigoHasNext(self) == 1;
   }

   public int index ()
   {
      return dispatcher.indigoIndex(self);
   }

   public String toString ()
   {
      return dispatcher.indigoToString(self);
   }

   public byte[] toBuffer ()
   {
      return dispatcher.indigoToBuffer(self);
   }
}
