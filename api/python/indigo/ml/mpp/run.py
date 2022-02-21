import click
import config as config
import torch  # type: ignore
from datasets import MolDataset, load_data
from eval import evaluate
from feat_params import FeaturizeParams
from models import MPNNRegressor
from tqdm import trange  # type: ignore


@click.command()
@click.argument("filename", type=click.Path(exists=True))
@click.argument("smiles", type=str)
@click.argument("target", type=str)
@click.option("--node_featurizers", "-n_f", default=(), multiple=True)
@click.option("--edge_featurizers", "-e_f", default=(), multiple=True)
@click.option("--mol_data_features", "-md_f", default=(), multiple=True)
@click.option("--mol_func_features", "-mf_f", default=(), multiple=True)
def main(
    filename: str,
    smiles: str,
    target: str,
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
    """

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
    model = MPNNRegressor(
        dataset.dim_nfeats, dataset.dim_efeats, **config.MPNN_params
    )
    optimizer = torch.optim.Adam(model.parameters(), lr=config.LEARNING_RATE)
    loss_fcn = torch.nn.SmoothL1Loss()

    for epoch in trange(config.NUM_EPOCH):
        losses = list()
        for batched_graph, labels in train_loader:

            node_feats = batched_graph.ndata["n_features"].float()
            edge_feats = batched_graph.edata["e_features"].float()
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
