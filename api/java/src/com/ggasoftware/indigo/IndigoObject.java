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
   protected Indigo dispatcher;
   private Object parent; // should keep the parent so that GC will not remove it
   protected IndigoLib _lib;

   public IndigoObject (Indigo dispatcher, int id)
   {
      this.dispatcher = dispatcher;
      this.self = id;
      _lib = Indigo.getLibrary();
   }

   public IndigoObject (Indigo dispatcher, Object parent, int id)
   {
      this.dispatcher = dispatcher;
      this.self = id;
      this.parent = parent;
      _lib = Indigo.getLibrary();
   }

   public Indigo getIndigo ()
   {
      return dispatcher;
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
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoClone(self)));
   }

   public String molfile ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoMolfile(self));
   }

   public void saveMolfile (String filename)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSaveMolfileToFile(self, filename));
   }

   public String cml ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoCml(self));
   }

   public void saveCml (String filename)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSaveCmlToFile(self, filename));
   }

   public byte[] mdlct ()
   {
      dispatcher.setSessionID();
      IndigoObject buf = dispatcher.writeBuffer();
      Indigo.checkResult(this, _lib.indigoSaveMDLCT(self, buf.self));
      return buf.toBuffer();
   }

   public void addReactant (IndigoObject molecule)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAddReactant(self, molecule.self));
   }

   public void addProduct (IndigoObject molecule)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAddProduct(self, molecule.self));
   }

   public void addCatalyst (IndigoObject molecule)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAddCatalyst(self, molecule.self));
   }

   public int countReactants ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountReactants(self));
   }

   public int countProducts ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountProducts(self));
   }

   public int countCatalysts ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountCatalysts(self));
   }

   public int countMolecules ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountMolecules(self));
   }

   public IndigoObject iterateReactants ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateReactants(self)));
   }

   public IndigoObject iterateProducts ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateProducts(self)));
   }

   public IndigoObject iterateCatalysts ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateCatalysts(self)));
   }

   public IndigoObject iterateMolecules ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateMolecules(self)));
   }

   public String rxnfile ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoRxnfile(self));
   }

   public void saveRxnfile (String filename)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSaveRxnfileToFile(self, filename));
   }

   public void automap (String mode)
   {
      if (mode == null)
         mode = "";
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAutomap(self, mode));
   }

   public int atomMappingNumber (IndigoObject reaction_atom)
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoGetAtomMappingNumber(self, reaction_atom.self));
   }

   public void setAtomMappingNumber (IndigoObject reaction_atom, int number)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetAtomMappingNumber(self, reaction_atom.self, number));
   }
   
   public void clearAAM ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClearAAM(self));
   }
   
   public IndigoObject iterateAtoms ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateAtoms(self)));
   }

   public IndigoObject iteratePseudoatoms ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIteratePseudoatoms(self)));
   }

   public IndigoObject iterateRSites ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateRSites(self)));
   }

   public IndigoObject iterateStereocenters ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateStereocenters(self)));
   }

   public IndigoObject iterateAlleneCenters ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateAlleneCenters(self)));
   }

   public IndigoObject iterateRGroups ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateRGroups(self)));
   }

   public IndigoObject iterateRGroupFragments ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateRGroupFragments(self)));
   }

   public int countAttachmentPoints ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountAttachmentPoints(self));
   }

   public boolean isPseudoatom ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoIsPseudoatom(self)) == 1;
   }

   public boolean isRSite ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoIsRSite(self)) == 1;
   }

   public int stereocenterType ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoStereocenterType(self));
   }

   public int singleAllowedRGroup ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoSingleAllowedRGroup(self));
   }

   public String symbol ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoSymbol(self));
   }

   public int degree ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoDegree(self));
   }

   public Integer charge ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(this, _lib.indigoGetCharge(self, res)) == 1)
         return res.getValue();
      return null;
   }

   public int reactingCenter (IndigoObject bond)
   {
      Object[] guard = { this, bond };
      dispatcher.setSessionID();

      IntByReference res = new IntByReference();
      if (Indigo.checkResult(guard, _lib.indigoGetReactingCenter(self, bond.self, res)) == 1)
         return res.getValue();
      throw new IndigoException(guard, "reactingCenter(): unexpected result");
   }
   
   public void setReactingCenter (IndigoObject bond, int type)
   {
      Object[] guard = { this, bond };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoSetReactingCenter(self, bond.self, type));
   }

   public Integer explicitValence ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(this, _lib.indigoGetExplicitValence(self, res)) == 1)
         return res.getValue();
      return null;
   }
   public Integer radicalElectrons ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(this, _lib.indigoGetRadicalElectrons(self, res)) == 1)
         return res.getValue();
      return null;
   }

   public int  atomicNumber ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoAtomicNumber(self));
   }

   public int isotope ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoIsotope(self));
   }

   public int valence ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoValence(self));
   }

   public Integer countHydrogens ()
   {
      IntByReference res = new IntByReference();
      dispatcher.setSessionID();
      if (Indigo.checkResult(this, _lib.indigoCountHydrogens(self, res)) == 1)
         return res.getValue();
      return null;
   }

   public int countImplicitHydrogens ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountImplicitHydrogens(self));
   }

   public void resetCharge ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoResetCharge(self));
   }

   public void resetExplicitValence ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoResetExplicitValence(self));
   }

   public void resetRadical ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoResetRadical(self));
   }

   public void resetIsotope ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoResetIsotope(self));
   }

   public void setAttachmentPoint (int order)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetAttachmentPoint(self, order));
   }

   public void clearAttachmentPoints ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClearAttachmentPoints(self));
   }

   public void removeConstraints (String type)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoRemoveConstraints(self, type));
   }

   public void addConstraint (String type, String value)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAddConstraint(self, type, value));
   }

   public void addConstraintNot (String type, String value)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAddConstraintNot(self, type, value));
   }

   public void resetStereo ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoResetStereo(self));
   }

   public void invertStereo ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoInvertStereo(self));
   }

   public int  countAtoms ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountAtoms(self));
   }

   public int countBonds ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountBonds(self));
   }

   public int countPseudoatoms ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountPseudoatoms(self));
   }

   public int countRSites ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountRSites(self));
   }

   public IndigoObject iterateBonds ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateBonds(self)));
   }

   public int bondOrder ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoBondOrder(self));
   }

   public int bondStereo ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoBondStereo(self));
   }

   public int topology ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoTopology(self));
   }

   public IndigoObject iterateNeighbors ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateNeighbors(self)));
   }

   public IndigoObject bond ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoBond(self)));
   }

   public IndigoObject getAtom (int idx)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoGetAtom(self, idx)));
   }

   public IndigoObject getBond (int idx)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoGetBond(self, idx)));
   }

   public IndigoObject source ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoSource(self)));
   }

   public IndigoObject destination ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoDestination(self)));
   }

   public void clearCisTrans ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClearCisTrans(self));
   }

   public void clearStereocenters ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClearStereocenters(self));
   }

   public void clearAlleneCenters ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClearAlleneCenters(self));
   }

   public int countStereocenters ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountStereocenters(self));
   }

   public int countAlleneCenters ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountAlleneCenters(self));
   }

   public int resetSymmetricCisTrans ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoResetSymmetricCisTrans(self));
   }

   public int markEitherCisTrans ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoMarkEitherCisTrans(self));
   }

   public IndigoObject addAtom (String symbol)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoAddAtom(self, symbol)));
   }

   public IndigoObject resetAtom (String symbol)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoResetAtom(self, symbol)));
   }

   public IndigoObject addRSite (String name)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoAddRSite(self, name)));
   }

   public IndigoObject setRSite (String name)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoSetRSite(self, name)));
   }

   public void setCharge (int charge)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetCharge(self, charge));
   }

   public void setIsotope (int isotope)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetIsotope(self, isotope));
   }

   public void setImplicitHCount (int hcount)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetImplicitHCount(self, hcount));
   }

   public IndigoObject addBond (IndigoObject atom, int order)
   {
      Object[] guard = { this, atom };
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(guard, _lib.indigoAddBond(self, atom.self, order)));
   }

   public void setBondOrder (int order)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetBondOrder(self, order));
   };

   public IndigoObject merge (IndigoObject other)
   {
      Object[] guard = { this, other };
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(guard, _lib.indigoMerge(self, other.self)));
   }

   public void highlight ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoHighlight(self));
   }

   public void unhighlight ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoUnhighlight(self));
   }

   public boolean isHighlighted ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoIsHighlighted(self)) == 1;
   }

   public int countComponents ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountComponents(self));
   }

   public int componentIndex ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoComponentIndex(self));
   }

   public IndigoObject iterateComponents ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateComponents(self)));
   }

   public IndigoObject component (int index)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoComponent(self, index)));
   }

   public int countSSSR ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountSSSR(self));
   }

   public IndigoObject iterateSSSR ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateSSSR(self)));
   }

   public IndigoObject iterateSubtrees (int min_vertices, int max_vertices)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateSubtrees(self, min_vertices, max_vertices)));
   }

   public IndigoObject iterateRings (int min_vertices, int max_vertices)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateRings(self, min_vertices, max_vertices)));
   }

   public IndigoObject iterateEdgeSubmolecules (int min_edges, int max_edges)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateEdgeSubmolecules(self, min_edges, max_edges)));
   }

   public int countHeavyAtoms ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountHeavyAtoms(self));
   }

   public String grossFormula ()
   {
      dispatcher.setSessionID();
      int gf = _lib.indigoGrossFormula(self);
      String result = Indigo.checkResultString(this, _lib.indigoToString(gf));
      _lib.indigoFree(gf);
      return result;
   }
   
   public float molecularWeight ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(this, _lib.indigoMolecularWeight(self));
   }

   public float mostAbundantMass()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(this, _lib.indigoMostAbundantMass(self));
   }

   public float monoisotopicMass()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(this, _lib.indigoMonoisotopicMass(self));
   }

   public String canonicalSmiles()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoCanonicalSmiles(self));
   }

   public String layeredCode()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoLayeredCode(self));
   }

   public boolean hasCoord()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoHasCoord(self)) == 1;
   }

   public boolean hasZCoord()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoHasZCoord(self)) == 1;
   }

   public boolean isChiral()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoIsChiral(self)) == 1;
   }

   public float[] xyz()
   {
      dispatcher.setSessionID();
      Pointer ptr = Indigo.checkResultPointer(this, _lib.indigoXYZ(self));
      return ptr.getFloatArray(0, 3);
   }

   public void setXYZ (float x, float y, float z)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetXYZ(self, x, y, z));
   }

   public void setXYZ (float[] xyz)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetXYZ(self, xyz[0], xyz[1], xyz[2]));
   }

   public int countSuperatoms()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountSuperatoms(self));
   }

   public int countDataSGgroups()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountDataSGroups(self));
   }

   public int countRepeatingUnits()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountRepeatingUnits(self));
   }

   public int countMultipleGroups()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountMultipleGroups(self));
   }

   public int countGenericSGroups()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountGenericSGroups(self));
   }

   public IndigoObject iterateSuperatoms()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateSuperatoms(self)));
   }

   public IndigoObject iterateDataSGroups()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateDataSGroups(self)));
   }

   public IndigoObject iterateRepeatingUnits()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateRepeatingUnits(self)));
   }

   public IndigoObject iterateMultipleGroups()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateMultipleGroups(self)));
   }

   public IndigoObject iterateGenericSGroups()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateGenericSGroups(self)));
   }

   public String description()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoDescription(self));
   }

   public IndigoObject addDataSGroup (int[] atoms, int[] bonds, String description, String data)
   {
      dispatcher.setSessionID();
      if (description == null)
         description = "";
      if (data == null)
         data = "";
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoAddDataSGroup(self, atoms.length,
              atoms, bonds.length, bonds, description, data)));
   }

   public IndigoObject addDataSGroup (AbstractCollection<Integer> atoms,
            AbstractCollection<Integer> bonds, String description, String data)
   {
      return addDataSGroup(Indigo.toIntArray(atoms), Indigo.toIntArray(bonds), description, data);
   }

   public void addStereocenter (int type, int v1, int v2, int v3)
   {
      addStereocenter(type, v1, v2, v3, -1);
   }

   public void addStereocenter (int type, int v1, int v2, int v3, int v4)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAddStereocenter(self, type, v1, v2, v3, v4));
   }
   
   public void setDataSGroupXY(float x, float y, String options)
   {
      dispatcher.setSessionID();

      if (options == null)
         options = "";
      
      Indigo.checkResult(this, _lib.indigoSetDataSGroupXY(self, x, y, options));
   }

   public IndigoObject createSubmolecule (int[] vertices)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher,
              Indigo.checkResult(this, _lib.indigoCreateSubmolecule(self, vertices.length, vertices)));
   }

   public IndigoObject createSubmolecule (AbstractCollection<Integer> vertices)
   {
      return createSubmolecule(Indigo.toIntArray(vertices));
   }

   public IndigoObject createEdgeSubmolecule (int[] vertices, int[] edges)
   {
      return new IndigoObject(dispatcher,
              Indigo.checkResult(this, _lib.indigoCreateEdgeSubmolecule(self, vertices.length,
              vertices, edges.length, edges)));
   }

   public IndigoObject createEdgeSubmolecule (AbstractCollection<Integer> vertices, AbstractCollection<Integer> edges)
   {
      return createEdgeSubmolecule(Indigo.toIntArray(vertices), Indigo.toIntArray(edges));
   }

   public void removeAtoms (int[] vertices)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoRemoveAtoms(self, vertices.length, vertices));
   }

   public void removeAtoms (AbstractCollection<Integer> vertices)
   {
      removeAtoms(Indigo.toIntArray(vertices));
   }
   
   public float alignAtoms (int[] atom_ids, float[] desired_xyz)
   {
      if (atom_ids.length * 3 != desired_xyz.length)
         throw new IndigoException(this, "desired_xyz[] must be exactly 3 times bigger than atom_ids[]");
      dispatcher.setSessionID();
      return Indigo.checkResultFloat(this, _lib.indigoAlignAtoms(self, atom_ids.length, atom_ids, desired_xyz));
   }

   public float alignAtoms (AbstractCollection<Integer> atom_ids, AbstractCollection<Float> desired_xyz)
   {
      return alignAtoms(Indigo.toIntArray(atom_ids), Indigo.toFloatArray(desired_xyz));
   }

   public void aromatize ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoAromatize(self));
   }

   public void dearomatize ()
   {
      dispatcher.setSessionID();
     Indigo.checkResult(this, _lib.indigoDearomatize(self));
   }

   public void foldHydrogens()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoFoldHydrogens(self));
   }

   public void unfoldHydrogens()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoUnfoldHydrogens(self));
   }

   public void layout()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoLayout(self));
   }

   public String smiles()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoSmiles(self));
   }

   public String name()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoName(self));
   }

   public void setName(String name)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetName(self, name));
   }

   public byte[] serialize ()
   {
      PointerByReference ptr = new PointerByReference();
      IntByReference size = new IntByReference();

      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSerialize(self, ptr, size));
      Pointer p = ptr.getValue();
      return p.getByteArray(0, size.getValue());
   }
   
   public boolean hasProperty(String prop)
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoHasProperty(self, prop)) == 1;
   }

   public String getProperty(String prop)
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoGetProperty(self, prop));
   }

   public void setProperty(String prop, String value)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoSetProperty(self, prop, value));
   }

   public void removeProperty(String prop)
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoRemoveProperty(self, prop));
   }

   public IndigoObject iterateProperties ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateProperties(self)));
   }

   public void clearProperties ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClearProperties(self));
   }

   public String checkBadValence ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoCheckBadValence(self));
   }

   public String checkAmbiguousH ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoCheckAmbiguousH(self));
   }

   public IndigoObject fingerprint ()
   {
      return fingerprint("");
   }

   public IndigoObject fingerprint (String type)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoFingerprint(self, type)));
   }

   public int countBits ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCountBits(self));
   }

   public String rawData ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoRawData(self));
   }

   public int tell ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoTell(self));
   }

   public void sdfAppend (IndigoObject item)
   {
      Object[] guard = { this, item };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoSdfAppend(self, item.self));
   }

   public void smilesAppend (IndigoObject item)
   {
      Object[] guard = { this, item };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoSmilesAppend(self, item.self));
   }

   public void rdfHeader()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoRdfHeader(self));
   }

   public void rdfAppend(IndigoObject item)
   {
      Object[] guard = { this, item };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoRdfAppend(self, item.self));
   }

   public void cmlHeader()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoCmlHeader(self));
   }

   public void cmlAppend(IndigoObject item)
   {
      Object[] guard = { this, item };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoCmlAppend(self, item.self));
   }

   public void cmlFooter()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoCmlFooter(self));
   }

   public IndigoObject iterateArray ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateArray(self)));
   }

   public int count ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoCount(self));
   }

   public void clear ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClear(self));
   }

   public int arrayAdd (IndigoObject other)
   {
      Object[] guard = { this, other };
      dispatcher.setSessionID();
      return Indigo.checkResult(guard, _lib.indigoArrayAdd(self, other.self));
   }

   public IndigoObject at (int idx)
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoAt(self, idx)));
   }

   public void ignoreAtom (IndigoObject atom)
   {
      Object[] guard = { this, atom };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoIgnoreAtom(self, atom.self));
   }

   public void unignoreAtom (IndigoObject atom)
   {
      Object[] guard = { this, atom };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoUnignoreAtom(self, atom.self));
   }

   public void unignoreAllAtoms ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoUnignoreAllAtoms(self));
   }

   public IndigoObject match (IndigoObject query)
   {
      Object[] guard = { this, query };
      dispatcher.setSessionID();
      int res = Indigo.checkResult(guard, _lib.indigoMatch(self, query.self));

      if (res == 0)
         return null;
      
      return new IndigoObject(dispatcher, this, res);
   }

   public int countMatches (IndigoObject query)
   {
      Object[] guard = { this, query };
      dispatcher.setSessionID();
      return Indigo.checkResult(guard, _lib.indigoCountMatches(self, query.self));
   }
   
   public int countMatchesWithLimit (IndigoObject query, int embeddings_limit)
   {
      Object[] guard = { this, query };
      dispatcher.setSessionID();
      return Indigo.checkResult(guard, _lib.indigoCountMatchesWithLimit(self, query.self, embeddings_limit));
   }
   
   public IndigoObject iterateMatches (IndigoObject query)
   {
      Object[] guard = { this, query };
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this,
              Indigo.checkResult(guard, _lib.indigoIterateMatches(self, query.self)));
   }

   public IndigoObject highlightedTarget ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher,
              Indigo.checkResult(this, _lib.indigoHighlightedTarget(self)));
   }

   public IndigoObject mapAtom (IndigoObject query_atom)
   {
      Object[] guard = { this, query_atom };
      dispatcher.setSessionID();
      int res = Indigo.checkResult(guard, _lib.indigoMapAtom(self, query_atom.self));
      if (res == 0)
         return null;
      return new IndigoObject(dispatcher, this, res);
   }

   public IndigoObject mapBond (IndigoObject query_bond)
   {
      Object[] guard = { this, query_bond };
      dispatcher.setSessionID();
      int res = Indigo.checkResult(guard, _lib.indigoMapBond(self, query_bond.self));
      if (res == 0)
         return null;
      return new IndigoObject(dispatcher, this, res);
   }

   public IndigoObject allScaffolds ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoAllScaffolds(self)));
   }

   public IndigoObject decomposedMoleculeScaffold ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoDecomposedMoleculeScaffold(self)));
   }

   public IndigoObject iterateDecomposedMolecules ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, this, Indigo.checkResult(this, _lib.indigoIterateDecomposedMolecules(self)));
   }

   public IndigoObject decomposedMoleculeHighlighted ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoDecomposedMoleculeHighlighted(self)));
   }

   public IndigoObject decomposedMoleculeWithRGroups ()
   {
      dispatcher.setSessionID();
      return new IndigoObject(dispatcher, Indigo.checkResult(this, _lib.indigoDecomposedMoleculeWithRGroups(self)));
   }

   public Iterator<IndigoObject> iterator ()
   {
      return this;
   }

   public void remove ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoRemove(self));
   }

   public void close ()
   {
      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoClose(self));
   }

   public IndigoObject next () throws NoSuchElementException
   {
      dispatcher.setSessionID();
      int next = Indigo.checkResult(this, _lib.indigoNext(self));

      if (next == 0)
         throw new NoSuchElementException("iterator has ended");

      return new IndigoObject(dispatcher, this, next);
   }

   public boolean hasNext ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoHasNext(self)) == 1;
   }

   public int index ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResult(this, _lib.indigoIndex(self));
   }

   @Override
   public String toString ()
   {
      dispatcher.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoToString(self));
   }

   public byte[] toBuffer ()
   {
      PointerByReference ptr = new PointerByReference();
      IntByReference size = new IntByReference();

      dispatcher.setSessionID();
      Indigo.checkResult(this, _lib.indigoToBuffer(self, ptr, size));
      Pointer p = ptr.getValue();
      return p.getByteArray(0, size.getValue());
   }
   
   public void append (IndigoObject obj)
   {
      Object[] guard = { this, obj };
      dispatcher.setSessionID();
      Indigo.checkResult(guard, _lib.indigoAppend(self, obj.self));
   }
   
   public void optimize ()
   {
      optimize(null);
   }

   public void optimize (String options)
   {
      dispatcher.setSessionID();
      if (options == null)
         options = "";
      _lib.indigoOptimize(self, options);
   }
}
