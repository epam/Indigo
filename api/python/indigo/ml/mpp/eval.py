import torch  # type: ignore
from sklearn.metrics import mean_absolute_error, r2_score  # type: ignore


def evaluate(model, loader):
    preds = []
    labels = []
    model.eval()

    with torch.no_grad():
        for batched_graph, label in loader:

            node_feats = batched_graph.ndata["atomic"].float()
            edge_feats = batched_graph.edata["ord"].float()
            prediction = model(batched_graph, node_feats, edge_feats)
            preds.extend(prediction.flatten().tolist())
            labels.extend(label.tolist())

        print(f"R2 score: {r2_score(labels, preds):.2f}")
        print(f"MAE: {mean_absolute_error(labels, preds):.2f}")

    model.train()
