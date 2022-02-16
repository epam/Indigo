import dgl  # type: ignore
import dgl.function as fn  # type: ignore
import torch  # type: ignore
import torch.nn as nn  # type: ignore
import torch.nn.functional as F  # type: ignore
from dgl.nn.pytorch import edge_softmax  # type: ignore

from indigo.ml.mpp.models.regressor import BaseGNNRegressor  # type: ignore


class GlobalPool(nn.Module):
    """One-step readout in AttentiveFP"""

    def __init__(self, feat_size, dropout):
        super(GlobalPool, self).__init__()

        self.compute_logits = nn.Sequential(
            nn.Linear(2 * feat_size, 1), nn.LeakyReLU()
        )
        self.project_nodes = nn.Sequential(
            nn.Dropout(dropout), nn.Linear(feat_size, feat_size)
        )
        self.gru = nn.GRUCell(feat_size, feat_size)

    def forward(self, g, node_feats, g_feats, get_node_weight=False):
        """Perform one-step readout

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Input node features. V for the number of nodes.
        g_feats : float32 tensor of shape (G, graph_feat_size)
            Input graph features. G for the number of graphs.
        get_node_weight : bool
            Whether to get the weights of atoms during readout.

        Returns
        -------
        float32 tensor of shape (G, graph_feat_size)
            Updated graph features.
        float32 tensor of shape (V, 1)
            The weights of nodes in readout.
        """
        with g.local_scope():
            g.ndata["z"] = self.compute_logits(
                torch.cat(
                    [dgl.broadcast_nodes(g, F.relu(g_feats)), node_feats],
                    dim=1,
                )
            )
            g.ndata["a"] = dgl.softmax_nodes(g, "z")
            g.ndata["hv"] = self.project_nodes(node_feats)

            g_repr = dgl.sum_nodes(g, "hv", "a")
            context = F.elu(g_repr)

            if get_node_weight:
                return self.gru(context, g_feats), g.ndata["a"]
            else:
                return self.gru(context, g_feats)


class AttentiveFPReadout(nn.Module):
    """Readout in AttentiveFP

    AttentiveFP is introduced in `Pushing the Boundaries of Molecular Representation for
    Drug Discovery with the Graph Attention Mechanism
    <https://www.ncbi.nlm.nih.gov/pubmed/31408336>`__

    This class computes graph representations out of node features.

    Parameters
    ----------
    feat_size : int
        Size for the input node features, graph features and output graph
        representations.
    num_timesteps : int
        Times of updating the graph representations with GRU. Default to 2.
    dropout : float
        The probability for performing dropout. Default to 0.
    """

    def __init__(self, feat_size, num_timesteps=2, dropout=0.0):
        super(AttentiveFPReadout, self).__init__()

        self.readouts = nn.ModuleList()
        for _ in range(num_timesteps):
            self.readouts.append(GlobalPool(feat_size, dropout))

    def forward(self, g, node_feats, get_node_weight=False):
        """Computes graph representations out of node features.

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Input node features. V for the number of nodes.
        get_node_weight : bool
            Whether to get the weights of nodes in readout. Default to False.

        Returns
        -------
        g_feats : float32 tensor of shape (G, graph_feat_size)
            Graph representations computed. G for the number of graphs.
        node_weights : list of float32 tensor of shape (V, 1), optional
            This is returned when ``get_node_weight`` is ``True``.
            The list has a length ``num_timesteps`` and ``node_weights[i]``
            gives the node weights in the i-th update.
        """
        with g.local_scope():
            g.ndata["hv"] = node_feats
            g_feats = dgl.sum_nodes(g, "hv")

        if get_node_weight:
            node_weights = []

        for readout in self.readouts:
            if get_node_weight:
                g_feats, node_weights_t = readout(
                    g, node_feats, g_feats, get_node_weight
                )
                node_weights.append(node_weights_t)
            else:
                g_feats = readout(g, node_feats, g_feats)

        if get_node_weight:
            return g_feats, node_weights
        else:
            return g_feats


