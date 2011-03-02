using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace com.ggasoftware.indigo
{
   public unsafe class IndigoObject : IEnumerable, IDisposable
   {
      public int self;
      private Indigo dispatcher;
      private IndigoObject parent; // to prevent GC killing the parent object

      public IndigoObject (Indigo dispatcher, int id)
      {
         this.dispatcher = dispatcher;
         this.self = id;
      }

      public IndigoObject (Indigo dispatcher, IndigoObject parent, int id)
      {
         this.dispatcher = dispatcher;
         this.self = id;
         this.parent = parent;
      }

      ~IndigoObject ()
      {
         Dispose();
      }

      public void Dispose ()
      {
         if (dispatcher == null)
         {
            // This happens exclusively in 32-bit .NET environment
            // after an IndigoObject constructor throws an exception.
            // In fact, the object is not created in this case,
            // but for some reason the .NET VM disposes it, despite it
            // has not been initialized.
            return;
         }
         if (self >= 0)
         {
            // Check that the session is still alive
            // (.NET has no problem disposing referenced
            // objects before the objects that reference to them)
            if (dispatcher.getSID() >= 0)
            {
               dispatcher.setSessionID();
               dispatcher.free(self);
               self = -1;
            }
         }
      }

      public IndigoObject clone ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoClone(self));
      }

      public void close ()
      {
         dispatcher.setSessionID();
         Indigo.indigoClose(self);
      }

      public String molfile ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoMolfile(self));
      }

      public void saveMolfile (String filename)
      {
         dispatcher.setSessionID();
         int s = Indigo.indigoWriteFile(filename);
         Indigo.indigoSaveMolfile(self, s);
         Indigo.indigoFree(s);
      }

      public String cml ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoCml(self));
      }

      public void saveCml (String filename)
      {
         dispatcher.setSessionID();
         int s = Indigo.indigoWriteFile(filename);
         Indigo.indigoSaveCml(self, s);
         Indigo.indigoFree(s);
      }

      public byte[] mdlct ()
      {
         dispatcher.setSessionID();
         IndigoObject buf = dispatcher.writeBuffer();
         Indigo.indigoSaveMDLCT(self, buf.self);
         return buf.toBuffer();
      }

      public void addReactant (IndigoObject molecule)
      {
         dispatcher.setSessionID();
         Indigo.indigoAddReactant(self, molecule.self);
      }

      public void addProduct (IndigoObject molecule)
      {
         dispatcher.setSessionID();
         Indigo.indigoAddProduct(self, molecule.self);
      }

      public int countReactants ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountReactants(self);
      }

      public int countProducts ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountProducts(self);
      }

      public int countMolecules ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountMolecules(self);
      }

      public System.Collections.IEnumerable iterateReactants ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateReactants(self));
      }

      public System.Collections.IEnumerable iterateProducts ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateProducts(self));
      }

      public System.Collections.IEnumerable iterateMolecules ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateMolecules(self));
      }

      public String rxnfile ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoRxnfile(self));
      }

      public void saveRxnfile (String filename)
      {
         dispatcher.setSessionID();
         int s = Indigo.indigoWriteFile(filename);
         Indigo.indigoSaveRxnfile(self, s);
         Indigo.indigoFree(s);
      }

      public void automap (String mode)
      {
         if (mode == null)
            mode = "";
         dispatcher.setSessionID();
         Indigo.indigoAutomap(self, mode);
      }

      public System.Collections.IEnumerable iterateAtoms ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateAtoms(self));
      }

      public System.Collections.IEnumerable iteratePseudoatoms ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIteratePseudoatoms(self));
      }

      public System.Collections.IEnumerable iterateRSites ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateRSites(self));
      }

      public System.Collections.IEnumerable iterateStereocenters ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateStereocenters(self));
      }

      public System.Collections.IEnumerable iterateRGroups ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateRGroups(self));
      }

      public System.Collections.IEnumerable iterateRGroupFragments ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateRGroupFragments(self));
      }

      public int countAttachmentPoints ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountAttachmentPoints(self);
      }

      public bool isPseudoatom ()
      {
         dispatcher.setSessionID();
         if (Indigo.indigoIsPseudoatom(self) == 1)
            return true;
         return false;
      }

      public bool isRSite ()
      {
         dispatcher.setSessionID();
         if (Indigo.indigoIsRSite(self) == 1)
            return true;
         return false;
      }

      public int stereocenterType ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoStereocenterType(self);
      }

      public int singleAllowedRGroup ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoSingleAllowedRGroup(self);
      }

      public string symbol ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoSymbol(self));
      }

      public int degree ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoDegree(self);
      }

      public int? charge ()
      {
         int c;
         dispatcher.setSessionID();

         if (Indigo.indigoGetCharge(self, &c) == 1)
            return c;
         return null;
      }

      public int? explicitValence ()
      {
         int c;
         dispatcher.setSessionID();

         if (Indigo.indigoGetExplicitValence(self, &c) == 1)
            return c;
         return null;
      }

      public int? radicalElectrons ()
      {
         int c;
         dispatcher.setSessionID();

         if (Indigo.indigoGetRadicalElectrons(self, &c) == 1)
            return c;
         return null;
      }

      public int atomicNumber ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoAtomicNumber(self);
      }

      public int isotope ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoIsotope(self);
      }

      public int? countHydrogens ()
      {
         int h;
         dispatcher.setSessionID();

         if (Indigo.indigoCountHydrogens(self, &h) == 1)
            return h;
         return null;
      }

      public int countImplicitHydrogens ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountImplicitHydrogens(self);
      }

      public int countSuperatoms ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountSuperatoms(self);
      }

      public int countDataSGroups ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountDataSGroups(self);
      }

      public int countGenericSGroups ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountGenericSGroups(self);
      }

      public int countRepeatingUnits ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountRepeatingUnits(self);
      }

      public int countMultipleGroups ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountMultipleGroups(self);
      }

      public IndigoObject iterateSuperatoms ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateSuperatoms(self));
      }

      public IndigoObject iterateDataSGroups ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateDataSGroups(self));
      }

      public IndigoObject iterateGenericSGroups ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateGenericSGroups(self));
      }

      public IndigoObject iterateRepeatingUnits ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateRepeatingUnits(self));
      }

      public IndigoObject iterateMultipleGroups ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateMultipleGroups(self));
      }

      public string description ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoDescription(self));
      }

      public IndigoObject addDataSGroup (int[] atoms, int[] bonds, String description, String data)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoAddDataSGroup(self, atoms.Length, atoms, bonds.Length, bonds, description, data));
      }

      public void setDataSGroupXY (float x, float y, String options)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetDataSGroupXY(self, x, y, options);
      }

      public float[] xyz ()
      {
         dispatcher.setSessionID();
         float *ptr = Indigo.indigoXYZ(self);
         float[] res = new float[3];
         res[0] = ptr[0];
         res[1] = ptr[1];
         res[2] = ptr[2];
         return res;
      }

      public void resetCharge ()
      {
         dispatcher.setSessionID();
         Indigo.indigoResetCharge(self);
      }

      public void resetExplicitValence ()
      {
         dispatcher.setSessionID();
         Indigo.indigoResetExplicitValence(self);
      }

      public void resetRadical ()
      {
         dispatcher.setSessionID();
         Indigo.indigoResetRadical(self);
      }

      public void resetIsotope ()
      {
         dispatcher.setSessionID();
         Indigo.indigoResetIsotope(self);
      }

      public void setAttachmentPoint (int order)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetAttachmentPoint(self, order);
      }

      public void removeConstraints (string type)
      {
         dispatcher.setSessionID();
         Indigo.indigoRemoveConstraints(self, type);
      }

      public void addConstraint (string type, string value)
      {
         dispatcher.setSessionID();
         Indigo.indigoAddConstraint(self, type, value);
      }

      public void addConstraintNot (string type, string value)
      {
         dispatcher.setSessionID();
         Indigo.indigoAddConstraintNot(self, type, value);
      }

      public void invertStereo ()
      {
         dispatcher.setSessionID();
         Indigo.indigoInvertStereo(self);
      }

      public void resetStereo ()
      {
         dispatcher.setSessionID();
         Indigo.indigoResetStereo(self);
      }

      public int countAtoms ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountAtoms(self);
      }

      public int countBonds ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountBonds(self);
      }

      public int countPseudoatoms ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountPseudoatoms(self);
      }

      public int countRSites ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountRSites(self);
      }

      public System.Collections.IEnumerable iterateBonds ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateBonds(self));
      }

      public int bondOrder ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoBondOrder(self);
      }

      public int bondStereo ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoBondStereo(self);
      }

      public int topology ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoTopology(self);
      }

      public System.Collections.IEnumerable iterateNeighbors ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateNeighbors(self));
      }

      public IndigoObject bond ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoBond(self));
      }

      public IndigoObject getAtom (int idx)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoGetAtom(self, idx));
      }

      public IndigoObject getBond (int idx)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoGetBond(self, idx));
      }

      public IndigoObject source ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoSource(self));
      }

      public IndigoObject destination ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoDestination(self));
      }

      public void clearCisTrans ()
      {
         dispatcher.setSessionID();
         Indigo.indigoClearCisTrans(self);
      }

      public void clearStereocenters()
      {
         dispatcher.setSessionID();
         Indigo.indigoClearStereocenters(self);
      }

      public int countStereocenters ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountStereocenters(self);
      }

      public int resetSymmetricCisTrans ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoResetSymmetricCisTrans(self);
      }

      public int markEitherCisTrans ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoMarkEitherCisTrans(self);
      }

      public IndigoObject addAtom (string symbol)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoAddAtom(self, symbol));
      }

      public void setCharge (int charge)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetCharge(self, charge);
      }

      public void setIsotope (int isotope)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetIsotope(self, isotope);
      }

      public IndigoObject addBond (IndigoObject dest, int order)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoAddBond(self, dest.self, order));
      }

      public void setBondOrder (int order)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetBondOrder(self, order);
      }

      public IndigoObject merge (IndigoObject what)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoMerge(self, what.self));
      }

      public int countComponents ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountComponents(self);
      }

      public int componentIndex ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoComponentIndex(self);
      }

      public void remove ()
      {
         dispatcher.setSessionID();
         Indigo.indigoRemove(self);
      }

      public IndigoObject iterateComponents ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateComponents(self));
      }

      public IndigoObject component (int index)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoComponent(self, index));
      }

      public int countSSSR ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountSSSR(self);
      }

      public IndigoObject iterateSSSR ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateSSSR(self));
      }

      public IndigoObject iterateSubtrees (int min_vertices, int max_vertices)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateSubtrees(self, min_vertices, max_vertices));
      }

      public IndigoObject iterateRings (int min_vertices, int max_vertices)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateRings(self, min_vertices, max_vertices));
      }

      public IndigoObject iterateEdgeSubmolecules (int min_edges, int max_edges)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateEdgeSubmolecules(self, min_edges, max_edges));
      }

      public int countHeavyAtoms ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountHeavyAtoms(self);
      }

      public String grossFormula ()
      {
         dispatcher.setSessionID();
         int gf = Indigo.indigoGrossFormula(self);
         String result = new String(Indigo.indigoToString(gf));
         Indigo.indigoFree(gf);
         return result;
      }

      public float molecularWeight ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoMolecularWeight(self);
      }

      public float mostAbundantMass ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoMostAbundantMass(self);
      }

      public float monoisotopicMass ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoMonoisotopicMass(self);
      }

      public string canonicalSmiles ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoCanonicalSmiles(self));
      }

      public string layeredCode ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoLayeredCode(self));
      }

      public bool hasCoord ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoHasCoord(self) == 1;
      }

      public bool hasZCoord ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoHasZCoord(self) == 1;
      }

      public bool isChiral ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoIsChiral(self) == 1;
      }

      public IndigoObject createSubmolecule (int[] vertices)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoCreateSubmolecule(self, vertices.Length, vertices));
      }

      public IndigoObject createEdgeSubmolecule (int[] vertices, int[] edges)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoCreateEdgeSubmolecule(self,
            vertices.Length, vertices, edges.Length, edges));
      }

      public void removeAtoms (int[] vertices)
      {
         dispatcher.setSessionID();
         Indigo.indigoRemoveAtoms(self, vertices.Length, vertices);
      }

      public float alignAtoms (int[] atom_ids, float[] desired_xyz)
      {
         dispatcher.setSessionID();
         if (atom_ids.Length * 3 != desired_xyz.Length)
            throw new IndigoException("alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]");
         return Indigo.indigoAlignAtoms(self, atom_ids.Length, atom_ids, desired_xyz);
      }

      public void aromatize ()
      {
         dispatcher.setSessionID();
         Indigo.indigoAromatize(self);
      }

      public void dearomatize ()
      {
         dispatcher.setSessionID();
         Indigo.indigoDearomatize(self);
      }

      public void foldHydrogens ()
      {
         dispatcher.setSessionID();
         Indigo.indigoFoldHydrogens(self);
      }

      public void unfoldHydrogens ()
      {
         dispatcher.setSessionID();
         Indigo.indigoUnfoldHydrogens(self);
      }

      public void layout ()
      {
         dispatcher.setSessionID();
         Indigo.indigoLayout(self);
      }

      public string smiles ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoSmiles(self));
      }

      public String name ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoName(self));
      }

      public void setName (string name)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetName(self, name);
      }

      public bool hasProperty (string name)
      {
         dispatcher.setSessionID();
         return Indigo.indigoHasProperty(self, name) == 1;
      }

      public string getProperty (string name)
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoGetProperty(self, name));
      }

      public void setProperty (string name, string value)
      {
         dispatcher.setSessionID();
         Indigo.indigoSetProperty(self, name, value);
      }

      public void removeProperty (string name)
      {
         dispatcher.setSessionID();
         Indigo.indigoRemoveProperty(self, name);
      }

      public System.Collections.IEnumerable iterateProperties ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateProperties(self));
      }

      public void clearProperties ()
      {
         dispatcher.setSessionID();
         Indigo.indigoClearProperties(self);
      }

      public string checkBadValence ()
      {
         dispatcher.setSessionID();
         sbyte *res = Indigo.indigoCheckBadValence(self);
         if (res == (sbyte *)IntPtr.Zero)
            return null;
         return new String(res);
      }

      public string checkAmbiguousH ()
      {
         dispatcher.setSessionID();
         sbyte* res = Indigo.indigoCheckAmbiguousH(self);
         if (res == (sbyte*)IntPtr.Zero)
            return null;
         return new String(res);
      }

      public IndigoObject fingerprint (string type)
      {
         dispatcher.setSessionID();
         if (type == null)
            type = "";
         return new IndigoObject(dispatcher, Indigo.indigoFingerprint(self, type));
      }

      public int countBits ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountBits(self);
      }

      public string rawData ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoRawData(self));
      }

      public int tell ()
      {

         dispatcher.setSessionID();
         return Indigo.indigoTell(self);
      }

      public void sdfAppend (IndigoObject item)
      {
         dispatcher.setSessionID();
         Indigo.indigoSdfAppend(self, item.self);
      }

      public void smilesAppend (IndigoObject item)
      {
         dispatcher.setSessionID();
         Indigo.indigoSmilesAppend(self, item.self);
      }

      public void rdfHeader ()
      {
         dispatcher.setSessionID();
         Indigo.indigoRdfHeader(self);
      }

      public void rdfAppend (IndigoObject item)
      {
         dispatcher.setSessionID();
         Indigo.indigoRdfAppend(self, item.self);
      }

      public void cmlHeader ()
      {
         dispatcher.setSessionID();
         Indigo.indigoCmlHeader(self);
      }

      public void cmlAppend (IndigoObject item)
      {
         dispatcher.setSessionID();
         Indigo.indigoCmlAppend(self, item.self);
      }

      public void cmlFooter ()
      {
         dispatcher.setSessionID();
         Indigo.indigoCmlFooter(self);
      }

      public int arrayAdd (IndigoObject item)
      {
         dispatcher.setSessionID();
         return Indigo.indigoArrayAdd(self, item.self);
      }

      public IndigoObject at (int index)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoAt(self, index));
      }

      public int count ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCount(self);
      }

      public void clear ()
      {
         dispatcher.setSessionID();
         Indigo.indigoClear(self);
      }

      public System.Collections.IEnumerable iterateArray ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateArray(self));
      }

      public void ignoreAtom (IndigoObject atom)
      {
         dispatcher.setSessionID();
         Indigo.indigoIgnoreAtom(self, atom.self);
      }

      public void unignoreAllAtoms ()
      {
         dispatcher.setSessionID();
         Indigo.indigoUnignoreAllAtoms(self);
      }

      public IndigoObject match (IndigoObject query)
      {
         dispatcher.setSessionID();
         int res = Indigo.indigoMatch(self, query.self);
         if (res == 0)
            return null;
         return new IndigoObject(dispatcher, this, res);
      }

      public int countMatches (IndigoObject query)
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountMatches(self, query.self);
      }

      public System.Collections.IEnumerable iterateMatches (IndigoObject query)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateMatches(self, query.self));
      }

      public IndigoObject highlightedTarget ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoHighlightedTarget(self));
      }

      public IndigoObject mapAtom (IndigoObject query_atom)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoMapAtom(self, query_atom.self));
      }

      public IndigoObject mapBond (IndigoObject query_bond)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoMapBond(self, query_bond.self));
      }

      public IndigoObject allScaffolds ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoAllScaffolds(self));
      }

      public IndigoObject decomposedMoleculeScaffold ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoDecomposedMoleculeScaffold(self));
      }

      public System.Collections.IEnumerable iterateDecomposedMolecules ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, this, Indigo.indigoIterateDecomposedMolecules(self));
      }

      public IndigoObject decomposedMoleculeHighlighted ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoDecomposedMoleculeHighlighted(self));
      }

      public IndigoObject decomposedMoleculeWithRGroups ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoDecomposedMoleculeWithRGroups(self));
      }

      public IEnumerator GetEnumerator ()
      {
         while (true)
         {
            dispatcher.setSessionID();
            int next = Indigo.indigoNext(self);
            if (next == 0)
               break;
            yield return new IndigoObject(dispatcher, this, next);
         }
      }

      public IndigoObject next ()
      {
         dispatcher.setSessionID();
         int next = Indigo.indigoNext(self);
         if (next == 0)
            return null;
         return new IndigoObject(dispatcher, this, next);
      }

      public bool hasNext ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoHasNext(self) == 1;
      }

      public int index ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoIndex(self);
      }

      public String toString ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoToString(self));
      }

      public byte[] toBuffer ()
      {
         dispatcher.setSessionID();
         byte* buf;
         int bufsize;
         Indigo.indigoToBuffer(self, &buf, &bufsize);

         byte[] res = new byte[bufsize];
         for (int i = 0; i < bufsize; ++i)
            res[i] = buf[i];
         return res;
      }
   }
}
