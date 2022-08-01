import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo_inchi = IndigoInchi(indigo)

print(indigo_inchi.version())
print("*** Basic *** ")
m = indigo_inchi.loadMolecule(
    "InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2"
)
print(m.canonicalSmiles())
print(indigo_inchi.getInchi(m))
print(indigo_inchi.getWarning())

print("*** Error handling *** ")
m = indigo.loadMolecule("B1=CB=c2cc3B=CC=c3cc12")
try:
    print(indigo_inchi.getInchi(m))
except IndigoException as e:
    print("Error: %s\n" % (getIndigoExceptionText(e)))

print("*** Options *** ")


def testOpt(m, opt):
    try:
        indigo.setOption("inchi-options", opt)
        print(indigo_inchi.getInchi(m))
    except IndigoException as e:
        print("Error: %s\n" % (getIndigoExceptionText(e)))


m = indigo.loadMolecule("CC1CC(C)OC(C)N1")
testOpt(m, "")
testOpt(m, "/SUU")
testOpt(m, "-SUU")
testOpt(m, "/DoNotAddH /SUU /SLUUD")
testOpt(m, "-DoNotAddH -SUU -SLUUD")
testOpt(m, "/DoNotAddH -SUU -SLUUD")
testOpt(m, "/invalid -option")

print("*** Some molecules *** ")
indigo.setOption("inchi-options", "")
input = "InChI=1S/C6H5.C2H4O2.Hg/c1-2-4-6-5-3-1;1-2(3)4;/h1-5H;1H3,(H,3,4);"
print(input)
m2 = indigo_inchi.loadMolecule(input)

print("Arom")
try:
    inchi2 = indigo_inchi.getInchi(m2)
    print(inchi2)
    m2.aromatize()
    inchi2 = indigo_inchi.getInchi(m2)
    print(inchi2)
except IndigoException as e:
    print("Error: %s\n" % (getIndigoExceptionText(e)))

print("Arom/dearom")
try:
    inchi2 = indigo_inchi.getInchi(m2)
    print(inchi2)
    m2.aromatize()
    m2.dearomatize()
    inchi2 = indigo_inchi.getInchi(m2)
    print(inchi2)
except IndigoException as e:
    print("Error: %s\n" % (getIndigoExceptionText(e)))

print("*** Non-unqiue dearomatization ***")
try:
    m = indigo.loadMolecule("Cc1nnc2c(N)ncnc12")
    inchi = indigo_inchi.getInchi(m)
    print(inchi)
except IndigoException as e:
    print("Error: %s\n" % (getIndigoExceptionText(e)))

print("*** Aux info ***")
m = indigo.loadMolecule("Cc1nnc2c(N)ncnc12")
m.dearomatize()
inchi = indigo_inchi.getInchi(m)
aux = indigo_inchi.getAuxInfo()
print(inchi)
print(aux)
m2 = indigo_inchi.loadMolecule(aux)
print(m2.smiles())

print("*** Extra stereo info for large cycles with alternated bonds ***")
try:
    m = indigo_inchi.loadMolecule(
        "InChI=1S/C18H18/c1-2-4-6-8-10-12-14-16-18-17-15-13-11-9-7-5-3-1/h1-18H/b2-1-,3-1+,4-2+,5-3+,6-4+,7-5-,8-6-,9-7+,10-8+,11-9+,12-10+,13-11-,14-12-,15-13+,16-14+,17-15+,18-16+,18-17-"
    )
    print(m.canonicalSmiles())
except IndigoException as e:
    print("Error: %s\n" % (getIndigoExceptionText(e)))

print("*** Tetra-valent S special case ***")
mol1 = indigo.loadMolecule("C(N(N1CCC(C(=O)C2C=CC(F)=CC=2)CC1)C=SC)#N")
print(indigo_inchi.getInchi(mol1))
mol2 = indigo.loadMolecule("C(N(N1CCC(C(=O)C2C=CC(F)=CC=2)CC1)C=[SH]C)#N")
print(indigo_inchi.getInchi(mol2))

print("*** New elements support test ***")
mol1 = indigo.loadMolecule("[Nh]")
print(indigo_inchi.getInchi(mol1))
mol1 = indigo.loadMolecule("[Mc]")
print(indigo_inchi.getInchi(mol1))
mol1 = indigo.loadMolecule("[Ts]")
print(indigo_inchi.getInchi(mol1))
mol1 = indigo.loadMolecule("[Og]")
print(indigo_inchi.getInchi(mol1))

print("*** Empty input structure test ***")
mol1 = indigo.loadMolecule("")
indigo.setOption("inchi-options", "/WarnOnEmptyStructure")
try:
    print(indigo_inchi.getInchi(mol1))
except IndigoException as e:
    print(getIndigoExceptionText(e))

print("*** Load Marvin InChI test ***")
m = indigo_inchi.loadMolecule(
    "InChI=1S/C5H12/c1-3-5-4-2/h3-5H2,1-2H3\nAuxInfo=1/0/N:1,5,2,4,3/E:(1,2)(3,4)/rA:5CCCCC/rB:s1;s2;s3;s4;/rC:.935,4.785,0;2.2687,4.015,0;3.6024,4.785,0;4.936,4.015,0;6.2697,4.785,0;"
)
indigo.setOption("molfile-saving-skip-date", "1")
print(m.molfile())

print("*** Stereo info saving and loading test ***")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/racemic.mol", __file__))
m.dearomatize()
inchi = indigo_inchi.getInchi(m)
aux = indigo_inchi.getAuxInfo()
print(inchi)
print(aux)
m2 = indigo_inchi.loadMolecule(inchi + "\n" + aux)
print(m2.molfile())

m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/unknown_stereo.mol", __file__)
)
m.dearomatize()
inchi = indigo_inchi.getInchi(m)
aux = indigo_inchi.getAuxInfo()
print(inchi)
print(aux)
m2 = indigo_inchi.loadMolecule(inchi + "\n" + aux)
print(m2.molfile())

m = indigo.loadMoleculeFromFile(joinPathPy("molecules/Double.mol", __file__))
m.dearomatize()
inchi = indigo_inchi.getInchi(m)
aux = indigo_inchi.getAuxInfo()
print(inchi)
print(aux)
m2 = indigo_inchi.loadMolecule(inchi + "\n" + aux)
print(m2.molfile())

indigo.setOption("ignore-stereochemistry-errors", "1")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/Single.mol", __file__))
m.dearomatize()
inchi = indigo_inchi.getInchi(m)
aux = indigo_inchi.getAuxInfo()
print(inchi)
print(aux)
m2 = indigo_inchi.loadMolecule(inchi + "\n" + aux)
print(m2.molfile())
