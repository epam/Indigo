import sys, threading
sys.path.append('../../common')

from env_indigo import *

threading.stack_size(2*1024*1024)
def stereo_desc_test():
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", "1")
    indigo.setOption("molfile-saving-add-stereo-desc", "1")
    print("This test checks correct determination and saving of CIP stereo descriptors")

    for filename in sorted(os.listdir(joinPath("molecules/CIP"))):
        mol = indigo.loadMoleculeFromFile(os.path.join(joinPath("molecules/CIP"), filename))
        print("%s" % filename[:-4])
        print(mol.molfile())

    indigo.setOption("ignore-stereochemistry-errors", "true")
    rxn = indigo.loadReactionFromFile(joinPath("reactions/crazystereo.rxn"))
    print(rxn.rxnfile())

test_thread = threading.Thread( target = stereo_desc_test )
test_thread.start()
