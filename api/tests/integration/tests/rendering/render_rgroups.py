import errno
import sys

sys.path.append("../../common")
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPathPy("out/rgroups", __file__)):
    try:
        os.makedirs(joinPathPy("out/rgroups", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def testRenderRGroups(filename, idx, format):
    indigo.setOption("render-output-format", format)
    mol = indigo.loadMoleculeFromFile(filename)
    qmol = indigo.loadQueryMoleculeFromFile(filename)
    renderer.renderToFile(
        mol,
        joinPathPy("out/rgroups/rgroup-mol-%s.%s" % (idx, format), __file__),
    )
    print(checkImageSimilarity("rgroups/rgroup-mol-%s.%s" % (idx, format)))
    renderer.renderToFile(
        qmol,
        joinPathPy("out/rgroups/rgroup-qmol-%s.%s" % (idx, format), __file__),
    )
    print(checkImageSimilarity("rgroups/rgroup-qmol-%s.%s" % (idx, format)))
    for rgp in mol.iterateRGroups():
        for frag in rgp.iterateRGroupFragments():
            frag.clearAttachmentPoints()
    for rgp in qmol.iterateRGroups():
        for frag in rgp.iterateRGroupFragments():
            frag.clearAttachmentPoints()
    renderer.renderToFile(
        mol, joinPathPy("out/rgroups/noap-mol-%s.%s" % (idx, format), __file__)
    )
    print(checkImageSimilarity("rgroups/noap-mol-%s.%s" % (idx, format)))
    renderer.renderToFile(
        qmol,
        joinPathPy("out/rgroups/noap-qmol-%s.%s" % (idx, format), __file__),
    )
    print(checkImageSimilarity("rgroups/noap-qmol-%s.%s" % (idx, format)))
    for rgp in mol.iterateRGroups():
        for frag in rgp.iterateRGroupFragments():
            frag.remove()
    for rgp in qmol.iterateRGroups():
        for frag in rgp.iterateRGroupFragments():
            frag.remove()
    renderer.renderToFile(
        mol,
        joinPathPy("out/rgroups/norgroup-mol-%s.%s" % (idx, format), __file__),
    )
    print(checkImageSimilarity("rgroups/norgroup-mol-%s.%s" % (idx, format)))
    renderer.renderToFile(
        qmol,
        joinPathPy(
            "out/rgroups/norgroup-qmol-%s.%s" % (idx, format), __file__
        ),
    )
    print(checkImageSimilarity("rgroups/norgroup-qmol-%s.%s" % (idx, format)))
    print(idx + " OK")


testRenderRGroups(
    joinPathPy("molecules/recursive1.mol", __file__), "rec1", "png"
)
testRenderRGroups(
    joinPathPy("molecules/r_occur.mol", __file__), "occur", "png"
)
testRenderRGroups(
    joinPathPy("molecules/r_resth.mol", __file__), "resth", "png"
)
testRenderRGroups(
    joinPathPy("molecules/r1-2ap-aal.mol", __file__), "2ap", "png"
)
testRenderRGroups(
    joinPathPy("molecules/recursive1.mol", __file__), "rec1", "svg"
)
testRenderRGroups(
    joinPathPy("molecules/r_occur.mol", __file__), "occur", "svg"
)
testRenderRGroups(
    joinPathPy("molecules/r_resth.mol", __file__), "resth", "svg"
)
testRenderRGroups(
    joinPathPy("molecules/r1-2ap-aal.mol", __file__), "2ap", "svg"
)

print("*** R-groups and attachment points ***")


def testDecoRender(structures, prefix):
    mols = []
    for smiles in structures:
        mol = indigo.loadMolecule(smiles)
        mol.layout()
        mols.append(mol)

    scaffold = indigo.extractCommonScaffold(mols, "exact")
    decomp_iter = indigo.decomposeMolecules(scaffold, mols)
    cnt = 0
    for decomp in decomp_iter.iterateDecomposedMolecules():
        print("%d:" % cnt)
        cnt += 1
        high_mol = decomp.decomposedMoleculeHighlighted()
        mol = decomp.decomposedMoleculeWithRGroups()
        mol.layout()
        high_mol.layout()
        for format in ["png", "svg"]:
            indigo.setOption("render-output-format", format)
            renderer.renderToFile(
                high_mol,
                joinPathPy(
                    "out/rgroups/deco_%s_%d_hi.%s" % (prefix, cnt, format),
                    __file__,
                ),
            )
            print(
                checkImageSimilarity(
                    "rgroups/deco_%s_%d_hi.%s" % (prefix, cnt, format)
                )
            )
            renderer.renderToFile(
                mol,
                joinPathPy(
                    "out/rgroups/deco_%s_%d.%s" % (prefix, cnt, format),
                    __file__,
                ),
            )
            print(
                checkImageSimilarity(
                    "rgroups/deco_%s_%d.%s" % (prefix, cnt, format)
                )
            )


def testRenderRGroupAtomsRemove(mol_string):
    def remove_r_bonds(mol_):
        for bond in mol_.iterateBonds():
            symbols = [
                bond.source().symbol()[0],
                bond.destination().symbol()[0],
            ]
            if "R" in symbols:
                bond.remove()

    def remove_r_atoms(mol_):
        for atom in mol_.iterateAtoms():
            if atom.symbol().startswith("R"):
                atom.remove()

    def render_molobj(mol_):
        indigo.setOption("render-image-width", 200)
        indigo.setOption("render-image-height", 200)
        indigo.setOption("render-coloring", True)
        indigo.setOption("render-atom-ids-visible", True)
        indigo.setOption("render-output-format", "svg")
        renderer.renderToBuffer(mol_)

    mol = indigo.loadMolecule(mol_string)
    render_molobj(mol)
    remove_r_bonds(mol)
    render_molobj(mol)
    remove_r_atoms(mol)
    render_molobj(indigo.loadMolecule(mol.smiles()))
    render_molobj(mol)  # this should not raise an exception


testDecoRender(
    [
        "C1C2CC3CC1C23",
        "C1CCCCC1",
        "C1C2CC3CC1C1C2C31",
        "C1C23CC22CC132",
        "CCCCC",
        "CCCCCC",
    ],
    "set1",
)

testRenderRGroupAtomsRemove(
    """
  MJ200900                      

  9  9  0  0  0  0  0  0  0  0999 V2000
   -0.2232   -0.1347    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.9376   -0.5472    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.9376   -1.3723    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.2232   -1.7848    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.4912   -1.3723    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.4912   -0.5472    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.6521   -0.1347    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
   -1.7222   -0.8022    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.3353   -0.2501    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  3  4  2  0  0  0  0
  4  5  1  0  0  0  0
  5  6  2  0  0  0  0
  6  1  1  0  0  0  0
  2  8  1  0  0  0  0
  8  9  1  0  0  0  0
  2  7  1  0  0  0  0
M  RGP  2   7   1   9   2
M  END
"""
)

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
