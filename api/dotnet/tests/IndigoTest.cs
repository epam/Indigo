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
            var indigo = new Indigo();
            Console.WriteLine(indigo.getSID());
            Assert.AreNotEqual(indigo.version(), null);
            Console.WriteLine(indigo.version());
            indigo.Dispose();
        }

        [TestMethod]
        public void TestIndigoGetOneBitsList()
        {
            var indigo = new Indigo();
            var indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

            Assert.AreEqual(
                "1698 1719 1749 1806 1909 1914 1971 2056",
                indigoObject.fingerprint().oneBitsList(),
                "same one bits as in string 1698 1719 1749 1806 1909 1914 1971 205");
        }

        [TestMethod]
        public void TestIndigoSmiles()
        {
            var indigo = new Indigo();
            Console.WriteLine(indigo.getSID());
            var molecule = indigo.loadMolecule("c1ccccc1");
            Console.WriteLine(molecule.self);
            Assert.AreEqual(molecule.smiles(), "c1ccccc1");
            indigo.Dispose();
        }

        [TestMethod]
        public void TestIndigoMultipleInstances()
        {
            var indigo1 = new Indigo();
            var indigo2 = new Indigo();
            Console.WriteLine(indigo1.getSID());
            Console.WriteLine(indigo2.getSID());
            Assert.AreNotEqual(indigo1.getSID(), indigo2.getSID());
            var molecule1 = indigo1.loadMolecule("c1ccccc1");
            var molecule2 = indigo2.loadMolecule("c1ccccc1");
            Console.WriteLine(molecule1.self);
            Console.WriteLine(molecule2.self);
            Assert.AreEqual(molecule1.smiles(), molecule2.canonicalSmiles());
            indigo2.Dispose();
            indigo1.Dispose();
        }

        [TestMethod]
        public void IndigoInchiTest()
        {
            var indigo = new Indigo();
            var indigoInchi = new IndigoInchi(indigo);
            var m = indigo.loadMolecule("C");
            Assert.AreEqual("InChI=1S/CH4/h1H4", indigoInchi.getInchi(m));
        }

        [TestMethod]
        public void IndigoRendererTest()
        {
            var indigo = new Indigo();
            var indigoRenderer = new IndigoRenderer(indigo);
            indigo.setOption("render-output-format", "png");
            var m = indigo.loadMolecule("C");
            Assert.IsTrue(indigoRenderer.renderToBuffer(m).Length > 0);
        }

        [TestMethod]
        public void TestBingo()
        {
            try
            {
                var indigo = new Indigo();
                var bingo = Bingo.createDatabaseFile(indigo, "test.db", "molecule");
                bingo.close();
                System.Console.WriteLine(bingo.version());
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
    }
}
