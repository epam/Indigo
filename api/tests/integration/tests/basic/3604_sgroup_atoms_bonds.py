"""
Issue #3604: Add API methods to interact with atoms and bonds in S-groups.
https://github.com/epam/Indigo/issues/3604

This test covers new and updated SGroup API methods:
  - addSGroup(type, extindex)
  - setSGroupAtoms(atom_indices)
  - setSGroupBonds(bond_indices)  (DAT only)
  - iterateSGroupCrossBonds()
  - Updated iterateBonds/countBonds behavior
  - Updated createCrossBonds/clearSGroupCrossBonds for all types
"""

import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoException, getIndigoExceptionText  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)


# ===== addSGroup =====

print("****** addSGroup: create empty SGroups of each type ********")

mol = indigo.loadMolecule("CCCCCC")

sg_sup = mol.addSGroup("SUP", 0)
print("SUP type: {0}".format(sg_sup.getSGroupType()))
print("SUP atoms: {0}".format(sg_sup.countAtoms()))
print("SUP bonds: {0}".format(sg_sup.countBonds()))

sg_dat = mol.addSGroup("DAT", 0)
print("DAT type: {0}".format(sg_dat.getSGroupType()))
print("DAT atoms: {0}".format(sg_dat.countAtoms()))
print("DAT bonds: {0}".format(sg_dat.countBonds()))

sg_sru = mol.addSGroup("SRU", 0)
print("SRU type: {0}".format(sg_sru.getSGroupType()))
print("SRU atoms: {0}".format(sg_sru.countAtoms()))

sg_mul = mol.addSGroup("MUL", 0)
print("MUL type: {0}".format(sg_mul.getSGroupType()))
print("MUL atoms: {0}".format(sg_mul.countAtoms()))

sg_gen = mol.addSGroup("GEN", 0)
print("GEN type: {0}".format(sg_gen.getSGroupType()))
print("GEN atoms: {0}".format(sg_gen.countAtoms()))

# ===== setSGroupAtoms =====

print("****** setSGroupAtoms: set atoms on empty SGroup ********")

mol3 = indigo.loadMolecule("CCCCCC")
sg = mol3.addSGroup("SUP", 0)
sg.setSGroupAtoms([0, 1, 2])
print("atoms after set: {0}".format(sg.countAtoms()))
atoms = []
for a in sg.iterateAtoms():
    atoms.append(a.symbol())
print("atom symbols: {0}".format(" ".join(atoms)))

print("****** setSGroupAtoms: replace existing atoms ********")

sg.setSGroupAtoms([3, 4, 5])
print("atoms after replace: {0}".format(sg.countAtoms()))
indices = []
for a in sg.iterateAtoms():
    indices.append(str(a.index()))
print("atom indices: {0}".format(" ".join(indices)))

print("****** setSGroupAtoms: clear atoms ********")

sg.setSGroupAtoms([])
print("atoms after clear: {0}".format(sg.countAtoms()))

print("****** setSGroupAtoms: works for all SGroup types ********")

mol4 = indigo.loadMolecule("CCCCCC")
for stype in ["SUP", "DAT", "SRU", "GEN"]:
    sg = mol4.addSGroup(stype, 0)
    sg.setSGroupAtoms([0, 1])
    print("{0} atoms: {1}".format(stype, sg.countAtoms()))


# ===== setSGroupBonds =====

print("****** setSGroupBonds: set bonds on DAT SGroup ********")

mol5 = indigo.loadMolecule("CCCCCC")
sg = mol5.addSGroup("DAT", 0)
sg.setSGroupAtoms([0, 1, 2])
sg.setSGroupBonds([0, 1])
print("DAT bonds: {0}".format(sg.countBonds()))

print("****** setSGroupBonds: error on SUP SGroup ********")

mol6 = indigo.loadMolecule("CCCCCC")
sg = mol6.addSGroup("SUP", 0)
sg.setSGroupAtoms([0, 1, 2])
try:
    sg.setSGroupBonds([0, 1])
    print("ERROR: should have raised exception")
