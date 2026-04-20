import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa
from rendering import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(
    joinPathPy("out/sgroups-instrumentation.sdf", __file__), "sdf"
)


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
    sgroup3.setDataSGroupXY(0.3, 0.3, "relative")
    sgroup4.setDataSGroupXY(5, 6, "absolute")
    print(mol.molfile())
    saver.append(mol)

    mol2 = indigo.unserialize(mol.serialize())

    print(mol2.molfile())
    saver.append(mol2)


testSGroupsInstrumentation()

print("****** Create/Modify/Remove S-groups ********")
indigo.setOption("molfile-saving-mode", "2000")
m = indigo.loadMoleculeFromFile(dataPath("molecules/sgroups/sgroups-base.mol"))
t = indigo.loadQueryMoleculeFromFile(
    dataPath("molecules/sgroups/sgroups-template.mol")
)
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

print("****** SGroup Attachment Points ********")


def testIterateSGroupAttachmentPoints():
    """Test basic iteration and property access of SGroup attachment points."""
    indigo.setOption("molfile-saving-mode", "2000")
    m = indigo.loadMoleculeFromFile(
        dataPath("molecules/sgroups/sgroups-base.mol")
    )
    t = indigo.loadQueryMoleculeFromFile(
        dataPath("molecules/sgroups/sgroups-template.mol")
    )
    matcher = indigo.substructureMatcher(m)
    for match in matcher.iterateMatches(t):
        sg = m.createSGroup("SUP", match, "IterTest")
        sg.addSGroupAttachmentPoint(0, -1, "Al")
        sg.addSGroupAttachmentPoint(1, -1, "Br")
        print(
            "Iterating attachment points for SGroup '%s':" % sg.getSGroupName()
        )
        iterated = []
        for ap in sg.iterateSGroupAttachmentPoints():
            leave_atom_idx = ap.getSGroupAttachmentPointLeaveAtom()
            if leave_atom_idx is None:
                leave_atom_idx = -1
            iterated.append(
                (
                    ap.index(),
                    ap.getSGroupAttachmentPointAtomIdx(),
                    leave_atom_idx,
                    ap.getSGroupAttachmentPointLabel(),
                )
            )

        print("Attachment points count: %d" % len(iterated))
        for ap_idx, atom_idx, leave_atom_idx, label in sorted(
            iterated, key=lambda x: x[3]
        ):
            print(
                "  ap_idx=%d atom_idx=%d leave_atom_idx=%d label=%s"
                % (ap_idx, atom_idx, leave_atom_idx, label)
            )
        break  # test with first match only


def testSGroupAttachmentPointStaleHandle():
    """Test that deleted AP handles throw clear errors."""
    indigo.setOption("molfile-saving-mode", "2000")
    m = indigo.loadMoleculeFromFile(
        dataPath("molecules/sgroups/sgroups-base.mol")
    )
    t = indigo.loadQueryMoleculeFromFile(
        dataPath("molecules/sgroups/sgroups-template.mol")
    )
    matcher = indigo.substructureMatcher(m)
    for match in matcher.iterateMatches(t):
        sg = m.createSGroup("SUP", match, "StaleHandleTest")
        sg.addSGroupAttachmentPoint(0, -1, "Al")
        sg.addSGroupAttachmentPoint(1, -1, "Br")

        ap_objects = []
        for ap in sg.iterateSGroupAttachmentPoints():
            ap_objects.append(ap)

        stale_ap = ap_objects[0]
        sg.deleteSGroupAttachmentPoint(stale_ap.index())
        try:
            stale_ap.getSGroupAttachmentPointLabel()
        except IndigoException as e:
            print(
                "Expected error for removed attachment point handle: %s"
                % getIndigoExceptionText(e)
            )
        break  # test with first match only


def testSGroupAttachmentPointInvalidTarget():
    """Test that AP iteration is rejected for non-superatom S-group types."""
    indigo.setOption("molfile-saving-mode", "2000")
    m = indigo.loadMolecule("c1ccccc1")
    data_sg = m.addDataSGroup([0, 1], [], "ID", "test")
    try:
        data_sg.iterateSGroupAttachmentPoints()
    except IndigoException as e:
        print(
            "Expected error for iterateSGroupAttachmentPoints on data SGroup: %s"
            % getIndigoExceptionText(e)
        )


