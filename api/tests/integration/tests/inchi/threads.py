import os
import sys
from threading import Thread

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa


class testThread(Thread):
    def __init__(self, id):
        Thread.__init__(self)
        self.id = id

    def run(self):
        indigo = Indigo()
        indigo_inchi = IndigoInchi(indigo)
        self.result = "???"  # indigo._sid
        id = 0

        for item in indigo.iterateSmilesFile(
            joinPathPy(
                "../../../../../data/molecules/basic/helma.smi", __file__
            )
        ):
            c1 = ""
            c2 = ""
            try:
                c1 = str(indigo_inchi.getInchi(item))
                c2 = str(indigo_inchi.getInchi(item))
            except IndigoException as e:
                pass
            if c1 != c2:
                sys.stderr.write("Thread %d: %s != %s\n" % (self.id, c1, c2))
            if id == self.id:
                self.result = c1, c2
            id += 1


threads = []
for i in range(100):
    current = testThread(i)
    threads.append(current)
    current.start()
for thread in threads:
    thread.join()
    print("Result from thread %s is %s" % (thread.id, str(thread.result)))
