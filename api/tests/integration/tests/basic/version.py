import os
import sys
import json

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

def test_string_is_readable():
    print("*** Testing string is readable ***")
    try:
        print(indigo.versionInfo())
    except:
        print("String can't be read")

def test_string_is_valid_json():
    print("*** Testing string is valid json ***")
    try:
        json_object = json.loads(indigo.versionInfo())
        print(json.dumps(json_object, indent=2))
    except:
        print("String is not valid json")

def test_string_has_specific_key(key):
    print("*** Testing string contains '{}' key ***".format(key))
    try:
        print(json.loads(indigo.versionInfo())[key])
    except:
        print("String does not contain '{}' key".format(key)) 

test_string_is_readable()
test_string_is_valid_json()
test_string_has_specific_key('majorVersion')
test_string_has_specific_key('minorVersion')
test_string_has_specific_key('devTag')
test_string_has_specific_key('commitHash')
test_string_has_specific_key('compilerVersion')
test_string_has_specific_key('compilerPlatform')
