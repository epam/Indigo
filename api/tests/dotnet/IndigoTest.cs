using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

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
