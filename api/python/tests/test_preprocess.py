import torch

from indigo.ml.mpp.feat_params import FeaturizeParams  # type: ignore
from indigo.ml.mpp.preprocess import (  # type: ignore
    create_graph,
    featurize,
    featurize_mol,
    update_graph,
)
from tests import TestIndigoBase


class TestIndigoPreprocess(TestIndigoBase):
    def assert_graph_nodes_edges(self, input: tuple, expected: tuple):
        assert input == expected

    def test_node_featurizers(self):
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("CCC")
        m3 = self.indigo.loadMolecule("C#C")

        self.assert_graph_nodes_edges(
            (create_graph(m1).num_nodes(), create_graph(m1).num_edges()),
            (6, 12),
        )

        self.assert_graph_nodes_edges(
            (create_graph(m2).num_nodes(), create_graph(m2).num_edges()),
            (3, 4),
        )

        self.assert_graph_nodes_edges(
            (create_graph(m3).num_nodes(), create_graph(m3).num_edges()),
            (2, 2),
        )

    def assert_dict_equal(self, input: dict, expected: dict):
        for key_inp, key_exp in zip(input, expected):
            assert key_inp == key_exp
            assert torch.equal(input[key_inp], expected[key_exp])

    def test_update_graph(self):
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("C#C")
        g1 = create_graph(m1)
        g2 = create_graph(m2)
        param1 = FeaturizeParams(node_featurizers=["atomic_number"])

        self.assert_dict_equal(
            update_graph(m1, g1, param1, True).ndata,
            {"atomic": torch.tensor([6, 6, 6, 6, 6, 6]).unsqueeze(1)},
        )

        self.assert_dict_equal(
            update_graph(m2, g2, param1, False).edata,
            {"ord": torch.tensor([3.0, 3.0]).unsqueeze(1)},
        )

    def assert_mol_feat_dict(self, input, expected):
        for key_inp, key_exp in zip(input, expected):
            assert key_inp == key_exp
            assert torch.equal(input[key_inp], expected[key_exp])

    def test_featurize_mol(self):
        m1 = self.indigo.loadMolecule("CCC")
        m2 = self.indigo.loadMolecule("C#C")
        mol_func_features = ["countAtoms"]
        mol_data_features = [3, 7, 22]

        self.assert_dict_equal(
            featurize_mol(m1, mol_func_features=mol_func_features),
            {"mol": torch.tensor([3, 3, 3], dtype=torch.float).unsqueeze(1)},
        )

        self.assert_dict_equal(
            featurize_mol(m2, mol_data_features),
            {
                "mol": torch.tensor(
                    [mol_data_features for _ in range(2)], dtype=torch.float
                )
            },
        )

        self.assert_dict_equal(
            featurize_mol(m2, mol_data_features, mol_func_features),
            {
                "mol": torch.tensor(
                    [[2, 3, 7, 22] for _ in range(2)], dtype=torch.float
                )
            },
        )

    def assert_tensor_equal(self, input, expected):
        assert torch.equal(input, expected)

    def test_featurize(self):
        m1 = self.indigo.loadMolecule("C#C")
        g1 = create_graph(m1)
        mol_data = [17, 22]
        node_feat = ["atomic_number", "atomic_degrees"]
        edge_feat = ["aromatic_bonds", "bond_order"]
        params = FeaturizeParams(
            node_featurizers=node_feat, edge_featurizers=edge_feat
        )
        upd_graph = featurize(m1, g1, params, mol_data)

        n_expected = torch.tensor(
            [[6, 1, 17, 22] for _ in range(2)], dtype=torch.float
        )
        e_expected = torch.tensor(
            [[0, 3] for _ in range(2)], dtype=torch.float
        )

        self.assert_tensor_equal(upd_graph.ndata["n_features"], n_expected)

        self.assert_tensor_equal(upd_graph.edata["e_features"], e_expected)
