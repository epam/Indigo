import os
import sys
sys.path.append('../../common')
from env_indigo import *

import collections

indigo = Indigo()
indigo.setOption("ignore-noncritical-query-features", "true")

print("***** Aromaticity models *****")

def executeOperation (m, func, msg):
    try:
        func(m)
        print(msg + m.smiles())
    
    except IndigoException as e:
        print(msg + getIndigoExceptionText(e))

def arom (m):
    m.aromatize()
def dearom (m):
    m.dearomatize()
def noneFunc (m):
    pass
        
groups = [
    ("molecules/basic-generic.sdf", indigo.iterateSDFile),
    ("molecules/basic-generic.smi", indigo.iterateSmilesFile),
]

for file, method in groups:
    print(file)
    for idx, m in enumerate(method(joinPath(file))):
        print(idx)
        for model in ["basic", "generic"]:
            print(model)
            indigo.setOption("aromaticity-model", model)
            try:
                m1 = indigo.loadMolecule(m.rawData())
                m2 = indigo.loadMolecule(m.rawData())
                
                executeOperation(m1, noneFunc, "  Original:     ")
                executeOperation(m1, arom,     "  Arom:         ")
                executeOperation(m2, dearom,   "  Dearom:       ")
                executeOperation(m1, dearom,   "  Arom->Dearom: ")
                executeOperation(m2, arom,     "  Dearom->Arom: ")
                
            except IndigoException as e:
                print("  %s" % (getIndigoExceptionText(e)))

                
print("**** Dearomatization ***")                
indigo.setOption("aromaticity-model", "generic")
total_count = 0

bad_mols = collections.defaultdict(list)

for m2 in indigo.iterateSDFile(joinPath("molecules", "molecules-to-dearom.sdf")):
    indigo.setOption("aromaticity-model", "basic")
    sm = m2.smiles()
    print(sm)
    m2.aromatize()
    sm2 = m2.smiles()
    if sm2 != sm:
        print("  aromatized: " + sm2)
        
    indigo.setOption("aromaticity-model", "generic")
    for value in [True, False]:
        indigo.setOption("dearomatize-verification", value)
        m = indigo.loadMolecule(m2.rawData())
        try:
            m.dearomatize()
            sys.stdout.write("%d -> %s" % (value, m.smiles()))
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))
            
        # check if a structre still has aromatic bonds
        arom = any(b.bondOrder() == 4 for b in m.iterateBonds())
        if arom:
            bad_mols[value].append(m2.smiles())
            sys.stdout.write("   <- Aromatic")

        sys.stdout.write("\n")
        m.aromatize()
        if m.smiles() != m2.smiles():
            print("    arom -> " + m.smiles())
            
    total_count += 1
    
print("Number of molecules that cannot be dearomatized out of %d:" % (total_count))
print(" without verification: %d" % (len(bad_mols[False])))
print(" with verification:    %d" % (len(bad_mols[True])))
