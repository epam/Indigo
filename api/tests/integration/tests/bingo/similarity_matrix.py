import sys
sys.path.append('../../common')
from env_indigo import *

if not os.access(joinPathPy("out", __file__),os.F_OK):
    os.makedirs(joinPathPy("out", __file__))

def getTuple(search):
    topn_ids = []
    topn_sims = []
    while search.next():
           cur_sim = search.getCurrentSimilarityValue()
           cur_id = search.getCurrentId()
           topn_ids.append(cur_id)
           topn_sims.append(cur_sim)

    return topn_ids, topn_sims

def searchSim(bingo, q, minSim, maxSim, metric, verify):
    print("  {0}, {1}, {2}:".format(minSim, maxSim, metric))
    result = bingo.searchSim(q, minSim, maxSim, metric)
    rm = result.getIndigoObject()
    while result.next():
        sim = result.getCurrentSimilarityValue()
        print("      %4d: %0.3f" % (result.getCurrentId(), sim))

        if verify:
            q.aromatize()
            rm.aromatize()
            vsim = indigo.similarity(rm, q, metric)
            if abs(sim - vsim) > 1e-4:
                print("        Error:")
                print("          sim  = %0.6f" % (sim))
                print("          vsim = %0.6f" % (vsim))
                print("          " + rm.smiles())
                print("          " + q.smiles())

    result.close()

def testTopN(bingo, q, limit):
    ids, sims = getTuple(bingo.searchSimTopN(q, limit, 0.3, 'tanimoto'))
    print("  TopN results : {0}".format(len(ids)))
    for i in range(0, len(ids)):
        print("      %4d: %0.3f" % (ids[i], sims[i]))

def testTopNWithExtFP(bingo, q, limit):
    fp = q.fingerprint("sim")
    ids, sims = getTuple(bingo.searchSimTopNWithExtFP(q, limit, 0.3, fp, 'tanimoto'))
    print("  TopN results (with external FP): {0}".format(len(ids)))
    for i in range(0, len(ids)):
        print("      %4d: %0.3f" % (ids[i], sims[i]))

indigo = Indigo()
bingo = Bingo.createDatabaseFile(indigo, joinPathPy('out/db_molecule_sim', __file__), 'molecule', '')

indigo.setOption("fp-sim-qwords", 8)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)
indigo.setOption("fp-ext-enabled", False)


wrongStructures = 0
for index, mol in enumerate(indigo.iterateSmilesFile(joinPathPy('molecules/sample_2000_1.smi', __file__))):
    try:
        bingo.insert(mol, index)
    except BingoException as e:
        print('Structure {0} excluded: {1}'.format(index, getIndigoExceptionText(e)))
        wrongStructures += 1
    if not (index % 1000):
        print('Processed {0} structures...'.format(index))

print('Finished indexing {0} structures. {1} wrong structures excluded'.format(index, wrongStructures))

for index, mol in enumerate(indigo.iterateSmilesFile(joinPathPy('molecules/sample_2000_1.smi', __file__))):
    print("** Query %d: %s" % (index, mol.smiles()))
    searchSim(bingo, mol, 0.9, 1, 'tanimoto', False)
    searchSim(bingo, mol, 0.9, 1, 'tversky 0.3 0.7', False)
    searchSim(bingo, mol, 0.9, 1, 'euclid-sub', False)

print("*** Similarity search with verification **** ")
# Search smaller set with verification
for index, mol in enumerate(indigo.iterateSmilesFile(joinPathPy('molecules/sample_200.smi', __file__))):
    print("** Query %d: %s" % (index, mol.smiles()))
    searchSim(bingo, mol, 0.9, 1, 'tanimoto', True)
    searchSim(bingo, mol, 0.9, 1, 'tversky 0.3 0.7', True)
    searchSim(bingo, mol, 0.9, 1, 'euclid-sub', True)
    testTopN(bingo, mol, 10)
    testTopNWithExtFP(bingo, mol, 10)

bingo.close()

print("*** Similarity search verification small example **** ")
bingo = Bingo.createDatabaseFile(indigo, joinPathPy('out/db_molecule_sim_small', __file__), 'molecule', '')
bingo.insert(indigo.loadMolecule("Fc1cccc(NC(c2ccc(NC(C(C)N3CCC(C(N4CCCC4C(Nc4cccc(C(Nc5ccccc5)=O)c4)=O)=O)CC3)=O)c(C)c2)=O)c1"))
q = indigo.loadMolecule("O=C1NCCN(C(=O)c2ccc(-c3ccccc3)cc2)C1(C)C")
searchSim(bingo, q, 0.5, 1, 'tanimoto', True)
searchSim(bingo, q, 0.5, 1, 'euclid-sub', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.1 0.9', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.3 0.7', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.5 0.5', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.7 0.3', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.9 0.1', True)

bingo.close()
