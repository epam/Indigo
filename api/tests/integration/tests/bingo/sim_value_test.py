import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

if not os.path.exists(joinPathPy("out/sim_value_test", __file__)):
    makedirs(joinPathPy("out/sim_value_test", __file__))

indigo = Indigo()
bingo = Bingo.createDatabaseFile(indigo, joinPathPy('out/sim_value_test/db_molecule_sim', __file__), 'molecule', '')
 

def searchSim(bingo, q, minSim, maxSim, metric, verify):
    print("  {0}, {1}, {2}:".format(minSim, maxSim, metric))
    result = bingo.searchSim(q, minSim, maxSim, metric)
    rm = result.getIndigoObject()
    while result.next():
        sim = result.getCurrentSimilarityValue()
        print("      %4d: %0.4f" % (result.getCurrentId(), sim))

        if verify:
            #q.aromatize()
            #rm.aromatize()
            vsim = indigo.similarity(rm, q, metric)
            vsim2 = indigo.similarity(q, rm, metric)
            if abs(sim - vsim) > 1e-4:
                print("        Error:")
                print("          sim  = %0.6f" % (sim))
                print("          vsim = %0.6f" % (vsim))
                print("          vsim2 = %0.6f" % (vsim2))
                print("          " + rm.smiles())
                print("          " + q.smiles())

bingo = Bingo.createDatabaseFile(indigo, joinPathPy('out/sim_value_test/db_molecule_sim_small', __file__), 'molecule', '')
bingo.insert(indigo.loadMolecule("Fc1cccc(NC(c2ccc(NC(C(C)N3CCC(C(N4CCCC4C(Nc4cccc(C(Nc5ccccc5)=O)c4)=O)=O)CC3)=O)c(C)c2)=O)c1"))
q = indigo.loadMolecule("O=C1NCCN(C(=O)c2ccc(-c3ccccc3)cc2)C1(C)C")
searchSim(bingo, q, 0.5, 1, 'tanimoto', True)
searchSim(bingo, q, 0.5, 1, 'euclid-sub', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.1 0.9', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.3 0.7', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.5 0.5', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.7 0.3', True)
searchSim(bingo, q, 0.5, 1, 'tversky 0.9 0.1', True)