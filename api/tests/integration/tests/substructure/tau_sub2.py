import sys
import json

sys.path.append('../../common')
from env_indigo import *
indigo = Indigo()

mol_iter = indigo.iterateSDFile('molecules/tautomers/targets.sdf')
target_list = [mol for mol in mol_iter]
query_iter = indigo.iterateSDFile('molecules/tautomers/queries.sdf')
queries_list = [indigo.loadQueryMolecule(q.smiles()) for q in query_iter]

matches = dict()
for t, target in enumerate(target_list):
	for q, query in enumerate(queries_list):
		match = indigo.substructureMatcher(target, 'TAU INCHI').match(query)
		if match:
			matches[t + 1, q + 1] = (target, query, match)

for match in sorted(matches):
	print('targertId = {0}, query = {1}:'.format(match[0], match[1]))
	target, query, match = matches[match]
	print('target = ' + target.smiles())
	print('query = ' + query.smiles())
