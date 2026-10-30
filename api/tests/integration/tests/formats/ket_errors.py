import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

root = joinPathPy("molecules/", __file__)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET errors ***")

ket_errors = {
    "ket_invalid_seqid_0": 'Invalid "seqid" value',
    "ket_invalid_seqid_1": 'Invalid "seqid" value',
}
for filename in sorted(ket_errors.keys()):
    error = ket_errors[filename]
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
        print("Test %s failed: exception expected." % filename)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (filename, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (filename, error, text)
            )
