import torch
from config import LEARNING_RATE, NUM_EPOCH, MPNN_params
from datasets import MolDataset, load_data
from eval import evaluate
from models import MPNNRegressor
from tqdm import trange

dataset = MolDataset()
train_loader, val_loader, test_loader = load_data(dataset)
model = MPNNRegressor(dataset.dim_nfeats, dataset.dim_efeats, **MPNN_params)
optimizer = torch.optim.Adam(model.parameters(), lr=LEARNING_RATE)
loss_fcn = torch.nn.SmoothL1Loss()

for epoch in trange(NUM_EPOCH):
    losses = list()
    for batched_graph, labels in train_loader:

        node_feats = batched_graph.ndata["atomic"].float()
        edge_feats = batched_graph.edata["ord"].float()
        prediction = model(batched_graph, node_feats, edge_feats)
        loss = loss_fcn(prediction, labels)
        losses.append(loss.item())
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

    print("\nEpoch: {}/{}.............".format(epoch, NUM_EPOCH), end=" ")
    print("Loss: {:.4f}".format(loss.mean()))


evaluate(model, test_loader)
