using System.IO;
using FluentAssertions;
using Xunit;

namespace com.epam.indigo
{
    public class IndigoTest
    {
        [Fact]
        public void Indigo_Version_Should_NotBeNull()
        {
            using var indigo = new Indigo();
            
            indigo.version().Should().NotBe(null);
        }

        [Fact]
        public void Indigo_Fingerprint_Should_GenerateOneBitsList()
        {
            using var indigo = new Indigo();
            var indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

            indigoObject.fingerprint().oneBitsList().Should().Be("1698 1719 1749 1806 1909 1914 1971 2056");
        }

        [Fact]
        public void Indigo_Should_GenerateSmiles()
        {
            using var indigo = new Indigo();
            var molecule = indigo.loadMolecule("c1ccccc1");
            
            molecule.smiles().Should().Be("c1ccccc1");
        }

        [Fact]
        public void Indigo_Should_WorkInMultiInstanceMode()
        {
            using var indigo1 = new Indigo();
            using var indigo2 = new Indigo();
            indigo1.getSID().Should().NotBe(indigo2.getSID());
            
            var molecule1 = indigo1.loadMolecule("c1ccccc1");
            var molecule2 = indigo2.loadMolecule("c1ccccc1");
            var smiles = molecule1.smiles();
            var canonicalSmiles = molecule2.canonicalSmiles();
            smiles.Should().Be(canonicalSmiles);
        }

        [Fact]
        public void IndigoInchi_Should_GenerateInchi()
        {
            using var indigo = new Indigo();
            using var indigoInchi = new IndigoInchi(indigo);
            var m = indigo.loadMolecule("C");
            
            indigoInchi.getInchi(m).Should().Be("InChI=1S/CH4/h1H4");
        }

        [Fact]
        public void IndigoRenderer_Should_RenderToBuffer()
        {
            using var indigo = new Indigo();
            using var indigoRenderer = new IndigoRenderer(indigo);
            indigo.setOption("render-output-format", "svg");
            var m = indigo.loadMolecule("C1=CC=CC=C1");

            indigoRenderer.renderToBuffer(m).Length.Should().BePositive();
        }

        [Fact]
        public void Bingo_DatabaseFile_Should_SearchSub()
        {
            var testDbFolder = Path.Combine(Path.GetTempPath(), "indigo-db-test");
            try
            {
                using var indigo = new Indigo();
                using var bingoDb = Bingo.createDatabaseFile(indigo, testDbFolder, "molecule");
                var m = indigo.loadMolecule("C");
                bingoDb.insert(m);
                var q = indigo.loadQueryMolecule("C");
                var bingoObject = bingoDb.searchSub(q);
                var count = 0;
                while (bingoObject.next())
                {
                    count++;
                }

                count.Should().Be(1);
            }
            finally
            {
                if (Directory.Exists(testDbFolder))
                {
                    Directory.Delete(testDbFolder, true);
                }
            }
        }

        [Fact]
        public void Indigo_Cml_Should_SupportUtf8()
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
            
            cml.Should().Contain("бензол");
            cml.Should().NotContain("??????");
        }
    }
}
