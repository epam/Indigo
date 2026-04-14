import gzip
import os
import shutil
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa
    Bingo,
    BingoException,
    Indigo,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()


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


def test_database(bingo):
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
        indigo.iterateSmilesFile(
            joinPathPy("molecules/gross_200.smi", __file__)
        )
    ):
        searchGross(bingo, mol.grossFormula(), requiredId=idx)


dir_name = joinPathPy("out/backward_compatibility", __file__)
if os.path.isfile(dir_name):
    os.remove(dir_name)
if not os.path.isdir(dir_name):
    os.mkdir(dir_name)

infile_prefix = joinPathPy("molecules/mmf_storage0", __file__)

for suffix in ("win", "lin"):
    print("Test format %s" % suffix)
    infile_name = infile_prefix + "." + suffix + ".gz"
    outfile_name = dir_name + "/mmf_storage0"
    if os.path.isfile(outfile_name):
        os.remove(outfile_name)
    with gzip.open(infile_name, "rb") as f_in:
        with open(outfile_name, "wb") as f_out:
            shutil.copyfileobj(f_in, f_out)
    bingo = Bingo.loadDatabaseFile(
        indigo, joinPathPy("out/backward_compatibility", __file__)
    )
    test_database(bingo)
    bingo.close()
