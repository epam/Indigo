import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

t = indigo.loadMoleculeFromFile(joinPathPy('molecules/t_795.mol', __file__))
q = indigo.loadMoleculeFromFile(joinPathPy('molecules/q_42.mol', __file__))

print(indigo.exactMatch(q, t, 'ALL') is not None)
print(indigo.exactMatch(q, t, 'ALL -FRA') is not None)

print(indigo.exactMatch(q, t, '') is not None)
print(indigo.exactMatch(q, t, '-FRA') is not None)
