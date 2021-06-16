from __future__ import print_function
import sys
sys.path.append('../../common')
from env_indigo import Indigo, IndigoException

indigo = Indigo()
indigo.setOption('ignore-stereochemistry-errors', True)
indigo.setOption('smart-layout', True)


for struct in ('''
  Ketcher 10171617122D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
   20.5253   -4.7001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.3913   -5.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.3913   -6.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.5253   -6.7001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.6593   -6.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.6593   -5.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  5  6  2  0     0  0
  2  3  1  0     0  0
  3  4  2  0     0  0
  4  5  1  0     0  0
  6  1  1  0     0  0
  1  2  2  0     0  0
M  END
''', 'C1=CC=CC=C1', 'F/C=C/F', 'F/C=C\F'):
    print(struct)
    m = indigo.loadMolecule(struct)
    for b in m.iterateBonds():
        print(b.bondStereo())
