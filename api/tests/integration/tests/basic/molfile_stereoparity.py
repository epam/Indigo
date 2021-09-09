import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1");

print("This test checks correct saving of parity values for stereocenters for atoms")
print("For molfile 3000 format parity value is stored after an atom with CFG=val")
print("For molfile 2000 format parity value is stored in the 3rd value after an atom label")

print("Molecules:")
for mol, num in zip(indigo.iterateSDFile(joinPathPy("molecules/stereo_parity.sdf", __file__)), range(100000)):
    print("\n*** Molecule #%d ***" % (num + 1))
    saving_modes = [ "2000", "3000", "auto" ]
    for mode in saving_modes:
        indigo.setOption("molfile-saving-mode", mode);
        print("molfile-saving-mode = %s" % (mode))
        print(mol.molfile())

print("Query molecules:")
fnames = [ "molecules/stereo_parity_query.sdf", "molecules/stereo_parity.sdf" ]

for f in fnames:
    for mol, num in zip(indigo.iterateSDFile(joinPathPy(f, __file__)), range(100000)):
        qmol = indigo.loadQueryMolecule(mol.rawData())
        print("\n*** Query molecule #%d ***" % (num + 1))
        saving_modes = [ "2000", "3000", "auto" ]
        for mode in saving_modes:
            indigo.setOption("molfile-saving-mode", mode);
            print("molfile-saving-mode = %s" % (mode))
            print(qmol.molfile())
