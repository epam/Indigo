import config
import pandas as pd  # type: ignore
from dgl.data import DGLDataset  # type: ignore
from dgl.data.utils import split_dataset  # type: ignore
from dgl.dataloading import GraphDataLoader  # type: ignore
from mol_to_graph import smiles_to_graph
from torch import FloatTensor  # type: ignore


class MolDataset(DGLDataset):
    """Create dataset object from csv file"""

    def __init__(self):
        super().__init__(name="mols")

    def process(self):
        "Process graph data and set attributes to dataset"
        df = pd.read_csv(config.file_name)
        df = df.loc[df[config.target].notnull()]
        data = dict(zip(df[config.smiles], df[config.target]))
        self.graphs = []
        self.labels = []
        for smiles, label in data.items():
            self.graphs.append(smiles_to_graph(smiles))
            self.labels.append(label)

        self.gclasses = len(self.labels)
        self.dim_nfeats = len(self.graphs[0].ndata["atomic"][0])
        self.dim_efeats = len(self.graphs[0].edata["ord"][0])
        self.labels = FloatTensor(self.labels)

    def __getitem__(self, i):
        return self.graphs[i], self.labels[i]

    def __len__(self):
        return len(self.graphs)


def load_data(dataset):
    """Split dataset to train, test and validation"""
    train_set, val_set, test_set = split_dataset(
        dataset, frac_list=None, shuffle=False, random_state=None
    )
    train_loader = GraphDataLoader(
        dataset=train_set, shuffle=True, drop_last=False
    )
    val_loader = GraphDataLoader(
        dataset=val_set, shuffle=True, drop_last=False
    )
    test_loader = GraphDataLoader(
        dataset=test_set, shuffle=True, drop_last=False
    )
    return train_loader, val_loader, test_loader
