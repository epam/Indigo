import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *


indigo = Indigo()

smilesList = [
    'O(C1CCCC1)C1C=C(C2(CC(C(OC)=O)C(=O)CC2)C#N)C=CC=1OC',
    'O(CCCC)C1=CC=C(C(OC(C(=O)NC2C=C(C#N)C=CC=2)C)=O)C=C1',
    'ClCC(CC#N)=C',
    'O1[C@H](C)CC2C1=CC(=C(OCC)C=2)/C=C(/C(OC)=O)\C#N',
    'O(C1C(C#CC(OCC=C)=O)=CC=CC=1)C',
    'O=C(NC1C(CC)=CC=CC=1)/C(=C/C1=CC2C(CC(N(CCC)C=2C=C1)(C)C)C)/C#N',
    'IC1C(OCC(=O)NCCC#N)=CC=CC=1',
    'O=C(NC1=CC=C(C#N)C=C1)C[NH+](C1CCCCC1)CC(=O)NC1=CC=C(C#N)C=C1',
    'O=C(NC1CCCC1)C([NH+](CCC#N)C)C1=CC=C(C)C=C1',
    'F/C(=C/CCC1CCC(OC(=O)C2=CC=C(CCC)C=C2)CC1)/C#N',
    'O=C(NC1=CC=C(CC)C=C1)C(=CC1=CC=C(NC(=O)C)C=C1)C#N',
    'C'
]

out_triple_dir = joinPathPy("out/triple", __file__)
if dir_exists(out_triple_dir):
    rmdir(out_triple_dir)
makedirs(out_triple_dir)

for qs in smilesList:
    bingo = Bingo.createDatabaseFile(indigo, out_triple_dir, 'molecule', '')
    bingo.insert(indigo.loadMolecule(qs))
    qm = indigo.loadQueryMolecule(qs)
    print('Query: {0}'.format(qs))
    result = bingo.searchSub(qm, '')
    while result.next():
        print('\t#{0}'.format(result.getCurrentId()))
    bingo.close()
