import click
import torch  # type: ignore
from datasets import MolDataset, load_data
from torch.optim.lr_scheduler import (
    ChainedScheduler,
    ConstantLR,
    ExponentialLR,
)
from tqdm import trange  # type: ignore

import indigo.ml.mpp.config as config  # type: ignore
from indigo.ml.mpp.eval import evaluate  # type: ignore
from indigo.ml.mpp.feat_params import FeaturizeParams  # type: ignore
from indigo.ml.mpp.utils import load_model  # type: ignore


@click.command()
@click.argument("filename", type=click.Path(exists=True))
@click.argument("smiles", type=str)
@click.argument("target", type=str)
@click.option("--model_type", default="MPNN", type=str)
@click.option("--node_featurizers", "-n_f", default=(), multiple=True)
@click.option("--edge_featurizers", "-e_f", default=(), multiple=True)
@click.option("--mol_data_features", "-md_f", default=(), multiple=True)
@click.option("--mol_func_features", "-mf_f", default=(), multiple=True)
def main(
    filename: str,
    smiles: str,
    target: str,
    model_type: str,
    node_featurizers: str,
    edge_featurizers: str,
    mol_data_features: str,
    mol_func_features: str,
):
    """Simple property prediction

    Example:
        python run.py Adrenergic_dataset.csv Structure logP -n_f atomic_number -n_f atomic_degrees -md_f Flex -md_f AromaticRings

        this will result loading Adrenergic_dataset.csv, where smiles are at Structure column and value we want to
        predict at LogP column. We will use 2 node features (atomic_number and atomic_degrees) and also load
        whole molecule features from dataset (Flex and Aromatic Rings)
    """  # TODO: pass as string, with values separated as commas, then split it.
    config.file_name = filename
    config.smiles = smiles
    config.target = target
    params = FeaturizeParams(
        node_featurizers=node_featurizers,
        edge_featurizers=edge_featurizers,
        mol_data_features=mol_data_features,
        mol_func_features=mol_func_features,
    )
    dataset = MolDataset(params)
    train_loader, val_loader, test_loader = load_data(dataset)
    model_constructor, model_params = load_model(model_type)
    model = model_constructor(
        dataset.dim_nfeats, dataset.dim_efeats, **model_params
    )
    optimizer = torch.optim.Adam(model.parameters(), lr=config.LEARNING_RATE)
    scheduler1 = ConstantLR(optimizer, factor=0.1, total_iters=10)
    scheduler2 = ExponentialLR(optimizer, gamma=0.9)
    scheduler = ChainedScheduler([scheduler1, scheduler2])
    loss_fcn = torch.nn.HuberLoss()

    for epoch in trange(config.NUM_EPOCH):
        losses = list()
        for batched_graph, labels in train_loader:
            optimizer.zero_grad()
            node_feats = batched_graph.ndata["n_features"].float()
            edge_feats = batched_graph.edata["e_features"].float()
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
