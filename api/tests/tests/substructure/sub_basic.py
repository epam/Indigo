import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")

def testSSS(mol, q):
   matcher = indigo.substructureMatcher(mol)
   try:
      print("Query: " + q.name())
      print("Target: " + mol.name())
      match = (matcher.match(q) != None)
      cnt = matcher.countMatches(q)
      q.optimize()
      match_opt = (matcher.match(q) != None)
      cnt_opt = matcher.countMatches(q)
      if (match != None) != (match_opt != None):
         msg = "match before and after optimization is different: match=%s and match_opt=%s" % (match, match_opt)
         print(msg)
         sys.stderr.write(msg + "\n")
      
      if cnt != cnt_opt:
         msg = "count before and after optimization is different: cnt=%d and cnt_opt=%d" % (cnt, cnt_opt)
         print(msg)
         sys.stderr.write(msg + "\n")
         
      print("  count = %d" % (cnt))
      if (cnt > 0) != match:
         msg = "match and countMatches returns contradicting results: match=%s and count=%d" % (match, cnt)
         print(msg)
         sys.stderr.write(msg + "\n")
   except IndigoException, e:      
      print("Error: " % (getIndigoExceptionText(e)))
      
def loadWithCheck(func):
    def wrapper(param):
        try:
            mol = func(param)
            if func == indigo.loadMoleculeFromFile:             
                mol.setName(relativePath(param))
            else:
                suffix = ""
                if func == indigo.loadSmarts:             
                    suffix = " (SMARTS)"
                mol.setName(param + suffix)
            return mol
        except IndigoException, e:
            print(getIndigoExceptionText(e))
        return None
    return wrapper
         
def loadAromWithCheck(func):
    def loader(param):
        try:
            mol = func(param)
            if func == indigo.loadQueryMoleculeFromFile:
                mol.setName(relativePath(param))
            else:
                mol.setName(param)
            mol.aromatize()
            return mol  
        except IndigoException, e:
            print(getIndigoExceptionText(e))
            return None
    return loader
      
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
   (lmol('c1cc2concc2cn1'), lqmolf(joinPath("molecules/r1_2ap.mol"))),
   (lmol('c1cc2cnocc2cn1'), lqmolf(joinPath("molecules/r1_2ap_aal.mol"))),
   (lmolf(joinPath('molecules/r2_target.mol')), lqmolf(joinPath("molecules/r2.mol"))),
   (lmol('c1ccccc1'), lsmarts("[#6]cccc[#6,#7]")),
   (lmol('c1ccccc1'), lqmolf(joinPath("molecules/q_rg_recurs.mol"))),
   (lmol('c1ccccc1.c1ccccc1'), lqmolf(joinPath("molecules/q_rg_recurs.mol"))),
   (lmol('c1ccccc1'), lqmolf(joinPath("molecules/q_rg_recurs2.mol"))),
   (lmol('C1CCCCCC1'), lqmolf(joinPath("molecules/q_rg_recurs2.mol"))),
   (lmol('C1CCCCCC1.C1CCCCCC1'), lqmolf(joinPath("molecules/q_rg_recurs2.mol"))),
   (lmol('OC(=O)C1=CC=CC=C1'), lqmolf(joinPath("molecules/rgroups/c11100_3.mol"))),
   (lmol('NC=O'), lsmarts("[N;!$(NC=O)]")),
   (lmol('NC=O'), lsmarts("[N;!$([*,H]C=O)]")),
   (lmol('N'), lsmarts("[#1][N]")),
   (lmol('N'), lsmarts("[$([#1][N])]")),
   (lmol('NC'), lsmarts("[$([#1][N])]C")),
   (lmol('NC'), lsmarts("[$([#1][N])]N")),
   (lmol('O'), lsmarts("[!#6;!#1;!H0]")),
   (lmol('C* |$;Some value_p$|'), lsmarts("[!#6;!#1;!H0]")),
   (lmol('ClC1C(Cl)C(Cl)C(Cl)C(Cl)C1Cl'), lsmarts("[H]C(*)(*)C(*)(*)*")),
   (lmol('ClC1C(Cl)C(Cl)C(Cl)C(Cl)C1Cl'), lqmol("[H]C(*)(*)C(*)(*)*")),
   (lmol('ClC1C(Cl)C(Cl)C(Cl)C(Cl)C1Cl'), lqmol("C(C)(Cl)C(C)(Cl)*")),
   (lmol('ClC1C(Cl)C(Cl)C(Cl)C(Cl)C1Cl'), lqmol("C(C)(Cl)C(C)(Cl)[H,*]")),
   (lmol('ClC1C(Cl)C(Cl)C(Cl)C(Cl)C1Cl'), lqmol("[H]C(C)(Cl)C([H])(C)Cl")),
   (lmol('C'), lqmol("C[*,H]")),
   (lmol('C'), lqmol("C[H,*]")),
   (lmol('[H]'), lqmol("[*,H]")),
   (lmol('[H]'), lqmol("[H,*]")),
   (lmol('N1OCCCC=1'), lqmol("N1OCCCC=1")),
   (lmol('N1OCCCC=1'), lqmol("N=1OCCCC1")),
   (lmol('C-1CCCCC=1'), lqmol("C=1CCCCC-1")),
   (lmol('C=1=C=C=C=C=1'), lqmol("C~1~C~C~C~C~1")),
   (lmol('C=1=C=C=C=C=1'), lsmarts("C~1~C~C~C~C~1")),
   (lmol('C=1=C=C=C=C=1'), lqmol("C1CCCC1")),
   (lmol('C=1=C=C=C=C=1'), lsmarts("C1CCCC1")),
   (lmol('C1C=CC=C1'), lsmarts("C1CCCC1")),
   (lmol('C1C=CC=C1'), lsmarts("C~1~C~C~C~C~1")),
   (lmol('c1cccc1'), lsmarts("C~1~C~C~C~C~1")),
   (lmol('c1cccc1'), lqmol("C~1~C~C~C~C~1")),
   (lmol('c1cccc1'), lsmarts("C1CCCC1")),
   (lmol('c1cccc1'), lsmarts("[#6]1[#6][#6][#6][#6]1")),
]
for i in range(len(tests)):
   print("\n*** Test %d ***" % (i))
   (mol, q) = tests[i]
   if mol != None and q != None:
      testSSS(mol, q)
