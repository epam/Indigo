import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
target_mol = indigo.loadMoleculeFromFile(joinPath("molecules/for_unignore.mol"))
queries = [ "CC(C)C1CCCCC1", "C1CCCC1" ] 
queries_mol = []
for qsm in queries:
   q = indigo.loadQueryMolecule(qsm)
   q.setName(qsm)
   queries_mol.append(q)
def testIgnoreUnignoreAtoms(matcher, level, queries):
   for q in queries:
      for m in matcher.iterateMatches(q):
         global total_matches
         total_matches += 1
         
         # Mark atoms for ignore
         mapped_atoms = [m.mapAtom(qa) for qa in q.iterateAtoms()]
         for ta in mapped_atoms:
            matcher.ignoreAtom(ta)
         mapped_atoms_numbers = [ta.index() for ta in mapped_atoms]
         print("%s%s: %s" % ("  " * level, q.name(), mapped_atoms_numbers))
            
         # Call recursively
         testIgnoreUnignoreAtoms(matcher, level + 1, queries)
         # Restore state
         for ta in mapped_atoms:
            matcher.unignoreAtom(ta)
         
def runSingleTest (target_mol, queries):
   matcher = indigo.substructureMatcher(target_mol)
    
   for i in range(3):
      print("\n  Subtest #%d" % i)
      global total_matches
      total_matches = 0
      first_total_matches = -1
      testIgnoreUnignoreAtoms(matcher, 1, queries)
      print("  Total matches = %d" % total_matches)
      if first_total_matches == -1:
         first_total_matches = total_matches
      if first_total_matches != total_matches:
         sys.stderr.write("first_total_matches != total_matches. State wasn't restored properly\n")
      
print("*** Test 1 ***")
runSingleTest(target_mol, queries_mol)
print("*** Test 2 ***")
queries_mol = [ indigo.loadQueryMolecule("CC") ]
queries_mol[0].setName("CC")
runSingleTest(indigo.loadMolecule("C1CCC1"), queries_mol)
print("*** Test 3 ***")
runSingleTest(indigo.loadMolecule("C1CCCC1"), queries_mol)
