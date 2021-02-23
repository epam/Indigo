from typing import Callable

import pytest

from bingo_elastic.model import record


def test_empty_create(indigo_fixture):
    mol = indigo_fixture.createMolecule()
    with pytest.raises(ValueError):
        record.IndigoRecord(indigo_object=mol)


def test_create(indigo_fixture):
    mol = indigo_fixture.loadMolecule("N1(CC)C2=C(C(=NC=N2)N)N=C1")
    indigo_record = record.IndigoRecord(indigo_object=mol)
    assert len(indigo_record.sim_fingerprint) == 70
    assert len(indigo_record.sub_fingerprint) == 644
    assert len(indigo_record.cmf) == 140


def test_create_with_name(
    indigo_fixture, resource_loader: Callable[[str], str]
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("resources/composition1.mol")
    )
    indigo_record = record.IndigoRecord(indigo_object=mol)
    assert indigo_record.name == "Composition1"
