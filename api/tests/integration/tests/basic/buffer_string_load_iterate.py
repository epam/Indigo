import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

with open(joinPath('molecules/stereo_parity.sdf')) as f:
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

with open(joinPath('molecules/helma.smi')) as f:
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

with open(joinPath('../../../../../data/molecules/basic/tetrahedral-all.cml')) as f:
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

with open(joinPath('reactions/rxns.rdf')) as f:
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
