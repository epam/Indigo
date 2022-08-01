import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

with open(joinPathPy("molecules/stereo_parity.sdf", __file__)) as f:
    data = f.read()
print(data)
print("*** SDF loadString ***")
for m in indigo.iterateSDF(indigo.loadString(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
print("*** SDF loadBuffer ***")
if isIronPython():
    from System import Array, Byte

    data = Array[Byte]([Byte(ord(symbol)) for symbol in data])
else:
    data = map(ord, data)
for m in indigo.iterateSDF(indigo.loadBuffer(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))

with open(joinPathPy("molecules/helma.smi", __file__)) as f:
    data = f.read()
print(data)
print("*** SMILES loadString ***")
for m in indigo.iterateSmiles(indigo.loadString(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
print("*** SMILES loadBuffer ***")
if isIronPython():
    from System import Array, Byte

    data = Array[Byte]([Byte(ord(symbol)) for symbol in data])
else:
    data = map(ord, data)
for m in indigo.iterateSmiles(indigo.loadBuffer(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))

with open(
    joinPathPy(
        "../../../../../data/molecules/basic/tetrahedral-all.cml", __file__
    )
) as f:
    data = f.read()
print(data)
print("*** CML loadString ***")
for m in indigo.iterateCML(indigo.loadString(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
print("*** CML loadBuffer ***")
if isIronPython():
    from System import Array, Byte

    data = Array[Byte]([Byte(ord(symbol)) for symbol in data])
else:
    data = map(ord, data)
for m in indigo.iterateCML(indigo.loadBuffer(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))

with open(joinPathPy("reactions/rxns.rdf", __file__)) as f:
    data = f.read()
print(data)
print("*** RDF loadString ***")
for m in indigo.iterateRDF(indigo.loadString(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
print("*** RDF loadBuffer ***")
if isIronPython():
    from System import Array, Byte

    data = Array[Byte]([Byte(ord(symbol)) for symbol in data])
else:
    data = map(ord, data)
for m in indigo.iterateRDF(indigo.loadBuffer(data)):
    try:
        print(m.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
