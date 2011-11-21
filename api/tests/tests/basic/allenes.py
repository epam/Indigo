import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1");

if not os.path.exists("out"):
   os.makedirs("out")
saver = indigo.createFileSaver("out/allenes_out.sdf", "sdf")

allenes_sets = [ 
    ("molecules/allenes/allenes.smi", indigo.iterateSmilesFile),
    ("../../data/all-allenes.sdf", indigo.iterateSDFile),
    ("molecules/allenes/two-allenes.mol", indigo.iterateSDFile)
    ]

def testSingle(m):
    print("  countAlleneCenters = %d" % (m.countAlleneCenters()))

    print("  Smiles: " + m.canonicalSmiles())
    print("  Cano Smiles: " + m.canonicalSmiles())
    m.layout()
    print("  Molfile: " + m.molfile())
    saver.append(m)

    mh = m.clone()
    mh.foldHydrogens()
    print("  Fold: " + mh.smiles())
    mh.unfoldHydrogens()
    print("  Unfold: " + mh.smiles())
    print("  Cano Unfold: " + mh.canonicalSmiles())
    
    print("  Removing atoms:")
    mr = m.clone()
    while mr.countAtoms() != 0:
        a = mr.iterateAtoms().next()
        a.remove()
        print("    %s" % (mr.smiles()))

mols = []
        
idx = 1
for file, func in allenes_sets:
    print("Molecules set: %s" % (file))
    it = func(file)
    for m in it:
        print("%d" % idx)
        try:
            testSingle(m.clone())
            mols.append((idx, m.clone()))
        except IndigoException, e:
            print("  Error: %s" % (getIndigoExceptionText(e)))        
        idx += 1
        
print("Substructure matching")
qmols = [(idx, indigo.loadQueryMolecule(mq.smiles()), []) for idx, mq in mols]
    
for ti, t in mols:
    matcher = indigo.substructureMatcher(t)
    for qi, q, matched in qmols:
        match = matcher.match(q)
        if match != None:
            matched.append(ti)
            
for qi, q, matched in qmols:
    matched_str = ""
    for ti in matched:
        matched_str += " %d" % (ti)
    print("%d: %s" % (qi, matched_str))
    
    