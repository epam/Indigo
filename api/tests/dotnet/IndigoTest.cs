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

        [TestMethod]
        public void TestLoadKetcher()
        {
            var molfile = @"
  Ketcher 08191612012D 1   1.00000     0.00000     0

 36 35  0     0  0            999 V2000
    7.8000   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.6660   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.5321   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3981   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.2641   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.1301   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.9962   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.8622   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.7282   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.5942   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.4603   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.3264   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.1924   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.0584   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.9245   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.7905   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.6660   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.5321   -6.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3981   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.2641   -6.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.9962   -9.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.8622   -6.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.7282   -9.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.5942   -6.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.3264   -9.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.1924   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.0584   -9.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.4976   -9.2321    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.4635   -8.9733    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.1706   -9.6804    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.9245   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.7223   -8.0074    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.2388  -10.1980    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.2905   -7.6590    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.6301   -6.6590    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.9603   -7.1590    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
  7  8  1  0     0  0
  8  9  1  0     0  0
  9 10  1  0     0  0
 10 11  1  0     0  0
 11 12  1  0     0  0
 12 13  1  0     0  0
 13 14  1  0     0  0
 14 15  1  0     0  0
 15 16  1  0     0  0
  2 17  1  1     0  0
  3 18  1  1     0  0
  4 19  1  1     0  0
  5 20  1  1     0  0
  7 21  1  1     0  0
  8 22  1  1     0  0
  9 23  1  1     0  0
 10 24  1  1     0  0
 12 25  1  1     0  0
 13 26  1  1     0  0
 14 27  1  1     0  0
 16 28  1  0     0  0
 28 29  1  0     0  0
 29 30  1  0     0  0
 15 31  1  1     0  0
 29 32  1  1     0  0
 28 33  1  1     0  0
 16 34  1  1     0  0
  6 35  1  1     0  0
 11 36  1  1     0  0
M END
";
            var indigo = new Indigo();
            indigo.setOption("molfile-saving-skip-date", true);
            indigo.setOption("molfile-saving-add-stereo-desc", true);
            var m = indigo.loadMolecule(molfile);
            System.Console.WriteLine(m.molfile());
        }
    }
}
