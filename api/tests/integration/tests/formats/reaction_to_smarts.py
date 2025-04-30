import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
print(
    indigo.loadReactionFromFile(
        joinPathPy("reactions/973-reaction-smarts.ket", __file__)
    ).smarts()
)
