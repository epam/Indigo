import os
import sys
import errno

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
   qa.addConstraintOr("atomic-number", "17")
   qa.addConstraintNot("atomic-number", "16")
   qa.addConstraintNot("charge", "2")
   return qm
      
def makeReaction (m):
   m2 = m.clone()
   r = indigo.createReaction()
   r.addReactant(m2)
   r.addProduct(m2)
   r.setName(m2.name())
   r.automap()
   return r
      
def makeQueryReaction (qm):
   r = indigo.createQueryReaction()
   r.addReactant(qm)
   r.addProduct(qm)
   r.setName(qm.name())
   return r
   
mols = []
print("*** Loading molecules *** ")
for i, m in zip(infrange(1), indigo.iterateCMLFile(joinPath("../../../../../data/molecules/basic/tetrahedral-all.cml"))):
   try:
      mols.append((i, m.clone()))
   except IndigoException as e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
      
# add molecule with pseudoatoms
mol_with_pseudo = indigo.loadMoleculeFromFile(joinPath("molecules/aniline_pol_psd.mol"))
mol_with_pseudo.setName("Molecule with pseudoatoms")
mols.append((len(mols) + 1, mol_with_pseudo))
      
print("*** Making query molecules *** ")
qmols = []
for i, m in mols:
   try:
      q = makeQueryMol(m)
      qmols.append((i, q))
   except IndigoException as e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
print("*** Making reactions *** ")
reacts = []
for i, m in mols:
   try:
      r = makeReaction(m)
      reacts.append((i, r))
   except IndigoException as e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
      
print("*** Making query reactions *** ")
qreacts = []
for i, m in mols:
   try:
      qm = makeQueryMol(m)
      qr = makeQueryReaction(qm)
      qreacts.append((i, qr))
   except IndigoException as e:
      print("%d: %s" % (i, getIndigoExceptionText(e)))
   
def compare_names (m1, m2, reloaded_has_name):
   name1 = m1.name()
   name2 = m2.name()
   if reloaded_has_name and name2 == "":
      return "Reloaded object must have name %s but has empty name %s " % (name1, name2)
   if not reloaded_has_name and name2 != "":
      return "Reloaded object name must be empty but has %s. Source name was %s" % (name2, name1)
   if reloaded_has_name and name1 != name2:
      return "Molecules have different names: %s and %s" % (name1, name2)
   return None
   
def testSaveLoad (objs, filename, format, loader_func, compare_func, reloaded_has_name):
   saver = indigo.createFileSaver(filename, format)
   try:
      saved = []
      cur = 0
      for i, obj in objs:
         try:
            saver.append(obj)
            saved.append(cur)
         except IndigoException as e:
            print("%d: %s" % (i, getIndigoExceptionText(e)))
         cur += 1
   finally:
      saver.close()
         
   loader = loader_func(filename)
   try:
      for i, m in zip(saved, loader):
         orig_obj_id = None
         try:
            orig_obj = objs[i][1]
            orig_obj_id = objs[i][0]
            err_names = compare_names(orig_obj, m, reloaded_has_name)
            if err_names:
               print("Original object #%d name doesn't match object name #%d in %s: %s" % (orig_obj_id, i + 1, relativePath(filename), err_names))
            err = compare_func(orig_obj, m)
            if err:
               print("Original object #%d doesn't match object #%d in %s: %s" % (orig_obj_id, i + 1, relativePath(filename), err))
         except IndigoException as e:
            print("Exception on compare #%d from %s with original object #%d: %s" % (i + 1, relativePath(filename), orig_obj_id, getIndigoExceptionText(e)))
            print(orig_obj.cml())
            print(m.cml())
   finally:
      #loader.close()
      pass
def compare(m1, m2):
   m2_reloaded = indigo.loadMolecule(m2.rawData())
   data1 = m1.canonicalSmiles()
   data2 = m2_reloaded.canonicalSmiles()
   if data1 != data2:
      return "Molecules representation doesn't match: %s and %s" % (data1, data2)
   return None
      
