import errno
import math
import os
import sys
from math import *

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("treat-x-as-pseudoatom", "1")


def check_case(name):
    print("**** Test {} ****".format(name))
    saver = indigo.writeFile(joinPathPy("out/{}.sdf".format(name), __file__))
    # print(joinPathPy("out", "%s.sdf" % name))
    ref_path = getRefFilepath("{}.sdf".format(name))
    ref = indigo.iterateSDFile(ref_path)
    for idx, item in enumerate(
        indigo.iterateSDFile(joinPathPy("molecules/orientation.sdf", __file__))
    ):
        try:
            mol = item.clone()
            mol.layout()
            res = moleculeLayoutDiff(
                indigo, mol, ref.at(idx).rawData(), ref_is_file=False
            )
            print("  Item #{}: Result: {}".format(idx, res))
            saver.sdfAppend(mol)
        except IndigoException as e:
            print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
    saver.close()


indigo.setOption("smart-layout", "0")
check_case("orientation_smart0")
indigo.setOption("smart-layout", "1")
check_case("orientation_smart1")

indigo.setOption("layout-orientation", "horizontal")
indigo.setOption("smart-layout", "0")
check_case("orientation_horizontal_smart0")
indigo.setOption("smart-layout", "1")
check_case("orientation_horizontal_smart1")

indigo.setOption("layout-orientation", "vertical")
indigo.setOption("smart-layout", "0")
check_case("orientation_vertical_smart0")
indigo.setOption("smart-layout", "1")
check_case("orientation_vertical_smart1")

indigo.setOption("layout-orientation", "unspecified")
indigo.setOption("smart-layout", "0")
check_case("orientation_unspecified_smart0")
indigo.setOption("smart-layout", "1")
check_case("orientation_unspecified_smart1")
