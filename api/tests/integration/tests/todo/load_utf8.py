# coding=utf-8
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

print("****** Load molfile with UTF-8 characters in Data S-group ********")
m = indigo.loadMoleculeFromFile(joinPath("molecules/sgroups_utf8.mol"))
indigo.setOption("molfile-saving-mode", "2000")
res = m.molfile()
m = indigo.loadMolecule(res)

# TODO: Fails on IronPython 2.7.9:
# - M  SED   1 single-value-бензол
# + M  SED   1 single-value-������

if isIronPython():
    from System.Text import Encoding
    from System import Console

    print(m.molfile())
    print(res)
    print(m.cml())

    # reload(sys)
    # sys.setdefaultencoding('utf-8')
    # sys.stdout = codecs.getwriter('utf8')(sys.stdout)
    # Console.WriteLine(m.molfile().encode("utf-8-sig"))
    # print(Encoding.UTF8.GetString(Encoding.Default.GetBytes(m.molfile().encode("utf-8-sig"))))

    # Console.Write(Encoding.UTF8.GetString(Encoding.UTF8.GetBytes(m.molfile().encode("utf-8"))))
    # Console.Write(Encoding.UTF8.GetString(Encoding.UTF8.GetBytes(res.encode("utf-8"))))
    # Console.Write(Encoding.UTF8.GetString(Encoding.UTF8.GetBytes(m.cml().encode("utf-8"))))
    # m.saveMolfile("test.mol")
    # with codecs.open(joinPath("test.mol"), "r", "utf-8-sig") as temp:
        # print(temp.read()[510:])

        # with codecs.open('test', 'w', "utf-8") as f:
            # f.write(m.molfile())
            # Console.WriteLine(m.molfile())
            # f.write(repr(Encoding.UTF8.GetString(Encoding.Default.GetBytes(m.molfile()))))
            # f.write(temp.read())
            # f.write(Encoding.UTF8.GetString(Encoding.Default.GetBytes(m.molfile().encode('utf-8'))))
        # Console.Write(str(temp.read()).encode('utf-8'))
else:
    if sys.version_info[0] < 3:
        print(m.molfile().encode("utf-8"))
        print(res.encode("utf-8"))
        print(m.cml().encode("utf-8"))
    else:
        print(m.molfile())
        print(res)
        print(m.cml())
