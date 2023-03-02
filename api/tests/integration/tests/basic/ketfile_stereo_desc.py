import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

threading.stack_size(2 * 1024 * 1024)


def stereo_desc_test(py_file, out_queue):
    indigo = Indigo()
    indigo.setOption("json-saving-pretty", True)
    indigo.setOption("json-saving-add-stereo-desc", "1")
    str_res = "This test checks correct determination and saving of CIP stereo descriptors\n"
    for filename in sorted(os.listdir(joinPathPy("molecules/CIP", py_file))):
        mol = indigo.loadMoleculeFromFile(
            joinPathPy("molecules/CIP/" + filename, py_file)
        )
        str_res = str_res + "%s" % filename[:-4] + "\n"
        str_res = str_res + mol.json() + "\n"

    indigo.setOption("ignore-stereochemistry-errors", "true")
    rxn = indigo.loadReactionFromFile(
        joinPathPy("reactions/crazystereo.rxn", py_file)
    )
    str_res = str_res + rxn.json()
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
