import errno
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()


if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(joinPathPy("out/basic.rdf", __file__), "rdf")

print("*** Test 1 ***")
mol = indigo.loadMolecule("")
print(mol.smiles())
mol.foldHydrogens()
print(mol.smiles())
rxn = indigo.createReaction()
rxn.addProduct(mol)
rxn.layout()
print(rxn.smiles())

print("*** Test 2 ***")
mol = indigo.loadMolecule("[H][H]")
print(mol.smiles())
mol.foldHydrogens()
print(mol.smiles())
rxn = indigo.createReaction()
rxn.addProduct(mol)
rxn.layout()
print(rxn.smiles())

print("*** Test 3 ***")
mol = indigo.loadMolecule("")
print(mol.smiles())
mol.foldHydrogens()
print(mol.smiles())
rxn = indigo.createReaction()
rxn.addProduct(indigo.loadMolecule("CCCC"))
rxn.addProduct(mol)
rxn.addReactant(indigo.loadMolecule("CCCC"))
rxn.addReactant(mol)
print("Before layout:")
print("  Has coord: %s" % (rxn.hasCoord()))
print("  Has Z coord: %s" % (rxn.hasZCoord()))
rxn.layout()
print("After layout:")
print("  Has coord: %s" % (rxn.hasCoord()))
print("  Has Z coord: %s" % (rxn.hasZCoord()))
print(rxn.smiles())
print("After z set:")
for m in rxn.iterateMolecules():
    if m.countAtoms() > 0:
        m.getAtom(0).setXYZ(1, 2, 3)
        break
print("  Has coord: %s" % (rxn.hasCoord()))
print("  Has Z coord: %s" % (rxn.hasZCoord()))
rxn.layout()
print("After layout:")
print("  Has coord: %s" % (rxn.hasCoord()))
print("  Has Z coord: %s" % (rxn.hasZCoord()))
print("*** Serialize from SMILES ***")
r = indigo.loadReaction("CCCCCC>>CNCNCNCNCN")
data = r.serialize()
r2 = indigo.unserialize(data)
if r.smiles() != r2.smiles():
    sys.stderr.write(
        "Canonical smiles after serialize: %s != %s"
        % (r.smiles(), r2.smiles())
    )
print("*** Unfold hydrogens ***")
r = indigo.loadReaction("CCCCCC>>CNCNCNCNCN")
print(r.smiles())
r.unfoldHydrogens()
print(r.smiles())

print("*** Get and Map molecule ***")
rxn = indigo.loadReaction("CCC.NNN.OOO>[W]>CNC.NCN.CNC.N.OOO.CCC.NNN")
for m in rxn.iterateMolecules():
    print(
        "%d: %s, %s"
        % (m.index(), m.smiles(), rxn.getMolecule(m.index()).smiles())
    )

q = indigo.loadQueryReaction("NN.OO>>NC.N.O")
matcher = indigo.substructureMatcher(rxn)
match = matcher.match(q)
for qm in q.iterateMolecules():
    tm = match.mapMolecule(qm)
    print(tm.smiles())
    for qma in qm.iterateAtoms():
        ta = match.mapAtom(qma)
        ta.highlight()
    for qmb in qm.iterateBonds():
        tb = match.mapBond(qmb)
        tb.highlight()
saver.append(rxn)

print("*** Serialize catalysts ***")
r = indigo.loadReaction("CCCCCC>CNO>CNCNCNCNCN")
data = r.serialize()
r2 = indigo.unserialize(data)
print(r.smiles())
print(r2.smiles())
