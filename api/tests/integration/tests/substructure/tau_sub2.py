import sys, os
import json

sys.path.append('../../common')
from env_indigo import *
indigo = Indigo()
indigo.clearTautomerRules()

base_tau = os.path.abspath(__file__)
mol_iter = indigo.iterateSDFile( joinPathPy( 'molecules/tautomers/targets.sdf', base_tau))
target_list = [mol for mol in mol_iter]
query_iter = indigo.iterateSDFile( joinPathPy( 'molecules/tautomers/queries.sdf', base_tau))
queries_list = [indigo.loadQueryMolecule(q.smiles()) for q in query_iter]

matches = dict()
for t, target in enumerate(target_list):
    for q, query in enumerate(queries_list):
        print( t, q )
        match = indigo.substructureMatcher(target, 'TAU INCHI').match(query)
        if match:
            matches[t+1, q+1] = (target, query, match)

for m in sorted(matches):
    print('targertId = {0}, query = {1}:'.format(m[0], m[1]))
    target, query, m = matches[m]
    print('target = ' + target.smiles())
    print('query = ' + query.smiles())
