import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
# load queries
targets = list(indigo.iterateSDFile(joinPath("../../data/thiazolidines.sdf")))
queries = [indigo.loadQueryMolecule(x.rawData()) for x in indigo.iterateSDFile(joinPath("../../data/rand_queries_small.sdf"))]
for i in range(len(queries)):
   queries[i].setName("#%s" % i)
   
query_results = {}
for q in queries:
   query_results[q.name()] = 0
   
for t in targets:
   matcher = indigo.substructureMatcher(t)
   for q in queries:
      m = matcher.match(q)
      if m:
         query_results[q.name()] += 1
         print("%s %s" % (t.name(), q.name()))
         cnt = matcher.countMatches(q)
         print("  cnt=%d" % cnt)
print("\nResults per query:")
for q in queries:
   print("%s: %d" % (q.name(), query_results[q.name()]))
