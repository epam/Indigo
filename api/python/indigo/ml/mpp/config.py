"""Config file"""


# Dataset

file_name = ""
target = ""
smiles = ""

# Training parametrs

NUM_EPOCH = 20
LEARNING_RATE = 1e-3

MPNN_params = {
    "node_hidden_dim": 64,
    "edge_hidden_dim": 16,
    "num_step_message_passing": 2,
    "num_step_set2set": 3,
    "num_layer_set2set": 2,
    "regressor_hidden_feats": 32,
    "dropout": 0.0,
    "n_tasks": 1,
}
