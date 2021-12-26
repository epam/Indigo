import sys

sys.path.append("../../common")
from env_indigo import *


def outPrint(s, pid, output):
    # output = None
    if output == None:
        print(s)
    else:
        old_out = output[pid]
        output[pid] = "{0}\n{1}".format(old_out, s)


def insertProc(db, pid, input_smi, output=None):
    index = 0
    wrongStructures = 0
    # outPrint('Inserting molecules from:{1}'.format(pid, input_smi), pid, output)

    smi_path = joinPathPy(os.path.join("molecules", input_smi), __file__)
    for mol in indigo.iterateSmilesFile(smi_path):
        try:
            db.insert(mol)
        except BingoException as e:
            outPrint(
                "Structure {0} excluded: {1}".format(
                    index, getIndigoExceptionText(e)
                ),
                pid,
                output,
            )
            wrongStructures += 1
        index += 1

    outPrint(
        "Finished indexing {1} structures. {2} wrong structures excluded".format(
            pid, index, wrongStructures
        ),
        pid,
        output,
    )


def makeSearchSim(db, pid, query, min, max, options, output=None):
    outPrint("\n\nSimSearch with metric {0}".format(options), pid, output)

    search = db.searchSim(query, min, max, options)
    cnt = 0
    while search.next():
        # outPrint('Mol #{0} with sim value {1}'.format(search.getCurrentId(), round(search.getCurrentSimilarityValue(), 4)), pid, output)
        cnt += 1
    # outPrint('{0} results'.format(cnt))
    # f1=open('sim_out.txt', 'a+')
    # f1.write('PID {0}) Total count in db #{1} {2}\n'.format(pid, db, cnt))
    outPrint("PID {0}) Total count {1}\n\n".format(pid, cnt), pid, output)


def makeSearchSub(db, pid, query, options, output=None):
    outPrint("\n\nSubSearch:".format(db), pid, output)
    search = db.searchSub(query, options)
    cnt = 0
    while search.next():
        # outPrint('Mol #{0}'.format(search.getCurrentId()), pid, output)
        cnt = cnt + 1
    # f1=open('sub_out.txt', 'a+')
    # f1.write('PID {0}) Total count in db #{1} {2}\n'.format(pid, db, cnt))
    outPrint("PID {0}) Total count {1}\n".format(pid, cnt), pid, output)


def makeSearchExact(db, pid, query, options, output=None):
    outPrint("ExactSearch:".format(db), pid, output)
    search = db.searchExact(query, options)
    cnt = 0
    while search.next():
        # outPrint('Mol #{0}'.format(search.getCurrentId()), pid, output)
        cnt = cnt + 1
    # f1=open('./exact_out.txt', 'a+')
    # f1.write('PID {0}) Total count in db #{1} {2}\n'.format(pid, db, cnt))
    outPrint("PID {0}) Total count {1}\n".format(pid, cnt), pid, output)


indigo = Indigo()
indigo1 = Indigo()


def consTest():
    bingo1 = Bingo.createDatabaseFile(
        indigo, joinPathPy("mol_test_db1", __file__), "molecule", ""
    )
    bingo2 = Bingo.createDatabaseFile(
        indigo, joinPathPy("mol_test_db2", __file__), "molecule", ""
    )

    insertProc(bingo1, 0, "sample_2000_2.smi")
    insertProc(bingo1, 0, "sample_2000_2.smi")
    insertProc(bingo1, 0, "sample_2000_3.smi")

    insertProc(bingo2, 0, "sample_2000_1.smi")
    insertProc(bingo2, 0, "sample_2000_2.smi")
    insertProc(bingo2, 0, "sample_2000_4.smi")
    insertProc(bingo2, 0, "sample_2000_5.smi")

    query = indigo.loadMoleculeFromFile(
        joinPathPy("molecules/query_382.smi", __file__)
    )
    q_query = indigo.loadQueryMoleculeFromFile(
        joinPathPy("molecules/query_382.smi", __file__)
    )

    cnt = 1
    for bingo in [bingo1, bingo2]:
        print("Database #{0}".format(cnt))
        makeSearchSim(bingo, 0, query, 0.7, 1, "tanimoto")
        makeSearchSim(bingo, 0, query, 0.7, 1, "tversky 0.5 0.5")
        makeSearchSim(bingo, 0, query, 0.7, 1, "euclid-sub")
        makeSearchSub(bingo, 0, q_query, "")
        makeSearchExact(bingo, 0, query, "")
        cnt = cnt + 1

    bingo1.close()
    bingo2.close()


def insertThr(db, pid, samples, output=None):
    for i in samples:
        name = "sample_2000_" + str(i)
        insertProc(db, pid, name + ".smi", output)


def paralSameTest():
    bingo1 = Bingo.createDatabaseFile(
        indigo, joinPathPy("mol_test_db1", __file__), "molecule", ""
    )

    thrs = []

    for i in range(1, 51):
        thrs.insert(
            i - 1,
            threading.Thread(
                target=insertThr, args=(bingo1, [(i - 1) % 5 + 1])
            ),
        )

    for t in thrs:
        t.start()

    for t in thrs:
        t.join()

    query = indigo.loadMoleculeFromFile(
        joinPathPy("molecules/query_2_182.smi", __file__)
    )
    q_query = indigo.loadQueryMoleculeFromFile(
        joinPathPy("molecules/query_2_182.smi", __file__)
    )

    makeSearchSim(bingo1, 0, query, 0.3, 1, "tanimoto")

    makeSearchSub(bingo1, 0, q_query, "")

    makeSearchExact(bingo1, 0, query, "")

    bingo1.close()


