import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
def infrange(start):
   cur = start
   while True:
      yield cur
      cur += 1
def makeQueryMol (m):
   qm = indigo.loadQueryMolecule(m.molfile())
   qa = qm.getAtom(0)
   qa.addConstraint("atomic-number", "17")
   return qm
      
def makeReaction (m):
   r = indigo.createReaction()
   r.addReactant(m)
   r.addProduct(m)
   return r
      
def makeQueryReaction (qm):
   r = indigo.createQueryReaction()
   r.addReactant(qm)
   r.addProduct(qm)
   return r
   
mols = []
print("*** Loading molecules *** ")
for i, m in zip(infrange(1), indigo.iterateCMLFile(joinPath("../../data/tetrahedral-all.cml"))):
   try:
      mols.append((i, m.clone()))
   except IndigoException, e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
      
print("*** Molecule canonical smiles *** ")
for i, m in mols:
   try:
      print("%d: %s" % (i, m.canonicalSmiles()))
   except IndigoException, e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
      
print("*** Query molecule molfiles *** ")
for i, m in mols:
   try:
      qm = makeQueryMol(m)
      print("%d: %s" % (i, qm.molfile()))
   except IndigoException, e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
print("*** Reaction smiles *** ")
for i, m in mols:
   try:
      r = makeReaction(m)
      print("%d: %s" % (i, r.smiles()))
   except IndigoException, e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
      
print("*** Query reaction rxnfiles *** ")
for i, m in mols:
   try:
      qm = makeQueryMol(m)
      qr = makeQueryReaction(qm)
      print("%d: %s" % (i, qr.rxnfile()))
   except IndigoException, e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
      
