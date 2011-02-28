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
import com.sun.jna.*;
import com.sun.jna.ptr.*;
import java.util.*;

public class IndigoObject implements Iterator<IndigoObject>, Iterable<IndigoObject>
{
   public int self;
   private Indigo dispatcher;
   private IndigoObject parent; // should keep the parent so that GC will not remove it
   private IndigoLib _lib;

   public IndigoObject (Indigo dispatcher, int id)
   {
      this.dispatcher = dispatcher;
      this.self = id;
      _lib = Indigo.getLibrary();
   }

   public IndigoObject (Indigo dispatcher, IndigoObject parent, int id)
   {
      this.dispatcher = dispatcher;
      this.self = id;
      this.parent = parent;
      _lib = Indigo.getLibrary();
   }

   @Override
   @SuppressWarnings("FinalizeDeclaration")
   protected void finalize () throws Throwable
   {
      dispatcher.setSessionID();
      _lib.indigoFree(self);
      super.finalize();
   }

   @Override
   public IndigoObject clone ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(_lib.indigoClone(self)));
   }

   public String molfile ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoMolfile(self));
   }

   public void saveMolfile (String filename)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSaveMolfileToFile(self, filename));
   }

   public String cml ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoCml(self));
   }

   public void saveCml (String filename)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSaveCmlToFile(self, filename));
   }

   public byte[] mdlct ()
   {
      dispatcher.setSessionID();
      IndigoObject buf = dispatcher.writeBuffer();
      Indigo.checkResult(_lib.indigoSaveMDLCT(self, buf.self));
      return buf.toBuffer();
   }

   public void addReactant (IndigoObject molecule)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoAddReactant(self, molecule.self));
   }

   public void addProduct (IndigoObject molecule)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoAddProduct(self, molecule.self));
   }

   public int countReactants ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountReactants(self));
   }

   public int countProducts ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountProducts(self));
   }

   public int countMolecules ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountMolecules(self));
   }

   public IndigoObject iterateReactants ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateReactants(self)));
   }

   public IndigoObject iterateProducts ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateProducts(self)));
   }

   public IndigoObject iterateMolecules ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateMolecules(self)));
   }

   public String rxnfile ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoRxnfile(self));
   }

   public void saveRxnfile (String filename)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSaveRxnfileToFile(self, filename));
   }

   public void automap (String mode)
   {
      if (mode == null)
         mode = "";
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoAutomap(self, mode));
   }

   public IndigoObject iterateAtoms ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateAtoms(self)));
   }

   public IndigoObject iteratePseudoatoms ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIteratePseudoatoms(self)));
   }

   public IndigoObject iterateRSites ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateRSites(self)));
   }

   public IndigoObject iterateStereocenters ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateStereocenters(self)));
   }

   public IndigoObject iterateRGroups ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateRGroups(self)));
   }

   public IndigoObject iterateRGroupFragments ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateRGroupFragments(self)));
   }

   public int countAttachmentPoints ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountAttachmentPoints(self));
   }

   public boolean isPseudoatom ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoIsPseudoatom(self)) == 1;
   }

   public boolean isRSite ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoIsRSite(self)) == 1;
   }

   public int stereocenterType ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoStereocenterType(self));
   }

   public int singleAllowedRGroup ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoSingleAllowedRGroup(self));
   }

   public String symbol ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoSymbol(self));
   }

   public int degree ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoDegree(self));
   }

   public Integer charge ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(_lib.indigoGetCharge(self, res)) == 1)
         return res.getValue();
      return null;
   }

   public Integer explicitValence ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(_lib.indigoGetExplicitValence(self, res)) == 1)
         return res.getValue();
      return null;
   }
   public Integer radicalElectrons ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(_lib.indigoGetRadicalElectrons(self, res)) == 1)
         return res.getValue();
      return null;
   }

   public int  atomicNumber ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoAtomicNumber(self));
   }
   public int  isotope ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoIsotope(self));
   }

   public Integer countHydrogens ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(_lib.indigoCountHydrogens(self, res)) == 1)
         return res.getValue();
      return null;
   }

   public int countImplicitHydrogens ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountImplicitHydrogens(self));
   }

   public void resetCharge ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoResetCharge(self));
   }

   public void resetExplicitValence ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoResetExplicitValence(self));
   }

   public void resetRadical ()
   {
      dispatcher.setSessionID();
      _lib.indigoResetRadical(self);
   }

   public void resetIsotope ()
   {
      dispatcher.setSessionID();
      _lib.indigoResetIsotope(self);
   }

   public void setAttachmentPoint (int order)
   {
      dispatcher.setSessionID();
      _lib.indigoSetAttachmentPoint(self, order);
   }
   public void removeConstraints (String type)
   {
      dispatcher.setSessionID();
      _lib.indigoRemoveConstraints(self, type);
   }

   public void addConstraint (String type, String value)
   {
      dispatcher.setSessionID();
      _lib.indigoAddConstraint(self, type, value);
   }

   public void addConstraintNot (String type, String value)
   {
      dispatcher.setSessionID();
      _lib.indigoAddConstraintNot(self, type, value);
   }

   public void resetStereo ()
   {
      dispatcher.setSessionID();
      _lib.indigoResetStereo(self);
   }

   public void invertStereo ()
   {
      dispatcher.setSessionID();
      _lib.indigoInvertStereo(self);
   }

   public int  countAtoms ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountAtoms(self);
   }

   public int countBonds ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountBonds(self);
   }

   public int countPseudoatoms ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountPseudoatoms(self);
   }

   public int  countRSites ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountRSites(self);
   }

   public IndigoObject iterateBonds ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateBonds(self));
   }

   public int bondOrder ()
   {
      dispatcher.setSessionID();
      return _lib.indigoBondOrder(self);
   }

   public int bondStereo ()
   {
      dispatcher.setSessionID();
      return _lib.indigoBondStereo(self);
   }

   public int topology ()
   {
      dispatcher.setSessionID();
      return _lib.indigoTopology(self);
   }

   public IndigoObject iterateNeighbors ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateNeighbors(self));
   }

   public IndigoObject bond ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, _lib.indigoBond(self));
   }

   public IndigoObject getAtom (int idx)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoGetAtom(self, idx));
   }

   public IndigoObject getBond (int idx)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoGetBond(self, idx));
   }

   public IndigoObject source ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, _lib.indigoSource(self));
   }

   public IndigoObject destination ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, _lib.indigoDestination(self));
   }

   public void clearCisTrans ()
   {
      dispatcher.setSessionID();
      _lib.indigoClearCisTrans(self);
   }

   public void clearStereocenters ()
   {
      dispatcher.setSessionID();
      _lib.indigoClearStereocenters(self);
   }

   public int countStereocenters ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountStereocenters(self);
   }

   public int resetSymmetricCisTrans ()
   {
      dispatcher.setSessionID();
      return _lib.indigoResetSymmetricCisTrans(self);
   }

   public int markEitherCisTrans ()
   {
      dispatcher.setSessionID();
      return _lib.indigoMarkEitherCisTrans(self);
   }

   public IndigoObject addAtom (String symbol)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoAddAtom(self, symbol));
   }

   public void setCharge (int charge)
   {
      dispatcher.setSessionID();
      _lib.indigoSetCharge(self, charge);
   }

   public void setIsotope (int isotope)
   {
      dispatcher.setSessionID();
      _lib.indigoSetIsotope(self, isotope);
   }

   public IndigoObject addBond (IndigoObject atom, int order)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoAddBond(self, atom.self, order));
   }

   public void setBondOrder (int order)
   {
      dispatcher.setSessionID();
      _lib.indigoSetBondOrder(self, order);
   };

   public IndigoObject merge (IndigoObject other)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoMerge(self, other.self));
   }

   public int countComponents ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountComponents(self);
   }

   public int componentIndex ()
   {
      dispatcher.setSessionID();
      return _lib.indigoComponentIndex(self);
   }

   public IndigoObject iterateComponents ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateComponents(self));
   }

   public IndigoObject component (int index)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoComponent(self, index));
   }

   public int countSSSR ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountSSSR(self);
   }

   public IndigoObject iterateSSSR ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateSSSR(self));
   }

   public IndigoObject iterateSubtrees (int min_vertices, int max_vertices)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateSubtrees(self, min_vertices, max_vertices));
   }

   public IndigoObject iterateRings (int min_vertices, int max_vertices)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateRings(self, min_vertices, max_vertices));
   }

   public IndigoObject iterateEdgeSubmolecules (int min_edges, int max_edges)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, _lib.indigoIterateEdgeSubmolecules(self, min_edges, max_edges));
   }

   public int countHeavyAtoms ()
   {
      dispatcher.setSessionID();
      return _lib.indigoCountHeavyAtoms(self);
   }

   public String grossFormula ()
   {
      dispatcher.setSessionID();
      int gf = _lib.indigoGrossFormula(self);
      String result = Indigo.checkResultString(_lib.indigoToString(gf));
      _lib.indigoFree(gf);
      return result;
   }
   
   public float molecularWeight ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(_lib.indigoMolecularWeight(self));
   }

   public float mostAbundantMass()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(_lib.indigoMostAbundantMass(self));
   }

   public float monoisotopicMass()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(_lib.indigoMonoisotopicMass(self));
   }

   public String canonicalSmiles()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoCanonicalSmiles(self));
   }

   public String layeredCode()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoLayeredCode(self));
   }

   public boolean hasCoord()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoHasCoord(self)) == 1;
   }

   public boolean hasZCoord()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoHasZCoord(self)) == 1;
   }

   public boolean isChiral()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoIsChiral(self)) == 1;
   }

   public float[] xyz()
   {
      dispatcher.setSessionID();
      Pointer ptr = Indigo.checkResultPointer(_lib.indigoXYZ(self));
      return ptr.getFloatArray(0, 3);
   }

   public int countSuperatoms()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountSuperatoms(self));
   }

   public int countDataSGgroups()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountDataSGroups(self));
   }

   public int countRepeatingUnits()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountRepeatingUnits(self));
   }

   public int countMultipleGroups()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountMultipleGroups(self));
   }

   public int countGenericSGroups()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountGenericSGroups(self));
   }

   public IndigoObject iterateSuperatoms()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateSuperatoms(self)));
   }

   public IndigoObject iterateDataSGroups()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateDataSGroups(self)));
   }

   public IndigoObject iterateRepeatingUnits()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateRepeatingUnits(self)));
   }

   public IndigoObject iterateMultipleGroups()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateMultipleGroups(self)));
   }

   public IndigoObject iterateGenericSGroups()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateGenericSGroups(self)));
   }

   public String description()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoDescription(self));
   }

   public IndigoObject addDataSGroup(int[] atoms, int[] bonds, String description, String data)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoAddDataSGroup(self, atoms.length,
              atoms, bonds.length, bonds, description, data)));
   }

   public void setDataSGroupXY(float x, float y, String options)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSetDataSGroupXY(self, x, y, options));
   }

   public IndigoObject createSubmolecule (int[] vertices)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher,
              Indigo.checkResult(_lib.indigoCreateSubmolecule(self, vertices.length, vertices)));
   }

   public IndigoObject createEdgeSubmolecule (int[] vertices, int[] edges)
   {
      return new IndigoObject(dispatcher,
              Indigo.checkResult(_lib.indigoCreateEdgeSubmolecule(self, vertices.length,
              vertices, edges.length, edges)));
   }

   public void removeAtoms (int[] vertices)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoRemoveAtoms(self, vertices.length, vertices));
   }

   public float alignAtoms (int[] atom_ids, float[] desired_xyz)
   {
      if (atom_ids.length * 3 != desired_xyz.length)
         throw new IndigoException("desired_xyz[] must be exactly 3 times bigger than atom_ids[]");
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(_lib.indigoAlignAtoms(self, atom_ids.length, atom_ids, desired_xyz));
   }

   public void aromatize()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoAromatize(self));
   }

   public void dearomatize()
   {
      dispatcher.setSessionID();
     Indigo.checkResult(_lib.indigoDearomatize(self));
   }

   public void foldHydrogens()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoFoldHydrogens(self));
   }

   public void unfoldHydrogens()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoUnfoldHydrogens(self));
   }

   public void layout()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoLayout(self));
   }

   public String smiles()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoSmiles(self));
   }

   public String name()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoName(self));
   }

   public void setName(String name)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSetName(self, name));
   }

   public boolean hasProperty(String prop)
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoHasProperty(self, prop)) == 1;
   }

   public String getProperty(String prop)
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoGetProperty(self, prop));
   }

   public void setProperty(String prop, String value)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSetProperty(self, prop, value));
   }

   public void removeProperty(String prop)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoRemoveProperty(self, prop));
   }

   public IndigoObject iterateProperties ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateProperties(self)));
   }

   public String checkBadValence ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoCheckBadValence(self));
   }

   public String checkAmbiguousH ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoCheckAmbiguousH(self));
   }

   public IndigoObject fingerprint ()
   {
      return fingerprint("");
   }

   public IndigoObject fingerprint (String type)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(_lib.indigoFingerprint(self, type)));
   }

   public int countBits ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountBits(self));
   }

   public String rawData ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoRawData(self));
   }

   public int tell ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoTell(self));
   }

   public void sdfAppend (IndigoObject item)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSdfAppend(self, item.self));
   }

   public void smilesAppend (IndigoObject item)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoSmilesAppend(self, item.self));
   }

   public void rdfHeader()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoRdfHeader(self));
   }

   public void rdfAppend(IndigoObject item)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoRdfAppend(self, item.self));
   }

   public void cmlHeader()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoCmlHeader(self));
   }

   public void cmlAppend(IndigoObject item)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoCmlAppend(self, item.self));
   }

   public void cmlFooter()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoCmlFooter(self));
   }

   public IndigoObject iterateArray ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateArray(self)));
   }

   public int count ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCount(self));
   }

   public void clear ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoClear(self));
   }

   public int arrayAdd (IndigoObject other)
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoArrayAdd(self, other.self));
   }

   public IndigoObject at (int idx)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoAt(self, idx)));
   }

   public IndigoObject match (IndigoObject query)
   {
      dispatcher.setSessionID();
      int res = Indigo.checkResult(_lib.indigoMatch(self, query.self));

      if (res == 0)
         return null;
      
      return new IndigoObject(dispatcher, this, res);
   }

   public int countMatches (IndigoObject query)
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoCountMatches(self, query.self));
   }
   
   public IndigoObject iterateMatches (IndigoObject query)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this,
              Indigo.checkResult(_lib.indigoIterateMatches(self, query.self)));
   }

   public IndigoObject highlightedTarget ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher,
              Indigo.checkResult(_lib.indigoHighlightedTarget(self)));
   }

   public IndigoObject mapAtom (IndigoObject query_atom)
   {
      dispatcher.setSessionID();
      int res = Indigo.checkResult(_lib.indigoMapAtom(self, query_atom.self));
      if (res == 0)
         return null;
      return new IndigoObject(dispatcher, res);
   }

   public IndigoObject mapBond (IndigoObject query_bond)
   {
      dispatcher.setSessionID();
      int res = Indigo.checkResult(_lib.indigoMapBond(self, query_bond.self));
      if (res == 0)
         return null;
      return new IndigoObject(dispatcher, res);
   }

   public IndigoObject allScaffolds ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(_lib.indigoAllScaffolds(self)));
   }

   public IndigoObject decomposedMoleculeScaffold ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(_lib.indigoDecomposedMoleculeScaffold(self)));
   }

   public IndigoObject iterateDecomposedMolecules ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(_lib.indigoIterateDecomposedMolecules(self)));
   }

   public IndigoObject decomposedMoleculeHighlighted ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(_lib.indigoDecomposedMoleculeHighlighted(self)));
   }

   public IndigoObject decomposedMoleculeWithRGroups ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(_lib.indigoDecomposedMoleculeWithRGroups(self)));
   }

   public Iterator<IndigoObject> iterator ()
   {
      return this;
   }

   public void remove ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoRemove(self));
   }

   public void close ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoClose(self));
   }

   public IndigoObject next () throws NoSuchElementException
   {
      dispatcher.setSessionID();
      int next = Indigo.checkResult(_lib.indigoNext(self));

      if (next == 0)
         throw new NoSuchElementException("iterator has ended");

      return new IndigoObject(dispatcher, this, next);
   }

   public boolean hasNext ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoHasNext(self)) == 1;
   }

   public int index ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(_lib.indigoIndex(self));
   }

   public String toString ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(_lib.indigoToString(self));
   }

   public byte[] toBuffer ()
   {
      PointerByReference ptr = new PointerByReference();
      IntByReference size = new IntByReference();

      dispatcher.setSessionID();
      Indigo.checkResult(_lib.indigoToBuffer(self, ptr, size));
      Pointer p = ptr.getValue();
      return p.getByteArray(0, size.getValue());
   }
}
