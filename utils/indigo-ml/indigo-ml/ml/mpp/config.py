"""Config file"""


# Dataset

file_name = ""
target = ""
smiles = ""

# Training parametrs

NUM_EPOCH = 50
LEARNING_RATE = 2e-3

MPNN_params = {
    "node_hidden_dim": 128,
    "edge_hidden_dim": 64,
    "num_step_message_passing": 2,
    "num_step_set2set": 3,
    "num_layer_set2set": 2,
    "regressor_hidden_feats": 32,
    "dropout": 0.2,
    "n_tasks": 1,
}

AttentiveFP_params = {
    "num_layers": 5,
    "gnn_out_feats": 128,
    "num_timesteps": 4,
    "regressor_hidden_feats": 64,
    "dropout": 0.2,
    "n_tasks": 1,
}

GCN_params = {
    "gcn_hidden_feats": [128, 128, 128],
    "regressor_hidden_feats": 64,
    "dropout": 0.0,
    "n_tasks": 1,
}

GAT_params = {
    "gat_hidden_feats": [64, 64],
    "num_heads": [4, 4],
    "regressor_hidden_feats": 128,
    "dropout": 0.0,
    "n_tasks": 1,
}
