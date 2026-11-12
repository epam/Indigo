import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)

print("*** KET version ***")

root = joinPathPy("molecules/ket_version/", __file__)
ref_path = joinPathPy("ref/ket_version/", __file__)

# Every version from 2.0.0 on keeps the text nodes of the KET 2.0 form;
# 10.0.0 is earlier than 2.0.0 as a string, not as a number.
saved_as_2_1 = "text-saved-2.1.0.ket"
saved_by_version = (
    ("2.0.0", "text-saved-2.0.0.ket"),
    ("2.1.0", saved_as_2_1),
)
for filename in ("text-2.0.0", "text-2.1.0", "text-10.0.0", "text-2.1.0-rc.1"):
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    for saving_version, ref_filename in saved_by_version:
        indigo.setOption("ket-saving-version", saving_version)
        diff = compare_diff(ref_path, ref_filename, mol.json(), stdout=False)
        print(
            "%s.ket saved as %s:%s"
            % (
                filename,
                saving_version,
                "FAILED\n" + diff if diff else "SUCCEED",
            )
        )

indigo.setOption("ket-saving-version", "2.1.0")
saved = indigo.loadMoleculeFromFile(
    os.path.join(root, "text-2.1.0.ket")
).json()
diff = compare_diff(
    ref_path, saved_as_2_1, indigo.loadMolecule(saved).json(), stdout=False
)
print(
    "%s reloaded:%s" % (saved_as_2_1, "FAILED\n" + diff if diff else "SUCCEED")
)

for filename in (
    "version-not-a-number",
    "version-four-parts",
    "version-trailing-dot",
    "version-number",
):
    try:
        indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
        print("%s.ket: loaded" % filename)
    except IndigoException as e:
        print("%s.ket: %s" % (filename, getIndigoExceptionText(e)))

for option_value in ("2.1", "2.1.0-rc.1", "abc"):
    try:
        indigo.setOption("ket-saving-version", option_value)
        print(
            "ket-saving-version %s: %s"
            % (option_value, indigo.getOption("ket-saving-version"))
        )
    except IndigoException as e:
        print(
            "ket-saving-version %s: %s"
            % (option_value, getIndigoExceptionText(e))
        )
