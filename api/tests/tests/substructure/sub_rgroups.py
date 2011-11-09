import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
# load queries
targets = list(indigo.iterateSDFile(joinPath("molecules/rgroup_targets.sdf")))
queries = []
query_dir = joinPath("molecules", "rgroups")
for root, dirnames, filenames in os.walk(query_dir):
   filenames.sort()
   for filename in filenames:
      q = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
      q.setName(filename)
      queries.append(q)
res_targets = []
cnt = 1
for t in targets:
   if t.name() == "":
      try:
         t.setName("#%d" % cnt)
         res_targets.append(t)
      except IndigoException, e:
         sys.stderr.write("setName for #%s: %s\n" % (cnt, getIndigoExceptionText(e)))
   else:
      res_targets.append(t)
   cnt += 1
def performTest():   
   query_results = {}
   for q in queries:
      query_results[q.name()] = 0
   for t in res_targets:
      print("Target %s" % (t.name()))
      try:
         matcher = indigo.substructureMatcher(t)
      except IndigoException, e:
         sys.stderr.write("substructureMatcher for %s: %s\n" % (t.name(), getIndigoExceptionText(e)))
         continue
      for q in queries:
         try:
            m = matcher.match(q)
            if m:
               query_results[q.name()] += 1
               print("  %s %s" % (t.name(), q.name()))
               cnt = matcher.countMatches(q)
               print("    cnt=%d" % cnt)
         except IndigoException, e:
            sys.stderr.write("%s %s: %s\n" % (t.name(), q.name(), getIndigoExceptionText(e)))
   print("\nResults per query:")
   for q in queries:
      print("%s: %d" % (q.name(), query_results[q.name()]))
print("*** Non-aromatized queries *** ")
performTest()
      
print("*** Aromatized queries *** ")
for q in queries:
   q.aromatize()
performTest()
