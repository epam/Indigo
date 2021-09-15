import os
import sys
from itertools import product
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *



indigo = Indigo()
mol = indigo.loadMolecule('C1C=CC=CC=1')
mol.aromatize()
q = indigo.loadQueryMolecule("C:C:C")
m = indigo.substructureMatcher(mol).match(q)
if m is None:
   print("not matched")
else:
   print(m.highlightedTarget().smiles())
   
def testSSS(mol, q):
   matcher = indigo.substructureMatcher(mol)
   try:
      cnt = matcher.countMatches(q)
      print("count = " + str(cnt))
   except IndigoException as e:
      print(getIndigoExceptionText(e))
   def testUnmappedAtoms (q, match, t):
      unmapped = 0
      for atom in q.iterateAtoms():
         mapped = match.mapAtom(atom)
         if mapped is None:
            unmapped += 1
         else:
            mapped.index()
      print("  unmapped = " + str(unmapped))
   
   def testEmbeddingCount(matcher, q, t, emb_limit):
      cnt = -1
      try:
         cnt = matcher.countMatches(q)
         print("  count = " + str(cnt))
      except IndigoException as e:
         print(getIndigoExceptionText(e))
      
      cnt2 = 0
      try:
         for m in matcher.iterateMatches(q):
            cnt2 += 1
            if cnt2 < 4:
               testUnmappedAtoms(q, m, t)
      except IndigoException as e:
         print(getIndigoExceptionText(e))
      print("  count by iterate = " + str(cnt2))
      if cnt != -1 and cnt2 != cnt:
         sys.stderr.write("    countMatches(q) != len(iterateMatches(q))\n")
         print("    cnt2 != cnt: %d != %d" % (cnt, cnt2))
         
      cnt3 = matcher.countMatchesWithLimit(q, emb_limit)
      print("  count with limit %d = %d" % (emb_limit, cnt3))
      if emb_limit != 0 and cnt3 > emb_limit:
         sys.stderr.write("    cnt3 > emb_limit\n")
         print("    cnt3 > emb_limit: %d > %d" % (cnt3, emb_limit))
         
   opset = [
      product(["embedding-uniqueness"], [ "atoms", "bonds", "none" ]),
      product(["max-embeddings"], [ 20, 0, 1, 5, 500, 10000, 50000 ]),
      product(["*embeddings limit*"], [ 0, 200 ]),
   ]
   opt_combintations = product(*opset)
   for opt_set in opt_combintations:
      print("Test set:")
      try:
         emb_limit = -1
         for opt_tuple in opt_set:
            print("  " + str(opt_tuple))
            if opt_tuple[0] != '*embeddings limit*':
               indigo.setOption(*opt_tuple)
            else:
               emb_limit = opt_tuple[1]
         testEmbeddingCount(matcher, q, mol, emb_limit)
      except IndigoException as e:
         print(getIndigoExceptionText(e))
         
def loadWithCheck(func):
   def wrapper(param):
      try:
         return func(param)
      except IndigoException as e:
         print(getIndigoExceptionText(e))
         return None
   return wrapper
         
def loadAromWithCheck(func):
   def loader(param):
      m = func(param)
      m.aromatize()
      return m
   return loadWithCheck(loader)
      
lmol = loadWithCheck(indigo.loadMolecule)
lsmarts = loadWithCheck(indigo.loadSmarts)
lqmol = loadAromWithCheck(indigo.loadQueryMolecule)
lmolf = loadWithCheck(indigo.loadMoleculeFromFile)
lqmolf = loadAromWithCheck(indigo.loadQueryMoleculeFromFile)
tests = [
   (lmol('c'), lsmarts("[#1]")),
   (lmol('C'), lsmarts("[#1]")),
   (lmol('c1cc2cc3ccc4cc5cc6cccc7cc8ccc9cc%10cc(c1)c2c1c3c4c2c5c(c67)c8c9c2c%101'), lqmol("*~*~*~*~*~*~*~*~*~*~*~*~*~*~*")),
   (lmol('c1cc2cc3ccc4cc5cc6cccc7cc8ccc9cc%10cc(c1)c2c1c3c4c2c5c(c67)c8c9c2c%101'), lqmol("*~*~*~*~*~*~*~*~*~*~*~*1~*~*~*C=C1")),
   (lmol('c1cc2cc3ccc4cc5cc6cccc7cc8ccc9cc%10cc(c1)c2c1c3c4c2c5c(c67)c8c9c2c%101'), lqmol("*~*~*~*~*~*~*~*~*~*~*~*~1~*~*~*~*~*~1")),
   (lmol('c1cc2cc3ccc4cc5cc6cccc7cc8ccc9cc%10cc(c1)c2c1c3c4c2c5c(c67)c8c9c2c%101'), lsmarts("*~*~*~*~*~*~*~*~*~*~*~[#1,#6]")),
   (lmol('c1cc2concc2cn1'), lqmolf(joinPathPy("molecules/r1_2ap.mol", __file__))),
   (lmol('c1cc2cnocc2cn1'), lqmolf(joinPathPy("molecules/r1_2ap_aal.mol", __file__))),
   (lmolf(joinPathPy('molecules/r2_target.mol', __file__)), lqmolf(joinPathPy("molecules/r2.mol", __file__))),
   (lmol('c1ccccc1'), lsmarts("[#6]cccc[#6,#7]")),
   (lmol('c1ccccc1'), lqmolf(joinPathPy("molecules/q_rg_recurs.mol", __file__))),
   (lmol('c1ccccc1.c1ccccc1'), lqmolf(joinPathPy("molecules/q_rg_recurs.mol", __file__))),
   (lmol('c1ccccc1'), lqmolf(joinPathPy("molecules/q_rg_recurs2.mol", __file__))),
   (lmol('C1CCCCCC1'), lqmolf(joinPathPy("molecules/q_rg_recurs2.mol", __file__))),
   (lmol('C1CCCCCC1.C1CCCCCC1'), lqmolf(joinPathPy("molecules/q_rg_recurs2.mol", __file__))),
   (lmol('OC(=O)C1=CC=CC=C1'), lqmolf(joinPathPy("molecules/rgroups/c11100_3.mol", __file__))),
   (lmol('N'), lsmarts("N-[#1,#112]")),
   (lmol('N'), lsmarts("N-[#1]")),
]
for i in range(len(tests)):
   print("\n*** Test %d ***" % i)
   (mol, q) = tests[i]
   if mol and q:
      testSSS(mol, q)
