import json
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def test_string_is_valid_json():
    print("*** Testing string is valid json ***")
    try:
        json_object = json.loads(indigo.versionInfo())
        json.loads(json.dumps(json_object, indent=2))
    except:
        print("False")
        return
    print("True")


def test_string_has_specific_key(key):
    print("*** Testing string contains '{}' key ***".format(key))
    try:
        json.loads(indigo.versionInfo())[key]
    except:
        print("False")
        return
    print("True")


test_string_is_valid_json()
test_string_has_specific_key("majorVersion")
test_string_has_specific_key("minorVersion")
test_string_has_specific_key("devTag")
test_string_has_specific_key("commitHash")
test_string_has_specific_key("compilerVersion")
test_string_has_specific_key("compilerPlatform")
