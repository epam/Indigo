import os
import sys

sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *


def exact_check(indigo, m1, m2):
    assert m1.countAtoms() == m2.countAtoms()
    for i in range(m1.countAtoms()):
        assert m1.getAtom(i).symbol() == m2.getAtom(i).symbol()
    assert m1.countBonds() == m2.countBonds()
    for i in range(m1.countBonds()):
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
        print(struct)
        assert exact_check(indigo, m1, m2)


def test_simple_cis_trans_load(indigo):
    m1 = indigo.loadMoleculeFromFile(joinPath("molecules/cis_trans.mol"))
    js = m1.json()
    m2 = indigo.loadMolecule(js)
    exact_check(indigo, m2, m2)


def test_complex_load_save_load(indigo):
    """Check load-save-load for some files"""
    paths = ("molecules/ketcher.mol", "molecules/all2000.mol")
    for path in paths:
        print(path)
        m1 = indigo.loadMoleculeFromFile(joinPath(path))
        js = m1.json()
        m2 = indigo.loadMolecule(js)
        # indigo.dbgBreakpoint()
        exact_check(indigo, m1, m2)


def test_reactions_load_save_load(indigo):
    paths = ("reactions/cdxml/AmideFormation.rxn", "reactions/cdxml/Claisen.rxn",
             "reactions/cdxml/CN_Bond-S-GRP.rxn", "reactions/cdxml/CN_Bond.rxn",
             "reactions/cdxml/CN_Bond_map.rxn")
    for path in paths:
        m1 = indigo.loadReactionFromFile(joinPath(path))
        js = m1.json()
        m2 = indigo.loadReaction(js)
        # indigo.dbgBreakpoint()
        assert indigo.exactMatch(m1, m2)


if __name__ == '__main__':
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", "1")
    # test_simple_load_save_load(indigo)
    # test_simple_cis_trans_load(indigo)
    test_complex_load_save_load(indigo)
    test_reactions_load_save_load(indigo)
