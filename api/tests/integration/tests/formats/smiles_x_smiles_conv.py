import sys
import errno

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()


def testMultipleSave(smifile, iterfunc, issmi):
    print("TESTING " + relativePath(smifile))
    sdfout = indigo.writeFile(joinPathPy("out/structures.sdf", __file__))
    cmlout = indigo.writeFile(joinPathPy("out/structures.cml", __file__))
    rdfout = indigo.writeFile(joinPathPy("out/structures.rdf", __file__))
    smiout = indigo.writeFile(joinPathPy("out/structures.smi", __file__))
    rdfout.rdfHeader()
    cmlout.cmlHeader()
    for item in iterfunc(smifile):
        exc = False
        try:
            item.countAtoms()
            item.smiles()
        except IndigoException as e:
            print('{0} : {1}'.format(item.index(), getIndigoExceptionText(e)))
            if issmi:
                print(item.rawData())
            exc = True
        if not exc:
            # item.clearCisTrans()
            for bond in item.iterateBonds():
                if bond.topology() == Indigo.RING and bond.bondOrder() == 2:
                    bond.resetStereo()
            try:
                item.markEitherCisTrans()
            except IndigoException as e:
                print('{0} (while markEitherCisTrans) : {1}'.format(item.index(), getIndigoExceptionText(e)))
                if issmi:
                    print(item.rawData())
                continue

            if issmi:
                item.setName("structure-{0} {1}".format((item.index()), item.rawData()))
            else:
                item.setName("structure-{0}".format(item.index()))
            item.setProperty("NUMBER", str(item.index()))
            cmlout.cmlAppend(item)
            smiout.smilesAppend(item)
            item.layout()
            indigo.setOption("molfile-saving-mode", "2000")
            sdfout.sdfAppend(item)
            indigo.setOption("molfile-saving-mode", "3000")
            rdfout.rdfAppend(item)
    cmlout.cmlFooter()

    sdfout.close()
    cmlout.close()
    rdfout.close()
    smiout.close()

    cmliter = indigo.iterateCMLFile(joinPathPy("out/structures.cml", __file__))
    sdfiter = indigo.iterateSDFile(joinPathPy("out/structures.sdf", __file__))
    rdfiter = indigo.iterateRDFile(joinPathPy("out/structures.rdf", __file__))
    smiiter = indigo.iterateSmilesFile(joinPathPy("out/structures.smi", __file__))

    idx = 1
    while sdfiter.hasNext():
        cml = cmliter.next()
        sdf = sdfiter.next()
        rdf = rdfiter.next()
        smi = smiiter.next()

        print('{0} {1}'.format(sdf.index(), sdf.name()))
        sdf.resetSymmetricCisTrans()
        rdf.resetSymmetricCisTrans()
        try:
            cs1 = sdf.canonicalSmiles()
            cs2 = rdf.canonicalSmiles()
            cs3 = smi.canonicalSmiles()
            cs4 = cml.canonicalSmiles()
        except IndigoException as e:
            print(getIndigoExceptionText(e))
            continue
        print(cs1)
        print(cs2)
        print(cs3)
        print(cs4)
        if cs2 != cs1:
            print("MISMATCH")
        if cs3 != cs1:
            print("MISMATCH")
        if cs4 != cs1:
            print("MISMATCH")
        idx += 1


testMultipleSave(joinPathPy("../../../../../data/molecules/basic/helma.smi", __file__), indigo.iterateSmilesFile, True)
testMultipleSave(joinPathPy("molecules/chemical-structures.smi", __file__), indigo.iterateSmilesFile, True)
testMultipleSave(joinPathPy("molecules/pubchem_7m_err.sdf", __file__), indigo.iterateSDFile, False)
testMultipleSave(joinPathPy("molecules/acd2d_err.sdf", __file__), indigo.iterateSDFile, False)
