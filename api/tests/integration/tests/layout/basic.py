import errno
import os
import sys

sys.path.append("../../common")

from env_indigo import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

print("*** Reaction layout testing ***")
test = indigo.loadReaction(
    "ClCC1CO1>>CN(CC1CO1)S(=O)(=O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F"
)
test.layout()
res = reactionLayoutDiff(indigo, test, "test1.rxn")
test.saveRxnfile(joinPathPy("out/test1.rxn", __file__))
print("  Result: {}".format(res))

print("*** Check relative layout for the fixed components ***")


def testLayout(m, subset):
    x = [2, 0, 1]

    m.getAtom(0).setXYZ(x[0], 0, 0)
    m.getAtom(1).setXYZ(x[0], 0.1, 0)

    m.getAtom(2).setXYZ(x[1], 0, 0)
    m.getAtom(3).setXYZ(x[1], 0.1, 0)

    m.getAtom(4).setXYZ(x[2], 1, 0)
    m.getAtom(5).setXYZ(x[2], 1.1, 0)

    sub = m.getSubmolecule(subset)
    sub.layout()

    # relative components should not be swapped
    print(m.molfile())

    nx = [m.getAtom(0).xyz()[0], m.getAtom(2).xyz()[0], m.getAtom(4).xyz()[0]]

    # Check if order has been changed
    for i in range(3):
        for j in range(3):
            if (x[i] < x[j]) != (nx[i] < nx[j]):
                print(
                    "Error: order has been changed:"
                    "x[%d]=%0.4f, x[%d]=%0.4f vs nx[%d]=%0.4f, nx[%d]=%0.4f"
                    % (i, x[i], j, x[j], i, nx[i], j, nx[j])
                )


m = indigo.loadMolecule("CC.NN.PP.OO")
testLayout(m, [6, 7])
testLayout(m, [1, 3, 5, 6, 7])  # this has to be fixed...

print("*** Molecule layout for data sgroups ***")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/sgroups/datasgroup.mol", __file__)
)
m.layout()
res = moleculeLayoutDiff(indigo, m, "test_sgroup.mol")
print("  Result: {}".format(res))
m.saveMolfile(joinPathPy("out/test_sgroup.mol", __file__))

print("*** Reaction layout for data sgroups ***")
ref = joinPathPy("molecules/sgroups/reaction_datasgroup.rxn", __file__)
test = indigo.loadReactionFromFile(ref)
test.layout()
res = reactionLayoutDiff(indigo, test, "test_sgroup.rxn")
print("  Result: {}".format(res))
test.saveRxnfile(joinPathPy("out/test_sgroup.rxn", __file__))


print("*** Check issue 468 ***")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/mol-ind-468.mol", __file__)
)
m.layout()
res = moleculeLayoutDiff(indigo, m, "mol-ind-468.mol")
print("  Result: {}".format(res))
m.saveMolfile(joinPathPy("out/mol-ind-468.mol", __file__))

print("*** Test aromatic ***")

m = indigo.loadMolecule(
    "O[C@H]1C[C@@H](O[C@@H]1CO)[n]1cc(c(=O)[nH]c1=O)C(F)(F)F"
)
m.layout()
# print(m.molfile())
res = moleculeLayoutDiff(indigo, m, "mol-indsp-93.mol")
print("  Result: {}".format(res))
m.saveMolfile(joinPathPy("out/mol-indsp-93.mol", __file__))


print("*** IND-617: fixed ends ***")
atoms = [2, 3, 4, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 24]
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ind-617-test-fixed.mol", __file__)
)
mol2 = mol.clone()
sub = mol.getSubmolecule(atoms)
sub.layout()  # IND-617 -> bug: exception

res = moleculeLayoutDiff(indigo, mol, "ind-617-test-fixed-1.mol")
print("  mol Result: {}".format(res))
res = moleculeLayoutDiff(indigo, mol2, "ind-617-test-fixed-2.mol")
print("  mol2 Result: {}".format(res))
mol2.saveMolfile(joinPathPy("out/ind-617-test-fixed-1.mol", __file__))
mol.saveMolfile(joinPathPy("out/ind-617-test-fixed-2.mol", __file__))
# Ensure that coordinates have been changed
if indigo.exactMatch(mol, mol2, "0.0000001"):
    sys.stderr.write("Error: Match should have failed")
