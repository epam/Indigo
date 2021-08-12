import errno
import sys

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("treat-x-as-pseudoatom", "1")

print("***** Test cleand2d.sdf *****")
ref_path = getRefFilepath('clean2d.sdf')
ref = indigo.iterateSDFile(ref_path)
saver = indigo.writeFile(joinPathPy('out/clean2d.sdf', __file__))

for idx, item in enumerate(indigo.iterateSDFile(joinPathPy("molecules/cleand2d.sdf", __file__))):
    try:
        item.clean2d()
        res = moleculeLayoutDiff(indigo, item, ref.at(idx).rawData(), ref_is_file=False)
        print('  Item #{}: Result: {}'.format(idx, res))
        saver.sdfAppend(item.clone())
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))

print("***** Test cleand2d for SMILES 1 *****")
m = indigo.loadMolecule('C1=CC=CC=C1')
m.clean2d()
res = moleculeLayoutDiff(indigo, m, 'clean2d_test1.mol')
print('  Result: {}'.format(res))
m.saveMolfile(joinPathPy('out/clean2d_test1.mol', __file__))

print("***** Test cleand2d for SMILES 1 *****")
m = indigo.loadMolecule('C=C=C')
sm = m.getSubmolecule([0, 1, 2])
sm.clean2d()
res = moleculeLayoutDiff(indigo, sm, 'clean2d_test2.mol')
print('  Result: {}'.format(res))
sm.saveMolfile(joinPathPy('out/clean2d_test2.mol', __file__))
