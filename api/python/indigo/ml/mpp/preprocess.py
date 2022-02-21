from typing import Callable, Collection, Dict, List, Optional

import dgl
import featurizers
import torch
import torch.nn.functional as F  # type: ignore
from dgl.heterograph import DGLHeteroGraph
from feat_params import FeaturizeParams

from indigo import Indigo  # type: ignore
from indigo import IndigoObject

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", True)


def featurize_mol(
    mol: IndigoObject,
    mol_data_features: Optional[List[float]] = None,
    mol_func_features: Optional[Collection[str]] = None,
) -> Dict:
    """Get whole molecule features.

        Get all molecule features either from IndigoObject methods or/and from dataset. Then pass them all into dict
        with key "mol" and values as tensor
    Args:
        mol: molecule object
        mol_data_features: List of features, that can be taken from dataset
        mol_func_features: molecule features, that can be taken from IndigoObject methods. example: molecularWeight as
        mol.molecularWeight().
    Returns:
        Dict with key 'mol' and tenser with size[num of molecules, num of features] as value
    """
    feats = []
    if mol_func_features is not None:
        for fun in mol_func_features:
            feats.append(getattr(mol, fun)())
    if mol_data_features is not None:
        feats = feats + mol_data_features
    return {"mol": torch.tensor([feats for _ in mol.iterateAtoms()])}


def featurize_atoms(mol: IndigoObject) -> Dict:
    feats = []
    for atom in mol.iterateAtoms():
        feats.append(atom.atomicNumber())
    return {"atomic": F.one_hot(torch.tensor(feats), 60)}


def featurize_edges(mol: IndigoObject) -> Dict:
    feats = []
    for bond in mol.iterateBonds():
        feats.append(float(bond.bondOrder()))
    return {"ord": torch.tensor(feats * 2).reshape(-1, 1).float()}


def featurize(
    mol: IndigoObject,
    g: DGLHeteroGraph,
    params: FeaturizeParams,
    mol_data_features: Optional[Collection[float]] = None,
) -> DGLHeteroGraph:
    """Core function for final tensors.

    Used to get all descriptors for edges, nodes or whole molecule, than packs this descriptors into final tensors.

    Args:
        params: Dataclass, that holds information about our descriptors
        mol: molecule object
        g: instance of DGLHeteroGraph for molecule
        mol_data_features: collection of values, that was taken from dataset

    Returns:
        updated instance of DGLHeteroGraph
    """

    g = update_graph(mol, g, params, True)
    g = update_graph(mol, g, params, False)

    if mol_data_features or params.mol_func_features:
        g.ndata.update(
            featurize_mol(mol, mol_data_features, params.mol_func_features)
        )

    g.ndata["n_features"] = torch.hstack([v for k, v in g.ndata.items()])
    g.edata["e_features"] = torch.hstack([v for k, v in g.edata.items()])
    return g


def update_graph(
    mol: IndigoObject,
    g: DGLHeteroGraph,
    params: FeaturizeParams,
    node: bool = True,
):
    """Update node or edge graph's features.

    Update node or edge graph's features depending on 'node' bool parameter.
    Args:
        mol: molecule object
        g: instance of DGLHeteroGraph for molecule
        params: Dataclass, that holds information about our descriptors
        node: bool to know what to update

    Returns:
        updated instance of DGLHeteroGraph
    """
    prams = params.node_featurizers if node else params.edge_featurizers
    func = g.ndata.update if node else g.edata.update
    default_featurizer = featurize_atoms if node else featurize_edges
    if not prams:
        node_featurizers = default_featurizer
        func(node_featurizers(mol))
    else:
        for featurizer in prams:
            func(getattr(featurizers, featurizer)(mol))
    return g


def mol_to_graph(
    smiles: str, params: FeaturizeParams, mol_d_feat: Optional[Collection]
) -> DGLHeteroGraph:
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

    return featurize(mol, g, params, mol_d_feat)