testIterateSGroupAttachmentPoints()
testSGroupAttachmentPointStaleHandle()
testSGroupAttachmentPointInvalidTarget()


def testClearSGroupCrossBonds():
    """Test clearing cross bonds and invalid target error path."""
    indigo.setOption("molfile-saving-mode", "2000")

    def _cross_bond_indices(sg):
        return sorted([bond.index() for bond in sg.iterateBonds()])

    # Build a deterministic superatom with >1 crossing bond
    # Chain: 0-1-2-3-4, superatom atoms = [1,2,3]
    # Crossing bonds expected: (0-1) and (3-4)
    m = indigo.loadMolecule("CSOCC")
    t = indigo.loadQueryMolecule("SOC")

    matcher = indigo.substructureMatcher(m)
    for match in matcher.iterateMatches(t):
        sg = m.createSGroup("SUP", match, "MID")

        before_count = sg.getSGroupNumCrossBonds()
        before_indices = _cross_bond_indices(sg)
        print("Cross bonds before clear: %d" % before_count)
        print("Cross bond indices before clear: %s" % before_indices)

        sg.clearSGroupCrossBonds()
        after_clear_count = sg.getSGroupNumCrossBonds()
        after_clear_indices = _cross_bond_indices(sg)
        print("Cross bonds after clear: %d" % after_clear_count)
        print("Cross bond indices after clear: %s" % after_clear_indices)

        sg.createCrossBonds()
        after_create_count = sg.getSGroupNumCrossBonds()
        after_create_indices = _cross_bond_indices(sg)
        print("Cross bonds after create: %d" % after_create_count)
        print("Cross bond indices after create: %s" % after_create_indices)
        break  # test with first match only

    data_sg = m.addDataSGroup([0, 1], [], "ID", "test")
    try:
        data_sg.clearSGroupCrossBonds()
    except IndigoException as e:
        print(
            "Expected error for clearSGroupCrossBonds on data SGroup: %s"
            % getIndigoExceptionText(e)
        )
    try:
        data_sg.createCrossBonds()
    except IndigoException as e:
        print(
            "Expected error for createCrossBonds on data SGroup: %s"
            % getIndigoExceptionText(e)
        )


print("****** SGroup Cross Bonds ********")
testClearSGroupCrossBonds()

print("****** Get/Set Multiplier ********")
indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(
    dataPath("molecules/basic/all_features_mol.mol")
)
print(m.molfile())
for sg in m.iterateMultipleGroups():
    mp = sg.getSGroupMultiplier()
    print(mp)
    sg.setSGroupMultiplier(mp + 1)
    sg.setSGroupBrackets(1, 1.0, 1.0, 1.0, 2.0, 3.0, 1.0, 3.0, 2.0)

print(m.molfile())

print("****** Get/Set DataSGroup properties ********")

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(
    dataPath("molecules/basic/all_features_mol.mol")
)
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
m = indigo.loadMoleculeFromFile(
    dataPath("molecules/basic/all_features_mol.mol")
)
print(m.molfile())

sgs = m.findSGroups("SG_TYPE", "SUP")

for sg in sgs:
    print(
        "Superatom with label %s found"
        % (m.getSuperatom(sg.getSGroupIndex())).getSGroupName()
    )