def paralBigTest():
    bingo1 = Bingo.createDatabaseFile(
        indigo,
        joinPathPy("mol_test_db1", __file__),
        "molecule",
        "read_only:false",
    )
    bingo2 = Bingo.createDatabaseFile(
        indigo,
        joinPathPy("mol_test_db2", __file__),
        "molecule",
        "read_only:false",
    )

    thrs = []

    query = indigo.loadMoleculeFromFile(
        joinPathPy("molecules/query_382.smi", __file__)
    )
    q_query = indigo.loadQueryMoleculeFromFile(
        joinPathPy("molecules/query_382.smi", __file__)
    )

    outp = ["" for x in range(70)]
    for i in range(0, 5):
        thrs.append(
            threading.Thread(
                target=insertThr, args=(bingo1, i, [i % 5 + 1], outp)
            )
        )
    for i in range(5, 10):
        thrs.append(
            threading.Thread(
                target=insertThr, args=(bingo2, i, [i % 5 + 1], outp)
            )
        )

    for i in range(10, 20):
        thrs.append(
            threading.Thread(
                target=makeSearchSim,
                args=(bingo1, i, query, 0.7, 1, "tanimoto", outp),
            )
        )
    for i in range(20, 30):
        thrs.append(
            threading.Thread(
                target=makeSearchSub, args=(bingo1, i, q_query, "", outp)
            )
        )
    for i in range(30, 40):
        thrs.append(
            threading.Thread(
                target=makeSearchExact, args=(bingo1, i, query, "", outp)
            )
        )

    for i in range(40, 50):
        thrs.append(
            threading.Thread(
                target=makeSearchSim,
                args=(bingo2, i, query, 0.7, 1, "tanimoto", outp),
            )
        )
    for i in range(50, 60):
        thrs.append(
            threading.Thread(
                target=makeSearchSub, args=(bingo2, i, q_query, "", outp)
            )
        )
    for i in range(60, 70):
        thrs.append(
            threading.Thread(
                target=makeSearchExact, args=(bingo2, i, query, "", outp)
            )
        )

    for t in thrs:
        t.start()

    for t in thrs:
        t.join()

    for i in range(0, 10):
        sys.stdout.write(outp[i])

    print("\n\ncheck..")

    del thrs[:]
    thr_outputs = ["" for x in range(10)]
    cnt = 1
    for bingo in [bingo1, bingo2]:
        print("Database #{0}".format(cnt))
        off = (cnt - 1) * 5
        thrs.append(
            threading.Thread(
                target=makeSearchSim,
                args=(bingo, 0 + off, query, 0.7, 1, "tanimoto", thr_outputs),
            )
        )
        thrs.append(
            threading.Thread(
                target=makeSearchSim,
                args=(
                    bingo,
                    1 + off,
                    query,
                    0.7,
                    1,
                    "tversky 0.2 0.8",
                    thr_outputs,
                ),
            )
        )
        thrs.append(
            threading.Thread(
                target=makeSearchSim,
                args=(
                    bingo,
                    2 + off,
                    query,
                    0.7,
                    1,
                    "euclid-sub",
                    thr_outputs,
                ),
            )
        )
        thrs.append(
            threading.Thread(
                target=makeSearchSub,
                args=(bingo, 3 + off, q_query, "", thr_outputs),
            )
        )
        thrs.append(
            threading.Thread(
                target=makeSearchExact,
                args=(bingo, 4 + off, query, "", thr_outputs),
            )
        )
        cnt = cnt + 1

    for t in thrs:
        t.start()

    for t in thrs:
        t.join()

    for output in thr_outputs:
        print(output)

    bingo1.close()
    bingo2.close()


def difIndigoTest():
    indigo1 = Indigo()
    indigo2 = Indigo()
    bingo1 = Bingo.createDatabaseFile(
        indigo1, joinPathPy("mol_test_db1", __file__), "molecule", ""
    )
    bingo2 = Bingo.createDatabaseFile(
        indigo2, joinPathPy("mol_test_db2", __file__), "molecule", ""
    )

    t1 = threading.Thread(target=insertThr, args=(bingo1, [2, 3]))
    t2 = threading.Thread(target=insertThr, args=(bingo2, [1, 4, 5]))

    t1.start()
    t2.start()

    t1.join()
    t2.join()

    query = indigo.loadMoleculeFromFile(
        joinPathPy("molecules/query_382.smi", __file__)
    )
    q_query = indigo.loadQueryMoleculeFromFile(
        joinPathPy("molecules/query_382.smi", __file__)
    )

    makeSearchSim(bingo1, 0, query, 0.3, 1, "tanimoto")
    makeSearchSim(bingo2, 0, query, 0.3, 1, "tanimoto")

    makeSearchSub(bingo1, 0, q_query, "")
    makeSearchSub(bingo2, 0, q_query, "")

    makeSearchExact(bingo1, 0, query, "")
    makeSearchExact(bingo2, 0, query, "")

    bingo1.close()
    bingo2.close()


print("\n2 BINGO WITH SAME INDIGO\n")
consTest()
print("\nMULTITHREADING TEST\n")
paralBigTest()
