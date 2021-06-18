import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
# load queries
targets = list(indigo.iterateSDFile(joinPath("../../../../../data/molecules/basic/thiazolidines.sdf")))
targets_unser = [indigo.deserialize(t.serialize()) for t in targets]

queries = []
queries_file = open(joinPath("molecules/smarts.sma"), "r")
cnt = 1
for x in queries_file:
   try:
      q = indigo.loadSmarts(x)
      q.setName("#%s" % cnt)
      queries.append(q)
   except IndigoException as e:
      print("Cannot load query molecules from SMARTS\n  %s\n  Error: %s" % (x, getIndigoExceptionText(e)))
   cnt += 1
   
query_results = {}
for q in queries:
   query_results[q.name()] = 0
def testQueryMatch (matcher, q, t):
   global query_results
   m = None
   try:
      m = matcher.match(q)
   except IndigoException as e:
      sys.stderr.write("%s %s: %s\n" % (t.name(), q.name(), getIndigoExceptionText(e)))
   if m:
      return matcher.countMatches(q)
   return 0
   
for t, ts in zip(targets, targets_unser):
    matcher = indigo.substructureMatcher(t)
    matcher2 = indigo.substructureMatcher(ts)
    for q in queries:
        cnt = testQueryMatch(matcher, q, t)
        if cnt > 0:
            query_results[q.name()] += 1
            print("%s %s" % (t.name(), q.name()))
            print("  cnt=%d" % cnt)
        # Check unserialized molecule
        m2 = matcher2.match(q)
        if (cnt > 0) != (m2 != None):
            print("  %s %s: Matching is different for the original and unserialized molecule, %s %s" % 
                (t.name(), q.name(), cnt > 0, m2 != None))
        
        q.optimize()
        cnt2 = testQueryMatch(matcher, q, t)
        if cnt != cnt2:
            print("  %s %s: different match count: %d and %d for optimized" % (t.name(), q.name(), cnt, cnt2))
         
         
print("\nResults per query:")
for q in queries:
   print("%s: %d" % (q.name(), query_results[q.name()]))
