import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, Dataset


class FingerprintDataset(Dataset):
    """
    FingerprintDataset is a dataset wrapper for pytorch dataloader
    that converts data to pytorch tensors
    """

    def __init__(self, X, y):
        self.inputs = torch.as_tensor(X)
        self.targets = torch.as_tensor(y, dtype=torch.float32)

    def __len__(self):
        return len(self.targets)

    def __getitem__(self, index):
        if torch.is_tensor(index):
            index = index.tolist()

        out = self.targets[index]
        fp = self.inputs[index]
        return fp, out


class Perceptron(nn.Module):
    """
    Simple MLP PyTorch model with a Scikit-Learn like fit/predict interface
    """

    def __init__(
        self,
        input_size,
        hidden_size=128,
        n_layers=1,
        n_epochs=100,
        batch_size=32,
        lr=1e-2,
        verbose=False,
        p_dropout=0.01,
        optimizer="Adam",
        **kwargs,
    ):
        super(Perceptron, self).__init__()

        layers_size = [kwargs[key] for key in kwargs if "n_units" in key]
        if not layers_size:
            for i in range(n_layers):
                layers_size.append(hidden_size)
                hidden_size = hidden_size // 2

        layers = []
        for i in range(n_layers):
            layers.append(nn.Linear(input_size, layers_size[i]))
            input_size = layers_size[i]
        layers.append(nn.Linear(input_size, 1))
        self.layers = nn.ModuleList(layers)

        self.act = torch.nn.Mish()
        self.n_epochs = n_epochs
        self.batch_size = batch_size
        self.lr = lr
        self.verbose = verbose
        self.optimizer_name = optimizer

        dropouts = [kwargs[key] for key in kwargs if "dropout_l" in key]
        if not dropouts:
            for i in range(n_layers):
                dropouts.append(p_dropout)
        for i, drop in enumerate(dropouts):
            setattr(self, f"dropout_l{i}", nn.Dropout(drop))

    def forward(self, input):
        x = input
        for i, layer in enumerate(self.layers[:-1]):
            x = getattr(self, f"dropout_l{i}")(self.act(layer(x)))
        x = self.layers[-1](x)
        return x

    def fit(self, X, y):
        for layer in self.layers:
            if self.verbose:
                print("resetting ", layer)
            layer.reset_parameters()
        self.train()
        dataset = FingerprintDataset(X, y)
        loader = DataLoader(dataset, batch_size=self.batch_size)
        loss = torch.nn.MSELoss()
        optimizer = getattr(optim, self.optimizer_name)(
            self.parameters(), lr=self.lr
        )
        scheduler = torch.optim.lr_scheduler.ExponentialLR(
            optimizer, gamma=0.997
        )
        for i in range(self.n_epochs):
            for inputs, targets in loader:
                optimizer.zero_grad()
                preds = self.forward(inputs)
                targets = targets.reshape(-1, 1)

                error = loss(preds, targets)
                error.backward()
                optimizer.step()
            scheduler.step()
            if self.verbose:
                if (i + 1) % 50:
                    print(
                        "Epoch: {}/{}.............".format(
                            i + 1, self.n_epochs
                        ),
                        end=" ",
                    )
                    print("MSE Loss: {:.4f}".format(error.item()), end=" ")
                    print("lr: {:.4f}".format(optimizer.param_groups[0]["lr"]))

    def predict(self, X):
        X = torch.as_tensor(X)
        self.eval()
        with torch.no_grad():
            outputs = self.forward(X)

        return outputs.flatten().numpy()


class PerceptronOptuna(nn.Module):
    """
    Simple MLP PyTorch model with a Scikit-Learn like fit/predict interface
    """

    def __init__(
        self,
        trial,
        input_size,
        n_layers=1,
        max_features=1024,
        n_epochs=(50, 600),
        batch_size=[64, 128, 256],
        lr_interval=(1e-5, 1e-3),
        max_dropout=0.5,
        optims=["Adam"],
        verbose=False,
    ):
        super(PerceptronOptuna, self).__init__()

        if isinstance(n_layers, int):
            n_layers = trial.suggest_int("n_layers", n_layers, n_layers)
        elif isinstance(n_layers, tuple):
            n_layers = trial.suggest_int("n_layers", n_layers[0], n_layers[1])
        layers = []

        for i in range(n_layers):
            out_features = trial.suggest_int(f"n_units{i}", 4, max_features)
            layers.append(nn.Linear(input_size, out_features))
            input_size = out_features
            layers.append(torch.nn.Mish())
            p = trial.suggest_float("dropout_l{}".format(i), 0.01, max_dropout)
            layers.append(nn.Dropout(p))

        layers.append(nn.Linear(input_size, 1))
        self.layers = nn.ModuleList(layers)

        self.n_epochs = trial.suggest_int("n_epochs", n_epochs[0], n_epochs[1])
        self.batch_size = trial.suggest_categorical("batch_size", batch_size)
        self.lr = trial.suggest_float(
            "lr", lr_interval[0], lr_interval[1], log=True
        )
        self.optimizer_name = trial.suggest_categorical("optimizer", optims)
        self.verbose = verbose

    def forward(self, input):
        x = input
        for layer in self.layers[:-1]:
            x = layer(x)
        x = self.layers[-1](x)
        return x

    def fit(self, X, y):
        for layer in self.layers:
            if self.verbose:
                print("resetting ", layer)
            if not (isinstance(layer, (nn.Mish, nn.Dropout))):
                layer.reset_parameters()
        self.train()
        dataset = FingerprintDataset(X, y)
        loader = DataLoader(dataset, batch_size=self.batch_size)
        loss = torch.nn.MSELoss()
        optimizer = getattr(optim, self.optimizer_name)(
            self.parameters(), lr=self.lr
        )
        scheduler = torch.optim.lr_scheduler.ExponentialLR(
            optimizer, gamma=0.997
        )
        for i in range(self.n_epochs):
            for inputs, targets in loader:
                optimizer.zero_grad()
                preds = self.forward(inputs)
                targets = targets.reshape(-1, 1)

                error = loss(preds, targets)
                error.backward()
                optimizer.step()
            scheduler.step()
            if self.verbose:
                if (i + 1) % 50:
                    print(
                        "Epoch: {}/{}.............".format(
                            i + 1, self.n_epochs
                        ),
                        end=" ",
                    )
                    print("MSE Loss: {:.4f}".format(error.item()), end=" ")
                    print("lr: {:.4f}".format(optimizer.param_groups[0]["lr"]))

    def predict(self, X):
        X = torch.as_tensor(X)
        self.eval()
        with torch.no_grad():
            outputs = self.forward(X)

        return outputs.flatten().numpy()
