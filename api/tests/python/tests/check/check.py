import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-noncritical-query-features", "true")

tests = [
    { "name" : "issue269test_RGroupsWarning",  "test": "rgroup"}, 
    { "name" : "issue269test_ChiralityWarning",  "test": "chirality"},
    { "name" : "issue269test_StereochemistryWarning",  "test": "stereo"},
    { "name" : "issue269test_PseudoatomWarning",  "test": "pseudoatom"},
    { "name" : "issue269test_RadicalWarning",  "test": "radical"},
    { "name" : "issue269test_QueryWarning",  "test": "query"},
    { "name" : "issue269test_All",  "test": "all"},
    { "name" : "issue269test_Issue_293_All",  "test": "radical;pseudoatom;stereo;query;overlap_atom;overlap_bond;rgroup;chirality;3d_coord"},
    ] 

errors = ''
for test in tests:
    with open(joinPath(f"molecules/{test['name']}.mol"), 'r') as file:
        molfile = file.read()
    print(f"\nTEST: {test['name']}\nResult: {indigo.check2(molfile, test['test'])}")
    