sgs = m.findSGroups("SG_LABEL", "Z")
print("SGroups with label Z:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_CLASS", "AA")
print("SGroups with class AA:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_DISPLAY_OPTION", "0")
print("SGroups expanded:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_BRACKET_STYLE", "0")
print("SGroups with square brackets:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_DATA", "Selection")
print("SGroups with data contains Selection:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_DATA_NAME", "comment")
print("SGroups with data field name comment:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_DATA_DISPLAY", "detached")
print("SGroups with detached data field:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_DATA_LOCATION", "relative")
print("SGroups with relative data field:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_ATOMS", "103, 104")
print("SGroups with atoms 103 and 104:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

sgs = m.findSGroups("SG_BONDS", "249, 245")
print("SGroups with bonds 245 and 249:")
for sg in sgs:
    print(
        "SGroup Index = %d " % sg.getSGroupIndex()
        + ", SGroup Type = %s" % sg.getSGroupType()
    )

print("****** Remove SGroups and check hierarchy ********")

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(
    dataPath("molecules/sgroups/sgroups-V3000.mol")
)
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

m = indigo.loadMoleculeFromFile(dataPath("molecules/sgroups/rep-dat.mol"))
indigo.setOption("render-atom-ids-visible", "true")
renderer.renderToFile(m, joinPathPy("out/result_1.png", __file__))

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
renderer.renderToFile(m, joinPathPy("out/result_2.png", __file__))

# print data s-group description and data
data_group = m.getDataSGroup(0)
print("data s-group description = %s" % data_group.description())
print("data s-group data = %s" % data_group.data())

print("****** Iterate and Count Atoms/Bonds on SGroups ********")


def _collect_indices(iterator):
    result = []
    for item in iterator:
        result.append(item.index())
    return result


def testIterateAtomsBondsOnSGroups():
    # 1) DataSGroup: deterministic hand-crafted case
    m = indigo.loadMolecule("CCCO")
    dsg = m.addDataSGroup([0, 1, 2], [0, 1], "ID", "iter")
    dsg_atoms = _collect_indices(dsg.iterateAtoms())
    dsg_bonds = _collect_indices(dsg.iterateBonds())
    print(
        "DataSGroup iterate: countAtoms=%d iterAtoms=%d atoms=%s"
        % (dsg.countAtoms(), len(dsg_atoms), dsg_atoms)
    )
    print(
        "DataSGroup iterate: countBonds=%d iterBonds=%d bonds=%s"
        % (dsg.countBonds(), len(dsg_bonds), dsg_bonds)
    )

    # 2) Common SGroup
    for sg in m.iterateSGroups():
        sg_atoms = _collect_indices(sg.iterateAtoms())
        sg_bonds = _collect_indices(sg.iterateBonds())
        print(
            "SGroup iterate: countAtoms=%d iterAtoms=%d"
            % (sg.countAtoms(), len(sg_atoms))
        )
        print(
            "SGroup iterate: countBonds=%d iterBonds=%d"
            % (sg.countBonds(), len(sg_bonds))
        )

    # 3) Superatom typed object
    indigo.setOption("molfile-saving-mode", "2000")
    # 3a) Try explicit empty superatom
    m_empty = indigo.loadMolecule("CC")
    try:
        empty_sup = m_empty.addSuperatom([], "EMPTY")
        empty_atoms = _collect_indices(empty_sup.iterateAtoms())
        empty_bonds = _collect_indices(empty_sup.iterateBonds())
        print(
            "Empty Superatom iterate: countAtoms=%d iterAtoms=%d"
            % (empty_sup.countAtoms(), len(empty_atoms))
        )
        print(
            "Empty Superatom iterate: countBonds=%d iterBonds=%d"
            % (empty_sup.countBonds(), len(empty_bonds))
        )
    except IndigoException as e:
        print("%s" % (getIndigoExceptionText(e)))

    # 3b) Superatom from template match
    m2 = indigo.loadMoleculeFromFile(
        dataPath("molecules/sgroups/sgroups-base.mol")
    )
    t2 = indigo.loadQueryMoleculeFromFile(
        dataPath("molecules/sgroups/sgroups-template.mol")
    )
    matcher2 = indigo.substructureMatcher(m2)
    for match in matcher2.iterateMatches(t2):
        sup = m2.createSGroup("SUP", match, "IterAtomsBonds")
        sup_atoms = _collect_indices(sup.iterateAtoms())
        sup_bonds = _collect_indices(sup.iterateBonds())
        print(
            "Superatom iterate: countAtoms=%d iterAtoms=%d"
            % (sup.countAtoms(), len(sup_atoms))
        )
        print(
            "Superatom iterate: countBonds=%d iterBonds=%d"
            % (sup.countBonds(), len(sup_bonds))
        )
        break  # test with first match only

    # 4) MultipleGroup from fixture
    m3 = indigo.loadMoleculeFromFile(dataPath("molecules/sgroups/rep-dat.mol"))
    mul = m3.getMultipleGroup(1)
    mul_atoms = _collect_indices(mul.iterateAtoms())
    mul_bonds = _collect_indices(mul.iterateBonds())
    print(
        "MultipleGroup iterate: countAtoms=%d iterAtoms=%d"
        % (mul.countAtoms(), len(mul_atoms))
    )
    print(
        "MultipleGroup iterate: countBonds=%d iterBonds=%d"
        % (mul.countBonds(), len(mul_bonds))
    )


testIterateAtomsBondsOnSGroups()

if isIronPython():
    saver.Dispose()
    renderer.Dispose()
    indigo.Dispose()
