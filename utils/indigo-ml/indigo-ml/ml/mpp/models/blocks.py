import dgl  # type: ignore
import dgl.function as fn  # type: ignore
import torch
import torch.nn as nn
import torch.nn.functional as F
from dgl.nn.pytorch import WeightAndSum  # type: ignore


class WeightedSumAndMax(nn.Module):
    r"""Apply weighted sum and max pooling to the node
    representations and concatenate the results.
    Parameters
    ----------
    in_feats : int
        Input node feature size
    """

    def __init__(self, in_feats):
        super(WeightedSumAndMax, self).__init__()

        self.weight_and_sum = WeightAndSum(in_feats)

    def forward(self, bg, feats):
        """Readout
        Parameters
        ----------
        bg : DGLGraph
            DGLGraph for a batch of graphs.
        feats : FloatTensor of shape (N, M1)
            * N is the total number of nodes in the batch of graphs
            * M1 is the input node feature size, which must match
              in_feats in initialization
        Returns
        -------
        h_g : FloatTensor of shape (B, 2 * M1)
            * B is the number of graphs in the batch
            * M1 is the input node feature size, which must match
              in_feats in initialization
        """
        h_g_sum = self.weight_and_sum(bg, feats)
        with bg.local_scope():
            bg.ndata["h"] = feats
            h_g_max = dgl.max_nodes(bg, "h")
        h_g = torch.cat([h_g_sum, h_g_max], dim=1)
        return h_g


class InputInitializer(nn.Module):
    """Initializde edge representations based on input node and edge features
    Parameters
    ----------
    in_node_feats : int
        Number of input node features
    in_edge_feats : int
        Number of input edge features
    """

    def __init__(self, in_node_feats, in_edge_feats):
        super(InputInitializer, self).__init__()

        self.project_nodes = nn.Linear(in_node_feats, in_node_feats)
        self.project_edges = nn.Linear(in_edge_feats, in_edge_feats)

    def forward(self, bg, node_feats, edge_feats):
        """Initialize input representations.
        Project the node/edge features and then concatenate the edge representations with the
        representations of their source nodes.
        """
        node_feats = self.project_nodes(node_feats)
        edge_feats = self.project_edges(edge_feats)

        bg = bg.local_var()
        bg.ndata["hv"] = node_feats
        bg.apply_edges(fn.copy_u("hv", "he"))
        return torch.cat([bg.edata["he"], edge_feats], dim=1)


class EdgeGraphConv(nn.Module):
    """Apply graph convolution over an input edge signal.
    Parameters
    ----------
    in_feats : int
        Input feature size.
    out_feats : int
        Output feature size.
    activation : callable activation function/layer or None, optional
        If not None, applies an activation function to the updated node features.
    """

    def __init__(self, in_feats, out_feats, activation=F.relu):
        super(EdgeGraphConv, self).__init__()
        self.in_feats = in_feats
        self.out_feats = out_feats
        self.linear = nn.Linear(in_feats, out_feats)
        self.activation = activation

    def forward(self, graph, feat):
        """Compute graph convolution.
        Parameters
        ----------
        graph : DGLGraph
            The graph.
        feat : torch.Tensor
            The input edge features.
        Returns
        -------
        torch.Tensor
            The output features.
        """
        graph = graph.local_var()

        if self.in_feats > self.out_feats:
            # multiply by W first to reduce the feature size for aggregation.
            feat = self.linear(feat)
            graph.edata["h"] = feat
            graph.update_all(fn.copy_e("h", "m"), fn.sum("m", "h"))
            rst = graph.ndata["h"]
        else:
            # aggregate first then multiply by W
            graph.edata["h"] = feat
            graph.update_all(fn.copy_e("h", "m"), fn.sum("m", "h"))
            rst = graph.ndata["h"]
            rst = self.linear(rst)

        if self.activation is not None:
            rst = self.activation(rst)

        return rst
