import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
target_mol = indigo.loadMoleculeFromFile(joinPath("molecules/for_ignore.mol"))
queries = [ "C1CCCC1", "C1CCCCC1", "OC=O", "C=C", "C1CCNCC1" ] 
queries_mol = []
match_dict = dict()
for qsm in queries:
   q = indigo.loadQueryMolecule(qsm)
   q.setName(qsm)
   queries_mol.append(q)
   match_dict[q.name()] = 0
   
def testIgnoreAtoms(matcher):
   for k in match_dict.keys():
      match_dict[k] = 0
      
   matcher.unignoreAllAtoms()
   prev_match_count = -1
   match_count = 0
   while prev_match_count != match_count:
      prev_match_count = match_count
      for q in queries_mol:
         m = matcher.match(q)
         if m:
            match_dict[q.smiles()] += 1
            match_count += 1
            # mark matched atoms as ignored
            for qa in q.iterateAtoms():
               ta = m.mapAtom(qa)
               matcher.ignoreAtom(ta)
               
   for k in sorted(match_dict.keys()):
      print("%s: %d" % (k, match_dict[k]))
matcher = indigo.substructureMatcher(target_mol)
 
for i in range(10):
   print("\nTest #%d" % i)
   testIgnoreAtoms(matcher)
