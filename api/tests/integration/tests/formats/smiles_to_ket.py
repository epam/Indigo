import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)

print("*** SMILES to KET ***")

ref_path = joinPathPy("ref/", __file__)

tests = {
    "sgroup_data_all_fields": (
        "CCCCC |SgD:1,2,3:"
        "name&#44;&#59;&#58;&#124;&#123;&#125;&#36;field:"
        "data&#44;&#59;&#58;&#124;&#123;&#125;&#36;value:"
        "query&#44;&#59;&#58;&#124;&#123;&#125;&#36;op:"
        "unit&#44;&#59;&#58;&#124;&#123;&#125;&#36;text: :|"
    ),
    "sgroup_sru_subscript": (
        "CCCCC |Sg:n:1,2,3:sub&#44;&#59;&#58;&#124;&#123;&#125;&#36;script:hh|"
    ),
}

for filename, smiles in tests.items():
    ket = indigo.loadMolecule(smiles).json()
    compare_diff(ref_path, filename + ".ket", ket)
