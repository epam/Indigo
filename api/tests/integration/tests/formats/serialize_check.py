import array
import binascii
import collections
import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("molfile-saving-add-implicit-h", False)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

all_features_mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/all_features_mol.mol", __file__)
)

# Add highlighting
for index in [1, 4, 5, 6, 7, 10, 40, 12, 13, 18, 20]:
    a = all_features_mol.getAtom(index)
    a.highlight()

for index in [5, 8, 1, 4, 5, 85, 10, 15, 112, 13, 2]:
    b = all_features_mol.getBond(index)
    b.highlight()

hex_output = open(joinPathPy("serialized/serialized.out", __file__), "w")
unser1 = indigo.createFileSaver(
    joinPathPy("serialized/unser1.sdf", __file__), "sdf"
)
unser2 = indigo.createFileSaver(
    joinPathPy("serialized/unser2_sm.sdf", __file__), "sdf"
)

unser1_prev = indigo.createFileSaver(
    joinPathPy("serialized/unser1_prev.sdf", __file__), "sdf"
)
unser2_prev = indigo.createFileSaver(
    joinPathPy("serialized/unser2_sm_prev.sdf", __file__), "sdf"
)

hex_output_prev = open(joinPathPy("serialized/serialized.std", __file__), "r")

saver = indigo.createFileSaver(
    joinPathPy("out/serialize_check.sdf", __file__), "sdf"
)

cano_sm_file = open(joinPathPy("out/cano_sm.smi", __file__), "w")


def getMolProperties(mol):
    return {
        "atoms": mol.countAtoms(),
        "bonds": mol.countBonds(),
        "attachment points": mol.countAttachmentPoints(),
        "hydrogens": mol.countHydrogens(),
        "superatoms": mol.countSuperatoms(),
        "data Sgroups": mol.countDataSGroups(),
        "repeating units": mol.countRepeatingUnits(),
        "multiple groups": mol.countMultipleGroups(),
        "generic Sgroups": mol.countGenericSGroups(),
        "pseudoatoms": mol.countPseudoatoms(),
        "rsites": mol.countRSites(),
        "stereocenters": mol.countStereocenters(),
        "allenecenters": mol.countAlleneCenters(),
        "components": mol.countComponents(),
        "sssr": mol.countSSSR(),
        "heavy atoms": mol.countHeavyAtoms(),
    }


def checkMolEqualProperties(p1, p2):
    print("* checkEqualProperties *")
    keys = sorted(list(set(list(p1.keys()) + list(p2.keys()))))
    for k in keys:
        message = ""
        p1v = "None"
        p2v = "None"
        if k in p1:
            p1v = p1[k]
        if k in p2:
            p2v = p2[k]
        if p1v != p2v:
            message = " <- Error"
            # sys.stderr.write("Molecule properties are different\n")
        print("{0}: {1} {2} {3}".format(k, p1v, p2v, message))


def getMoleculeCanonicalSmilesOrEmpty(m):
    try:
        return m.canonicalSmiles()
    except IndigoException as e:
        return ""


def printSGroupsInfo(m):
    for gr in m.iterateSuperatoms():
        str = ""
        for a in gr.iterateAtoms():
            str += " " + a.symbol()
        print(str)


