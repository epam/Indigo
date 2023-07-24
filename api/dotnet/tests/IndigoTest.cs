using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;

namespace com.epam.indigo
{
    [TestClass]
    public class IndigoTest
    {
        [TestMethod]
        public void TestIndigoVersion()
        {
            using var indigo = new Indigo();
            Assert.AreNotEqual(indigo.version(), null);
        }

        [TestMethod]
        public void TestIndigoGetOneBitsList()
        {
            using var indigo = new Indigo();
            // var indigo = new Indigo();
            var indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

            Assert.AreEqual(
                "1698 1719 1749 1806 1909 1914 1971 2056",
                indigoObject.fingerprint().oneBitsList(),
                "same one bits as in string 1698 1719 1749 1806 1909 1914 1971 205");
        }

        [TestMethod]
        public void TestIndigoSmiles()
        {
            using var indigo = new Indigo();
            var molecule = indigo.loadMolecule("c1ccccc1");
            Assert.AreEqual(molecule.smiles(), "c1ccccc1");
        }

        [TestMethod]
        public void TestIndigoMultipleInstances()
        {
            using var indigo1 = new Indigo();
            using var indigo2 = new Indigo();
            Assert.AreNotEqual(indigo1.getSID(), indigo2.getSID());
            var molecule1 = indigo1.loadMolecule("c1ccccc1");
            var molecule2 = indigo2.loadMolecule("c1ccccc1");
            Assert.AreEqual(molecule1.smiles(), molecule2.canonicalSmiles());
        }

        [TestMethod]
        public void IndigoInchiTest()
        {
            using var indigo = new Indigo();
            using var indigoInchi = new IndigoInchi(indigo);
            var m = indigo.loadMolecule("C");
            Assert.AreEqual("InChI=1S/CH4/h1H4", indigoInchi.getInchi(m));
        }

        [TestMethod]
        public void IndigoRendererTest()
        {
            using var indigo = new Indigo();
            using var indigoRenderer = new IndigoRenderer(indigo);
            indigo.setOption("render-output-format", "png");
            var m = indigo.loadMolecule("C");
            Assert.IsTrue(indigoRenderer.renderToBuffer(m).Length > 0);
        }

        [TestMethod]
        public void TestBingo()
        {
            try
            {
                using var indigo = new Indigo();
                using var bingo = Bingo.createDatabaseFile(indigo, "test.db", "molecule");
                var m = indigo.loadMolecule("C");
                bingo.insert(m);
                var q = indigo.loadQueryMolecule("C");
                var bingoObject = bingo.searchSub(q);
                var count = 0;
                while (bingoObject.next())
                {
                    count++;
                }
                Assert.AreEqual(count, 1);
                bingo.close();
                Assert.IsTrue(bingo.version().Length > 0);
            }
            finally
            {
                if (File.Exists("test.db"))
                {
                    File.Delete("test.db");
                }
            }
        }

        [TestMethod]
        public void TestUtf8()
        {
            const string molfile = @"
  Ketcher 02051318482D 1   1.00000     0.00000     0

  5  4  0     0  0            999 V2000
   -4.1250   -8.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.2590   -8.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.3929   -8.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.5269   -8.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.6609   -8.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
M  STY  1   1 DAT
M  SLB  1   1   1
M  SAL   1  2   4   5
M  SDT   1 single-name                    F                         
M  SDD   1     1.6314   -1.1000    DR    ALL  1      1  
M  SED   1 single-value-бензол                                                        
M  END
";
            using var indigo = new Indigo();
            var m = indigo.loadMolecule(molfile);
            var cml = m.cml();
            Assert.IsTrue(cml.Contains("бензол"));
            Assert.IsFalse(cml.Contains("??????"));
        }

        [TestMethod]
        public void IndigoTestCopyRGroup()
        {
            using var indigo = new Indigo();
            IndigoObject mol_with_rg = indigo.loadMolecule(
                "C%91C.[*:1]%91 |$;;_R1$,RG:_R1={F%91.Cl%92.Br%93.[*:1]%91.[*:1]%92.[*:1]%93 |$;;;_AP1;_AP1;_AP1$|}|"
            );
            Assert.IsTrue(mol_with_rg.countRGroups() == 1);
            IndigoObject mol_with_no_rg = indigo.loadMolecule(
                "C%91C.[*:1]%91 |$;;_R1$|"
            );
            Assert.IsTrue(mol_with_no_rg.countRGroups() == 0);
            mol_with_rg.copyRGroups(mol_with_no_rg);
            Assert.IsTrue(mol_with_no_rg.countRGroups() == 1);
        }
    }
}
