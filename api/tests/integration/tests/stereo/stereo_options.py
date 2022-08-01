import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", False)
indigo.setOption("molfile-saving-skip-date", "1")

print("****** Load/save with default options  *******")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** Load/save with treat-stereo-as options  *******")
print("****** abs *******")
indigo.setOption("treat-stereo-as", "abs")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))

print("****** rel *******")
indigo.setOption("treat-stereo-as", "rel")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** rac *******")
indigo.setOption("treat-stereo-as", "rac")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** any *******")
indigo.setOption("treat-stereo-as", "any")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** Load/save with chiral_flag option  *******")
print("****** default *******")
indigo.setOption("treat-stereo-as", "ucf")
indigo.setOption("molfile-saving-chiral-flag", -1)
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))
    print(m.molfile())

print("****** no chiral flag *******")
indigo.setOption("molfile-saving-chiral-flag", 0)
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))
    print(m.molfile())


print("****** chiral flag *******")
indigo.setOption("molfile-saving-chiral-flag", 1)
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))
    print(m.molfile())
