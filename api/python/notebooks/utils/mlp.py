import torch
import torch.nn as nn
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
        output_size=1,
        hidden_size=128,
        p_dropout=0.05,
        n_layers=1,
        n_epochs=100,
        batch_size=32,
        lr=1e-2,
        verbose=False
    ):
        super(Perceptron, self).__init__()
        self.layers = nn.ModuleList(
            [nn.Linear(input_size, hidden_size)] +
            [nn.Linear(hidden_size // 2**i, hidden_size // 2**(i+1)) for i in range(n_layers)] +
            [nn.Linear(hidden_size // 2**(n_layers), output_size)]
        )
        self.act = torch.nn.Mish()
        self.n_epochs = n_epochs
        self.batch_size = batch_size
        self.lr = lr
        self.verbose = verbose
        self.dropout = nn.Dropout(p=p_dropout)
    
    def forward(self, input):
        x = input
        for l in self.layers[:-1]:
            x = self.dropout(self.act(l(x)))
        x = self.layers[-1](x)
        return x

    def fit(self, X, y):
        for layer in self.layers:
            if self.verbose:
                print('resetting ', layer)
            layer.reset_parameters()
        self.train()
        dataset = FingerprintDataset(X, y)
        loader = DataLoader(dataset, batch_size=self.batch_size)
        loss = torch.nn.MSELoss()
        optimizer = torch.optim.AdamW(self.parameters(), lr=self.lr, weight_decay=1e-7)
        scheduler = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.997)
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
                if (i+1) % 50:
                    print('Epoch: {}/{}.............'.format(i+1, self.n_epochs), end=' ')
                    print("MSE Loss: {:.4f}".format(error.item()), end =' ')
                    print("lr: {:.4f}".format(optimizer.param_groups[0]['lr']))

    def predict(self, X):
        X = torch.as_tensor(X)
        self.eval()
        with torch.no_grad():
            outputs = self.forward(X)

        return outputs.flatten().numpy()