class AttentiveGRU1(nn.Module):
    """Update node features with attention and GRU.

    This will be used for incorporating the information of edge features
    into node features for message passing.

    Parameters
    ----------
    node_feat_size : int
        Size for the input node features.
    edge_feat_size : int
        Size for the input edge (bond) features.
    edge_hidden_size : int
        Size for the intermediate edge (bond) representations.
    dropout : float
        The probability for performing dropout.
    """

    def __init__(
        self, node_feat_size, edge_feat_size, edge_hidden_size, dropout
    ):
        super(AttentiveGRU1, self).__init__()

        self.edge_transform = nn.Sequential(
            nn.Dropout(dropout), nn.Linear(edge_feat_size, edge_hidden_size)
        )
        self.gru = nn.GRUCell(edge_hidden_size, node_feat_size)

    def reset_parameters(self):
        """Reinitialize model parameters."""
        self.edge_transform[1].reset_parameters()
        self.gru.reset_parameters()

    def forward(self, g, edge_logits, edge_feats, node_feats):
        """Update node representations.

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs
        edge_logits : float32 tensor of shape (E, 1)
            The edge logits based on which softmax will be performed for weighting
            edges within 1-hop neighborhoods. E represents the number of edges.
        edge_feats : float32 tensor of shape (E, edge_feat_size)
            Previous edge features.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Previous node features. V represents the number of nodes.

        Returns
        -------
        float32 tensor of shape (V, node_feat_size)
            Updated node features.
        """
        g = g.local_var()
        g.edata["e"] = edge_softmax(g, edge_logits) * self.edge_transform(
            edge_feats
        )
        g.update_all(fn.copy_edge("e", "m"), fn.sum("m", "c"))
        context = F.elu(g.ndata["c"])
        return F.relu(self.gru(context, node_feats))


class AttentiveGRU2(nn.Module):
    """Update node features with attention and GRU.

    This will be used in GNN layers for updating node representations.

    Parameters
    ----------
    node_feat_size : int
        Size for the input node features.
    edge_hidden_size : int
        Size for the intermediate edge (bond) representations.
    dropout : float
        The probability for performing dropout.
    """

    def __init__(self, node_feat_size, edge_hidden_size, dropout):
        super(AttentiveGRU2, self).__init__()

        self.project_node = nn.Sequential(
            nn.Dropout(dropout), nn.Linear(node_feat_size, edge_hidden_size)
        )
        self.gru = nn.GRUCell(edge_hidden_size, node_feat_size)

    def reset_parameters(self):
        """Reinitialize model parameters."""
        self.project_node[1].reset_parameters()
        self.gru.reset_parameters()

    def forward(self, g, edge_logits, node_feats):
        """Update node representations.

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs
        edge_logits : float32 tensor of shape (E, 1)
            The edge logits based on which softmax will be performed for weighting
            edges within 1-hop neighborhoods. E represents the number of edges.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Previous node features. V represents the number of nodes.

        Returns
        -------
        float32 tensor of shape (V, node_feat_size)
            Updated node features.
        """
        g = g.local_var()
        g.edata["a"] = edge_softmax(g, edge_logits)
        g.ndata["hv"] = self.project_node(node_feats)

        g.update_all(fn.src_mul_edge("hv", "a", "m"), fn.sum("m", "c"))
        context = F.elu(g.ndata["c"])
        return F.relu(self.gru(context, node_feats))


