import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "true")
  
m_sets = [
   'molecules/abbreviations_test.sdf',
   'molecules/abbreviations_tests2.sdf',
   'molecules/abbreviations_tests3.sdf',
]

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
   
saver = indigo.createFileSaver(joinPath("out/abbreviations_test_out.sdf"), "SDF")
saver_failed = indigo.createFileSaver(joinPath("out/abbreviations_test_failed.sdf"), "SDF")

for set in m_sets:
    print(set)
    for mol in indigo.iterateSDFile(joinPath(set)):
        print(mol.smiles())
        mol.saveMolfile(joinPath("out/last_abbr.mol"))
        
        try:
            mol.expandAbbreviations()
        except IndigoException as e:
            print("Exception:  %s" % (getIndigoExceptionText(e)))

        saver.append(mol)
        print(" ->  " + mol.smiles())

        for a in mol.iterateAtoms():
            if a.isPseudoatom():
                saver_failed.append(mol)
                break


print('####### Test expandAbbreviations for CML')

cm_mol = indigo.loadMolecule('<?xml version="1.0" ?> <cml>     <molecule title="">         <atomArray>             <atom id="a0" elementType="C" x2="-1.0018" y2="1.5616" />             <atom id="a1" elementType="C" x2="-1.7163" y2="1.1491" />             <atom id="a2" elementType="C" x2="-1.7163" y2="0.3241" />             <atom id="a3" elementType="C" x2="-1.0018" y2="-0.0884" />             <atom id="a4" elementType="C" x2="-0.2873" y2="0.3241" />             <atom id="a5" elementType="N" x2="-0.2873" y2="1.1491" />             <atom id="a6" elementType="NO2" x2="0.6334" y2="0.1768" />             <atom id="a7" elementType="SiPr" x2="-2.4307" y2="1.5616" />             <atom id="a8" elementType="NO2" x2="-1.1727" y2="-1.0253" />             <atom id="a9" elementType="NO2" x2="0.4566" y2="1.9165" />             <atom id="a10" elementType="C" x2="-2.4308" y2="-0.0884" />             <atom id="a11" elementType="C" x2="-2.4308" y2="-0.9134" />             <atom id="a12" elementType="C" x2="-3.1452" y2="-1.3259" />             <atom id="a13" elementType="C" x2="-3.1452" y2="-2.1509" />             <atom id="a14" elementType="Ph" x2="-3.8597" y2="-2.5634" />             <atom id="a15" elementType="COOH" x2="-4.3017" y2="-0.9723" />             <atom id="a16" elementType="COOH" x2="-1.9004" y2="-2.8875" />             <atom id="a17" elementType="C" x2="-5.705" y2="1.4879" />             <atom id="a18" elementType="NO2" x2="-4.7843" y2="1.3406" />             <atom id="a19" elementType="C" x2="2.103" y2="-2.1951" />             <atom id="a20" elementType="NO2" x2="3.0237" y2="-2.3424" />         </atomArray>         <bondArray>             <bond atomRefs2="a0 a1" order="1" />             <bond atomRefs2="a0 a5" order="1" />             <bond atomRefs2="a1 a2" order="1" />             <bond atomRefs2="a2 a3" order="1" />             <bond atomRefs2="a3 a4" order="1" />             <bond atomRefs2="a4 a5" order="1" />             <bond atomRefs2="a4 a6" order="1" />             <bond atomRefs2="a1 a7" order="1" />             <bond atomRefs2="a3 a8" order="1" />             <bond atomRefs2="a5 a9" order="1" />             <bond atomRefs2="a2 a10" order="1" />             <bond atomRefs2="a10 a11" order="1" />             <bond atomRefs2="a11 a12" order="1" />             <bond atomRefs2="a12 a13" order="1" />             <bond atomRefs2="a13 a14" order="1" />             <bond atomRefs2="a12 a15" order="1" />             <bond atomRefs2="a13 a16" order="1" />             <bond atomRefs2="a17 a18" order="1" />             <bond atomRefs2="a19 a20" order="1" />         </bondArray>     </molecule> </cml>')
# Should be no exception
print('before {}'.format(cm_mol.countAtoms()))
cm_mol.expandAbbreviations()
print('after {}'.format(cm_mol.countAtoms()))