class Processor(object):
    def iterateMols(self, item):
        raise NotImplementedError()

    def itemCanonicalCode(self, item):
        raise NotImplementedError()

    def getProperties(self, item):
        raise NotImplementedError()

    def checkEqualProperties(self, p1, p2):
        raise NotImplementedError()

    def loadItem(self, data):
        raise NotImplementedError()

    def process(self, mol):
        sm = ""
        try:
            sm = mol.smiles()
            print(sm)
        except IndigoException as e:
            print(getIndigoExceptionText(e))

        cano_sm = self.itemCanonicalCode(mol)
        print(cano_sm)
        # print(mol.molfile())

        saver.append(mol)
        mol.layout()
        saver.append(mol)

        m2 = self.loadItem(sm)
        sm2 = ""
        try:
            sm2 = m2.smiles()
            print(sm2)
        except IndigoException as e:
            print(getIndigoExceptionText(e))
        cano_sm2 = self.itemCanonicalCode(m2)
        print(cano_sm2)
        # print(m2.molfile())

        m2.layout()
        saver.append(m2)

        if cano_sm != cano_sm2 and cano_sm != "":
            sys.stderr.write(
                "Canonical smiles are different:\n%s\n%s\n"
                % (cano_sm, cano_sm2)
            )
            cano_sm_file.write(cano_sm)
            cano_sm_file.write(cano_sm2)

        # Output serialized data to check consistency
        buf = mol.serialize()
        buf2 = m2.serialize()

        hex_ser1 = binascii.hexlify(
            buf.tostring()
            if isJython()
            else buf
        )
        hex_ser2 = binascii.hexlify(
            buf2.tostring()
            if isJython()
            else buf
        )

        hex_output.write(hex_ser1.decode("ascii") + "\n")
        hex_output.write(hex_ser2.decode("ascii") + "\n")

        # Reload molecule from serialized buffer
        mol_rel = indigo.unserialize(buf)
        unser1.append(mol_rel)
        m2_rel = indigo.unserialize(buf2)
        unser2.append(m2_rel)

        mol_rel_prev = None
        m2_rel_prev = None
        try:
            s1 = next(hex_output_prev).strip()
            s2 = next(hex_output_prev).strip()
            arr = binascii.unhexlify(s1)
            if isIronPython():
                from System import Array, Byte

                arr = Array[Byte](arr)
            mol_rel_prev = indigo.unserialize(arr)
            arr = binascii.unhexlify(s2)
            if isIronPython():
                from System import Array, Byte

                arr = Array[Byte](arr)
            m2_rel_prev = indigo.unserialize(arr)

            unser1_prev.append(mol_rel_prev)
            unser2_prev.append(m2_rel_prev)

        except IndigoException as e:
            print("caught {0}\n".format(getIndigoExceptionText(e)))
        # except Exception, e:
        #    print("caught unknown exception: {0}\n".format(getIndigoExceptionText(e)))

        cano_sm_rel = self.itemCanonicalCode(mol_rel)
        cano_sm2_rel = self.itemCanonicalCode(m2_rel)

        if cano_sm != cano_sm_rel and cano_sm != "":
            sys.stderr.write(
                "Canonical smiles are different after unserialize(serialize()):\n%s\n%s\n"
                % (cano_sm, cano_sm_rel)
            )
            cano_sm_file.write(cano_sm)
            cano_sm_file.write(cano_sm_rel)

        if cano_sm2 != cano_sm2_rel:
            sys.stderr.write(
                "Canonical smiles are different after unserialize(serialize()):\n%s\n%s\n"
                % (cano_sm2, cano_sm2_rel)
            )
            cano_sm_file.write(cano_sm2)
            cano_sm_file.write(cano_sm2_rel)

        p1 = self.getProperties(mol)
        p1_rel = self.getProperties(mol_rel)

        self.checkEqualProperties(p1, p1_rel)

        p2 = self.getProperties(m2)
        p2_rel = self.getProperties(m2_rel)

        self.checkEqualProperties(p2, p2_rel)

        for i1, i2 in zip(self.iterateMols(mol), self.iterateMols(mol_rel)):
            if i1.countSuperatoms() > 0:
                print("Superatoms:")
                printSGroupsInfo(i1)
                printSGroupsInfo(i2)

        print("Properties unserialized from the previous version:")
        for prev in [mol_rel_prev, m2_rel_prev]:
            print("Item")
            if prev is None:
                print("  None")
            else:
                props = self.getProperties(prev)
                for p in sorted(props.keys()):
                    print("  %s:%s" % (p, props[p]))


class MoleculeProcessor(Processor):
    def iterateMols(self, item):
        yield item

    def itemCanonicalCode(self, item):
        return getMoleculeCanonicalSmilesOrEmpty(item)

    def getProperties(self, item):
        return getMolProperties(item)

    def checkEqualProperties(self, p1, p2):
        return checkMolEqualProperties(p1, p2)

    def loadItem(self, data):
        return indigo.loadMolecule(data)


