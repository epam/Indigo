import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(joinPath("out/sgroups-instrumentation.sdf"), "sdf")


def testSGroupsInstrumentation():
    mol = indigo.loadMolecule("c1ccccc1.CCC.O.N.P")
    mol.layout()
    saver.append(mol)
    sgroup1 = mol.addDataSGroup([6, 7, 8], [6, 7], "SG", "a")
    sgroup2 = mol.addDataSGroup([9], [], "ID", "b")
    sgroup3 = mol.addDataSGroup([10], [], "ID", "c")
    sgroup4 = mol.addDataSGroup([11], [], "ID", "d")
    print(mol.molfile())
    saver.append(mol)

    mol2 = indigo.unserialize(mol.serialize())

    print(mol2.molfile())
    saver.append(mol2)

    sgroup2.setDataSGroupXY(13, 1)
    sgroup3.setDataSGroupXY(.3, .3, "relative")
    sgroup4.setDataSGroupXY(5, 6, "absolute")
    print(mol.molfile())
    saver.append(mol)

    mol2 = indigo.unserialize(mol.serialize())

    print(mol2.molfile())
    saver.append(mol2)


testSGroupsInstrumentation()

print("****** Create/Modify/Remove S-groups ********")
indigo.setOption("molfile-saving-mode", "2000")
m = indigo.loadMoleculeFromFile(joinPath("molecules/sgroups-base.mol"))
t = indigo.loadQueryMoleculeFromFile(joinPath("molecules/sgroups-template.mol"))
matcher = indigo.substructureMatcher(m)
print(matcher.countMatches(t))

for match in matcher.iterateMatches(t):
    sg = m.createSGroup("SUP", match, "Asx")
    sg.setSGroupClass("AA")
    print(sg.getSGroupName())
    print(sg.getSGroupClass())
    print(sg.getSGroupNumCrossBonds())
    print(sg.getSGroupDisplayOption())
    sg.setSGroupName("As")
    apidx = sg.addSGroupAttachmentPoint(0, -1, "??")
    print(m.molfile())
    sg.setSGroupDisplayOption(0)
    sg.deleteSGroupAttachmentPoint(apidx)
    print(m.molfile())

print(m.countSuperatoms())
sg = m.getSuperatom(0)
sg.remove()
print(m.molfile())

print("****** Get/Set Multiplier ********")
indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(joinPath("molecules/all_features_mol.mol"))
print(m.molfile())
for sg in m.iterateMultipleGroups():
    mp = sg.getSGroupMultiplier()
    print(mp)
    sg.setSGroupMultiplier(mp + 1)
    sg.setSGroupBrackets(1, 1.0, 1.0, 1.0, 2.0, 3.0, 1.0, 3.0, 2.0)

print(m.molfile())

print("****** Get/Set DataSGroup properties ********")

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(joinPath("molecules/all_features_mol.mol"))
print(m.molfile())
for sg in m.iterateDataSGroups():
    sg.setSGroupData("Test Data S-group")
    sg.setSGroupCoords(1.0, 1.0)
    sg.setSGroupDescription("SGroup Description (FIELDINFO)")
    sg.setSGroupFieldName("SGroup (FIELDNAME)")
    sg.setSGroupQueryCode("SGroup (QUERYTYPE)")
    sg.setSGroupQueryOper("SGroup (QUERYOP)")
    sg.setSGroupDisplay("attached")
    sg.setSGroupLocation("relative")
    sg.setSGroupTag("G")
    sg.setSGroupTagAlign(9)
    sg.setSGroupDataType("T")
    sg.setSGroupXCoord(4.0)
    sg.setSGroupYCoord(5.0)

print(m.molfile())

indigo.setOption("molfile-saving-mode", "2000")

print(m.molfile())

print("****** Find SGroups by properties ********")

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(joinPath("molecules/all_features_mol.mol"))
print(m.molfile())

sgs = m.findSGroups("SG_TYPE", "SUP")

for sg in sgs:
    print("Superatom with label %s found" % (m.getSuperatom(sg.getSGroupIndex())).getSGroupName())

sgs = m.findSGroups("SG_LABEL", "Z")
print("SGroups with label Z:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_CLASS", "AA")
print("SGroups with class AA:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_DISPLAY_OPTION", "0")
print("SGroups expanded:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_BRACKET_STYLE", "0")
print("SGroups with square brackets:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_DATA", "Selection")
print("SGroups with data contains Selection:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_DATA_NAME", "comment")
print("SGroups with data field name comment:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_DATA_DISPLAY", "detached")
print("SGroups with detached data field:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_DATA_LOCATION", "relative")
print("SGroups with relative data field:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_ATOMS", "103, 104")
print("SGroups with atoms 103 and 104:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

sgs = m.findSGroups("SG_BONDS", "249, 245")
print("SGroups with bonds 245 and 249:")
for sg in sgs:
    print("SGroup Index = %d " % sg.getSGroupIndex() + ", SGroup Type = %s" % sg.getSGroupType())

print("****** Remove SGroups and check hierarchy ********")

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(joinPath("molecules/sgroups-V3000.mol"))
print(m.molfile())

sgs = m.findSGroups("SG_TYPE", "GEN")

for sg in sgs:
    print("Generic SGroup was found")
    sg.remove()

print(m.molfile())

sgs = m.findSGroups("SG_TYPE", "SUP")

for sg in sgs:
    print("Superatom SGroup was found")
    sg.remove()

print(m.molfile())

print("******  Data and Multiple SGroups manipulation ********")

m = indigo.loadMoleculeFromFile(joinPath("molecules/rep-dat.mol"))
indigo.setOption("render-atom-ids-visible", "true")
renderer.renderToFile(m, joinPath('out/result_1.png'))

# print multiple group information by index

try:
    mul_group = m.getMultipleGroup(-1)
except IndigoException as e:
    print("%s" % (getIndigoExceptionText(e)))

try:
    mul_group = m.getMultipleGroup(0)
except IndigoException as e:
    print("%s" % (getIndigoExceptionText(e)))

try:
    mul_group = m.getMultipleGroup(2)
except IndigoException as e:
    print("%s" % (getIndigoExceptionText(e)))

mul_group = m.getMultipleGroup(1)
print("Multiple group # %s atoms:" % mul_group.index())
for atom in mul_group.iterateAtoms():
    print("   %s" % atom.index())

mul_group.remove()
renderer.renderToFile(m, joinPath('out/result_2.png'))

# print data s-group description and data
data_group = m.getDataSGroup(0)
print("data s-group description = %s" % data_group.description())
print("data s-group data = %s" % data_group.data())
renderer.Dispose()