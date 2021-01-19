import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *


def exact_check(indigo, m1, m2):
    assert m1.countAtoms() == m2.countAtoms()
    for i in range(m1.countAtoms()):
        assert m1.getAtom(i).symbol() == m2.getAtom(i).symbol()
    assert m1.countBonds() == m2.countBonds()
    for i in range(m2.countBonds()):
        assert m1.getBond(i).bondOrder() == m2.getBond(i).bondOrder()
        assert m1.getBond(i).bondStereo() == m2.getBond(i).bondStereo()
    assert m1.isChiral() == m2.isChiral()
    assert indigo.exactMatch(m1, m2)
    return True


def test_simple_load_save_load(indigo):
    """Check load-save-load for some simple SMILES"""
    structs = ("C", "C1=CC=CC=C1", "Nc1[nH]cnc-2ncnc1-2")
    for struct in structs:
        m1 = indigo.loadMolecule(struct)
        m1.layout()
        js = m1.json()
        m2 = indigo.loadMolecule(js)
        assert exact_check(indigo, m1, m2)


def test_simple_cis_trans_load(indigo):
    m1 = indigo.loadMoleculeFromFile(joinPath("molecules/cis_trans.mol"))
    m2 = indigo.loadMoleculeFromFile(joinPath("molecules/cis_trans.ket.json"))
    exact_check(indigo, m1, m2)


def test_complex_load_save_load(indigo):
    """Check load-save-load for some files"""
    paths = ("molecules/1.ket.json", "molecules/sgroups.mol", "molecules/all_features_mol.mol")
    for path in paths:
        m1 = indigo.loadMoleculeFromFile(joinPath(path))
        js = m1.json()
        m2 = indigo.loadMolecule(js)
        exact_check(indigo, m1, m2)


if __name__ == '__main__':
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", "1")
    test_simple_load_save_load(indigo)
    test_simple_cis_trans_load(indigo)
    test_complex_load_save_load(indigo)
