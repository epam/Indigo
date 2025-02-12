import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to RDF ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "multi",
    "2404-metadata_detect",
    "pathway1",
    "pathway2",
    "pathway3",
    "pathway4",
    "pathway5",
    "pathway6",
    "pathway7",
    "pathway8",
    "pathway9",
    "pathway10",
    "pathway11",
    "pathway12",
    "pathway_merge1",
    "pathway_merge2",
    "multi_merge1",
    "multi_merge2",
    "multi_merge3",
    "multi_merge4",
    "multi_merge5",
    "multi_merge6",
]

files.sort()
for filename in files:
    try:
        ket = indigo.loadReactionFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        try:
            ket = indigo.loadQueryReactionFromFile(
                os.path.join(root, filename + ".ket")
            )
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

    buffer = indigo.writeBuffer()
    rdfSaver = indigo.createSaver(buffer, "rdf")
    buffer.rdfHeader()
    for rxn in ket.iterateReactions():
        rdfSaver.append(rxn.clone())
    rdfSaver.close()
    rdf = buffer.toString()

    # with open(os.path.join(ref_path, filename) + ".rdf", "w") as file:
    #    file.write(rdf)

    with open(os.path.join(ref_path, filename) + ".rdf", "r") as file:
        rdf_ref = file.read()

    diff = find_diff(rdf_ref, rdf)
    if not diff:
        print(filename + ".rdf:SUCCEED")
    else:
        print(filename + ".rdf:FAILED")
        print(diff)