class ReactionProcessor(Processor):
    def iterateMols(self, item):
        for m in item.iterateMolecules():
            yield m

    def iterateMolsWithType(self, item):
        for m in item.iterateReactants():
            yield m, "reactant"
        for m in item.iterateProducts():
            yield m, "product"
        for m in item.iterateCatalysts():
            yield m, "catalust"

    def itemCanonicalCode(self, item):
        cano = []
        for m in self.iterateMols(item):
            cano.append(getMoleculeCanonicalSmilesOrEmpty(m))
        return ".".join(sorted(cano))

    def getProperties(self, item):
        prop = {
            "reactants": item.countReactants(),
            "products": item.countProducts(),
            "catalyst": item.countCatalysts(),
        }
        subprops = collections.defaultdict(list)
        for m, type in self.iterateMolsWithType(item):
            subprops[type].append(getMolProperties(m))
        for type in subprops:
            for idx, subprop in enumerate(sorted(subprops[type])):
                for p in subprop:
                    prop["%s%02d.%s" % (type, idx + 1, p)] = subprop[p]
        return prop

    def checkEqualProperties(self, p1, p2):
        return checkMolEqualProperties(p1, p2)

    def loadItem(self, data):
        return indigo.loadReaction(data)


processMol = MoleculeProcessor().process

processMol(all_features_mol)
all_features_mol.aromatize()
processMol(all_features_mol)

# Process other molecules
test_sets = [
    (
        joinPathPy(
            "../../../../../data/molecules/basic/thiazolidines.sdf", __file__
        ),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy(
            "../../../../../data/molecules/allenes/all-allenes.sdf", __file__
        ),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy(
            "../../../../../data/molecules/sgroups/all_sgroups.sdf", __file__
        ),
        indigo.iterateSDFile,
    ),
    (joinPathPy("molecules/serialize.sdf", __file__), indigo.iterateSDFile),
]

for file, func in test_sets:
    print("Molecules set: %s" % (relativePath(file)))
    it = func(file)
    for m, idx in zip(it, range(100000)):
        print("\nTesting molecule #%d" % (idx))
        try:
            processMol(m)
            m.aromatize()
            processMol(m)
        except IndigoException as e:
            print("caught {0}\n".format(getIndigoExceptionText(e)))

print("*** Reactions serialization ***")
hex_output = open(
    joinPathPy("serialized/reaction_serialized.out", __file__), "w"
)
unser1 = indigo.createFileSaver(
    joinPathPy("serialized/reaction_unser1.rdf", __file__), "rdf"
)
unser2 = indigo.createFileSaver(
    joinPathPy("serialized/reaction_unser2_sm.rdf", __file__), "rdf"
)

unser1_prev = indigo.createFileSaver(
    joinPathPy("serialized/reaction_unser1_prev.rdf", __file__), "rdf"
)
unser2_prev = indigo.createFileSaver(
    joinPathPy("serialized/reaction_unser2_sm_prev.rdf", __file__), "rdf"
)

hex_output_prev = open(
    joinPathPy("serialized/reaction_serialized.std", __file__), "r"
)

saver = indigo.createFileSaver(
    joinPathPy("out/reaction_serialize_check.rdf", __file__), "rdf"
)

r = indigo.loadReactionFromFile(joinPathPy("reactions/ordering.rxn", __file__))
processReaction = ReactionProcessor().process
processReaction(r)


def iterateReactionSmilesFile(f):
    for item in indigo.iterateSmilesFile(f):
        yield indigo.loadReaction(item.rawData())


test_sets = [
    (
        joinPathPy("reactions/reactions.smi", __file__),
        iterateReactionSmilesFile,
    ),
]

for file, func in test_sets:
    print("Reactions set: %s" % (relativePath(file)))
    it = func(file)
    for idx, m in enumerate(it):
        print("\nTesting reaction #%d" % (idx))
        try:
            processReaction(m)
            m.aromatize()
            processReaction(m)
        except IndigoException as e:
            print("caught {0}\n".format(getIndigoExceptionText(e)))
