import torch.nn as nn
import torch.nn.functional as F
from dgl.nn.pytorch import GATConv  # type: ignore

from indigo.ml.mpp.models.blocks import (  # type: ignore
    EdgeGraphConv,
    InputInitializer,
    WeightedSumAndMax,
)
from indigo.ml.mpp.models.regressor import BaseGNNRegressor  # type: ignore


class GNN(nn.Module):
    """A GAT variant where we combine the node and edge features in the first layer.
    Parameters
    ----------
    in_node_feats : int
        Number of input node features
    in_edge_feats : int
        Number of input edge features
    gat_hidden_feats : list[int]
        gat_hidden_feats[i] gives the size for hidden representations in each head of
        the i-th attention layer.
    num_heads : list[int]
        num_heads[i] gives the number of attention heads in the i-th attention layer.
    dropout : float
        The probability for dropout. Default to 0, i.e. no dropout is performed.
    """

    def __init__(
        self,
        in_node_feats,
        in_edge_feats,
        gat_hidden_feats,
        num_heads,
        dropout=0.0,
    ):
        super(GNN, self).__init__()

        self.e_repr_initializer = InputInitializer(
            in_node_feats, in_edge_feats
        )
        self.edge_conv = EdgeGraphConv(
            in_node_feats + in_edge_feats, in_node_feats + in_edge_feats
        )
        num_gat_layers = len(gat_hidden_feats)
        self.gat = GAT(
            in_feats=in_node_feats + in_edge_feats,
            hidden_feats=gat_hidden_feats,
            num_heads=num_heads,
            feat_drops=[dropout] * num_gat_layers,
            attn_drops=[dropout] * num_gat_layers,
        )

    def forward(self, bg, node_feats, edge_feats):
        """Update node representations.
        Parameters
        ----------
        bg : DGLGraph
            DGLGraph for a batch of B graphs
        node_feats : FloatTensor of shape (N, D0)
            Initial features for all nodes in the batch of graphs
        edge_feats : FloatTensor of shape (M, D1)
            Initial features for all edges in the batch of graphs
        Returns
        -------
        feats : FloatTensor of shape (N, gat_hidden_feats[-1])
            Updated node representations
        """
        # Initialize edge representations.
        feats = self.e_repr_initializer(bg, node_feats, edge_feats)
        feats = self.edge_conv(bg, feats)

        return self.gat(bg, feats)


