import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
for m in indigo.iterateSDFile(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'molecules', 'mols.sdf')):
   print(m.canonicalSmiles())
