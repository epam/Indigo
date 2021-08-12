import os
import sys
import errno
import math

eps = 0.01

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")

saver = indigo.writeFile(joinPathPy("out/result_simple_cycles.sdf", __file__))
idx = 1

print("**** Test simple_cycles *****")
for item in indigo.iterateSmilesFile(joinPathPy("molecules/simple_cycles.smi", __file__)):
    try:
        mol = item.clone()
        mol.layout()
        saver.sdfAppend(mol)

        n = mol.countAtoms()
        co = [[0 for i in range(3)] for j in range(n)]
        angle = [[] for i in range(n)]
        for i in range(n):
            co[i] = mol.getAtom(i).xyz()

        sumx = 0
        sumy = 0
        for i in range(n):
            sumx += co[i][0]
            sumy += co[i][1]

        sumx /= n
        sumy /= n

        for i in range(n):
            co[i][0] -= sumx
            co[i][1] -= sumy

        radius = [0 for i in range(n)]
        for i in range(n):
            radius[i] = math.sqrt(co[i][0] * co[i][0] + co[i][1] * co[i][1])

        min_radius = radius[0]
        max_radius = radius[0]
        for i in range(n):
            min_radius = min(min_radius, radius[i])
            max_radius = max(max_radius, radius[i])

        if (max_radius - min_radius > eps):
            print("Vertices not lies of circle")
        else:
            dist = [0 for i in range(n)]
            for i in range(n):
                dist[i] = math.sqrt((co[(i + 1) % n][0] - co[i][0]) * (co[(i + 1) % n][0] - co[i][0]) + (co[(i + 1) % n][1] - co[i][1]) * (co[(i + 1) % n][1] - co[i][1]))

            min_dist = dist[0]
            max_dist = dist[0]
            for i in range(n):
                min_dist = min(min_dist, dist[i])
                max_dist = max(max_dist, dist[i])

            if (max_dist - min_dist > eps):
                print("Polygon is not right")
            else:
                up = 0
                for i in range(n):
                    if (co[i][1] > co[up][1]):
                        up = i

            if (abs(co[up][0]) > eps):
                print("Cycle is not suspended")
            else:
                print("OK")

    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
    idx += 1

saver.close()

saver = indigo.writeFile(joinPathPy("out/cyclopropadiens.sdf", __file__))

ref_path = getRefFilepath("cyclopropadiens.sdf")
ref = indigo.iterateSDFile(ref_path)

print("**** Test cyclopropadiens *****")
for idx, mol in enumerate(indigo.iterateSmilesFile(joinPathPy("molecules/cyclopropadiens.smi", __file__))):
    mol.layout()
    res = moleculeLayoutDiff(indigo, mol, ref.at(idx).rawData(), ref_is_file = False)
    print("  Item #{}: Result: {}".format(idx, res))
    saver.sdfAppend(mol)
