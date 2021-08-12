import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(joinPathPy("out/allenes_out.sdf", __file__), "sdf")

allenes_sets = [ 
    (joinPathPy("molecules/allenes/allenes.smi", __file__), indigo.iterateSmilesFile),
    (joinPathPy("../../../../../data/molecules/allenes/all-allenes.sdf", __file__), indigo.iterateSDFile),
    (joinPathPy("molecules/allenes/two-allenes.mol", __file__), indigo.iterateSDFile),
    (joinPathPy("molecules/allenes/allenes-angle.mol", __file__), indigo.iterateSDFile),
    (joinPathPy("molecules/allenes/no-coord.mol", __file__), indigo.iterateSDFile),
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
    print("Molecules set: %s" % (relativePath(file)))
    it = func(file)
    for m in it:
        print("%d" % idx)
        try:
            testSingle(m.clone())
            mols.append((idx, m.clone()))
        except IndigoException as e:
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
    
    