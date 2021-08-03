import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

if not os.access(joinPath("out"), os.F_OK):
    os.mkdir(joinPath("out"))

print("**** Read SDF.GZ ****")


def readSdfAndPrintInfo(fname):
    for idx, m in enumerate(indigo.iterateSDFile(joinPath(fname))):
        print("*****")
        print(idx)
        print("Smiles:")
        print(m.smiles())
        print("Molfile:")
        print(m.molfile())
        print("Rawdata:")
        print(m.rawData())
        print("Properties:")
        for prop in m.iterateProperties():
            print("%s: %s" % (prop.name(), prop.rawData()))


readSdfAndPrintInfo('../../../../../data/molecules/basic/Compound_0000001_0000250.sdf.gz')

print("**** Save and load molecule names from SDF ****")
sdf_file_name = joinPath("out", "sdf-names.sdf")
saver = indigo.createFileSaver(sdf_file_name, "sdf")
names = []
for i in range(10):
    names.append("%d" % i)
    names.append("Name%d" % i)
    names.append("Much longer name %d" % i)

for name in names:
    m = indigo.createMolecule()
    m.setName(name)
    saver.append(m)

saver.close()


def checkMolNames(names, sdf_file_name):
    for m, name in zip(indigo.iterateSDFile(sdf_file_name), names):
        print(m.name())
        if m.name() != name:
            print("Names are different: %s != %s" % (m.name(), name))


checkMolNames(names, sdf_file_name)

print("** Use sdfAppend **")
sdf_file_name = joinPath("out", "sdf-names-2.sdf")
sdf = indigo.writeFile(sdf_file_name)

for name in names:
    m = indigo.createMolecule()
    m.setName(name)
    sdf.sdfAppend(m)

sdf.close()

checkMolNames(names, sdf_file_name)

print("**** Read SDF with invalid header ****")
readSdfAndPrintInfo('molecules/bad-header.sdf')
