import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *
from rendering import *


def detect(indigo, molstr):
    try:
        mol = indigo.loadMolecule(molstr)
        print("detected as molecule")
    except:
        try:
            mol = indigo.loadQueryMolecule(molstr)
            print("detected as query")
        except:
            try:
                mol = indigo.loadReaction(molstr)
                print("detected as reaction")
            except:
                try:
                    mol = indigo.loadQueryReaction(molstr)
                    print("detected as query reaction")
                except:
                    print("bad ket data")


if __name__ == "__main__":
    indigo = Indigo()
    rfile = open(joinPathPy("molecules/reaction.ket", __file__))
    reaction = rfile.read()
    detect(indigo, reaction)
    rfile.close()
    qfile = open(joinPathPy("molecules/query.ket", __file__))
    query = qfile.read()
    detect(indigo, query)
    qfile.close()
    qrfile = open(joinPathPy("molecules/qreaction.ket", __file__))
    qreaction = qrfile.read()
    detect(indigo, qreaction)
    mfile = open(joinPathPy("molecules/arom.ket", __file__))
    mol = mfile.read()
    detect(indigo, mol)
    mfile.close()
    qfile = open(joinPathPy("molecules/query1.ket", __file__))
    query = qfile.read()
    detect(indigo, query)
    qfile.close()
    mfile = open(joinPathPy("molecules/pseudo.ket", __file__))
    mol = mfile.read()
    detect(indigo, mol)
    mfile.close()
