import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

def testAlignAtoms():
    query = indigo.loadSmarts("[#7]1~[#6]~[#6]~[#7]~[#6]~[#6]2~[#6]~[#6]~[#6]~[#6]~[#6]~1~2")
    sdfout = indigo.writeFile(joinPathPy("out/aligned.sdf", __file__))
    xyz = []
    collection = indigo.createArray()
    refatoms = []
    for structure in indigo.iterateSDFile(joinPathPy("molecules/benzodiazepine.sdf.gz", __file__)):
        match = indigo.substructureMatcher(structure).match(query)
        if not match:
            print("structure not matched, this is unexpected")
            return
        if not structure.index():
            for atom in query.iterateAtoms():
                xyz.extend(match.mapAtom(atom).xyz())
        else:
            atoms = [match.mapAtom(atom).index() for atom in query.iterateAtoms()]
            x = structure.alignAtoms(atoms, xyz)
            print('%.6f' % x)

            structure.foldHydrogens()
            sdfout.sdfAppend(structure)
        structure.setProperty("title", "Molecule:%d\nMass: %f\nFormula: %s" % (structure.index(), structure.molecularWeight(), structure.grossFormula()))
        refatoms.append(match.mapAtom(query.getAtom(0)).index())
        collection.arrayAdd(structure)
        if structure.index() == 15:
            break

    indigo.setOption("render-highlight-thickness-enabled", "true")
    indigo.setOption("render-image-size", "400, 400")
    indigo.setOption("render-grid-title-property", "PUBCHEM_COMPOUND_CID")
    indigo.setOption("render-grid-title-font-size", "10")
    indigo.setOption("render-grid-title-offset", "2")
    indigo.setOption("render-grid-title-alignment", "0.5")
    indigo.setOption("render-coloring", "true")

    indigo.setOption("render-output-format", "svg")
    renderer.renderGridToFile(collection, None, 4, joinPathPy("out/grid.svg", __file__))
    print(checkImageSimilarity('grid.svg'))
    indigo.setOption("render-output-format", "png")
    renderer.renderGridToFile(collection, None, 4, joinPathPy("out/grid.png", __file__))
    print(checkImageSimilarity('grid.png'))
    indigo.setOption("render-output-format", "svg")
    if isIronPython():
        from System import Array, Int32

        refatoms = Array[Int32](refatoms)
    renderer.renderGridToFile(collection, refatoms, 4, joinPathPy("out/grid1.svg", __file__))
    print(checkImageSimilarity('grid1.svg'))
    indigo.setOption("render-output-format", "png")
    renderer.renderGridToFile(collection, refatoms, 4, joinPathPy("out/grid1.png", __file__))
    print(checkImageSimilarity('grid1.png'))

    indigo.setOption("render-grid-title-property", "title")
    indigo.setOption("render-image-size", "-1, -1")
    indigo.setOption("render-bond-length", "30")
    indigo.setOption("render-output-format", "png")
    options_align = [ "left", "right", "center", "center-left", "center-right" ]
    for alignment in options_align:
        indigo.setOption("render-grid-title-alignment", alignment)
        fname = "grid-%s.png" % alignment
        renderer.renderGridToFile(collection, None, 4, joinPathPy("out/" + fname, __file__))
        print(checkImageSimilarity(fname))


testAlignAtoms()
renderer.Dispose()