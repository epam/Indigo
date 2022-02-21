import yaml

import indigo.ml.mpp.config as config  # type: ignore
from indigo.ml.mpp.feat_params import FeaturizeParams
from indigo.ml.mpp.models.attentivefp import (  # type: ignore
    AttentiveFPRegressor,
)
from indigo.ml.mpp.models.gat import GATRegressor  # type: ignore
from indigo.ml.mpp.models.gcn import GCNRegressor  # type: ignore
from indigo.ml.mpp.models.mpnn import MPNNRegressor  # type: ignore


def load_model(model_type: str):
    if model_type == "GCN":
        model_class = GCNRegressor
        params = config.GCN_params
    elif model_type == "GAT":
        model_class = GATRegressor
        params = config.GAT_params
    elif model_type == "AttentiveFP":
        model_class = AttentiveFPRegressor
        params = config.AttentiveFP_params
    elif model_type == "MPNN":
        model_class = MPNNRegressor
        params = config.MPNN_params
    else:
        raise ValueError("Unknown model type")

    return model_class, params


def load_params(file_name: str) -> FeaturizeParams:
    with open(file_name, "r") as f:
        data = yaml.safe_load(f)
        return FeaturizeParams(**data)
