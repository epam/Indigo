import json
import os
import sys

def find_diff(a, b):
    if a == b:
        return ''
    return 'not equal'

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), '..', '..', '..', 'common')
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)

print('*** KET to CDXML to KET ***')

root = joinPathPy('reactions/', __file__)
ref_path = joinPathPy('ref/', __file__)

files = [
    'agents',
    'multi',
    'multi_overlap',
    '961-text_size',
]

files.sort()

for filename in files:
    try:
        with open(os.path.join(root, filename + '.ket'), 'r') as file:
            ket_str = file.read()
        ket = indigo.loadReaction(ket_str)
        cdxml_text = ket.cdxml()
        ket = indigo.loadReaction(cdxml_text)
        ket_result = ket.json()
        with open(os.path.join(ref_path, filename) + '.ket', 'r') as file:
            ket_ref = file.read()
        with open(os.path.join(ref_path, filename) + '.cdxml', 'r') as file:
            cdxml_ref = file.read()
        diff = find_diff(cdxml_ref, cdxml_text)
        if not diff:
            print(filename + '.cdxml:SUCCEED')
        else:
            print(filename + '.cdxml:FAILED')
            print('difference:' + diff)
        diff = find_diff(ket_ref, ket_result)
        with open(os.path.join(ref_path, filename), 'w') as file:
            file.write( cdxml_text )
        if not diff:
            print(filename + '.ket:SUCCEED')
        else:
            print(filename + '.ket:FAILED')
            print('difference:' + diff)
    except IndigoException as e:
        print(getIndigoExceptionText(e))