class GetContext(nn.Module):
    """Generate context for each node by message passing at the beginning.

    This layer incorporates the information of edge features into node
    representations so that message passing needs to be only performed over
    node representations.

    Parameters
    ----------
    node_feat_size : int
        Size for the input node features.
    edge_feat_size : int
        Size for the input edge (bond) features.
    graph_feat_size : int
        Size of the learned graph representation (molecular fingerprint).
    dropout : float
        The probability for performing dropout.
    """

    def __init__(
        self, node_feat_size, edge_feat_size, graph_feat_size, dropout
    ):
        super(GetContext, self).__init__()

        self.project_node = nn.Sequential(
            nn.Linear(node_feat_size, graph_feat_size), nn.LeakyReLU()
        )
        self.project_edge1 = nn.Sequential(
            nn.Linear(node_feat_size + edge_feat_size, graph_feat_size),
            nn.LeakyReLU(),
        )
        self.project_edge2 = nn.Sequential(
            nn.Dropout(dropout),
            nn.Linear(2 * graph_feat_size, 1),
            nn.LeakyReLU(),
        )
        self.attentive_gru = AttentiveGRU1(
            graph_feat_size, graph_feat_size, graph_feat_size, dropout
        )

    def reset_parameters(self):
        """Reinitialize model parameters."""
        self.project_node[0].reset_parameters()
        self.project_edge1[0].reset_parameters()
        self.project_edge2[1].reset_parameters()
        self.attentive_gru.reset_parameters()

    def apply_edges1(self, edges):
        """Edge feature update.

        Parameters
        ----------
        edges : EdgeBatch
            Container for a batch of edges

        Returns
        -------
        dict
            Mapping ``'he1'`` to updated edge features.
        """
        return {"he1": torch.cat([edges.src["hv"], edges.data["he"]], dim=1)}

    def apply_edges2(self, edges):
        """Edge feature update.

        Parameters
        ----------
        edges : EdgeBatch
            Container for a batch of edges

        Returns
        -------
        dict
            Mapping ``'he2'`` to updated edge features.
        """
        return {
            "he2": torch.cat([edges.dst["hv_new"], edges.data["he1"]], dim=1)
        }

    def forward(self, g, node_feats, edge_feats):
        """Incorporate edge features and update node representations.

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Input node features. V for the number of nodes.
        edge_feats : float32 tensor of shape (E, edge_feat_size)
            Input edge features. E for the number of edges.

        Returns
        -------
        float32 tensor of shape (V, graph_feat_size)
            Updated node features.
        """
        g = g.local_var()
        g.ndata["hv"] = node_feats
        g.ndata["hv_new"] = self.project_node(node_feats)
        g.edata["he"] = edge_feats

        g.apply_edges(self.apply_edges1)
        g.edata["he1"] = self.project_edge1(g.edata["he1"])
        g.apply_edges(self.apply_edges2)
        logits = self.project_edge2(g.edata["he2"])

        return self.attentive_gru(g, logits, g.edata["he1"], g.ndata["hv_new"])


class GNNLayer(nn.Module):
    """GNNLayer for updating node features.

    This layer performs message passing over node representations and update them.

    Parameters
    ----------
    node_feat_size : int
        Size for the input node features.
    graph_feat_size : int
        Size for the graph representations to be computed.
    dropout : float
        The probability for performing dropout.
    """

    def __init__(self, node_feat_size, graph_feat_size, dropout):
        super(GNNLayer, self).__init__()

        self.project_edge = nn.Sequential(
            nn.Dropout(dropout),
            nn.Linear(2 * node_feat_size, 1),
            nn.LeakyReLU(),
        )
        self.attentive_gru = AttentiveGRU2(
            node_feat_size, graph_feat_size, dropout
        )

    def reset_parameters(self):
        """Reinitialize model parameters."""
        self.project_edge[1].reset_parameters()
        self.attentive_gru.reset_parameters()

    def apply_edges(self, edges):
        """Edge feature generation.

        Generate edge features by concatenating the features of the destination
        and source nodes.

        Parameters
        ----------
        edges : EdgeBatch
            Container for a batch of edges.

        Returns
        -------
        dict
            Mapping ``'he'`` to the generated edge features.
        """
        return {"he": torch.cat([edges.dst["hv"], edges.src["hv"]], dim=1)}

    def forward(self, g, node_feats):
        """Perform message passing and update node representations.

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Input node features. V for the number of nodes.

        Returns
        -------
        float32 tensor of shape (V, graph_feat_size)
            Updated node features.
        """
        g = g.local_var()
        g.ndata["hv"] = node_feats
        g.apply_edges(self.apply_edges)
        logits = self.project_edge(g.edata["he"])

        return self.attentive_gru(g, logits, node_feats)


