import sys
import threading
sys.path.append('../../common')
from env_indigo import *

def outPrint(str, pid, output):
    #output = None
    if output == None:
       print(str)
    else:
       old_out = output[pid]
       output[pid] = '{0}\n{1}'.format(old_out, str)

def insertSmi(db, pid, input_smi, output=None):
    index = 0
    wrongStructures = 0
    #outPrint('Inserting molecules from:{1}'.format(pid, input_smi), pid, output)

    smi_path = joinPathPy(os.path.join('molecules', input_smi), __file__)
    for mol in indigo.iterateSmilesFile(smi_path):
        try:
            db.insert(mol)
        except(BingoException, e):
            #outPrint('Structure {0} excluded: {1}'.format(index, getIndigoExceptionText(e)), pid, output)
            wrongStructures += 1
        index += 1

        if index % 1000 == 0:
           print('Structures inserted: {0}'.format(index))

    #outPrint('Finished indexing {1} structures. {2} wrong structures excluded'.format(pid, index, wrongStructures), pid, output)

def makeSearchSim(db, pid, query, min, max, options, output=None ):
    #outPrint('\n\nSimSearch with metric {0}'.format(options.encode('ascii')), pid, output)

    search = db.searchSim(query, min, max, options)
    cnt = 0
    while search.next():
       #outPrint('Mol #{0} with sim value {1}'.format(search.getCurrentId(), search.getCurrentSimilarityValue()), pid, output)
       cnt = cnt + 1;
    #f1=open('sim_out.txt', 'a+')
    #f1.write('PID {0}) Total count in db #{1} {2}\n'.format(pid, db, cnt))
    outPrint('PID {0}) Total count {1}'.format(pid, cnt), pid, output)

def makeSearchSub(db, pid, query, options, output=None):
    #outPrint('\n\nSubSearch:'.format(db), pid, output)
    search = db.searchSub(query, options)
    cnt = 0
    while search.next():
       #outPrint('Mol #{0}'.format(search.getCurrentId()), pid, output)
       cnt = cnt + 1
    #f1=open('sub_out.txt', 'a+')
    #f1.write('PID {0}) Total count in db #{1} {2}\n'.format(pid, db, cnt))
    outPrint('PID {0}) Total count {1}'.format(pid, cnt), pid, output)

def makeSearchExact(db, pid, query, options, output=None):
    #outPrint('ExactSearch:'.format(db), pid, output)
    search = db.searchExact(query, options)
    cnt = 0
    while search.next():
       #outPrint('Mol #{0}'.format(search.getCurrentId()), pid, output)
       cnt = cnt + 1
    #f1=open('./exact_out.txt', 'a+')
    #f1.write('PID {0}) Total count in db #{1} {2}\n'.format(pid, db, cnt))
    outPrint('PID {0}) Total count {1}'.format(pid, cnt), pid, output)


def partCreate():
    bingo = Bingo.createDatabaseFile(indigo, joinPathPy('mol_part_db', __file__), 'molecule', 'mt_size:2000')

    insertSmi(bingo, 0, 'sample_100000.smi')

    bingo.close()

def partTest(size, type = 'sub'):
    bingo = Bingo.loadDatabaseFile(indigo, joinPathPy('mol_part_db', __file__), '')
    index = 0

    for m in indigo.iterateSDFile(joinPathPy('molecules/rand_queries_small.sdf', __file__)):
        try:
            print('\nQuery #{0}'.format(index + 1))

            outputs = ['' for i in range(size + 1)]

            threads = []

            if type == 'sub':
               qmol = indigo.loadQueryMolecule(m.rawData())
               threads.append(threading.Thread(target=makeSearchSub, args=(bingo, 0, qmol, '', outputs)))
               for i in range(1, size + 1):
                  threads.append(threading.Thread(target=makeSearchSub, args=(bingo, i, qmol, 'part:{0}/{1}'.format(i, size), outputs)))
            elif type == 'exact':
               qmol = indigo.loadMolecule(m.rawData())
               threads.append(threading.Thread(target=makeSearchExact, args=(bingo, 0, qmol, '', outputs)))
               for i in range(1, size + 1):
                  threads.append(threading.Thread(target=makeSearchExact, args=(bingo, i, qmol, 'part:{0}/{1}'.format(i, size), outputs)))
            else:
               qmol = indigo.loadMolecule(m.rawData())
               threads.append(threading.Thread(target=makeSearchSim, args=(bingo, 0, qmol, 0.5, 1, '', outputs)))
               for i in range(1, size + 1):
                  threads.append(threading.Thread(target=makeSearchSim, args=(bingo, i, qmol, 0.5, 1, 'part:{0}/{1}'.format(i, size), outputs)))

            for t in threads:
               t.start()

            for t in threads:
               t.join()

            for out in outputs:
               print(out)

            inpex = index + 1
        except BingoException as e:
            print('Query {0} fail: {1}'.format(getIndigoExceptionText(e)))
        index += 1

    bingo.close()


indigo = Indigo()

print('\n2 Database creating..\n')
partCreate()
print('\n Partial similarity search:\n')
partTest(3, 'sim')
print('\n Partial substructure search:\n')
partTest(3, 'sub')
print('\n Partial exact search:\n')
partTest(3, 'exact')
