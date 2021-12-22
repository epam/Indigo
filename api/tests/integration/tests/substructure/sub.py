import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
# load queries


def doMatrixTest(queries_file, targets_file):
    if targets_file.endswith("sdf"):
        targets = list(
            indigo.iterateSDFile(joinPathPy(targets_file, __file__))
        )
    elif targets_file.endswith("smiles") or targets_file.endswith("smi"):
        targets = list(
            indigo.iterateSmilesFile(joinPathPy(targets_file, __file__))
        )
    else:
        raise ValueError("Unexpected type of file: {0}".format((targets_file)))
    if queries_file.endswith("sdf"):
        queries = [
            indigo.loadQueryMolecule(x.rawData())
            for x in indigo.iterateSDFile(joinPathPy(queries_file, __file__))
        ]
    elif queries_file.endswith("smiles") or queries_file.endswith("smi"):
        queries = [
            indigo.loadQueryMolecule(x.rawData())
            for x in indigo.iterateSmilesFile(
                joinPathPy(queries_file, __file__)
            )
        ]
    else:
        raise ValueError("Unexpected type of file: {0}".format((targets_file)))
    for i in range(len(queries)):
        name = queries[i].name()
        queries[i].setName("#%s %s" % (i, name))

    query_results = {}
    for q in queries:
        query_results[q.name()] = 0

    for t in targets:
        matcher = indigo.substructureMatcher(t)
        for q in queries:
            m = matcher.match(q)
            if m:
                query_results[q.name()] += 1
                print("%s %s" % (t.name(), q.name()))
                cnt = matcher.countMatches(q)
                print("  cnt=%d" % cnt)
    print("\nResults per query:")
    for q in queries:
        print("%s: %d" % (q.name(), query_results[q.name()]))


doMatrixTest(
    "../../../../../data/molecules/basic/rand_queries_small.sdf",
    "../../../../../data/molecules/basic/thiazolidines.sdf",
)

print("**** Stereocenters ****")
doMatrixTest(
    "molecules/stereocenters/queries.sdf",
    "molecules/stereocenters/targets.sdf",
)

print("**** Explicit valence ****")
doMatrixTest(
    "molecules/explicit_valence_queries.smiles",
    "../../../../../data/molecules/basic/explicit_valence.sdf",
)
