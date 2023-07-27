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

def get_value_by_key(key):
    return json.loads(indigo.versionInfo())[key]

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

def test_string_has_major_version():
    print("*** Testing string contains major version ***")
    try:
        print(get_value_by_key('majorVersion'))
    except:
        print("String does not contain major version")

def test_string_has_minor_version():
    print("*** Testing string contains minor version ***")
    try:
        print(get_value_by_key('minorVersion'))
    except:
        print("String does not contain minor version")

def test_string_has_dev_tag():
    print("*** Testing string contains dev tag ***")
    try:
        print(get_value_by_key('devTag'))
    except:
        print("String does not contain dev tag")  

def test_string_has_commit_hash():
    print("*** Testing string contains commit hash ***")
    try:
        print(get_value_by_key('commitHash'))
    except:
        print("String does not contain commit hash")  

def test_string_has_compiler_version():
    print("*** Testing string contains compiler version ***")
    try:
        print(get_value_by_key('compilerVersion'))
    except:
        print("String does not contain compiler version")  

def test_string_has_compiler_platform():
    print("*** Testing string contains compiler platform ***")
    try:
        print(get_value_by_key('compilerPlatform'))
    except:
        print("String does not contain compiler platform") 

test_string_is_readable()
test_string_is_valid_json()
test_string_has_major_version()
test_string_has_minor_version()
test_string_has_dev_tag()
test_string_has_commit_hash()
test_string_has_compiler_version()
test_string_has_compiler_platform()