class AttentiveFPGNN(nn.Module):
    """`Pushing the Boundaries of Molecular Representation for Drug Discovery with the Graph
    Attention Mechanism <https://www.ncbi.nlm.nih.gov/pubmed/31408336>`__

    This class performs message passing in AttentiveFP and returns the updated node representations.

    Parameters
    ----------
    node_feat_size : int
        Size for the input node features.
    edge_feat_size : int
        Size for the input edge features.
    num_layers : int
        Number of GNN layers. Default to 2.
    graph_feat_size : int
        Size for the graph representations to be computed. Default to 200.
    dropout : float
        The probability for performing dropout. Default to 0.
    """

    def __init__(
        self,
        node_feat_size,
        edge_feat_size,
        num_layers=2,
        graph_feat_size=200,
        dropout=0.0,
    ):
        super(AttentiveFPGNN, self).__init__()

        self.init_context = GetContext(
            node_feat_size, edge_feat_size, graph_feat_size, dropout
        )
        self.gnn_layers = nn.ModuleList()
        for _ in range(num_layers - 1):
            self.gnn_layers.append(
                GNNLayer(graph_feat_size, graph_feat_size, dropout)
            )

    def reset_parameters(self):
        """Reinitialize model parameters."""
        self.init_context.reset_parameters()
        for gnn in self.gnn_layers:
            gnn.reset_parameters()

    def forward(self, g, node_feats, edge_feats):
        """Performs message passing and updates node representations.

        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs.
        node_feats : float32 tensor of shape (V, node_feat_size)
            Input node features. V for the number of nodes.
        edge_feats : float32 tensor of shape (E, edge_feat_size)
            Input edge features. E for the number of edges.

        Returns
        -------
        node_feats : float32 tensor of shape (V, graph_feat_size)
            Updated node representations.
        """
        node_feats = self.init_context(g, node_feats, edge_feats)
        for gnn in self.gnn_layers:
            node_feats = gnn(g, node_feats)
        return node_feats


class AttentiveFPRegressor(BaseGNNRegressor):
    """AttentiveFP-based model for multitask molecular property prediction.
    We assume all tasks are regression problems.
    Parameters
    ----------
    in_node_feats : int
        Number of input node features
    in_edge_feats : int
        Number of input edge features
    gnn_out_feats : int
        The GNN output size
    num_layers : int
        Number of GNN layers
    num_timesteps : int
        Number of timesteps for updating molecular representations with GRU during readout
    n_tasks : int
        Number of prediction tasks
    regressor_hidden_feats : int
        Hidden size in MLP regressor
    dropout : float
        The probability for dropout. Default to 0, i.e. no dropout is performed.
    """

    def __init__(
        self,
        in_node_feats,
        in_edge_feats,
        gnn_out_feats,
        num_layers,
        num_timesteps,
        n_tasks,
        regressor_hidden_feats=128,
        dropout=0.0,
    ):
        super(AttentiveFPRegressor, self).__init__(
            readout_feats=gnn_out_feats,
            n_tasks=n_tasks,
            regressor_hidden_feats=regressor_hidden_feats,
            dropout=dropout,
        )
        self.gnn = AttentiveFPGNN(
            in_node_feats, in_edge_feats, num_layers, gnn_out_feats, dropout
        )
        self.readout = AttentiveFPReadout(
            gnn_out_feats, num_timesteps, dropout
        )
