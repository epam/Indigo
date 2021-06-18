import os
from threading import Thread
import sys
sys.path.append('../../common')
from env_indigo import *

class testThread(Thread):
   def __init__(self,id, indigo):
      Thread.__init__(self)
      self.id = id
      self.indigo = indigo
   def run(self):
      indigo = self.indigo
      self.result = "???" #indigo._sid
      id = 0

      for item in indigo.iterateSmilesFile(joinPath("molecules/helma.smi")):
         c1 = ""
         c2 = ""
         try:
            c1 = str(item.canonicalSmiles())
            c2 = str(item.canonicalSmiles())
         except IndigoException as e:
            pass
         if c1 != c2:
            sys.stderr.write("Thread %d: %s != %s\n" % (self.id, c1, c2))
         if id == self.id:
            self.result = (c1, c2)
         id += 1

print("*** Testing separate Indigo in each thread ***")
threads = []
for i in range(100):
   current = testThread(i, Indigo())
   threads.append(current)
   current.start()
for thread in threads:
   thread.join()
   print("Result from thread %s is %s" % (thread.id, str(thread.result)))

print("*** Testing same Indigo in each thread ***")
threads = []
indigo = Indigo()
for i in range(100):
   current = testThread(i, indigo)
   threads.append(current)
   current.start()
for thread in threads:
   thread.join()
   print("Result from thread %s is %s" % (thread.id, str(thread.result)))
