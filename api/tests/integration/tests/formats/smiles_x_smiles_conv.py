import sys
import errno

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()


def testMultipleSave(smifile, iterfunc, issmi):
    print("TESTING " + relativePath(smifile))
    sdfout = indigo.writeFile(joinPath("out/structures.sdf"))
    cmlout = indigo.writeFile(joinPath("out/structures.cml"))
    rdfout = indigo.writeFile(joinPath("out/structures.rdf"))
    smiout = indigo.writeFile(joinPath("out/structures.smi"))
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

    cmliter = indigo.iterateCMLFile(joinPath("out/structures.cml"))
    sdfiter = indigo.iterateSDFile(joinPath("out/structures.sdf"))
    rdfiter = indigo.iterateRDFile(joinPath("out/structures.rdf"))
    smiiter = indigo.iterateSmilesFile(joinPath("out/structures.smi"))

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


testMultipleSave(joinPath("../../../../../data/molecules/basic/helma.smi"), indigo.iterateSmilesFile, True)
testMultipleSave(joinPath("molecules/chemical-structures.smi"), indigo.iterateSmilesFile, True)
testMultipleSave(joinPath("molecules/pubchem_7m_err.sdf"), indigo.iterateSDFile, False)
testMultipleSave(joinPath("molecules/acd2d_err.sdf"), indigo.iterateSDFile, False)
