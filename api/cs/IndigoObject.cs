using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace com.gga.indigo
{
   public unsafe class IndigoObject : IEnumerable, IDisposable
   {
      public int self;
      private Indigo dispatcher;

      public IndigoObject (Indigo dispatcher, int id)
      {
         this.dispatcher = dispatcher;
         this.self = id;
      }

      ~IndigoObject ()
      {
         Dispose();
      }

      public void Dispose ()
      {
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
         return new IndigoObject(dispatcher, Indigo.indigoIterateReactants(self));
      }

      public System.Collections.IEnumerable iterateProducts ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateProducts(self));
      }

      public System.Collections.IEnumerable iterateMolecules ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateMolecules(self));
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
         return new IndigoObject(dispatcher, Indigo.indigoIterateAtoms(self));
      }

      public System.Collections.IEnumerable iteratePseudoatoms ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIteratePseudoatoms(self));
      }

      public System.Collections.IEnumerable iterateRSites ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateRSites(self));
      }

      public System.Collections.IEnumerable iterateRGroups ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateRGroups(self));
      }

      public System.Collections.IEnumerable iterateRGroupFragments ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateRGroupFragments(self));
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

      public int singleAllowedRGroup ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoSingleAllowedRGroup(self);
      }

      public string pseudoatomLabel ()
      {
         dispatcher.setSessionID();
         return new String(Indigo.indigoPseudoatomLabel(self));
      }

      public int atomDegree ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoAtomDegree(self);
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
         return new IndigoObject(dispatcher, Indigo.indigoIterateBonds(self));
      }

      public int bondOrder ()
      {
         return Indigo.indigoBondOrder(self);
      }

      public int bondStereo ()
      {
         return Indigo.indigoBondStereo(self);
      }

      public void cisTransClear ()
      {
         dispatcher.setSessionID();
         Indigo.indigoCisTransClear(self);
      }

      public void stereocentersClear ()
      {
         dispatcher.setSessionID();
         Indigo.indigoStereocentersClear(self);
      }

      public int countStereocenters ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountStereocenters(self);
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

      public int countComponents ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoCountComponents(self);
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

      public System.Collections.IEnumerable iterateProperties ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateProperties(self));
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

      public void arrayAdd (IndigoObject item)
      {
         dispatcher.setSessionID();
         Indigo.indigoArrayAdd(self, item.self);
      }

      public IndigoObject arrayAt (int index)
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoArrayAt(self, index));
      }

      public int arrayCount ()
      {
         dispatcher.setSessionID();
         return Indigo.indigoArrayCount(self);
      }

      public void arrayClear ()
      {
         dispatcher.setSessionID();
         Indigo.indigoArrayClear(self);
      }

      public System.Collections.IEnumerable iterateArray ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoIterateArray(self));
      }

      public IndigoObject matchHighlight ()
      {
         dispatcher.setSessionID();
         return new IndigoObject(dispatcher, Indigo.indigoMatchHighlight(self));
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
         return new IndigoObject(dispatcher, Indigo.indigoIterateDecomposedMolecules(self));
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
            yield return new IndigoObject(dispatcher, next);
         }
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