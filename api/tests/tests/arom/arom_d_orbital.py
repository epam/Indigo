import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
for m in indigo.iterateSDFile(joinPath('molecules', 'mols.sdf')):
   print(m.canonicalSmiles())
