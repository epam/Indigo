import concurrent.futures
import os
import sys

sys.path.append(os.path.join(os.path.abspath(__file__), '..', '..', 'common'))
from env_indigo import *

SMILES_LIST = [
    'C',
    'CC',
    'CCC',
    'CCCCC',
    'CCCCCC',
    'CCCCCCC',
    'CCCCCCCC',
] * 10


def render(smiles):
    ind = Indigo()
    imol = ind.loadMolecule(smiles)
    renderer = IndigoRenderer(ind)
    ind.setOption('render-output-format', 'png')
    buff = renderer.renderToBuffer(imol)
    return buff.tobytes()


if __name__ == '__main__':
    with concurrent.futures.ThreadPoolExecutor(max_workers=16) as executor:
        futures_dict = {
            executor.submit(render, smiles): smiles
            for smiles in SMILES_LIST
        }
        for future in concurrent.futures.as_completed(futures_dict):
            smi = futures_dict[future]
            data = future.result()
            print ("OK")
