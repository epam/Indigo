using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;

namespace com.epam.indigo
{
    [TestClass]
    public class InchiTest
    {

        [TestMethod]
        public void TestInchiCalculation_BaseTest()
        {
            using var indigo = new Indigo();

            var indigoObject = indigo.loadMolecule("C1=CC=CC=C1");
            IndigoInchi indigoInchi = new IndigoInchi(indigo);
            string inchi = indigoInchi.getInchi(indigoObject);
            Assert.AreEqual("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi);
        }

        [DataRow(null)]
        [DataRow("")]
        [DataRow("/WarnOnEmptyStructure")]
        [DataRow("-WarnOnEmptyStructure")]
        [TestMethod]
        public void TestInchiCalculation_ForceParameter(string parameter)
        {
            using var indigo = new Indigo();
            var indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

            using IndigoInchi indigoInchi = new IndigoInchi(indigo);
            string inchi = indigoInchi.getInchi(indigoObject, parameter);
            Assert.AreEqual("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi);
        }

        const string orEnantiomer =
                        @"
  TestStruct 2

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 8 8 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 9.5653 -6.4929 0 0
M  V30 2 C 9.5653 -7.3196 0 0
M  V30 3 C 10.2813 -7.733 0 0
M  V30 4 C 10.9972 -7.3196 0 0 CFG=2
M  V30 5 C 10.9972 -6.4929 0 0 CFG=1
M  V30 6 C 10.2813 -6.0795 0 0
M  V30 7 O 11.7131 -6.0795 0 0
M  V30 8 N 11.7131 -7.7329 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 6
M  V30 2 1 1 2
M  V30 3 1 2 3
M  V30 4 1 3 4
M  V30 5 1 4 5
M  V30 6 1 5 6
M  V30 7 1 5 7 CFG=1
M  V30 8 1 4 8 CFG=1
M  V30 END BOND
M  V30 BEGIN COLLECTION
M  V30 MDLV30/STEREL1 ATOMS=(2 5 4)
M  V30 END COLLECTION
M  V30 END CTAB
M  END
";

        [TestMethod]
        public void TestInchiCalculation_OrEnantiomer()
        {
            using var indigo = new Indigo();
            var indigoObject = indigo.loadMolecule(orEnantiomer);

            using IndigoInchi indigoInchi = new IndigoInchi(indigo);
            string inchi = indigoInchi.getInchi(indigoObject, "/SRel");
            Assert.AreEqual("InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s2", inchi);
        }
    }
}
