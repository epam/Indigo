/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

package com.gga.indigo;
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

   public int singleAllowedRGroup ()
   {
      return dispatcher.indigoSingleAllowedRGroup(self);
   }

   public String pseudoatomLabel ()
   {
      return dispatcher.indigoPseudoatomLabel(self);
   }

   public int degree ()
   {
      return dispatcher.indigoDegree(self);
   }

   public Integer charge ()
   {
      return dispatcher.indigoGetCharge(self);
   }

   public Integer explicitValence ()
   {
      return dispatcher.indigoGetExplicitValence(self);
   }

   public Integer radicalElectrons ()
   {
      return dispatcher.indigoGetRadicalElectrons(self);
   }

   public int  atomNumber ()   { return dispatcher.indigoAtomNumber(self);  }
   public int  atomIsotope ()  { return dispatcher.indigoAtomIsotope(self); }
   public void resetCharge () { dispatcher.indigoResetCharge(self); }
   public void resetExplicitValence () { dispatcher.indigoResetExplicitValence(self); }
   public void resetRadical () { dispatcher.indigoResetRadical(self); }
   public void resetIsotope () { dispatcher.indigoResetIsotope(self); }
   public int  countAtoms ()  { return dispatcher.indigoCountAtoms(self); }
   public int  countBonds () { return dispatcher.indigoCountBonds(self); }
   public int  countPseudoatoms () { return dispatcher.indigoCountPseudoatoms(self); }
   public int  countRSites () { return dispatcher.indigoCountRSites(self); }

   public IndigoObject iterateBonds ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateBonds(self));
   }

   public int bondOrder ()
   {
      return dispatcher.indigoBondOrder(self);
   }

   public int bondStereo ()
   {
      return dispatcher.indigoBondStereo(self);
   }

   public IndigoObject iterateNeighbors ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateNeighbors(self));
   }

   public IndigoObject bond ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoBond(self));
   }

   public IndigoObject getAtom (int idx)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoGetAtom(self, idx));
   }

   public IndigoObject getBond (int idx)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoGetBond(self, idx));
   }

   public void cisTransClear ()
   {
      dispatcher.indigoCisTransClear(self);
   }
   
   public void stereocentersClear ()
   {
      dispatcher.indigoStereocentersClear(self);
   }

   public int countStereocenters ()
   {
      return dispatcher.indigoCountStereocenters(self);
   }

   public String grossFormula ()
   {
      int gf = dispatcher.indigoGrossFormula(self);
      String result = dispatcher.indigoToString(gf);
      dispatcher.indigoFree(gf);
      return result;
   }
   
   public float molecularWeight ()
   {
      return dispatcher.indigoMolecularWeight(self);
   }

   public float mostAbundantMass ()
   {
      return dispatcher.indigoMostAbundantMass(self);
   }

   public float monoisotopicMass ()
   {
      return dispatcher.indigoMonoisotopicMass(self);
   }

   public String canonicalSmiles ()
   {
      return dispatcher.indigoCanonicalSmiles(self);
   }
   
   public String layeredCode ()
   {
      return dispatcher.indigoLayeredCode(self);
   }

   public int countComponents ()
   {
      return dispatcher.indigoCountComponents(self);
   }

   public IndigoObject createSubmolecule (int[] vertices)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoCreateSubmolecule(self, vertices));
   }

   public IndigoObject createEdgeSubmolecule (int[] vertices, int[] edges)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoCreateEdgeSubmolecule(self, vertices, edges));
   }

   public void aromatize ()
   {
      dispatcher.indigoAromatize(self);
   }

   public void dearomatize ()
   {
      dispatcher.indigoDearomatize(self);
   }

   public void foldHydrogens ()
   {
      dispatcher.indigoFoldHydrogens(self);
   }

   public void unfoldHydrogens ()
   {
      dispatcher.indigoUnfoldHydrogens(self);
   }

   public void layout ()
   {
      dispatcher.indigoLayout(self);
   }

   public String smiles ()
   {
      return dispatcher.indigoSmiles(self);
   }

   public String name ()
   {
      return dispatcher.indigoName(self);
   }

   public void setName (String name)
   {
      dispatcher.indigoSetName(self, name);
   }
   
   public boolean hasProperty (String prop)
   {
      return dispatcher.indigoHasProperty(self, prop) == 1;
   }

   public String getProperty (String prop)
   {
      return dispatcher.indigoGetProperty(self, prop);
   }

   public void setProperty (String prop, String value)
   {
      dispatcher.indigoSetProperty(self, prop, value);
   }

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

   public IndigoObject iterateArray ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoIterateArray(self));
   }

   public int arrayCount ()
   {
      return dispatcher.indigoArrayCount(self);
   }

   public void arrayClear ()
   {
      dispatcher.indigoArrayClear(self);
   }

   public IndigoObject arrayAdd (IndigoObject other)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoArrayAdd(self, other.self));
   }

   public IndigoObject arrayAt (int idx)
   {
      return new IndigoObject(dispatcher, dispatcher.indigoArrayAt(self, idx));
   }

   public IndigoObject matchHighlight ()
   {
      return new IndigoObject(dispatcher, dispatcher.indigoMatchHighlight(self));
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

   public void remove () throws UnsupportedOperationException
   {
      throw new UnsupportedOperationException();
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
