import click
import config as config
import torch  # type: ignore
from mpp.datasets import MolDataset, load_data
from mpp.eval import evaluate
from mpp.models import MPNNRegressor
from tqdm import trange  # type: ignore


@click.command()
@click.argument("filename", type=click.Path(exists=True))
@click.argument("smiles", type=str)
@click.argument("target", type=str)
def main(filename: str, smiles: str, target: str):
    """Simpel property prediction"""

    config.file_name = filename
    config.smiles = smiles
    config.target = target
    dataset = MolDataset()
    train_loader, val_loader, test_loader = load_data(dataset)
    model = MPNNRegressor(
        dataset.dim_nfeats, dataset.dim_efeats, **config.MPNN_params
    )
    optimizer = torch.optim.Adam(model.parameters(), lr=config.LEARNING_RATE)
    loss_fcn = torch.nn.SmoothL1Loss()

    for epoch in trange(config.NUM_EPOCH):
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

        print(
            "\nEpoch: {}/{}.............".format(epoch, config.NUM_EPOCH),
            end=" ",
        )
        print("Loss: {:.4f}".format(loss.mean()))

    evaluate(model, test_loader)


if __name__ == "__main__":
    main()
