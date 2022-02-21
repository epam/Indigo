import pandas as pd  # type: ignore
from dgl.data import DGLDataset  # type: ignore
from dgl.data.utils import split_dataset  # type: ignore
from dgl.dataloading import GraphDataLoader  # type: ignore
from torch import FloatTensor  # type: ignore

import indigo.ml.mpp.config as config  # type: ignore
from indigo.ml.mpp.feat_params import FeaturizeParams # type: ignore
from indigo.ml.mpp.preprocess import mol_to_graph  # type: ignore


class MolDataset(DGLDataset):
    """Create dataset object from csv file"""

    def __init__(self, params: FeaturizeParams = None):
        if not params:
            params = FeaturizeParams()
        self.params = params
        super().__init__(name="mols")

    def process(self):
        "Process graph data and set attributes to dataset"
        self.build_graphs()
        self.gclasses = len(self.labels)
        self.dim_nfeats = len(self.graphs[0].ndata["n_features"][0])
        self.dim_efeats = len(self.graphs[0].edata["e_features"][0])
        self.labels = FloatTensor(self.labels)

    def __getitem__(self, i):
        return self.graphs[i], self.labels[i]

    def __len__(self):
        return len(self.graphs)

    def build_graphs(self):
        df = pd.read_csv(config.file_name)
        df = df.loc[df[config.target].notnull()]

        self.graphs = []
        self.labels = []
        for m_feat in self.params.mol_data_features:
            df = df.loc[df[m_feat].notnull()]

        data = zip(
            df["Structure"],
            df["logP"],
            *(df[x] for x in self.params.mol_data_features)
        )

        for smiles, value, *mol_d_feat in data:
            self.graphs.append(mol_to_graph(smiles, self.params, mol_d_feat))
            self.labels.append(value)


def load_data(dataset: pd.DataFrame):
    """Split dataset to train, test and validation"""
    train_set, val_set, test_set = split_dataset(
        dataset, frac_list=None, shuffle=False, random_state=None
    )
    train_loader = GraphDataLoader(
        dataset=train_set, shuffle=True, drop_last=False, batch_size=4
    )
    val_loader = GraphDataLoader(
        dataset=val_set, shuffle=True, drop_last=False, batch_size=1
    )
    test_loader = GraphDataLoader(
        dataset=test_set, shuffle=True, drop_last=False, batch_size=1
    )
    return train_loader, val_loader, test_loader
