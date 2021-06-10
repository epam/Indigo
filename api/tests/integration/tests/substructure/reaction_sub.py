import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

def testSingleReactionSub (rxn1, rxn2, expected, hl):
  rxn1 = indigo.loadQueryReaction(rxn1)
  rxn1.aromatize()
  rxn2 = indigo.loadReaction(rxn2)
  match = indigo.substructureMatcher(rxn2).match(rxn1)
  if match:
    sys.stdout.write("matched")
  else:
    sys.stdout.write("unmatched")
  if (match is None) == expected:
    sys.stdout.write(" (unexpected)")
  print('')
  if not match or not hl:
    return
  for mol in rxn1.iterateMolecules():
    print('mol  {0}'.format(mol.index()))
    for atom in mol.iterateAtoms():
      mapped = match.mapAtom(atom)
      if not mapped:
        mapped = '?'
      else:
        mapped = mapped.index()
      print('atom  {0} -> {1}'.format(atom.index(), mapped))
    for bond in mol.iterateBonds():
      mapped = match.mapBond(bond)
      if not mapped:
        mapped = '?'
      else:
        mapped = mapped.index()
      print('bond  {0} -> {1}'.format(bond.index(), mapped))
  print(match.highlightedTarget().smiles())
  
testSingleReactionSub("[H,O]CN[H,O]>CCC>[H]PC", "OCN>CC>PCO[H]", True, True)
testSingleReactionSub("[Na+]>>", "[Na+]>>", True, True)
testSingleReactionSub("[Na+]>>C1=CC=CC=C1", "[Na+]>>C1=CC=CC=C1", True, True)
testSingleReactionSub("[Na+:5]>>C1=CC=CC=C1", "[Na+:5]>>[CH:32]1=[CH:36][CH:40]=[CH:44][CH:41]=[CH:37]1", True, True)
testSingleReactionSub("[Na+:5]>>[CH:1]1=CC=CC=C1", "[Na+:5]>>[CH:32]1=[CH:36][CH:40]=[CH:44][CH:41]=[CH:37]1", True, True)
testSingleReactionSub("[Na+:53]>>[CH:3]1=[CH:7][CH:14]=[CH:22][CH:15]=[CH:8]1", "[Na+:53]>>[CH:32]1=[CH:36][CH:40]=[CH:44][CH:41]=[CH:37]1", True, True)
testSingleReactionSub(
   "[Na+:52].[Na+:53].[C:31][C:27][N:24]([C:26][C:30]1=[C:34][C:38](=[C:42][C:39]=[C:35]1)[S:43]([O-:48])(=[O:47])=[O:46])[C:22]1=[C:14][C:7]=[C:3]([C:8]=[C:15]1)[C:1](=[C:4]1/[C:10]=[C:17]\[C:23](\[C:16]=[C:9]/1)=[N+:25](/[C:29][C:33])[C:28][C:32]1=[C:36][C:40](=[C:44][C:41]=[C:37]1)[S:45]([O-:51])(=[O:50])=[O:49])\[C:2]1=[C:5]([C:11]=[C:18][C:13]=[C:6]1)[S:12]([O-:21])(=[O:20])=[O:19]>>[C:3]1=[C:7][C:14]=[C:22][C:15]=[C:8]1.[C:32]1=[C:36][C:40]=[C:44][C:41]=[C:37]1.[C:30]1=[C:34][C:38]=[C:42][C:39]=[C:35]1.[C:4]1[C:10]=[C:17][C:23][C:16]=[C:9]1.[C:2]1=[C:5][C:11]=[C:18][C:13]=[C:6]1",
   "[Na+:52].[Na+:53].[CH3:31][CH2:27][N:24]([CH2:26][C:30]1=[CH:34][C:38](=[CH:42][CH:39]=[CH:35]1)[S:43]([O-:48])(=[O:47])=[O:46])[C:22]1=[CH:14][CH:7]=[C:3]([CH:8]=[CH:15]1)[C:1](=[C:4]1/[CH:10]=[CH:17]\[C:23](\[CH:16]=[CH:9]/1)=[N+:25](/[CH2:29][CH3:33])[CH2:28][C:32]1=[CH:36][C:40](=[CH:44][CH:41]=[CH:37]1)[S:45]([O-:51])(=[O:50])=[O:49])\[C:2]1=[C:5]([CH:11]=[CH:18][CH:13]=[CH:6]1)[S:12]([O-:21])(=[O:20])=[O:19]>>[CH:32]1=[CH:36][CH:40]=[CH:44][CH:41]=[CH:37]1.[CH2:4]1[CH:10]=[CH:17][CH2:23][CH:16]=[CH:9]1.[CH:30]1=[CH:34][CH:38]=[CH:42][CH:39]=[CH:35]1.[CH:3]1=[CH:7][CH:14]=[CH:22][CH:15]=[CH:8]1.[CH:2]1=[CH:5][CH:11]=[CH:18][CH:13]=[CH:6]1",
   True, True)
