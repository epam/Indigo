import torch  # type: ignore

from indigo.ml.mpp.featurizers import (  # type: ignore
    acid_pka_values,
    aromatic_bonds,
    atom_in_ring,
    atomic_charges,
    atomic_degrees,
    atomic_isotopes,
    atomic_masses,
    atomic_number,
    atomic_radicals,
    atomic_valences,
    basic_pka_values,
    bond_order,
    bond_stereo,
    implicit_hydrogens,
    stereocenter_types,
    topologies,
)
from tests import TestIndigoBase


class TestIndigoFeaturizers(TestIndigoBase):
    def assertTensorEqual(self, input: dict, expected: dict):
        for input_key, expected_key in zip(input, expected):
            assert input_key == expected_key
            assert torch.equal(input[input_key], expected[expected_key])

    def test_node_featurizers(self):
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("[2H][H]")
        m3 = self.indigo.loadMolecule("C[CH2]")
        m4 = self.indigo.loadMolecule("C1CC[C@H]([C@H](C1)Cl)Br")

        self.assertTensorEqual(
            atomic_number(m1),
            {"atomic": torch.tensor([6, 6, 6, 6, 6, 6]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_number(m2), {"atomic": torch.tensor([1, 1]).unsqueeze(1)}
        )
        self.assertTensorEqual(
            atomic_number(m3), {"atomic": torch.tensor([6, 6]).unsqueeze(1)}
        )
        self.assertTensorEqual(
            atomic_number(m4),
            {"atomic": torch.tensor([6, 6, 6, 6, 6, 6, 17, 35]).unsqueeze(1)},
        )

        self.assertTensorEqual(
            atomic_degrees(m1),
            {"degrees": torch.tensor([2, 2, 2, 2, 2, 2]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_degrees(m2), {"degrees": torch.tensor([1, 1]).unsqueeze(1)}
        )
        self.assertTensorEqual(
            atomic_degrees(m3), {"degrees": torch.tensor([1, 1]).unsqueeze(1)}
        )
        self.assertTensorEqual(
            atomic_degrees(m4),
            {"degrees": torch.tensor([2, 2, 2, 3, 3, 2, 1, 1]).unsqueeze(1)},
        )

        self.assertTensorEqual(
            atomic_isotopes(m1),
            {"isotopes": torch.tensor([0, 0, 0, 0, 0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_isotopes(m2),
            {"isotopes": torch.tensor([2, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_isotopes(m3),
            {"isotopes": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_isotopes(m4),
            {"isotopes": torch.tensor([0, 0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)},
        )

        self.assertTensorEqual(
            atomic_charges(m1),
            {"charges": torch.tensor([0, 0, 0, 0, 0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_charges(m2), {"charges": torch.tensor([0, 0]).unsqueeze(1)}
        )
        self.assertTensorEqual(
            atomic_charges(m3), {"charges": torch.tensor([0, 0]).unsqueeze(1)}
        )
        self.assertTensorEqual(
            atomic_charges(m4),
            {"charges": torch.tensor([0, 0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)},
        )

        self.assertTensorEqual(
            atomic_valences(m1),
            {"valences": torch.tensor([4, 4, 4, 4, 4, 4]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_valences(m2),
            {"valences": torch.tensor([1, 1]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_valences(m3),
            {"valences": torch.tensor([4, 4]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_valences(m4),
            {"valences": torch.tensor([4, 4, 4, 4, 4, 4, 1, 1]).unsqueeze(1)},
        )

        self.assertTensorEqual(
            atomic_radicals(m1),
            {"radicals": torch.tensor([0, 0, 0, 0, 0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_radicals(m2),
            {"radicals": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_radicals(m3),
            {"radicals": torch.tensor([0, 1]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_radicals(m4),
            {"radicals": torch.tensor([0, 0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)},
        )

        self.assertTensorEqual(
            atom_in_ring(m1),
            {
                "in_aromatic_ring": torch.tensor([1, 1, 1, 1, 1, 1]).unsqueeze(
                    1
                )
            },
        )
        self.assertTensorEqual(
            atom_in_ring(m2),
            {"in_aromatic_ring": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atom_in_ring(m3),
            {"in_aromatic_ring": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atom_in_ring(m4),
            {
                "in_aromatic_ring": torch.tensor(
                    [0, 0, 0, 0, 0, 0, 0, 0]
                ).unsqueeze(1)
            },
        )

        self.assertTensorEqual(
            stereocenter_types(m1),
            {
                "stereocenter_types": torch.tensor(
                    [0, 0, 0, 0, 0, 0]
                ).unsqueeze(1)
            },
        )
        self.assertTensorEqual(
            stereocenter_types(m2),
            {"stereocenter_types": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            stereocenter_types(m3),
            {"stereocenter_types": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            stereocenter_types(m4),
            {
                "stereocenter_types": torch.tensor(
                    [0, 0, 0, 1, 1, 0, 0, 0]
                ).unsqueeze(1)
            },
        )

        self.assertTensorEqual(
            implicit_hydrogens(m1),
            {
                "implicit_hydrogens": torch.tensor(
                    [1, 1, 1, 1, 1, 1]
                ).unsqueeze(1)
            },
        )
        self.assertTensorEqual(
            implicit_hydrogens(m2),
            {"implicit_hydrogens": torch.tensor([0, 0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            implicit_hydrogens(m3),
            {"implicit_hydrogens": torch.tensor([3, 2]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            implicit_hydrogens(m4),
            {
                "implicit_hydrogens": torch.tensor(
                    [2, 2, 2, 1, 1, 2, 0, 0]
                ).unsqueeze(1)
            },
        )

        self.assertTensorEqual(
            acid_pka_values(m1),
            {
                "acid_pka_values": torch.tensor(
                    [100.0, 100.0, 100.0, 100.0, 100.0, 100.0]
                ).unsqueeze(1)
            },
        )
        self.assertTensorEqual(
            acid_pka_values(m2),
            {"acid_pka_values": torch.tensor([100.0, 100.0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            acid_pka_values(m3),
            {"acid_pka_values": torch.tensor([100.0, 100.0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            acid_pka_values(m4),
            {
                "acid_pka_values": torch.tensor(
                    [100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0]
                ).unsqueeze(1)
            },
        )

        self.assertTensorEqual(
            basic_pka_values(m1),
            {
                "basic_pka_values": torch.tensor(
                    [-100.0, -100.0, -100.0, -100.0, -100.0, -100.0]
                ).unsqueeze(1)
            },
        )
        self.assertTensorEqual(
            basic_pka_values(m2),
            {"basic_pka_values": torch.tensor([-100.0, -100.0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            basic_pka_values(m3),
            {"basic_pka_values": torch.tensor([-100.0, -100.0]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            basic_pka_values(m4),
            {
                "basic_pka_values": torch.tensor(
                    [
                        -100.0,
                        -100.0,
                        -100.0,
                        -100.0,
                        -100.0,
                        -100.0,
                        -100.0,
                        -100.0,
                    ]
                ).unsqueeze(1)
            },
        )

        self.assertTensorEqual(
            atomic_masses(m1),
            {
                "atomic_masses": torch.tensor(
                    [12.0107, 12.0107, 12.0107, 12.0107, 12.0107, 12.0107]
                ).unsqueeze(1)
            },
        )
        self.assertTensorEqual(
            atomic_masses(m2),
            {"atomic_masses": torch.tensor([2.0141, 1.0079]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_masses(m3),
            {"atomic_masses": torch.tensor([12.0107, 12.0107]).unsqueeze(1)},
        )
        self.assertTensorEqual(
            atomic_masses(m4),
            {
                "atomic_masses": torch.tensor(
                    [
                        12.0107,
                        12.0107,
                        12.0107,
                        12.0107,
                        12.0107,
                        12.0107,
                        35.4530,
                        79.9040,
                    ]
                ).unsqueeze(1)
            },
        )


def test_edge_featurizers(self):
    m1 = self.indigo.loadMolecule("c1ccccc1")
    m2 = self.indigo.loadMolecule("[2H][H]")
    m3 = self.indigo.loadMolecule("C[CH2]")
    m4 = self.indigo.loadMolecule("C1CC[C@H]([C@H](C1)Cl)Br")

    self.assertTensorEqual(
        bond_order(m1),
        {
            "orders": torch.tensor(
                [4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4]
            ).unsqueeze(1)
        },
    )
    self.assertTensorEqual(
        bond_order(m2), {"orders": torch.tensor([1, 1]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        bond_order(m3), {"orders": torch.tensor([1, 1]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        bond_order(m4),
        {
            "orders": torch.tensor(
                [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
            ).unsqueeze(1)
        },
    )

    self.assertTensorEqual(
        topologies(m1),
        {
            "topologies": torch.tensor(
                [10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10]
            ).unsqueeze(1)
        },
    )
    self.assertTensorEqual(
        topologies(m2), {"topologies": torch.tensor([9, 9]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        topologies(m3), {"topologies": torch.tensor([9, 9]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        topologies(m4),
        {
            "topologies": torch.tensor(
                [10, 10, 10, 10, 10, 10, 9, 9, 10, 10, 10, 10, 10, 10, 9, 9]
            ).unsqueeze(1)
        },
    )

    self.assertTensorEqual(
        aromatic_bonds(m1),
        {
            "is_aromatic": torch.tensor(
                [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
            ).unsqueeze(1)
        },
    )
    self.assertTensorEqual(
        aromatic_bonds(m2), {"is_aromatic": torch.tensor([0, 0]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        aromatic_bonds(m3), {"is_aromatic": torch.tensor([0, 0]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        aromatic_bonds(m4),
        {
            "is_aromatic": torch.tensor(
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            ).unsqueeze(1)
        },
    )

    self.assertTensorEqual(
        bond_stereo(m1),
        {
            "bond_stereo": torch.tensor(
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            ).unsqueeze(1)
        },
    )
    self.assertTensorEqual(
        bond_stereo(m2), {"bond_stereo": torch.tensor([0, 0]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        bond_stereo(m3), {"bond_stereo": torch.tensor([0, 0]).unsqueeze(1)}
    )
    self.assertTensorEqual(
        bond_stereo(m4),
        {
            "bond_stereo": torch.tensor(
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            ).unsqueeze(1)
        },
    )
