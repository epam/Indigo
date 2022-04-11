from dataclasses import dataclass
from typing import Collection


@dataclass(frozen=True)
class FeaturizeParams:
    node_featurizers: Collection[str] = ()
    edge_featurizers: Collection[str] = ()
    mol_data_features: Collection[str] = ()
    mol_func_features: Collection[str] = ()
