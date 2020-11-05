import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(joinPath("out/rsite.sdf"), "sdf")
mol = indigo.loadMolecule("CCNNCN")
mol.addRSite("R")
mol.addRSite("R")
mol.addRSite("R1")
mol.addRSite("")
a3 = mol.addRSite("R3")
print(mol.molfile())
saver.append(mol)
mol.addRSite("R1, R3")
print(mol.molfile())
saver.append(mol)
a3.resetAtom("N")
print(mol.molfile())
saver.append(mol)
a0 = mol.getAtom(0)
a0.setRSite("R4")
print(mol.molfile())
saver.append(mol)
a1 = mol.getAtom(1)
a1.resetAtom("O")
print(mol.molfile())
saver.append(mol)
a1.setRSite("R4")
a1.highlight()
print(mol.molfile())
saver.append(mol)
mol = indigo.loadMolecule("CCNNCN")
print(mol.checkRGroups())
mol.addRSite("R1")
print(mol.checkRGroups())
mol = indigo.loadMolecule('''
  Ketcher 12091616232D 1   1.00000     0.00000     0

  2  1  0     0  0            999 V2000
   13.6750   -5.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.5410   -6.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
M  APO  1   2   1
M  END
''')
print(mol.checkRGroups())
mol = indigo.loadMolecule('''$MDL  REV  1
$MOL
$HDR



$END HDR
$CTAB
  2  1  0     0  0            999 V2000
   13.6750   -5.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.5410   -6.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
M  END
$END CTAB
$RGP
  1
$CTAB
  2  1  0     0  0            999 V2000
   13.3500   -9.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.2160  -10.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
M  END
$END CTAB
$END RGP
$END MOL
''')
print(mol.checkRGroups())


mol = indigo.loadMolecule('''$MDL  REV  1 0209181741
$MOL
$HDR

  Mrv0541 02091817412D          

$END HDR
$CTAB
  6  6  0  0  0  0            999 V2000
    0.0000    0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.7145    0.4125    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    0.7145   -0.4125    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
   -0.0000   -0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.7145   -0.4125    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -0.7145    0.4125    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  1  6  1  0  0  0  0
M  LOG  1   1   0   0    
M  LOG  1   2   0   0    
M  RGP  2   3   2   6   1
M  END
$END CTAB
$RGP
  1
$CTAB
  1  0  0  0  0  0            999 V2000
    3.8966   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$END CTAB
$CTAB
  1  0  0  0  0  0            999 V2000
    6.2538   -2.4750    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$END CTAB
$END RGP
$RGP
  2
$CTAB
  1  0  0  0  0  0            999 V2000
    3.8966   -4.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$END CTAB
$CTAB
  1  0  0  0  0  0            999 V2000
    6.2538   -4.9500    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$END CTAB
$END RGP
$END MOL
''')

print(mol.molfile())

print(mol.smiles())

mol = indigo.loadMolecule('''$MDL  REV  1
$MOL
$HDR



$END HDR
$CTAB
  8  8  0     0  0            999 V2000
    0.1786    1.3406    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
    0.1786    0.5156    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8931    0.1031    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8931   -0.7219    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1786   -1.1344    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.5359   -0.7219    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.5359    0.1031    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.2503    0.5156    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  2  7  1  0  0  0  0
  7  8  1  0  0  0  0
M  RGP  2   1   1   8   2
M  LOG  1   2   1   1   0,1
M  END
$END CTAB
$RGP
  2
$CTAB
  1  0  0     0  0            999 V2000
    4.0752   -5.2594    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$END CTAB
$END RGP
$RGP
  1
$CTAB
  3  2  0     0  0            999 V2000
    4.0752   -2.3719    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.7897   -2.7844    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5042   -2.3719    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
M  END
$END CTAB
$END RGP
$END MOL
''')

print(mol.smiles())


mol = indigo.loadMolecule('''$MDL  REV  1 0212181244
$MOL
$HDR

  Mrv0541 02121812442D          

$END HDR
$CTAB
  4  3  0  0  0  0            999 V2000
    0.4125    0.7145    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -0.0000    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
    0.4125   -0.7145    0.0000 Br  0  0  0  0  0  0  0  0  0  0  0  0
   -0.8250   -0.0000    0.0000 I   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  2  4  1  0  0  0  0
M  LOG  1   1   0   0
M  RGP  1   2   1
M  END
$END CTAB
$RGP
  1
$CTAB
  7  6  0  0  0  0            999 V2000
    3.8304   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5448   -2.8875    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.2593   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.9738   -2.8875    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.9738   -3.7125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.6882   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.4027   -2.8875    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  4  6  1  0  0  0  0
  6  7  1  0  0  0  0
M  APO  2   5   2   7   1
M  END
$END CTAB
$CTAB
  7  6  0  0  0  0            999 V2000
   10.7100   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.4245   -2.8875    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.1390   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.8535   -2.8875    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
   12.8535   -3.7125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.5679   -2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.2824   -2.8875    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  4  6  1  0  0  0  0
  6  7  1  0  0  0  0
M  APO  2   5   2   7   1
M  END
$END CTAB
$END RGP
$END MOL
''')

print(mol.smiles())


m = indigo.loadMolecule("C1O[*]CO[*]1 |$;;_R2;;;_R1$,RG:_R1={C},{N},_R2={C},{N}|")
print(m.molfile())


m = indigo.loadMolecule("[*]C1CCCCC1[*] |$_R1;;;;;;;_R2$,RG:_R1={CCC},_R2={N},LOG={_R1:;;>0._R2:_R1;H;0,1}|")
print(m.molfile())

m = indigo.loadMolecule("|RG:_R1={CCCCCC}|")
print(m.molfile())
