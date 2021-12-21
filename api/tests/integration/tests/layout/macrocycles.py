import errno
import math
import sys
from math import *

MIN_DIST = 0.1
eps = 0.01

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

print("**** Test Macrocycles ****")

saver = indigo.writeFile(joinPathPy("out/macrocycles.sdf", __file__))

ref_path = getRefFilepath("macrocycles.sdf")
ref = indigo.iterateSDFile(ref_path)
for idx, item in enumerate(
    indigo.iterateSmilesFile(
        joinPathPy("molecules/macrocycles_test.smi", __file__)
    )
):
    try:
        print("Test Item #{} ".format(idx))
        mol = item.clone()
        mol.layout()
        res = moleculeLayoutDiff(
            indigo, mol, ref.at(idx).rawData(), ref_is_file=False
        )
        print("  Result: {}".format(res))
        mol.setProperty("test", "Item #{} ".format(idx))
        saver.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
saver.close()


def area(x1, y1, x2, y2, x3, y3):
    return (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1)


def dif_free_try(item):
    try:
        mol = item.clone()
        cansm_before = mol.canonicalSmiles()
        mol.layout()
        saver.sdfAppend(mol)
        cansm_after = mol.canonicalSmiles()
        if cansm_before != cansm_after:
            sys.stderr.write("Different canonical smiles for #%s:\n" % idx)
            sys.stderr.write("  %s\n" % cansm_before)
            sys.stderr.write("  %s\n" % cansm_after)

        n = mol.countAtoms()

        co = [[0 for i in range(3)] for j in range(n)]
        angle = [[] for i in range(n)]
        for i in range(n):
            co[i] = mol.getAtom(i).xyz()

        dist = [[0 for i in range(n)] for j in range(n)]
        sum_dist = 0
        for i in range(n):
            for j in range(n):
                if i < j:
                    xyz = mol.getAtom(i).xyz()
                    xyz2 = mol.getAtom(j).xyz()
                    xyz[0] -= xyz2[0]
                    xyz[1] -= xyz2[1]
                    xyz[2] -= xyz2[2]
                    dist[i][j] = (
                        math.sqrt(
                            xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]
                        )
                        / 1.6
                    )
                    dist[i][j] = max(dist[i][j], eps)
                    dist[i][j] = 1 / dist[i][j]

        m = mol.countBonds()
        for t in range(m):
            e = mol.getBond(t)
            i = e.source().index()
            j = e.destination().index()
            if j < i:
                temp = i
                i = j
                j = temp
            if dist[i][j] < 1:
                dist[i][j] = 1 / dist[i][j]

        for i in range(n):
            for j in range(n):
                if i < j:
                    dist[i][j] -= 1
                    dist[i][j] = max(dist[i][j], 0)
                    sum_dist += dist[i][j] * dist[i][j]

        for e1 in mol.iterateBonds():
            for e2 in mol.iterateBonds():
                a1 = e1.source().index()
                b1 = e1.destination().index()
                a2 = e2.source().index()
                b2 = e2.destination().index()
                if (a1 - a2) * (a1 - b2) * (b1 - a2) * (b1 - b2) != 0:
                    s1 = area(
                        co[a1][0],
                        co[a1][1],
                        co[a2][0],
                        co[a2][1],
                        co[b2][0],
                        co[b2][1],
                    )
                    s2 = area(
                        co[b1][0],
                        co[b1][1],
                        co[a2][0],
                        co[a2][1],
                        co[b2][0],
                        co[b2][1],
                    )
                    s3 = area(
                        co[a2][0],
                        co[a2][1],
                        co[a1][0],
                        co[a1][1],
                        co[b1][0],
                        co[b1][1],
                    )
                    s4 = area(
                        co[b2][0],
                        co[b2][1],
                        co[a1][0],
                        co[a1][1],
                        co[b1][0],
                        co[b1][1],
                    )
                    if (s1 * s2 <= 0) and (s3 * s4 <= 0):
                        sum_dist += 1 / sqrt(eps)

        sum_dist /= m

        sum_angle = 0
        need = [[] for i in range(8)]
        for i in range(3, 8):
            need[i].append([2 * pi / i for j in range(i)])
        need[2].append([0, 0])
        need[2].append([pi / 3, pi / 3])
        need[4].append([pi / 3, pi / 3, 2 * pi / 3, 2 * pi / 3])
        need[4].append([pi / 3, pi / 2, pi / 2, 2 * pi / 3])
        for v in range(n):
            deg = mol.getAtom(v).degree()
            if deg > 1:
                best = 10
                for nei in mol.getAtom(v).iterateNeighbors():
                    u = nei.index()
                    x = co[u][0] - co[v][0]
                    y = co[u][1] - co[v][1]
                    ang = 0
                    if abs(x) < eps:
                        if abs(y) < eps:
                            ang = -1
                        elif y > 0:
                            ang = pi / 2
                        else:
                            ang = 3 * pi / 2
                    elif x > 0:
                        ang = atan(y / x)
                    else:
                        ang = pi + atan(y / x)
                    if ang < 0:
                        ang += 2 * pi
                    angle[v].append(ang)
                angle[v].sort()
                if angle[v][0] != -1:
                    angle[v].append(angle[v][0] + 2 * pi)
                    a1 = []
                    for i in range(deg):
                        a1.append(angle[v][i + 1] - angle[v][i])
                    a1.sort()
                    if deg == 2:
                        for i in range(2):
                            a1[i] = abs(pi - a1[i])
                    forbid = -1
                    if mol.getAtom(v).degree() == 2:
                        order = 0
                        for nei in mol.getAtom(v).iterateNeighbors():
                            if nei.bond().bondOrder() == 4:
                                order += 1
                            else:
                                order += nei.bond().bondOrder()
                        if order >= 4:
                            forbid = 1
                        else:
                            forbid = 0
                    for i in range(len(need[deg])):
                        if i != forbid:
                            mx = 0
                            for j in range(deg):
                                if need[deg][i][j] == 0:
                                    if abs(a1[j]) < eps:
                                        x = 1
                                    else:
                                        x = 0
                                else:
                                    x = a1[j] / need[deg][i][j]
                                x = max(x, eps)
                                if x < 1:
                                    x = 1 / x
                                mx = max(mx, x)
                            mx -= 1
                            best = min(best, mx)
                sum_angle += best * best
        sum_angle /= n
        print(int(floor((sum_angle + sum_dist) * 1000)))

    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
    idx += 1
