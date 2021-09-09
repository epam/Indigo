import sys, os
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

def testReload (mol):
    molfile = mol.molfile()
    mol2 = indigo.loadMolecule(molfile)
    molfile2 = mol2.molfile()
    if molfile != molfile:
        print("Molecule is different after resave")
        sys.stderr.write("Molecule is different after resave")
    return molfile2


if dir_exists(joinPathPy("out", __file__)):
    rmdir(joinPathPy("out", __file__))
makedirs(joinPathPy("out", __file__))

print("********* Save and load molecule properties from SDF ****")
sdf_file_name = joinPathPy("out/sdf-properties.sdf", __file__)

saver_sd = indigo.createFileSaver(sdf_file_name, "sdf")

rdf_file_name = joinPathPy("out/rdf-properties.sdf", __file__)
saver_rd = indigo.createFileSaver(rdf_file_name, "rdf")

mol = indigo.loadMolecule("CN(c1ccccc1)C=O")

mol.setProperty("test1", "test1")
mol.setProperty("test0", "test0")
mol.setProperty("test2", "test2")
mol.setProperty("0test02", "0test02")

print("********* Initial properties")
for p in mol.iterateProperties():
    print("%s: %s" % (p.name(), p.rawData()))

saver_sd.append(mol)
saver_sd.close()

saver_rd.append(mol)
saver_rd.close()



print("********* Properties after SDF reload")
for m in indigo.iterateSDFile(sdf_file_name):
    
    for p in m.iterateProperties():
        print("%s: %s" % (p.name(), p.rawData()))
        
print("********* Properties after RDF reload")
for m in indigo.iterateRDFile(rdf_file_name):
    
    for p in m.iterateProperties():
        print("%s: %s" % (p.name(), p.rawData()))


print("********* Test remove properties")
mol = indigo.loadMolecule("CN(c1ccccc1)C=O")

mol.setProperty("test1", "test1")
mol.setProperty("test0", "test0")
mol.setProperty("test2", "test2")
mol.setProperty("0test02", "0test02")

print("********* Remove test1")
mol.removeProperty("test1")
for p in mol.iterateProperties():
    print("%s: %s" % (p.name(), p.rawData()))
    
print("********* Remove test2")
mol.removeProperty("test2")
for p in mol.iterateProperties():
    print("%s: %s" % (p.name(), p.rawData()))

print("********* Add new property test000")
mol.setProperty("test000", "test000")
for p in mol.iterateProperties():
    print("%s: %s" % (p.name(), p.rawData()))
