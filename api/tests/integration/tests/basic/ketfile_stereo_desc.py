import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *

threading.stack_size(2 * 1024 * 1024)


def stereo_desc_test(py_file, out_queue):
    str_res = ""
    ref_path = joinPathPy("ref/", __file__)
    root = joinPathPy("molecules/CIP/", __file__)

    indigo = Indigo()
    indigo.setOption("json-saving-pretty", True)
    indigo.setOption("json-saving-add-stereo-desc", True)

    for filename in sorted(os.listdir(root)):
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        ketfile = joinPathPy(
            os.path.join(ref_path, filename[:-4] + ".ket"), __file__
        )
        with open(ketfile, "r") as file:
            ket_ref = file.read()
            diff = find_diff(ket_ref, mol.json())
            if not diff:
                str_res += filename + ":SUCCEED\n"
            else:
                str_res += filename + ":FAILED\n"
                str_res += diff

    indigo.setOption("ignore-stereochemistry-errors", "true")
    filename = "crazystereo.rxn"

    rxn = indigo.loadReactionFromFile(
        os.path.join(joinPathPy("reactions/", __file__), filename)
    )

    ketfile = joinPathPy(os.path.join(ref_path, "crazystereo.ket"), __file__)
    with open(ketfile, "r") as file:
        ket_ref = file.read()
        diff = find_diff(ket_ref, rxn.json())
        if not diff:
            str_res += filename + ":SUCCEED\n"
        else:
            str_res += filename + ":FAILED\n"
            str_res += diff + "\n"
    out_queue.put(str_res)


if isJython():
    from Queue import Queue
else:
    from queue import Queue

th_queue = Queue()
test_thread = threading.Thread(
    target=stereo_desc_test, args=(os.path.abspath(__file__), th_queue)
)
test_thread.start()
test_thread.join()
print(th_queue.get())
