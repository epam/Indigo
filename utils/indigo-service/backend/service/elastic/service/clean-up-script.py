import os

from indigo import Indigo
import sys
import re

if __name__ == '__main__':
    indigo = Indigo()
    try:
        fileName = sys.argv[1]
    except Exception:
        raise ValueError("FileName should be provided")

    match = re.search(r'([\w_-]+)\.\w+$', fileName)
    if match:
        filename_without_ext = match.group(1)
        newName = f"clean/{filename_without_ext}_clean.sdf"
        os.makedirs(os.path.dirname(newName), exist_ok=True)
        with open(newName, "w") as f:
            sdf = indigo.iterateSDFile(fileName)
            i = 0
            for mol in sdf:
                if indigo.check(mol.rawData()) == "{}":
                    f.write(mol.rawData())
                    f.write("\n$$$$\n")
                    print(i)
                    i += 1
    else:
        raise Exception("Cannot retrieve file name.")
