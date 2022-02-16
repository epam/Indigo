import indigo.ml.mpp.config as config  # type: ignore
from indigo.ml.mpp.models import (  # type: ignore
    AttentiveFPRegressor,
    GATRegressor,
    GCNRegressor,
    MPNNRegressor,
)


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