class GATRegressor(BaseGNNRegressor):
    """GAT-based model for multitask molecular property prediction.
    We assume all tasks are regression problems.
    Parameters
    ----------
    in_node_feats : int
        Number of input node features
    in_edge_feats : int
        Number of input edge features
    gat_hidden_feats : list[int]
        gat_hidden_feats[i] gives the size for hidden representations in each head of
        the i-th attention layer.
    num_heads : list[int]
        num_heads[i] gives the number of attention heads in the i-th attention layer.
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
        gat_hidden_feats,
        num_heads,
        n_tasks,
        regressor_hidden_feats=128,
        dropout=0.0,
    ):
        super(GATRegressor, self).__init__(
            readout_feats=2 * gat_hidden_feats[-1],
            n_tasks=n_tasks,
            regressor_hidden_feats=regressor_hidden_feats,
            dropout=dropout,
        )

        self.gnn = GNN(
            in_node_feats, in_edge_feats, gat_hidden_feats, num_heads, dropout
        )
        self.readout = WeightedSumAndMax(gat_hidden_feats[-1])


class GATLayer(nn.Module):
    r"""Single GAT layer from `Graph Attention Networks <https://arxiv.org/abs/1710.10903>`__
    Parameters
    ----------
    in_feats : int
        Number of input node features
    out_feats : int
        Number of output node features
    num_heads : int
        Number of attention heads
    feat_drop : float
        Dropout applied to the input features
    attn_drop : float
        Dropout applied to attention values of edges
    alpha : float
        Hyperparameter in LeakyReLU, which is the slope for negative values.
        Default to 0.2.
    residual : bool
        Whether to perform skip connection, default to True.
    agg_mode : str
        The way to aggregate multi-head attention results, can be either
        'flatten' for concatenating all-head results or 'mean' for averaging
        all head results.
    activation : activation function or None
        Activation function applied to the aggregated multi-head results, default to None.
    bias : bool
        Whether to use bias in the GAT layer.
    """

    def __init__(
        self,
        in_feats,
        out_feats,
        num_heads,
        feat_drop,
        attn_drop,
        alpha=0.2,
        residual=True,
        agg_mode="flatten",
        activation=None,
        bias=True,
    ):
        super(GATLayer, self).__init__()

        self.gat_conv = GATConv(
            in_feats=in_feats,
            out_feats=out_feats,
            num_heads=num_heads,
            feat_drop=feat_drop,
            attn_drop=attn_drop,
            negative_slope=alpha,
            residual=residual,
            bias=bias,
        )
        assert agg_mode in ["flatten", "mean"]
        self.agg_mode = agg_mode
        self.activation = activation

    def reset_parameters(self):
        """Reinitialize model parameters."""
        self.gat_conv.reset_parameters()

    def forward(self, bg, feats):
        """Update node representations
        Parameters
        ----------
        bg : DGLGraph
            DGLGraph for a batch of graphs.
        feats : FloatTensor of shape (N, M1)
            * N is the total number of nodes in the batch of graphs
            * M1 is the input node feature size, which equals in_feats in initialization
        Returns
        -------
        feats : FloatTensor of shape (N, M2)
            * N is the total number of nodes in the batch of graphs
            * M2 is the output node representation size, which equals
              out_feats in initialization if self.agg_mode == 'mean' and
              out_feats * num_heads in initialization otherwise.
        """
        feats = self.gat_conv(bg, feats)
        if self.agg_mode == "flatten":
            feats = feats.flatten(1)
        else:
            feats = feats.mean(1)

        if self.activation is not None:
            feats = self.activation(feats)

        return feats


class GAT(nn.Module):
    r"""GAT from `Graph Attention Networks <https://arxiv.org/abs/1710.10903>`__
    Parameters
    ----------
    in_feats : int
        Number of input node features
    hidden_feats : list of int
        ``hidden_feats[i]`` gives the output size of an attention head in the i-th GAT layer.
        ``len(hidden_feats)`` equals the number of GAT layers. By default, we use ``[32, 32]``.
    num_heads : list of int
        ``num_heads[i]`` gives the number of attention heads in the i-th GAT layer.
        ``len(num_heads)`` equals the number of GAT layers. By default, we use 4 attention heads
        for each GAT layer.
    feat_drops : list of float
        ``feat_drops[i]`` gives the dropout applied to the input features in the i-th GAT layer.
        ``len(feat_drops)`` equals the number of GAT layers. By default, this will be zero for
        all GAT layers.
    attn_drops : list of float
        ``attn_drops[i]`` gives the dropout applied to attention values of edges in the i-th GAT
        layer. ``len(attn_drops)`` equals the number of GAT layers. By default, this will be zero
        for all GAT layers.
    alphas : list of float
        Hyperparameters in LeakyReLU, which are the slopes for negative values. ``alphas[i]``
        gives the slope for negative value in the i-th GAT layer. ``len(alphas)`` equals the
        number of GAT layers. By default, this will be 0.2 for all GAT layers.
    residuals : list of bool
        ``residual[i]`` decides if residual connection is to be used for the i-th GAT layer.
        ``len(residual)`` equals the number of GAT layers. By default, residual connection
        is performed for each GAT layer.
    agg_modes : list of str
        The way to aggregate multi-head attention results for each GAT layer, which can be either
        'flatten' for concatenating all-head results or 'mean' for averaging all-head results.
        ``agg_modes[i]`` gives the way to aggregate multi-head attention results for the i-th
        GAT layer. ``len(agg_modes)`` equals the number of GAT layers. By default, we flatten
        all-head results for each GAT layer.
    activations : list of activation function or None
        ``activations[i]`` gives the activation function applied to the aggregated multi-head
        results for the i-th GAT layer. ``len(activations)`` equals the number of GAT layers.
        By default, no activation is applied for each GAT layer.
    biases : list of bool
        ``biases[i]`` gives whether to use bias for the i-th GAT layer. ``len(activations)``
        equals the number of GAT layers. By default, we use bias for all GAT layers.
    """

    def __init__(
        self,
        in_feats,
        hidden_feats=None,
        num_heads=None,
        feat_drops=None,
        attn_drops=None,
        alphas=None,
        residuals=None,
        agg_modes=None,
        activations=None,
        biases=None,
    ):
        super(GAT, self).__init__()

        if hidden_feats is None:
            hidden_feats = [32, 32]

        n_layers = len(hidden_feats)
        if num_heads is None:
            num_heads = [4 for _ in range(n_layers)]
        if feat_drops is None:
            feat_drops = [0.0 for _ in range(n_layers)]
        if attn_drops is None:
            attn_drops = [0.0 for _ in range(n_layers)]
        if alphas is None:
            alphas = [0.2 for _ in range(n_layers)]
        if residuals is None:
            residuals = [True for _ in range(n_layers)]
        if agg_modes is None:
            agg_modes = ["flatten" for _ in range(n_layers - 1)]
            agg_modes.append("mean")
        if activations is None:
            activations = [F.elu for _ in range(n_layers - 1)]
            activations.append(None)
        if biases is None:
            biases = [True for _ in range(n_layers)]
        lengths = [
            len(hidden_feats),
            len(num_heads),
            len(feat_drops),
            len(attn_drops),
            len(alphas),
            len(residuals),
            len(agg_modes),
            len(activations),
            len(biases),
        ]
        assert len(set(lengths)) == 1, (
            "Expect the lengths of hidden_feats, num_heads, "
            "feat_drops, attn_drops, alphas, residuals, "
            "agg_modes, activations, and biases to be the same, "
            "got {}".format(lengths)
        )
        self.hidden_feats = hidden_feats
        self.num_heads = num_heads
        self.agg_modes = agg_modes
        self.gnn_layers = nn.ModuleList()
        for i in range(n_layers):
            self.gnn_layers.append(
                GATLayer(
                    in_feats,
                    hidden_feats[i],
                    num_heads[i],
                    feat_drops[i],
                    attn_drops[i],
                    alphas[i],
                    residuals[i],
                    agg_modes[i],
                    activations[i],
                    biases[i],
                )
            )
            if agg_modes[i] == "flatten":
                in_feats = hidden_feats[i] * num_heads[i]
            else:
                in_feats = hidden_feats[i]

    def reset_parameters(self):
        """Reinitialize model parameters."""
        for gnn in self.gnn_layers:
            gnn.reset_parameters()

    def forward(self, g, feats):
        """Update node representations.
        Parameters
        ----------
        g : DGLGraph
            DGLGraph for a batch of graphs
        feats : FloatTensor of shape (N, M1)
            * N is the total number of nodes in the batch of graphs
            * M1 is the input node feature size, which equals in_feats in initialization
        Returns
        -------
        feats : FloatTensor of shape (N, M2)
            * N is the total number of nodes in the batch of graphs
            * M2 is the output node representation size, which equals
              hidden_sizes[-1] if agg_modes[-1] == 'mean' and
              hidden_sizes[-1] * num_heads[-1] otherwise.
        """
        for gnn in self.gnn_layers:
            feats = gnn(g, feats)
        return feats
