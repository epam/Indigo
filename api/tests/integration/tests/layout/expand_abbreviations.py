import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "true")

m_sets = [
    "molecules/abbreviations_test.sdf",
    "molecules/abbreviations_tests2.sdf",
    "molecules/abbreviations_tests3.sdf",
]

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(
    joinPathPy("out/abbreviations_test_out.sdf", __file__), "SDF"
)
saver_failed = indigo.createFileSaver(
    joinPathPy("out/abbreviations_test_failed.sdf", __file__), "SDF"
)

for set in m_sets:
    print(set)
    for mol in indigo.iterateSDFile(joinPathPy(set, __file__)):
        print(mol.smiles())
        mol.saveMolfile(joinPathPy("out/last_abbr.mol", __file__))

        try:
            mol.expandAbbreviations()
        except IndigoException as e:
            print("Exception:  %s" % (getIndigoExceptionText(e)))

        saver.append(mol)
        print(" ->  " + mol.smiles())

        for a in mol.iterateAtoms():
            if a.isPseudoatom():
                saver_failed.append(mol)
                break


print("####### Test expandAbbreviations for CML")

cm_mol = indigo.loadMolecule(
    '<?xml version="1.0" ?> <cml>     <molecule title="">         <atomArray>             <atom id="a0" elementType="C" x2="-1.0018" y2="1.5616" />             <atom id="a1" elementType="C" x2="-1.7163" y2="1.1491" />             <atom id="a2" elementType="C" x2="-1.7163" y2="0.3241" />             <atom id="a3" elementType="C" x2="-1.0018" y2="-0.0884" />             <atom id="a4" elementType="C" x2="-0.2873" y2="0.3241" />             <atom id="a5" elementType="N" x2="-0.2873" y2="1.1491" />             <atom id="a6" elementType="NO2" x2="0.6334" y2="0.1768" />             <atom id="a7" elementType="SiPr" x2="-2.4307" y2="1.5616" />             <atom id="a8" elementType="NO2" x2="-1.1727" y2="-1.0253" />             <atom id="a9" elementType="NO2" x2="0.4566" y2="1.9165" />             <atom id="a10" elementType="C" x2="-2.4308" y2="-0.0884" />             <atom id="a11" elementType="C" x2="-2.4308" y2="-0.9134" />             <atom id="a12" elementType="C" x2="-3.1452" y2="-1.3259" />             <atom id="a13" elementType="C" x2="-3.1452" y2="-2.1509" />             <atom id="a14" elementType="Ph" x2="-3.8597" y2="-2.5634" />             <atom id="a15" elementType="COOH" x2="-4.3017" y2="-0.9723" />             <atom id="a16" elementType="COOH" x2="-1.9004" y2="-2.8875" />             <atom id="a17" elementType="C" x2="-5.705" y2="1.4879" />             <atom id="a18" elementType="NO2" x2="-4.7843" y2="1.3406" />             <atom id="a19" elementType="C" x2="2.103" y2="-2.1951" />             <atom id="a20" elementType="NO2" x2="3.0237" y2="-2.3424" />         </atomArray>         <bondArray>             <bond atomRefs2="a0 a1" order="1" />             <bond atomRefs2="a0 a5" order="1" />             <bond atomRefs2="a1 a2" order="1" />             <bond atomRefs2="a2 a3" order="1" />             <bond atomRefs2="a3 a4" order="1" />             <bond atomRefs2="a4 a5" order="1" />             <bond atomRefs2="a4 a6" order="1" />             <bond atomRefs2="a1 a7" order="1" />             <bond atomRefs2="a3 a8" order="1" />             <bond atomRefs2="a5 a9" order="1" />             <bond atomRefs2="a2 a10" order="1" />             <bond atomRefs2="a10 a11" order="1" />             <bond atomRefs2="a11 a12" order="1" />             <bond atomRefs2="a12 a13" order="1" />             <bond atomRefs2="a13 a14" order="1" />             <bond atomRefs2="a12 a15" order="1" />             <bond atomRefs2="a13 a16" order="1" />             <bond atomRefs2="a17 a18" order="1" />             <bond atomRefs2="a19 a20" order="1" />         </bondArray>     </molecule> </cml>'
)
# Should be no exception
print("before {}".format(cm_mol.countAtoms()))
cm_mol.expandAbbreviations()
print("after {}".format(cm_mol.countAtoms()))

print("#3915 user abbreviations")
print("####### Test custom abbreviations loaded at runtime")

custom_mol = indigo.createMolecule()
c = custom_mol.addAtom("C")
pseudo = custom_mol.addAtom("Zzz")
c.addBond(pseudo, 1)

print("before custom load: {}".format(custom_mol.countAtoms()))
custom_mol.expandAbbreviations()
print("after unknown abbreviation: {}".format(custom_mol.countAtoms()))

indigo.loadAbbreviations(
    '<abbreviations><item name="Zzz" expansion="[*]CCCC"/></abbreviations>'
)
custom_mol.expandAbbreviations()
print("after custom load: {}".format(custom_mol.countAtoms()))

indigo.resetAbbreviations()
custom_mol2 = indigo.createMolecule()
c2 = custom_mol2.addAtom("C")
pseudo2 = custom_mol2.addAtom("Zzz")
c2.addBond(pseudo2, 1)
custom_mol2.expandAbbreviations()
print("after reset: {}".format(custom_mol2.countAtoms()))

indigo.loadAbbreviationsFromFile(
    joinPathPy("molecules/custom_abbreviations.xml", __file__)
)
custom_mol3 = indigo.createMolecule()
c3 = custom_mol3.addAtom("C")
pseudo3 = custom_mol3.addAtom("Test")
c3.addBond(pseudo3, 1)
custom_mol3.expandAbbreviations()
print("after load from file: {}".format(custom_mol3.countAtoms()))

print("####### Test that a user-defined abbreviation overrides a built-in one")

builtin_mol = indigo.createMolecule()
c4 = builtin_mol.addAtom("C")
pseudo4 = builtin_mol.addAtom("Ph")
c4.addBond(pseudo4, 1)
builtin_mol.expandAbbreviations()
print("Ph before override: {}".format(builtin_mol.countAtoms()))

indigo.loadAbbreviations(
    '<abbreviations><item name="Ph" expansion="[*]C"/></abbreviations>'
)

override_mol = indigo.createMolecule()
c5 = override_mol.addAtom("C")
pseudo5 = override_mol.addAtom("Ph")
c5.addBond(pseudo5, 1)
override_mol.expandAbbreviations()
print("Ph after override: {}".format(override_mol.countAtoms()))
