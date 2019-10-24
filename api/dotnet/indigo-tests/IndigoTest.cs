using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace com.epam.indigo
{
    [TestClass]
    public class IndigoTest
    {
        [TestMethod]
        public void TestIndigoVersion()
        {
            Indigo indigo = new Indigo();
            Console.WriteLine(indigo.getSID());
            Assert.AreNotEqual(indigo.version(), null);
            Console.WriteLine(indigo.version());
            indigo.Dispose();
        }

        [TestMethod]
        public void TestIndigoSmiles()
        {
            Indigo indigo = new Indigo();
            Console.WriteLine(indigo.getSID());
            var molecule = indigo.loadMolecule("c1ccccc1");
            Console.WriteLine(molecule.self);
            Assert.AreEqual(molecule.smiles(), "c1ccccc1");                        
            indigo.Dispose();
        }

        [TestMethod]
        public void TestIndigoMultipleInstances()
        {
            Indigo indigo1 = new Indigo();
            Indigo indigo2 = new Indigo();
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
    }
}