except IndigoException as e:
    print("Expected error: {0}".format(getIndigoExceptionText(e)))

print("****** setSGroupBonds: error on SRU SGroup ********")

mol7 = indigo.loadMolecule("CCCCCC")
sg = mol7.addSGroup("SRU", 0)
sg.setSGroupAtoms([0, 1, 2])
try:
    sg.setSGroupBonds([0, 1])
    print("ERROR: should have raised exception")
except IndigoException as e:
    print("Expected error: {0}".format(getIndigoExceptionText(e)))


# ===== iterateSGroupCrossBonds =====

print("****** iterateSGroupCrossBonds ********")

mol8 = indigo.loadMolecule("CCCCCC")
sg = mol8.addSGroup("SUP", 0)
sg.setSGroupAtoms([1, 2, 3])
sg.createCrossBonds()

cross_bonds = []
for b in sg.iterateSGroupCrossBonds():
    cross_bonds.append(str(b.index()))
print("cross bonds: {0}".format(" ".join(sorted(cross_bonds))))

print("****** iterateSGroupCrossBonds: empty ********")

mol9 = indigo.loadMolecule("CCCCCC")
sg = mol9.addSGroup("SUP", 0)
sg.setSGroupAtoms([0, 1, 2, 3, 4, 5])
sg.createCrossBonds()

count = 0
for _ in sg.iterateSGroupCrossBonds():
    count += 1
print("cross bonds when all atoms inside: {0}".format(count))


# ===== Updated countBonds/iterateBonds =====

print(
    "****** Updated countBonds: DAT returns CBONDS, others return cross bonds ********"
)

mol10 = indigo.loadMolecule("CCCCCC")

sg_dat = mol10.addSGroup("DAT", 0)
sg_dat.setSGroupAtoms([1, 2, 3])
sg_dat.setSGroupBonds([1, 2])
print("DAT countBonds (CBONDS): {0}".format(sg_dat.countBonds()))

mol11 = indigo.loadMolecule("CCCCCC")
sg_sup = mol11.addSGroup("SUP", 0)
sg_sup.setSGroupAtoms([1, 2, 3])
sg_sup.createCrossBonds()
print("SUP countBonds (cross): {0}".format(sg_sup.countBonds()))

mol12 = indigo.loadMolecule("CCCCCC")
sg_sru = mol12.addSGroup("SRU", 0)
sg_sru.setSGroupAtoms([1, 2, 3])
sg_sru.createCrossBonds()
print("SRU countBonds (cross): {0}".format(sg_sru.countBonds()))


# ===== createCrossBonds for all types =====

print("****** createCrossBonds: works for all SGroup types ********")

for stype in ["SUP", "SRU", "GEN", "MUL"]:
    mol = indigo.loadMolecule("CCCCCC")
    sg = mol.addSGroup(stype, 0)
    sg.setSGroupAtoms([1, 2, 3])
    sg.createCrossBonds()
    print("{0} createCrossBonds count: {1}".format(stype, sg.countBonds()))


# ===== clearSGroupCrossBonds for all types =====

print("****** clearSGroupCrossBonds: works for all SGroup types ********")

for stype in ["SUP", "SRU", "GEN"]:
    mol = indigo.loadMolecule("CCCCCC")
    sg = mol.addSGroup(stype, 0)
    sg.setSGroupAtoms([1, 2, 3])
    sg.createCrossBonds()
    print("{0} before clear: {1}".format(stype, sg.countBonds()))
    sg.clearSGroupCrossBonds()
    print("{0} after clear: {1}".format(stype, sg.countBonds()))


# ===== Molfile roundtrip =====

print("****** Molfile roundtrip: SUP with cross bonds ********")

indigo.setOption("molfile-saving-mode", "3000")
mol = indigo.loadMolecule("CCCCCC")
sg = mol.addSGroup("SUP", 0)
sg.setSGroupAtoms([1, 2, 3])
sg.createCrossBonds()
sg.setSGroupName("TEST_SUP")