def canonicalReactionSmiles (r):
   rsmiles = ""
   for m in r.iterateMolecules():
      rsmiles += "."
      rsmiles += m.canonicalSmiles()
   return rsmiles
   
def compareReactions(r1, r2):
   r2_reloaded = indigo.loadReaction(r2.rawData())
   data1 = canonicalReactionSmiles(r1)
   data2 = canonicalReactionSmiles(r2_reloaded)
   if data1 != data2:
      return "Reactions representation doesn't match: %s and %s" % (data1, data2)
   return None
      
def compareQueryReactions(m1, m2):
   m2_reloaded = indigo.loadQueryReaction(m2.rawData())
   data1 = m1.rxnfile()
   data2 = m2_reloaded.rxnfile()
   if data1 != data2:
      return "Query reactions representation doesn't match: %s and %s" % (data1, data2)
   return None
      
def compareQuery(m1, m2):
   m2_reloaded = indigo.loadQueryMolecule(m2.rawData())
   data1 = m1.molfile()
   data2 = m2_reloaded.molfile()
   if data1 != data2:
      return "Query molecules representation doesn't match: %s and %s" % (data1, data2)
   return None

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

if not os.path.exists(joinPath("savers_out")):
    try:
        os.makedirs(joinPath("savers_out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

# molecules   
print("*** Saving molecules in different formats *** ")
print("*** SDF *** ")
testSaveLoad(mols, joinPath("savers_out/out_mols.sdf"), "sdf", indigo.iterateSDFile, compare, True)
print("*** SMI with default name option. Should be disabled by default *** ")
testSaveLoad(mols, joinPath("savers_out/out_mols.smi"), "smiles", indigo.iterateSmilesFile, compare, False)
print("*** SMI with names *** ")
indigo.setOption("smiles-saving-write-name", "true")
testSaveLoad(mols, joinPath("savers_out/out_mols_with_names.smi"), "smiles", indigo.iterateSmilesFile, compare, True)
print("*** SMI without names *** ")
indigo.setOption("smiles-saving-write-name", "false")
testSaveLoad(mols, joinPath("savers_out/out_mols_without_names.smi"), "smiles", indigo.iterateSmilesFile, compare, False)
print("*** CML *** ")
testSaveLoad(mols, joinPath("savers_out/out_mols.cml"), "cml", indigo.iterateCMLFile, compare, True)
print("*** RDF *** ")
testSaveLoad(mols, joinPath("savers_out/out_mols.rdf"), "rdf", indigo.iterateRDFile, compare, True)
# query molecules   
print("*** Saving query molecules in different formats *** ")
print("*** SDF *** ")
testSaveLoad(qmols, joinPath("savers_out/out_qmols.sdf"), "sdf", indigo.iterateSDFile, compareQuery, True)
print("*** RDF *** ")
testSaveLoad(qmols, joinPath("savers_out/out_qmols.rdf"), "rdf", indigo.iterateRDFile, compareQuery, True)
# reactions
print("*** Saving reactions in different formats *** ")
print("*** RDF *** ")
testSaveLoad(reacts, joinPath("savers_out/out_reacts.rdf"), "rdf", indigo.iterateRDFile, compareReactions, True)
print("*** Reaction SMI without names *** ")
indigo.setOption("smiles-saving-write-name", "false")
testSaveLoad(reacts, joinPath("savers_out/out_reacts_without_names.smi"), "smi", indigo.iterateSmilesFile, compareReactions, False)
print("*** Reaction SMI with names *** ")
indigo.setOption("smiles-saving-write-name", "true")
testSaveLoad(reacts, joinPath("savers_out/out_reacts_with_names.smi"), "smi", indigo.iterateSmilesFile, compareReactions, True)
# query reactions
print("*** Saving query reactions in different formats *** ")
print("*** RDF *** ")
testSaveLoad(qreacts, joinPath("savers_out/out_qreacts.rdf"), "rdf", indigo.iterateRDFile, compareQueryReactions, True)
