import sys
import threading

sys.path.append("../../common")
from env_indigo import (
    Bingo,
    BingoException,
    Indigo,
    getIndigoExceptionText,
    joinPathPy,
    dataPath,
)


def outPrint(s, pid, output):
    if output is None:
        print(s)
    else:
        old_out = output[pid]
        output[pid] = "{0}\n{1}".format(old_out, s)


def insertSmi(db, smi_path):
    index = 0
    wrongStructures = 0

    for mol in indigo.iterateSmilesFile(smi_path):
        try:
            db.insert(mol)
        except BingoException:
            wrongStructures += 1
        index += 1

        if index % 1000 == 0:
            print("Structures inserted: {0}".format(index))


def makeSearchSim(db, pid, query, min_sim, max_sim, options, output=None):
    search = db.searchSim(query, min_sim, max_sim, options)
    cnt = 0
    while search.next():
        cnt = cnt + 1
    outPrint("PID {0}) Total count {1}".format(pid, cnt), pid, output)


def makeSearchSub(db, pid, query, options, output=None):
    search = db.searchSub(query, options)
    cnt = 0
    while search.next():
        cnt = cnt + 1
    outPrint("PID {0}) Total count {1}".format(pid, cnt), pid, output)


def makeSearchExact(db, pid, query, options, output=None):
    search = db.searchExact(query, options)
    cnt = 0
    while search.next():
        cnt = cnt + 1
    outPrint("PID {0}) Total count {1}".format(pid, cnt), pid, output)


def partCreate():
    bingo = Bingo.createDatabaseFile(
        indigo, joinPathPy("mol_part_db", __file__), "molecule", "mt_size:2000"
    )

    insertSmi(bingo, dataPath("molecules/basic/sample_100000.smi"))

    bingo.close()


def partTest(size, searchType="sub"):
    bingo = Bingo.loadDatabaseFile(
        indigo, joinPathPy("mol_part_db", __file__), ""
    )
    index = 0

    for m in indigo.iterateSDFile(
        dataPath("molecules/basic/rand_queries_small.sdf")
    ):
        try:
            print("\nQuery #{0}".format(index + 1))

            outputs = ["" for _ in range(size + 1)]

            threads = []

            if searchType == "sub":
                qmol = indigo.loadQueryMolecule(m.rawData())
                threads.append(
                    threading.Thread(
                        target=makeSearchSub,
                        args=(bingo, 0, qmol, "", outputs),
                    )
                )
                for i in range(1, size + 1):
                    threads.append(
                        threading.Thread(
                            target=makeSearchSub,
                            args=(
                                bingo,
                                i,
                                qmol,
                                "part:{0}/{1}".format(i, size),
                                outputs,
                            ),
                        )
                    )
            elif searchType == "exact":
                qmol = indigo.loadMolecule(m.rawData())
                threads.append(
                    threading.Thread(
                        target=makeSearchExact,
                        args=(bingo, 0, qmol, "", outputs),
                    )
                )
                for i in range(1, size + 1):
                    threads.append(
                        threading.Thread(
                            target=makeSearchExact,
                            args=(
                                bingo,
                                i,
                                qmol,
                                "part:{0}/{1}".format(i, size),
                                outputs,
                            ),
                        )
                    )
            else:
                qmol = indigo.loadMolecule(m.rawData())
                threads.append(
                    threading.Thread(
                        target=makeSearchSim,
                        args=(bingo, 0, qmol, 0.5, 1, "", outputs),
                    )
                )
                for i in range(1, size + 1):
                    threads.append(
                        threading.Thread(
                            target=makeSearchSim,
                            args=(
                                bingo,
                                i,
                                qmol,
                                0.5,
                                1,
                                "part:{0}/{1}".format(i, size),
                                outputs,
                            ),
                        )
                    )

            for t in threads:
                t.start()

            for t in threads:
                t.join()

            for out in outputs:
                print(out)

        except BingoException as e:
            print(
                "Query {0} fail: {1}".format(
                    m.rawData(), getIndigoExceptionText(e)
                )
            )
        index += 1

    bingo.close()


indigo = Indigo()

print("\n2 Database creating..\n")
partCreate()
print("\n Partial similarity search:\n")
partTest(3, "sim")
print("\n Partial substructure search:\n")
partTest(3, "sub")
print("\n Partial exact search:\n")
partTest(3, "exact")
