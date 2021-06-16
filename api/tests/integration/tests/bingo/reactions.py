import sys
sys.path.append('../../common')
from env_indigo import *

import itertools

def searchSim(bingo, q, minSim, maxSim, metric=None):
    print("** searchSim({0}, {1}, {2}, {3}) **".format(q.smiles(), minSim, maxSim, metric))
    result = bingo.searchSim(q, minSim, maxSim, metric)
    rm = result.getIndigoObject()
    while result.next():
        try:
            mol = bingo.getRecordById(result.getCurrentId())
            print('\t{0} {1} {2}'.format(result.getCurrentId(), rm.smiles(), mol.smiles()))
        except BingoException as e:
            print("BingoException: {0}".format(getIndigoExceptionText(e)))
    result.close()

def searchSub(bingo, q, options=''):
    print('** searchSub({0}, {1}) **'.format(q.smiles(), repr(options)))
    result = bingo.searchSub(q, options)
    rm = result.getIndigoObject()
    while result.next():
        try:
            mol = bingo.getRecordById(result.getCurrentId())
            print('\t{0} {1} {2}'.format(result.getCurrentId(), rm.smiles(), mol.smiles()))
        except BingoException as e:
            print("\tBingoException: {0}".format(getIndigoExceptionText(e)))
    result.close()

def searchExact(bingo, q, options=''):
    print('** searchExact({0}, {1}) **'.format(q.smiles(), repr(options)))
    result = bingo.searchExact(q, options)
    rm = result.getIndigoObject()
    while result.next():
        try:
            mol = bingo.getRecordById(result.getCurrentId())
            print('\t{0} {1} {2}'.format(result.getCurrentId(), rm.smiles(), mol.smiles()))
        except BingoException as e:
            print("\tBingoException: {0}".format(getIndigoExceptionText(e)))
    result.close()

indigo = Indigo()
bingo = Bingo.createDatabaseFile(indigo, joinPath('db_reaction'), 'reaction', '')

index = 0
wrongStructures = 0

for rxn in indigo.iterateSmilesFile(joinPath('reactions', 'rhea.smiles')):
    try:
        bingo.insert(rxn)
    except BingoException as e:
        #print('Structure {0} excluded: {1}'.format(index, getIndigoExceptionText(e)))
        wrongStructures += 1
    index += 1
    if not (index % 1000):
        print('Processed {0} structures...'.format(index))

print('Finished indexing {0} structures. {1} wrong structures excluded'.format(index, wrongStructures))

cnt = 0
for rxn in indigo.iterateSmilesFile(joinPath('reactions', 'smi_queries.smi')):
    print('Reaction #{0}'.format(cnt))
    cnt = cnt + 1;
    qrxn = indigo.loadQueryReaction(rxn.rawData())
    searchSub(bingo, qrxn, '')
    searchSim(bingo, rxn, 0.9, 1, 'tanimoto')
    searchSim(bingo, rxn, 0.9, 1, 'tversky 0.3 0.7')
    searchSim(bingo, rxn, 0.9, 1, 'euclid-sub')
    searchExact(bingo, rxn, '')

for rxn in itertools.islice(indigo.iterateSmilesFile(joinPath('reactions', 'rhea.smiles')), 100):
    searchExact(bingo, rxn)

bingo.close()