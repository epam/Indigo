import sys, os, threading

sys.path.append('../../common')

from env_indigo import *

threading.stack_size(2*1024*1024)
def stereo_desc_test( py_file, out_queue ):
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", "1")
    indigo.setOption("molfile-saving-add-stereo-desc", "1")
    str_res = "This test checks correct determination and saving of CIP stereo descriptors\n"
    for filename in sorted(os.listdir(joinPathPy("molecules/CIP", py_file))):
        mol = indigo.loadMoleculeFromFile( joinPathPy("molecules/CIP/" + filename , py_file) )
        str_res = str_res + "%s" % filename[:-4] + "\n"
        str_res = str_res + mol.molfile() + "\n"

    indigo.setOption("ignore-stereochemistry-errors", "true")
    rxn = indigo.loadReactionFromFile(joinPathPy("reactions/crazystereo.rxn", py_file))
    str_res = str_res + rxn.rxnfile()
    out_queue.put( str_res )

if isIronPython() or isJython():
    from Queue import Queue
else:
    from queue import Queue

th_queue = Queue()
test_thread = threading.Thread( target = stereo_desc_test, args=( os.path.abspath(__file__), th_queue ) )
test_thread.start()
test_thread.join()
print( th_queue.get() )