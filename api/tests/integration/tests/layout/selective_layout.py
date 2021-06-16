import sys
import os
import errno

sys.path.append('../../common')
from env_indigo import Indigo, IndigoException, joinPath, getRefFilepath, getIndigoExceptionText, moleculeLayoutDiff

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def create_sd_test_dict(fname):
    res = dict()
    for item in indigo.iterateSDFile(fname):
        prop = item.getProperty('test')
        res[prop] = item.rawData()
    return res


indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "0")

saver1 = indigo.writeFile(joinPath("out", "selective_layout.sdf"))
saver2 = indigo.writeFile(joinPath("out", "selective_clean2d.sdf"))

layout_dict = create_sd_test_dict(getRefFilepath("selective_layout.sdf"))
clean2d_dict = create_sd_test_dict(getRefFilepath("selective_clean2d.sdf"))

atoms_lists = []
atoms_lists.append([[37], [27], [13], [25], [21, 28], [58], [67]])
atoms_lists.append(atoms_lists[0])
atoms_lists.append([[7], [56], [26], [29], [66], [76]])
atoms_lists.append([[0, 1, 2, 3, 4, 5, 6, 7]])

for idx, item in enumerate(indigo.iterateSDFile(joinPath("molecules", "selective_layout.sdf"))):
    try:
        for atom_list in atoms_lists[idx]:
            mol_test = "item #{} selection {} ".format(idx, atom_list)
            print("Test layout for {}".format(mol_test))
            mol = item.clone()
            mol.getSubmolecule(atom_list).layout()
            res = moleculeLayoutDiff(indigo, mol, layout_dict[mol_test], ref_is_file=False)
            print("  Result: {}".format(res))
            mol.setProperty("test", mol_test)
            saver1.sdfAppend(mol)

            print("Test clean2d for {}".format(mol_test))
            mol = item.clone()
            mol.getSubmolecule(atom_list).clean2d()
            res = moleculeLayoutDiff(indigo, mol, clean2d_dict[mol_test], ref_is_file=False)
            print("  Result: {}".format(res))
            mol.setProperty("test", mol_test)
            saver2.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
