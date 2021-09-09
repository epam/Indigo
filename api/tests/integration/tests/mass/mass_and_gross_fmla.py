import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

print("*** Molecules ***")


def handleMolecule(mol):
    print('%0.3f %0.3f %0.3f %s %s' % (
        mol.molecularWeight(), mol.monoisotopicMass(), mol.mostAbundantMass(), mol.grossFormula(), mol.massComposition()))


def handleMoleculeWithExceptions(m):
    try:
        print(round(m.molecularWeight(), 3))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    try:
        print(round(m.mostAbundantMass(), 3))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    try:
        print(round(m.monoisotopicMass(), 3))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    try:
        print(m.massComposition())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    try:
        print(m.grossFormula())
    except IndigoException as e:
        print(getIndigoExceptionText(e))


for item in indigo.iterateSDFile(joinPathPy('molecules/mass.sdf', __file__)):
    handleMolecule(item)
handleMolecule(indigo.loadMolecule("CS=C"))
handleMolecule(indigo.loadMolecule("C[S]=C"))
for item in indigo.iterateSDFile(joinPathPy('molecules/elements.sdf', __file__)):
    handleMolecule(item)

print("*** Molecule fragments ***")
m = indigo.loadMoleculeFromFile(joinPathPy('molecules/components.mol', __file__))
big_fragment = None
for comp in m.iterateComponents():
    comp_mol = comp.clone()
    if big_fragment is None:
        big_fragment = comp_mol
    elif comp_mol.molecularWeight() > big_fragment.molecularWeight():
        big_fragment = comp_mol

print(big_fragment.smiles())

print("*** Invalid valence ***")
try:
    print(indigo.loadMoleculeFromFile(joinPathPy("molecules/invalid.mol", __file__)).grossFormula())
except IndigoException as e:
    print("Exception:  %s" % (getIndigoExceptionText(e)))

print("*** INDSP-222 [indigo-general] Problem with monoisotopic mass calculation in KNIME molecule properties ***")
m = indigo.loadMolecule("CBr")
handleMolecule(m)
m = indigo.loadMolecule("[C].[C].[C].[C].[C].[C].[C].[C].[O].[O].[O].[O].[O].[O].[O].[O].[Ru].[Ru]")
handleMolecule(m)

m = indigo.loadMolecule("ClCCN(CCCl)C1=CC=C(C=C1)C1N(CC2=C(Cl)C=C(C=C2)N(CCCl)CCCl)CCCN1CC1=C(Cl)C=C(C=C1)N(CCCl)CCCl")
handleMolecule(m)

m = indigo.loadMolecule("BrCCCCCCCCCCBr.BrCCCCCCCCCCBr.BrCCCCCCCCCCBr.BrCCCCCCCCCCCBr")
handleMolecule(m)

m = indigo.loadMolecule("CC(=O)C1=CC2=C(O[SnH2]O2)C=C1C(C)=O.CC(=O)C1=CC2=C(O[SnH2]O2)C=C1C(C)=O.CC(=O)C1=CC2=C(O[SnH2]O2)C=C1C(C)=O.CC(=O)C1=CC2=C(O[SnH2]O2)C=C1C(C)=O")
handleMolecule(m)

m = indigo.loadMolecule("[Sn].[Sn].[Sn].[Sn]")
handleMolecule(m)

m = indigo.loadMolecule("[Sn].[Sn].[Sn].[Sn].[Sn].[Sn].[Sn].[Sn].[Sn].[Sn].[Sn].[Sn]")
handleMolecule(m)

print("*** Unambigious Hydrogens in aromatic form ***")
m = indigo.loadMolecule("Cc1cnc(C)n1")
try:
    handleMolecule(m)
except IndigoException as e:
    print(getIndigoExceptionText(e))

# This should work fine
m = indigo.loadMolecule("n1cccnc1C")
print(m.grossFormula())
handleMolecule(m)

m = indigo.loadMolecule('')
handleMolecule(m)

m = indigo.loadMolecule('''
  Ketcher 11031611352D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
    4.1250   -6.1500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.9910   -6.6500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8571   -6.1500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT 
M  SMT   1 n
M  SAL   1  1   2
M  SBL   1  2   1   2
M  SDI   1  4    4.5580   -7.1500    4.5580   -5.6500
M  SDI   1  4    5.4240   -5.6500    5.4240   -7.1500
M  END
''')
handleMoleculeWithExceptions(m)