molfile = mol.molfile()
mol2 = indigo.loadMolecule(molfile)

sg_count = 0
for sg in mol2.iterateSGroups():
    sg_count += 1
    print("type: {0}".format(sg.getSGroupType()))
    print("atoms: {0}".format(sg.countAtoms()))
    print("bonds: {0}".format(sg.countBonds()))
print("sgroup count: {0}".format(sg_count))


print(
    "****** Molfile roundtrip: DAT with containment and cross bonds ********"
)

indigo.setOption("molfile-saving-mode", "auto")
mol = indigo.loadMolecule("CCCCCC")
sg = mol.addSGroup("DAT", 0)
sg.setSGroupAtoms([1, 2, 3])
sg.setSGroupBonds([1, 2])
sg.createCrossBonds()

molfile = mol.molfile()
print("DAT molfile V3000: {0}".format("V3000" in molfile))
sgroup_lines = [l.strip() for l in molfile.split("\n") if " DAT " in l]
for l in sgroup_lines:
    print("DAT line: {0}".format(l))

mol2 = indigo.loadMolecule(molfile)
for sg in mol2.iterateDataSGroups():
    cbonds = sorted([str(b.index()) for b in sg.iterateBonds()])
    xbonds = sorted([str(b.index()) for b in sg.iterateSGroupCrossBonds()])
    print("DAT containment bonds: {0}".format(" ".join(cbonds)))
    print("DAT cross bonds: {0}".format(" ".join(xbonds)))


# ===== ext_index roundtrip V3000 =====


def get_v3000_extindex(molfile_str, sg_type="SUP"):
    """Parse V3000 molfile and return (index, extindex) for the given SGroup type."""
    for l in molfile_str.split("\n"):
        parts = l.strip().split()
        if sg_type in parts and "SGROUP" not in l:
            idx = parts.index(sg_type)
            return parts[idx - 1], parts[idx + 1]
    return None, None


print("****** ext_index: V3000 roundtrip with explicit extindex ********")

indigo.setOption("molfile-saving-mode", "3000")
mol = indigo.loadMolecule("CCCCCC")
sg = mol.addSGroup("SUP", 42)
sg.setSGroupAtoms([0, 1, 2])
sg.setSGroupName("EXT42")
print("ext_index before save: 42")

molfile = mol.molfile()
idx, ext = get_v3000_extindex(molfile)
print("roundtrip: index={0} extindex={1}".format(idx, ext))


print("****** ext_index: V3000 roundtrip auto-assign (extindex=0) ********")

mol = indigo.loadMolecule("CCCCCC")
sg = mol.addSGroup("SUP", 0)
sg.setSGroupAtoms([0, 1, 2])
sg.setSGroupName("AUTO")
print("ext_index before save: 0")

molfile = mol.molfile()
idx, ext = get_v3000_extindex(molfile)
print("roundtrip: index={0} extindex={1}".format(idx, ext))


# ===== ext_index roundtrip V2000 =====

print("****** ext_index: V2000 roundtrip with explicit extindex ********")

indigo.setOption("molfile-saving-mode", "2000")
mol = indigo.loadMolecule("CCCCCC")
sg = mol.addSGroup("SUP", 55)
sg.setSGroupAtoms([0, 1, 2])
sg.setSGroupName("EXT55")
print("ext_index before save: 55")

molfile = mol.molfile()

slb_lines = [l for l in molfile.split("\n") if "M  SLB" in l]
for l in slb_lines:
    print("V2000 SLB: {0}".format(l.strip()))

mol2 = indigo.loadMolecule(molfile)
for sg2 in mol2.iterateSGroups():
    print("V2000 roundtrip original id: {0}".format(sg2.getSGroupOriginalId()))


print("****** ext_index: addSGroup without extindex (default=0) ********")

mol = indigo.loadMolecule("CCCCCC")
sg = mol.addSGroup("GEN")
sg.setSGroupAtoms([0, 1])
print("GEN type: {0}".format(sg.getSGroupType()))
print("GEN atoms: {0}".format(sg.countAtoms()))
