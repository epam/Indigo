import sys, os, threading
sys.path.append('../../common')

from env_indigo import *

threading.stack_size(2*1024*1024)
def stereo_desc_test( py_file ):
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", "1")
    indigo.setOption("molfile-saving-add-stereo-desc", "1")
    print("This test checks correct determination and saving of CIP stereo descriptors")
    print( joinPathPy( "molecules/CIP", py_file ) )
    for filename in sorted(os.listdir(joinPathPy("molecules/CIP", py_file))):
        mol = indigo.loadMoleculeFromFile( joinPathPy("molecules/CIP/" + filename , py_file) )
        print("%s" % filename[:-4])
        print(mol.molfile())

    indigo.setOption("ignore-stereochemistry-errors", "true")
    rxn = indigo.loadReactionFromFile(joinPath("reactions/crazystereo.rxn"))
    print(rxn.rxnfile())

test_thread = threading.Thread( target = stereo_desc_test, args=( os.path.abspath(__file__) ,) )
test_thread.start()
