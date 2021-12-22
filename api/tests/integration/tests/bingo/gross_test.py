import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
bingo = Bingo.createDatabaseFile(
    indigo, joinPathPy("db_gross_mol", __file__), "molecule", ""
)

for idx, mol in enumerate(
    indigo.iterateSmilesFile(joinPathPy("molecules/gross_200.smi", __file__))
):
    bingo.insert(mol, idx)


def searchGross(bingo, q, requiredId=None, options=""):
    info = ""
    if requiredId is not None:
        info = ", requiredId={0}".format(requiredId)

    print("** searchMolFormula({0}, {1}{2}) **".format(q, repr(options), info))
    result = bingo.searchMolFormula(q, options)
    rm = result.getIndigoObject()
    requiredIdFound = False
    while result.next():
        try:
            id = result.getCurrentId()
            if id == requiredId:
                requiredIdFound = True

            mol = bingo.getRecordById(id)
            print(
                "\t{0} {1} {2}. {3}".format(
                    result.getCurrentId(),
                    rm.smiles(),
                    mol.smiles(),
                    mol.grossFormula(),
                )
            )
        except BingoException as e:
            print("\tBingoException: {0}".format(getIndigoExceptionText(e)))
    result.close()

    if requiredId is not None and not requiredIdFound:
        print(
            "Error: cannot find molecule by the same molFormula: {0} by id={1}".format(
                q, requiredId
            )
        )


print("**** Basic test ****")
searchGross(bingo, "C5H11N1")
searchGross(bingo, "C5 H11 N1")
searchGross(bingo, "C3 C2 H11 N1")
searchGross(bingo, "Cl3")
try:
    print("Wrong MolFormula:")
    searchGross(bingo, "AnyText3")
except BingoException as e:
    print("BingoException: {0}".format(getIndigoExceptionText(e)))

print("*** Search ***")
for idx, mol in enumerate(
    indigo.iterateSmilesFile(joinPathPy("molecules/gross_200.smi", __file__))
):
    searchGross(bingo, mol.grossFormula(), requiredId=idx)

bingo.close()
