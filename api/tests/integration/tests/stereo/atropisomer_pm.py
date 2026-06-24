import os
import re
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

EXPECTED = {
    "case13-A.mol": "P",
    "case13-B.mol": "M",
    "case14-A.mol": "P",
    "case14-B.mol": "M",
}

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

fixtures = "../../../../../data/molecules/atropisomers"
print("Atropisomer P/M CIP descriptors")
for filename in sorted(EXPECTED):
    expected = EXPECTED[filename]
    path = joinPathPy(fixtures + "/" + filename, __file__)

    indigo.setOption("json-saving-add-stereo-desc", "1")
    ket_cips = re.findall(
        r'"cip"\s*:\s*"([^"]+)"', indigo.loadMoleculeFromFile(path).json()
    )

    indigo.setOption("molfile-saving-add-stereo-desc", "1")
    molfile_cips = re.findall(
        r"\((P|M)\)", indigo.loadMoleculeFromFile(path).molfile()
    )

    print("%s -> %s" % (filename, expected))
    assert ket_cips == [expected], "%s: KET cip %s, expected [%s]" % (
        filename,
        ket_cips,
        expected,
    )
    assert molfile_cips == [expected], "%s: molfile cip %s, expected [%s]" % (
        filename,
        molfile_cips,
        expected,
    )
