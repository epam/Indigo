import torch.nn as nn  # type: ignore
import torch.nn.functional as F  # type: ignore
from dgl.nn.pytorch import NNConv, Set2Set  # type: ignore

from indigo.ml.mpp.models.regressor import BaseGNNRegressor  # type: ignore


class MPNNGNN(nn.Module):
    """MPNN model"""

    def __init__(
        self,
        node_in_feats,
        edge_in_feats,
        node_out_feats=64,
        edge_hidden_feats=128,
        num_step_message_passing=6,
    ):
        super(MPNNGNN, self).__init__()

        self.project_node_feats = nn.Sequential(
            nn.Linear(node_in_feats, node_out_feats), nn.ReLU()
        )
        self.num_step_message_passing = num_step_message_passing
        edge_network = nn.Sequential(
            nn.Linear(edge_in_feats, edge_hidden_feats),
            nn.ReLU(),
            nn.Linear(edge_hidden_feats, node_out_feats * node_out_feats),
        )
        self.gnn_layer = NNConv(
            in_feats=node_out_feats,
            out_feats=node_out_feats,
            edge_func=edge_network,
            aggregator_type="sum",
        )
        self.gru = nn.GRU(node_out_feats, node_out_feats)

    def forward(self, g, node_feats, edge_feats):
        node_feats = self.project_node_feats(node_feats)
        hidden_feats = node_feats.unsqueeze(0)

        for _ in range(self.num_step_message_passing):
            node_feats = F.relu(self.gnn_layer(g, node_feats, edge_feats))
            node_feats, hidden_feats = self.gru(
                node_feats.unsqueeze(0), hidden_feats
            )
            node_feats = node_feats.squeeze(0)

        return node_feats


class MPNNRegressor(BaseGNNRegressor):
    """Regression model"""

    def __init__(
        self,
        in_node_feats,
        in_edge_feats,
        node_hidden_dim,
        edge_hidden_dim,
        num_step_message_passing,
        num_step_set2set,
        num_layer_set2set,
        n_tasks,
        regressor_hidden_feats=128,
        dropout=0.0,
    ):
        super(MPNNRegressor, self).__init__(
            readout_feats=2 * node_hidden_dim,
            n_tasks=n_tasks,
            regressor_hidden_feats=regressor_hidden_feats,
            dropout=dropout,
        )
        self.gnn = MPNNGNN(
            in_node_feats,
            in_edge_feats,
            node_hidden_dim,
            edge_hidden_dim,
            num_step_message_passing,
        )
        self.readout = Set2Set(
            node_hidden_dim, num_step_set2set, num_layer_set2set
        )
