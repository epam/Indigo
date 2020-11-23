from pathlib import Path
from typing import Callable

import bingo_elastic.model.helpers as helpers


def test_iterate_sdf(resource_loader: Callable[[str], str]):
    results = []
    for step in range(0, 2):
        if 0 == step:
            sdf = helpers.iterate_sdf(
                resource_loader("resources/rand_queries_small.sdf")
            )
        else:
            sdf = helpers.iterate_file(
                Path(resource_loader("resources/rand_queries_small.sdf"))
            )
        i = 0
        for i, _ in enumerate(sdf, start=1):
            pass
        results.append(i)
    assert results[0] == results[1]


def test_iterate_smiles(resource_loader: Callable[[str], str]):
    results = []
    for step in range(0, 2):
        if 0 == step:
            smiles = helpers.iterate_smiles(
                resource_loader("resources/pubchem_slice_50.smi")
            )
        else:
            smiles = helpers.iterate_file(
                Path(resource_loader("resources/pubchem_slice_50.smi"))
            )
        i = 0
        for i, _ in enumerate(smiles, start=1):
            pass
        results.append(i)
    assert results[0] == results[1]


def test_iterate_cml(resource_loader: Callable[[str], str]):
    results = []
    for step in range(0, 2):
        if 0 == step:
            cml = helpers.iterate_cml(
                resource_loader("resources/tetrahedral-all.cml")
            )
        else:
            cml = helpers.iterate_file(
                Path(resource_loader("resources/tetrahedral-all.cml"))
            )
        i = 0
        for i, _ in enumerate(cml, start=1):
            pass
        results.append(i)
    assert results[0] == results[1]
