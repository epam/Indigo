from __future__ import print_function
import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()


def checkMatch(query, mapping):
    d = dict()
    for query_atom in query.iterateAtoms():
        target_atom = mapping.mapAtom(query_atom)
        mapped_ind = '?'
        if not target_atom is None:
            mapped_ind = target_atom.index()
        if query_atom.atomicNumber() != target_atom.atomicNumber():
            print('Error! {0} -> {1}; atomic numbers differ: {2} {3}'.format(query_atom.index(), mapped_ind,
                                                                             query_atom.atomicNumber(),
                                                                             target_atom.atomicNumber()))
        else:
            print('{0} --> {1}; matched'.format(query_atom.index(), mapped_ind, query_atom.atomicNumber(),
                                                target_atom.atomicNumber()))
            d[query_atom.index()] = mapped_ind
    for query_bond in query.iterateBonds():
        target_bond = mapping.mapBond(query_bond)
        if (d[query_bond.source().index()] == target_bond.source().index() and d[
            query_bond.destination().index()] == target_bond.destination().index()) or (
                d[query_bond.source().index()] == target_bond.destination().index() and d[
            query_bond.destination().index()] == target_bond.source().index()):
            if query_bond.bondOrder() == target_bond.bondOrder() or query_bond.bondOrder() == 4 or target_bond.bondOrder() == 4:
                order = ['-', '=', '#', ':'][query_bond.bondOrder() - 1]
                print('Bond {1}{0}{2} --> {3}{0}{4}'.format(order, query_bond.source().index(),
                                                            query_bond.destination().index(),
                                                            target_bond.source().index(),
                                                            target_bond.destination().index()))
            else:
                order1 = ['-', '=', '#', ':'][query_bond.bondOrder() - 1]
                order2 = ['-', '=', '#', ':'][target_bond.bondOrder() - 1]
                print('Error! Bond type not matched. ', end=' ')
                print('Bond {2}{0}{3} --> {4}{1}{5}'.format(order1, order2, query_bond.source().index(),
                                                            query_bond.destination().index(),
                                                            target_bond.source().index(),
                                                            target_bond.destination().index()))
        else:
            print('Error! Bond not matched.')


def testMatch(query, mol, flags):
    print('Query: %s' % (query.smiles()))
    print('Molecule: %s' % (mol.smiles()))
    matcher = indigo.substructureMatcher(mol, flags)
    match = matcher.match(query)
    if match:
        target = match.highlightedTarget()
        print(target.smiles())
        checkMatch(query, match)
    else:
        print('Nothing found')


def testIterateMatches(query, mol, flags):
    print('Query: %s' % (query.smiles()))
    print('Molecule: %s' % (mol.smiles()))
    matcher = indigo.substructureMatcher(mol, flags)
    matches = matcher.iterateMatches(query)
    for mapping in matches:
        checkMatch(query, mapping)


def testEnumTautomersForMolecule(molecule):
    iter = indigo.tautomerEnumerate(molecule, 'INCHI')
    i = 1;
    l = list()
    for mol in iter:
        prod = mol.clone()
        l.append(prod.smiles())
        i += 1;
    print('.'.join(l))


def testBothMethods(query, molecule):
    print('Try TAU:')
    testMatch(query, molecule, "TAU")
    print('Try TAU INCHI (Match):')
    testMatch(query, molecule, "TAU INCHI")
    print('Try TAU RSMARTS (Match):')
    testMatch(query, molecule, "TAU RSMARTS")
    print('Try TAU INCHI (IterateMatches):')
    testIterateMatches(query, molecule, "TAU INCHI")
    print('Try TAU RSMARTS (IterateMatches):')
    testIterateMatches(query, molecule, "TAU RSMARTS")


query = indigo.loadQueryMolecule("OC=CCN")
mol = indigo.loadMolecule("O=C1N=CNC2=C1C=NN2")
testBothMethods(query, mol)

print('Test molecule with irregular atom numbering.')
mol.merge(indigo.loadMolecule("C(=O)1N=CNC2=C1C=NN2"))
mol.getAtom(0).remove()
mol.getAtom(1).remove()
mol.getAtom(2).remove()
mol.getAtom(3).remove()
mol.getAtom(4).remove()
mol.getAtom(5).remove()
mol.getAtom(6).remove()
mol.getAtom(7).remove()
mol.getAtom(8).remove()
mol.getAtom(9).remove()
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("OC=CC=N")
mol = indigo.loadMolecule("O=C1N=CNC2=C1C=NN2")
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("NNC")
mol = indigo.loadMolecule("O=C1N=CNC2=C1C=NN2")
# testEnumTautomersForMolecule(mol)
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("NNCCC")
mol = indigo.loadMolecule("O=C1C(C=NN2)=C2NC=N1")
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("OC1=CC=CNC1")
mol = indigo.loadMolecule("O=C1CNCC2=CC=CC=C12")
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("C=C1CCC(=C)CC1")
mol = indigo.loadMolecule("CC(C)=C1CCC(C)=CC1")
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("CC=CC")
mol = indigo.loadMolecule("CC(=C)NC1=CC=CC=C1C(N)=O")
testBothMethods(query, mol)

query = indigo.loadQueryMolecule("OC=CCN")
mol = indigo.loadMolecule("O=C1N=CNC2=C1C=NN2")
testBothMethods(query, mol)