m = indigo.loadMolecule('''
  Ketcher 11031612052D 1   1.00000     0.00000     0

 18 17  0     0  0            999 V2000
    6.7750   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.6410   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.5071   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.3731   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2391   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1051   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9712   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.8372   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.7032   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.5692   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4353   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.3013   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.1673   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.0333   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.8994   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.7654   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.6314   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.4974   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
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
 16 17  1  0     0  0
 17 18  1  0     0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT 
M  SMT   1 n
M  SAL   1  2   4   5
M  SBL   1  2   3   5
M  SDI   1  4    8.9401   -8.2500    8.9401   -6.7500
M  SDI   1  4   10.6721   -6.7500   10.6721   -8.2500
M  STY  1   2 SRU
M  SLB  1   2   2
M  SCN  1   2 HT 
M  SMT   2 k
M  SAL   2  3  11  12  13
M  SBL   2  2  10  13
M  SDI   2  4   15.0022   -8.2500   15.0022   -6.7500
M  SDI   2  4   17.6003   -6.7500   17.6003   -8.2500
M  END
''')
handleMoleculeWithExceptions(m)
m = indigo.loadMolecule('CCCCCCCCCCCCCCCCCC')
handleMoleculeWithExceptions(m)
m = indigo.loadMolecule('''
git Ketcher 11031612072D 1   1.00000     0.00000     0

 18 17  0     0  0            999 V2000
    6.7750   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.6410   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.5071   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.3731   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2391   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1051   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9712   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.8372   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.7032   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.5692   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4353   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.3013   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.1673   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.0333   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.8994   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.7654   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.6314   -7.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.4974   -7.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
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
 16 17  1  0     0  0
 17 18  1  0     0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT 
M  SMT   1 n
M  SAL   1  2   4   5
M  SBL   1  2   3   5
M  SDI   1  4    8.9401   -8.2500    8.9401   -6.7500
M  SDI   1  4   10.6721   -6.7500   10.6721   -8.2500
M  STY  1   2 SRU
M  SLB  1   2   2
M  SCN  1   2 HT 
M  SMT   2 k
M  SAL   2  3  11  12  13
M  SBL   2  2  10  13
M  SDI   2  4   15.0022   -8.2500   15.0022   -6.7500
M  SDI   2  4   17.6003   -6.7500   17.6003   -8.2500
M  STY  1   3 SRU
M  SLB  1   3   3
M  SCN  1   3 HT 
M  SMT   3 n
M  SAL   3  1   8
M  SBL   3  2   7   8
M  SDI   3  4   12.4042   -8.2500   12.4042   -6.7500
M  SDI   3  4   13.2702   -6.7500   13.2702   -8.2500
M  END
''')
handleMoleculeWithExceptions(m)
m = indigo.loadMolecule('''
  Ketcher 11231618132D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
   12.0750   -9.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.9410   -9.9000    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
   12.9410  -10.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0750  -11.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.2090  -10.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.2090   -9.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  RGP  1   2   1
M  END
''')
handleMoleculeWithExceptions(m)
m = indigo.loadMolecule('''
  Ketcher 11231618102D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
   12.0750   -9.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.9410   -9.9000    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
   12.9410  -10.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0750  -11.4000    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
   11.2090  -10.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.2090   -9.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  RGP  2   2   1   4   2
M  END
''')
handleMoleculeWithExceptions(m)

# Check option mass-skip-error-on-pseudoatoms
indigo.setOption('mass-skip-error-on-pseudoatoms', False)
handleMoleculeWithExceptions(m)


# Check option gross-formula-add-rsites
print("*** R-sites in gross formula ***")
indigo.setOption("gross-formula-add-rsites", True)
m = indigo.loadMolecule('''
  Ketcher 12091616032D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
   10.7500   -8.4500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.6160   -8.9500    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
   11.6160   -9.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.7500  -10.4500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.8840   -9.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.8840   -8.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  RGP  1   2   1
M  END
''')
handleMoleculeWithExceptions(m)
indigo.setOption("gross-formula-add-rsites", False)
handleMoleculeWithExceptions(m)


# Check option gross-formula-add-isotopes
print("*** Isotopic gross formula ***")
indigo.setOption("gross-formula-add-isotopes", True)
m = indigo.loadMolecule('''
  Ketcher  2 81812332D 1   1.00000     0.00000     0

  9  8  0     0  0            999 V2000
   10.0250   -7.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.8910   -8.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.7571   -7.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.6231   -8.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.4891   -7.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.3551   -8.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.2212   -7.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.4891   -6.9000    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   11.7571   -6.9000    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
  5  8  1  0     0  0
  3  9  1  0     0  0
M  ISO  4   2  11   4  14   8   2   9   3
M  END
''')
handleMoleculeWithExceptions(m)
print("*** Gross formula ignoring isotopes ***")
indigo.setOption("gross-formula-add-isotopes", False)
handleMoleculeWithExceptions(m)
