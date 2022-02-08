import torch
from tests import TestIndigoBase

from indigo.ml.clustering import (
    atomic_number,
    atomic_degrees,
    atomic_isotopes,
    atomic_charges,
    atomic_valences,
    atomic_radicals,
    atom_in_ring,
    stereocenter_types,
    implicit_hydrogens,
    bond_order,
    topologies,
    aromatic_bonds,
    bond_stereo
)


class TestIndigoFeaturizers(TestIndigoBase):
    def test_node_featurizers(self):
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("[2H][H]")
        m3 = self.indigo.loadMolecule("C[CH2]")
        m4 = self.indigo.loadMolecule("C1CC[C@H]([C@H](C1)Cl)Br")

        self.assertEqual(atomic_number(m1), {"atomic": torch.tensor([6, 6, 6, 6, 6, 6]).unsqueeze(1)})
        self.assertEqual(atomic_number(m2), {"atomic": torch.tensor([1, 1]).unsqueeze(1)})
        self.assertEqual(atomic_number(m3), {"atomic": torch.tensor([6, 6]).unsqueeze(1)})
        self.assertEqual(atomic_number(m4), {"atomic": torch.tensor([6, 6, 6, 6, 6, 6, 17, 35]).unsqueeze(1)})

        self.assertEqual(atomic_degrees(m1), {"degrees": torch.tensor([2, 2, 2, 2, 2, 2]).unsqueeze(1)})
        self.assertEqual(atomic_degrees(m2), {"degrees": torch.tensor([1, 1]).unsqueeze(1)})
        self.assertEqual(atomic_degrees(m3), {"degrees": torch.tensor([1, 1]).unsqueeze(1)})
        self.assertEqual(atomic_degrees(m4), {"degrees": torch.tensor([2, 2, 2, 3, 3, 2, 1, 1]).unsqueeze(1)})

        self.assertEqual(atomic_isotopes(m1), {"isotopes": torch.tensor([0, 0, 0, 0, 0, 0]).unsqueeze(1)})
        self.assertEqual(atomic_isotopes(m2), {"isotopes": torch.tensor([2, 0]).unsqueeze(1)})
        self.assertEqual(atomic_isotopes(m3), {"isotopes": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(atomic_isotopes(m4), {"isotopes": torch.tensor([0, 0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)})

        self.assertEqual(atomic_charges(m1), {"charges": torch.tensor([6, 6, 6, 6, 6, 6]).unsqueeze(1)})
        self.assertEqual(atomic_charges(m2), {"charges": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(atomic_charges(m3), {"charges": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(atomic_charges(m4), {"charges": torch.tensor([0, 0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)})

        self.assertEqual(atomic_valences(m1), {"valences": torch.tensor([4, 4, 4, 4, 4, 4]).unsqueeze(1)})
        self.assertEqual(atomic_valences(m2), {"valences": torch.tensor([1, 1]).unsqueeze(1)})
        self.assertEqual(atomic_valences(m3), {"valences": torch.tensor([4, 4]).unsqueeze(1)})
        self.assertEqual(atomic_valences(m4), {"valences": torch.tensor([4, 4, 4, 4, 4, 4, 1, 1]).unsqueeze(1)})

        self.assertEqual(atomic_radicals(m1), {"radicals": torch.tensor([0, 0, 0, 0, 0, 0]).unsqueeze(1)})
        self.assertEqual(atomic_radicals(m2), {"radicals": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(atomic_radicals(m3), {"radicals": torch.tensor([0, 1]).unsqueeze(1)})
        self.assertEqual(atomic_radicals(m4), {"radicals": torch.tensor([0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)})

        self.assertEqual(atom_in_ring(m1), {"in_aromatic_ring": torch.tensor([1, 1, 1, 1, 1, 1]).unsqueeze(1)})
        self.assertEqual(atom_in_ring(m2), {"in_aromatic_ring": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(atom_in_ring(m3), {"in_aromatic_ring": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(atom_in_ring(m4), {"in_aromatic_ring": torch.tensor([6, 6, 6, 6, 6, 6]).unsqueeze(1)})

        self.assertEqual(stereocenter_types(m1), {"stereocenter_types": torch.tensor([0, 0, 0, 0, 0, 0]).unsqueeze(1)})
        self.assertEqual(stereocenter_types(m2), {"stereocenter_types": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(stereocenter_types(m3), {"stereocenter_types": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(stereocenter_types(m4), {"stereocenter_types": torch.tensor([0, 0, 0, 0, 0, 0, 0, 0]).unsqueeze(1)})


        self.assertEqual(implicit_hydrogens(m1), {"implicit_hydrogens": torch.tensor([1, 1, 1, 1, 1, 1]).unsqueeze(1)})
        self.assertEqual(implicit_hydrogens(m2), {"implicit_hydrogens": torch.tensor([0, 0]).unsqueeze(1)})
        self.assertEqual(implicit_hydrogens(m3), {"implicit_hydrogens": torch.tensor([3, 2]).unsqueeze(1)})
        self.assertEqual(implicit_hydrogens(m4), {"implicit_hydrogens": torch.tensor([2, 2, 2, 1, 1, 2, 0,]).unsqueeze(1)})



