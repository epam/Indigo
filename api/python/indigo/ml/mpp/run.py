import click
import config as config
import torch  # type: ignore
from datasets import MolDataset, load_data
from eval import evaluate
from torch.optim.lr_scheduler import (
    ChainedScheduler,
    ConstantLR,
    ExponentialLR,
)
from tqdm import trange  # type: ignore
from utils import load_model  # type: ignore


@click.command()
@click.argument("filename", type=click.Path(exists=True))
@click.argument("smiles", type=str)
@click.argument("target", type=str)
@click.argument("model_type", type=str)
def main(filename: str, smiles: str, target: str, model_type: str):
    """Simple property prediction"""

    config.file_name = filename
    config.smiles = smiles
    config.target = target
    dataset = MolDataset()
    train_loader, val_loader, test_loader = load_data(dataset)
    model_constructor, params = load_model(model_type)
    model = model_constructor(dataset.dim_nfeats, dataset.dim_efeats, **params)
    optimizer = torch.optim.Adam(model.parameters(), lr=config.LEARNING_RATE)
    scheduler1 = ConstantLR(optimizer, factor=0.1, total_iters=10)
    scheduler2 = ExponentialLR(optimizer, gamma=0.9)
    scheduler = ChainedScheduler([scheduler1, scheduler2])
    loss_fcn = torch.nn.HuberLoss()

    for epoch in trange(config.NUM_EPOCH):
        losses = list()
        for batched_graph, labels in train_loader:
            optimizer.zero_grad()
            node_feats = batched_graph.ndata["atomic"].float()
            edge_feats = batched_graph.edata["ord"].float()
            prediction = model(batched_graph, node_feats, edge_feats)
            loss = loss_fcn(prediction, labels)
            losses.append(loss.item())
            loss.backward()
            optimizer.step()
        scheduler.step()

        print(
            "\nEpoch: {}/{}.............".format(epoch, config.NUM_EPOCH),
            end=" ",
        )
        print("Loss: {:.4f}".format(sum(losses) / len(losses)))

    evaluate(model, test_loader)


if __name__ == "__main__":
    main()
