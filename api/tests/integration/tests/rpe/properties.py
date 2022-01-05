import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()

reaction = indigo.loadQueryReaction(
    "Cl[C:1]([*:3])=O.[OH:2][*:4]>>[*:4][O:2][C:1]([*:3])=O"
)
print("=== Input Reaction ===\n{0}".format(reaction.canonicalSmiles()))

print("=== Input Monomers ===")

x = indigo.loadMolecule("CC(Cl)=O")
y = indigo.loadMolecule("OC1CCC(CC1)C(Cl)=O")
z = indigo.loadMolecule("O[C@H]1[C@H](O)[C@@H](O)[C@H](O)[C@@H](O)[C@@H]1O")

x.setProperty("name", "x")
y.setProperty("name", "y")
z.setProperty("name", "z")

i = 1
for monomer in [x, y, z]:
    print(
        "{0} = {1}".format(
            monomer.getProperty("name"), monomer.canonicalSmiles()
        )
    )
    i = i + 1

monomers_table = indigo.createArray()
monomers_table.arrayAdd(indigo.createArray())
monomers_table.arrayAdd(indigo.createArray())
monomers_table.at(0).arrayAdd(x)
monomers_table.at(0).arrayAdd(y)
monomers_table.at(1).arrayAdd(z)

output_reactions = indigo.reactionProductEnumerate(reaction, monomers_table)

i = 1
for reaction in output_reactions.iterateArray():
    print(
        "=== Output Reaction #{0} ===\n{1}".format(
            i, reaction.canonicalSmiles()
        )
    )
    for monomer in reaction.iterateReactants():
        print(
            "\t{0} = {1}".format(
                monomer.getProperty("name"), monomer.canonicalSmiles()
            )
        )
    i = i + 1
