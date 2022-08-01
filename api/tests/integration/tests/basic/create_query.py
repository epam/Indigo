import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

original = indigo.createFileSaver(
    joinPathPy("out/original.sdf", __file__), "sdf"
)
saver = indigo.createFileSaver(
    joinPathPy("out/create_query.sdf", __file__), "sdf"
)
saverRxn = indigo.createFileSaver(
    joinPathPy("out/create_query.rdf", __file__), "rdf"
)


def infrange(start):
    cur = start
    while True:
        yield cur
        cur += 1


def makeQueryMol(m):
    qm = indigo.loadQueryMolecule(m.molfile())
    qa = qm.getAtom(0)
    qa.removeConstraints("atomic-number")
    return qm


def makeReaction(m):
    r = indigo.createReaction()
    r.addReactant(m)
    r.addProduct(m)
    return r


def makeQueryReaction(qm):
    r = indigo.createQueryReaction()
    r.addReactant(qm)
    r.addProduct(qm)
    return r


mols = []
print("*** Loading molecules *** ")
for i, m in zip(
    infrange(1),
    indigo.iterateCMLFile(
        joinPathPy(
            "../../../../../data/molecules/basic/tetrahedral-all.cml", __file__
        )
    ),
):
    try:
        mols.append((i, m.clone()))
        original.append(m)
    except IndigoException as e:
        print("%d: %s" % (i, getIndigoExceptionText(e)))

print("*** Molecule canonical smiles *** ")
for i, m in mols:
    try:
        print("%d: %s" % (i, m.canonicalSmiles()))
    except IndigoException as e:
        print("%d: %s" % (i, getIndigoExceptionText(e)))

print("*** Query molecule molfiles *** ")
for i, m in mols:
    try:
        qm = makeQueryMol(m)
        print("%d: %s" % (i, qm.molfile()))
        saver.append(qm)
    except IndigoException as e:
        print("%d: %s" % (i, getIndigoExceptionText(e)))
print("*** Reaction smiles *** ")
for i, m in mols:
    try:
        r = makeReaction(m)
        print("%d: %s" % (i, r.smiles()))
    except IndigoException as e:
        print("%d: %s" % (i, getIndigoExceptionText(e)))

print("*** Query reaction rxnfiles *** ")
for i, m in mols:
    try:
        qm = makeQueryMol(m)
        qr = makeQueryReaction(qm)
        print("%d: %s" % (i, qr.rxnfile()))
        saverRxn.append(qr)
    except IndigoException as e:
        print("%d: %s" % (i, getIndigoExceptionText(e)))
