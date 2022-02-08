import dgl  # type: ignore
import torch  # type: ignore
import torch.nn.functional as F  # type: ignore

from indigo import Indigo  # type: ignore

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", True)


def featurize_atoms(mol):
    feats = []
    for atom in mol.iterateAtoms():
        feats.append(atom.atomicNumber())
    return {"atomic": F.one_hot(torch.tensor(feats), 60)}


def featurize_edges(mol):
    feats = []
    for bond in mol.iterateBonds():
        feats.append(float(bond.bondOrder()))
    return {"ord": torch.tensor(feats * 2).reshape(-1, 1).float()}


def featurize(mol, g, node_featurizer=None, edge_featurizer=None):
    if node_featurizer is not None:
        g.ndata.update(node_featurizer(mol))

    if edge_featurizer is not None:
        g.edata.update(edge_featurizer(mol))

    return g


def mol_to_graph(smiles: str):
    """Build a graph.

    returns: graph with featurazed nodes/edges
    """
    mol = indigo.loadMolecule(smiles)
    num_atoms = mol.countAtoms()
    # Create graph object
    g = dgl.graph(([], []), idtype=torch.int32)
    # Add nodes
    g.add_nodes(num_atoms)
    # Add edges
    src_list = []
    dst_list = []
    for bond in mol.iterateBonds():
        u = bond.source().index()
        v = bond.destination().index()
        src_list.extend([u, v])
        dst_list.extend([v, u])

    g.add_edges(torch.IntTensor(src_list), torch.IntTensor(dst_list))

    return featurize(
        mol,
        g,
        node_featurizer=featurize_atoms,
        edge_featurizer=featurize_edges,
    )
