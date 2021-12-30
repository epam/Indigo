'''Used running each model-training on different params in automation between them.
place into dgl-lifesci\examples\property_prediction\MTL with dataset and run.'''
import os

modls = ['GCN', 'GAT', 'MPNN', 'AttentiveFP']
params = ['AdrA1A_PCHEMBL_VALUE', 'logP']

for param in params:
    for model in modls:
        os.system(f'python main.py -c ad.csv -m {model} --mode parallel -p {param}_{model} -s Structure '
                  f'-t {param}')