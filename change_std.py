import json

from indigo import Indigo
from indigo.renderer import IndigoRenderer

i = Indigo()
ir = IndigoRenderer(i)


def change_std(old_file, new_file):
    out_file = open(new_file, 'w')

    with open(old_file, 'r') as infile:
        std_string = infile.read()

    std = json.loads(std_string)
    print(len(std))
    results = []
    with open('/Users/Alina_Tikhova/PycharmProjects/Indigo/results.txt', 'r') as f:
        for line in f:
            results.append(line)
    print(results)
    for idx in range(10095):
        std[idx]['expected'] = results[idx]

    out_file.write(json.dumps(std))
    out_file.close()


if __name__ == '__main__':
    old = '/Users/Alina_Tikhova/PycharmProjects/Indigo/bingo/tests/data/molecules/fingerprint/std.json'
    new = '/Users/Alina_Tikhova/PycharmProjects/Indigo/bingo/tests/data/molecules/fingerprint/std_new.json'
    change_std(old, new)
