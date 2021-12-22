import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def testCanonicalIdentity1():
    print("*** Simple test 1 ***")
    rxn1 = indigo.loadReaction(
        "[CH2:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1>>[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:2]1[CH2:1][CH2:6][CH2:5][CH2:4][CH2:3]1"
    )
    rxn2 = indigo.loadReaction(
        "[CH2:6]1[CH2:1][CH2:2][CH2:3][CH2:4][CH2:5]1>>[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:5]1[CH2:6][CH2:1][CH2:2][CH2:3][CH2:4]1"
    )
    print(rxn1.canonicalSmiles() == rxn1.canonicalSmiles())


def testCanonicalIdentity2():
    print("*** Simple test 2 ***")
    rxn1 = indigo.loadReaction(
        "[CH2:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1>>[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:2]1[CH2:1][CH2:6][CH2:5][CH2:4][CH2:3]1"
    )
    rxn2 = indigo.loadReaction(
        "[CH2:6]1[CH2:1][CH2:2][CH2:3][CH2:4][CH2:5]1>>[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:5]1[CH2:6][CH2:1][CH2:2][CH2:3][CH2:4]1"
    )
    print(rxn1.canonicalSmiles() == rxn1.canonicalSmiles())


def testCanonicalIdentity3():
    print("*** Simple test 3 ***")
    rxn1 = indigo.loadReaction(
        "[CH2:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1>>[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:2]1[CH2:1][CH2:6][CH2:5][CH2:4][CH2:3]1"
    )
    rxn2 = indigo.loadReaction(
        "[CH2:6]1[CH2:1][CH2:2][CH2:3][CH2:4][CH2:5]1>>[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:5]1[CH2:6][CH2:1][CH2:2][CH2:3][CH2:4]1"
    )
    print(rxn1.canonicalSmiles() == rxn1.canonicalSmiles())


def testCanonicalIdentity4():
    print("*** Simple test 4 ***")
    rxn1 = indigo.loadReaction(
        "[CH2:7]1[CH2:8][CH2:9][CH:10]([CH2:11][CH2:12]1)[CH:2]1[CH2:1][CH2:6][CH2:5][CH2:4][CH2:3]1>>[CH2:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1"
    )
    rxn2 = indigo.loadReaction(
        "[CH2:17]1[CH2:18][CH2:19][CH:20]([CH2:21][CH2:22]1)[CH:12]1[CH2:11][CH2:16][CH2:15][CH2:14][CH2:13]1>>[CH2:11]1[CH2:16][CH2:15][CH2:14][CH2:13][CH2:12]1"
    )
    print(rxn1.canonicalSmiles() == rxn1.canonicalSmiles())


def testCanonicalIdentity5():
    print("*** Simple test 5 ***")
    rxn1 = indigo.loadReaction(
        "[CH2:2]1[CH2:3][CH2:4][CH:5]([CH2:6][CH2:1]1)[CH:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1>>[CH2:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1"
    )
    rxn2 = indigo.loadReaction(
        "[CH2:22]1[CH2:23][CH2:24][CH:25]([CH2:26][CH2:21]1)[CH:21]1[CH2:26][CH2:25][CH2:24][CH2:23][CH2:22]1>>[CH2:21]1[CH2:26][CH2:25][CH2:24][CH2:23][CH2:22]1"
    )
    print(rxn1.canonicalSmiles() == rxn1.canonicalSmiles())


def testCanonicalIdentity6():
    print("*** Simple test 6 ***")
    rxn1 = indigo.loadReaction(
        "[CH2:2]1[CH2:98][CH2:97][CH:5]([CH2:6][CH2:1]1)[CH:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1>>[CH2:1]1[CH2:6][CH2:5][CH2:4][CH2:3][CH2:2]1"
    )
    rxn2 = indigo.loadReaction(
        "[CH2:12]1[CH2:198][CH2:197][CH:15]([CH2:16][CH2:11]1)[CH:11]1[CH2:16][CH2:15][CH2:14][CH2:13][CH2:12]1>>[CH2:11]1[CH2:16][CH2:15][CH2:14][CH2:13][CH2:12]1"
    )
    print(rxn1.canonicalSmiles() == rxn1.canonicalSmiles())


testCanonicalIdentity1()
testCanonicalIdentity2()
testCanonicalIdentity3()
testCanonicalIdentity4()
testCanonicalIdentity5()
testCanonicalIdentity6()
