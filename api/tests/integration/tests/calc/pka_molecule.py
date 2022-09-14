import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

import numpy as np

indigo = Indigo()
# indigo.setOption("ignore-bad-valence", True)
# indigo.setOption("ignore-stereochemistry-errors", True)


def r2(y_true, y_pred):
    assert len(y_true) == len(y_pred)
    y_true_mean = sum(y_true) / len(y_true)
    sses = []
    ssts = []
    for i in range(len(y_true)):
        if np.isnan(y_pred[i]):
            continue
        sses.append((y_true[i] - y_pred[i]) ** 2)
        ssts.append((y_true[i] - y_true_mean) ** 2)
    sse = sum(sses)
    sst = sum(ssts)
    return 1 - sse / sst


y_true = []
y_pred_simple = []
y_pred_advanced = []
y_pred_crippen = []
#
# for m in indigo.iterateSDFile(dataPath("molecules/pka/pka_in_water.sdf")):
#     print(m.rawData())
#     m2 = indigo.loadMolecule(m.rawData())
#     print(m2.molfile())


indigo.buildPkaModel(10, 0.5, dataPath("molecules/pka/pka_in_water.sdf"))

with open(dataPath("molecules/pka/pKaInWater.csv")) as f:
    next(f)
    for line in f:
        smiles, pka_value, pka_type = line.strip().split(',')
        if pka_type == "acidic":
            continue
        y_true.append(float(pka_value))

        m = indigo.loadMolecule(smiles)
        for options in (
                ("simple", 0, 2, y_pred_simple),
                ("advanced", 10, 2, y_pred_advanced),
        ):
            indigo.setOption("pKa-model", options[0])
            indigo.setOption("pKa-model-level", options[1])
            indigo.setOption("pKa-model-min-level", options[2])
            options[3].append(m.getAcidPkaValue())
        y_pred_crippen.append(m.crippenPka())
        # print(smiles, y_true[-1], y_pred_advanced[-1])
print("acidic simple", r2(y_true, y_pred_simple))
print("acidic advanced", r2(y_true, y_pred_advanced))
print("acidic crippen", r2(y_true, y_pred_crippen))

print()
y_true = []
y_pred_simple = []
y_pred_advanced = []
y_pred_crippen = []
with open(dataPath("molecules/pka/pKaInWater.csv")) as f:
    next(f)
    for line in f:
        smiles, pka_value, pka_type = line.strip().split(',')
        if pka_type != "basic":
            continue
        y_true.append(float(pka_value))
        m = indigo.loadMolecule(smiles)
        for options in (
                ("simple", 0, 0, y_pred_simple),
                ("advanced", 10, 0, y_pred_advanced),
        ):
            indigo.setOption("pKa-model", options[0])
            indigo.setOption("pKa-model-level", options[1])
            indigo.setOption("pKa-model-min-level", options[2])
            options[3].append(m.getBasicPkaValue())
        y_pred_crippen.append(m.crippenPka())
        # print(smiles, y_true[-1], y_pred_advanced[-1])
print("basic simple", r2(y_true, y_pred_simple))
print("basic advanced", r2(y_true, y_pred_advanced))
print("basic crippen", r2(y_true, y_pred_crippen))
