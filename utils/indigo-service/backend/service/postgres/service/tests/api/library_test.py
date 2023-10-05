#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# from app import app, is_indigo_db

import json
import os
import unittest
from io import BytesIO
from time import sleep

import requests


# @unittest.skip("Skip libraries test case")
class LibrariesTestCase(unittest.TestCase):
    def setUp(self):
        service_url = "http://front/v2"
        if (
            "INDIGO_SERVICE_URL" in os.environ
            and len(os.environ["INDIGO_SERVICE_URL"]) > 0
        ):
            service_url = os.environ["INDIGO_SERVICE_URL"]

        self.url_prefix = "{}/libraries".format(service_url)
        self.init_count = self.get_libraries_count()

    def tearDown(self):
        pass

    @staticmethod
    def get_headers(d):
        headers = {
            "Content-Type": "application/json",
            "Accept": "application/json",
        }
        data = json.dumps(d)
        headers["Content-Length"] = len(data)
        return headers, data

    def do_upload(self, library_id, file_path, content_type):
        # Upload file to library
        with open(file_path, "rb") as f:
            fileData = BytesIO(f.read())
        result = requests.post(
            self.url_prefix + "/libraries/{0}/uploads".format(library_id),
            headers={"Content-Type": content_type},
            data=fileData.read(),
        )
        self.assertEqual(200, result.status_code)
        data = json.loads(result.text)
        upload_id = data["upload_id"]
        # Test LibraryUploadStatus
        state = "PENDING"
        while state != "SUCCESS":
            result = requests.get(
                self.url_prefix
                + "/libraries/{0}/uploads/{1}".format(library_id, upload_id)
            )
            data = json.loads(result.text)
            state = data["state"] if "state" in data else "PENDING"
            if state == "FAILURE":
                raise RuntimeError("Failure during upload")
            sleep(1)

    def create_library(self, data):
        headers, data = self.get_headers(data)
        result = requests.post(
            self.url_prefix + "/libraries", headers=headers, data=data
        )
        self.assertEqual(201, result.status_code)
        self.assertIn("location", result.headers)
        result_data = json.loads(result.text)
        self.assertIn("id", result_data)
        return result_data["id"]

    def get_library_by_id(self, library_id):
        headers, data = self.get_headers({})
        result = requests.get(
            self.url_prefix + "/libraries/{0}".format(library_id),
            headers=headers,
            data=data,
        )
        self.assertEqual(200, result.status_code)
        library_data = json.loads(result.text)
        self.assertIn("user_data", library_data)
        self.assertIn("service_data", library_data)
        return library_data

    def delete_library_by_id(self, library_id):
        headers, data = self.get_headers({})
        result = requests.delete(
            self.url_prefix + "/libraries/{0}".format(library_id),
            headers=headers,
            data=data,
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("OK", result_data["status"])

    def get_libraries_count(self):
        headers, data = self.get_headers({})
        result = requests.get(
            self.url_prefix + "/libraries", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        return len(json.loads(result.text))

    def search_request(self, headers, data):
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("result", result_data)
        self.assertIn("search_id", result_data)
        return result_data["result"]

    def test_library_exceptions(self):
        headers, data = self.get_headers({})
        result = requests.get(
            self.url_prefix + "/libraries/foo", headers=headers, data=data
        )
        self.assertEqual(404, result.status_code)
        result = requests.put(
            self.url_prefix + "/libraries/foo", headers=headers, data=data
        )
        self.assertEqual(404, result.status_code)
        result = requests.delete(
            self.url_prefix + "/libraries/foo", headers=headers, data=data
        )
        self.assertEqual(404, result.status_code)

        # cannot create a library with missing or empty name
        result = requests.post(
            self.url_prefix + "/libraries", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("name", result_data["error"])

        headers, data = self.get_headers({"name": " "})
        result = requests.post(
            self.url_prefix + "/libraries", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("name", result_data["error"])

        # cannot create a library with missing user_data field
        headers, data = self.get_headers({"name": "test_library"})
        result = requests.post(
            self.url_prefix + "/libraries", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertNotIn("name", result_data["error"])
        self.assertIn("user_data", result_data["error"])

    def test_library_crud(self):
        library_data = {
            "name": "test_library",
            "user_data": {"description": "test description", "foo": "value"},
        }
        library_id = self.create_library(library_data)
        try:
            count = self.get_libraries_count()
            self.assertEqual(self.init_count + 1, count)
            # Get info for our new library
            result_data = self.get_library_by_id(library_id)
            self.assertEqual(
                "test_library", result_data["service_data"]["name"]
            )
            self.assertEqual(
                "test description", result_data["user_data"]["description"]
            )
            # Change library name
            headers, data = self.get_headers({"name": "new_name"})
            result = requests.put(
                self.url_prefix + "/libraries/{0}".format(library_id),
                headers=headers,
                data=data,
            )
            self.assertEqual(200, result.status_code)
            result_data = json.loads(result.text)
            self.assertEqual("OK", result_data["status"])
            result_data = self.get_library_by_id(library_id)
            self.assertEqual("new_name", result_data["service_data"]["name"])
            self.assertEqual(
                "test description", result_data["user_data"]["description"]
            )
            self.assertEqual("value", result_data["user_data"]["foo"])
            # Update with an empty name
            headers, data = self.get_headers({"name": ""})
            result = requests.put(
                self.url_prefix + "/libraries/{0}".format(library_id),
                headers=headers,
                data=data,
            )
            self.assertEqual(400, result.status_code)
            result_data = json.loads(result.text)
            self.assertIn("name", result_data["error"])
            # Change library description
            headers, data = self.get_headers(
                {"user_data": {"description": "new description"}}
            )
            result = requests.put(
                self.url_prefix + "/libraries/{0}".format(library_id),
                headers=headers,
                data=data,
            )
            self.assertEqual(200, result.status_code)
            result_data = json.loads(result.text)
            self.assertEqual("OK", result_data["status"])
            result_data = self.get_library_by_id(library_id)
            self.assertEqual("new_name", result_data["service_data"]["name"])
            self.assertEqual(
                "new description", result_data["user_data"]["description"]
            )
            self.assertEqual("value", result_data["user_data"]["foo"])
            # Change both library description & name
            headers, data = self.get_headers(
                {
                    "name": "brand_new_name",
                    "user_data": {
                        "description": "brand new description",
                        "bar": "value",
                    },
                }
            )
            result = requests.put(
                self.url_prefix + "/libraries/{0}".format(library_id),
                headers=headers,
                data=data,
            )
            self.assertEqual(200, result.status_code)
            result_data = json.loads(result.text)
            self.assertEqual("OK", result_data["status"])
            result_data = self.get_library_by_id(library_id)
            self.assertEqual(
                "brand_new_name", result_data["service_data"]["name"]
            )
            self.assertEqual(
                "brand new description",
                result_data["user_data"]["description"],
            )
            self.assertEqual("value", result_data["user_data"]["foo"])
            self.assertEqual("value", result_data["user_data"]["bar"])
            # Try to create the new library with the same name as the old one
            library_id_2 = self.create_library(library_data)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count + 2, count)
            self.delete_library_by_id(library_id_2)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count + 1, count)
        finally:
            self.delete_library_by_id(library_id)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count, count)

    def test_upload(self):
        library_id = self.create_library(
            {
                "name": "test_upload",
                "user_data": {"description": "test description"},
            }
        )
        try:
            self.do_upload(
                library_id,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "test-108.sd.gz",
                ),
                "application/x-gzip",
            )
            self.do_upload(
                library_id,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "test_pubchem_10.sdf",
                ),
                "chemical/x-mdl-sdfile",
            )
            # Check library structure count info
            result_data = self.get_library_by_id(library_id)
            self.assertEqual(
                118, result_data["service_data"]["structures_count"]
            )
            self.assertIn("properties", result_data["service_data"])
            self.assertEqual(
                65, len(result_data["service_data"]["properties"])
            )
            # Check library list
            headers, data = self.get_headers({})
            result = requests.get(
                self.url_prefix + "/libraries", headers=headers, data=data
            )
            self.assertEqual(200, result.status_code)
            result_data = list(
                filter(
                    lambda x: x["id"] == library_id, json.loads(result.text)
                )
            )
            self.assertEqual(118, result_data[0]["structures_count"])
            self.assertEqual("test_upload", result_data[0]["name"])
            self.assertIn("created_timestamp", result_data[0])
            self.assertIn("updated_timestamp", result_data[0])
        finally:
            self.delete_library_by_id(library_id)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count, count)

    def test_upload_more(self):
        library_id = self.create_library(
            {"name": "my_foo_library", "user_data": {}}
        )
        try:
            # test for #89: charset is appended to 'Content-Type' header
            self.do_upload(
                library_id,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "test_pubchem_10.sdf",
                ),
                "chemical/x-mdl-sdfile; charset=utf-8",
            )
            result_data = self.get_library_by_id(library_id)
            self.assertEqual(
                10, result_data["service_data"]["structures_count"]
            )
        finally:
            self.delete_library_by_id(library_id)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count, count)

    def test_library_search(self):
        library_id_1 = self.create_library(
            {
                "name": "test_search_1",
                "user_data": {"description": "test description 1"},
            }
        )
        library_id_2 = self.create_library(
            {
                "name": "test_search_2",
                "user_data": {"description": "test description 2"},
            }
        )
        try:
            # Upload data
            self.do_upload(
                library_id_1,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "test_pubchem_10.sdf",
                ),
                "chemical/x-mdl-sdfile",
            )
            self.do_upload(
                library_id_2,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "test-18.sd.gz",
                ),
                "application/x-gzip",
            )
            # Search
            # Search only structure query
            headers, data = self.get_headers(
                {
                    "query_structure": "C1~C~C~C~C~C1",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(10, len(structures))
            # search only text query
            headers, data = self.get_headers(
                {
                    "query_text": "PUBCHEM_HEAVY_ATOM_COUNT > 0",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(10, len(structures))
            found_keys = structures[0]["found_properties_keys"]
            self.assertLess(0, len(found_keys))
            headers, data = self.get_headers(
                {
                    "query_text": "acetate or triazol",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(3, len(structures))
            headers, data = self.get_headers(
                {
                    "query_text": "acetate",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(2, len(structures))
            headers, data = self.get_headers(
                {
                    "query_text": "acetate and cid = 975005",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(1, len(structures))
            # search that all structures are returned
            headers, data = self.get_headers(
                {
                    "query_text": "phenyl",
                    "library_ids": [library_id_2],
                    "type": "sub",
                    "limit": 20,
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(4, len(structures))
            # Search both text query and structure query
            headers, data = self.get_headers(
                {
                    "query_structure": "C1~C~C~C~C~C1",
                    "query_text": '"PUBCHEM_HEAVY_ATOM_COUNT" > 24',
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(3, len(structures))
            for s in structures:
                self.assertEqual(
                    "PUBCHEM_HEAVY_ATOM_COUNT", s["found_properties_keys"][0]
                )
            # Search for the molfile
            headers, data = self.get_headers(
                {
                    "query_structure": """
  Ketcher 09171520432D 1   1.00000     0.00000     0

  1  0  0     0  0            999 V2000
    6.1250   -3.7250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  END
""",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(10, len(structures))
            # Test exact key
            headers, data = self.get_headers(
                {
                    "query_text": "PUBCHEM_MOLECULAR_WEIGHT = 333.398703",
                    "library_ids": [library_id_1],
                    "type": "sub",
                }
            )
            structures = self.search_request(headers, data)
            self.assertEqual(1, len(structures))
            # Search in two libraries
            result_len = -1
            i = 0
            ids = {}
            while result_len != 0:
                headers, data = self.get_headers(
                    {
                        "query_structure": "C",
                        "library_ids": [library_id_1, library_id_2],
                        "type": "sub",
                        "limit": 1,
                        "offset": i,
                    }
                )
                structures = self.search_request(headers, data)
                result_len = len(structures)
                if not result_len:
                    break
                self.assertEqual(1, result_len)
                if not structures[0]["library_id"] in ids:
                    ids[structures[0]["library_id"]] = [structures[0]["id"]]
                else:
                    ids[structures[0]["library_id"]].append(
                        structures[0]["id"]
                    )
                i += 1
            self.assertItemsEqual([i for i in range(1, 11)], ids[library_id_1])
            self.assertItemsEqual([i for i in range(1, 19)], ids[library_id_2])
            # text query is not valid
            headers, data = self.get_headers(
                {
                    "library_ids": [library_id_1],
                    "type": "sub",
                    "query_text": "bar quux",
                }
            )
            result = requests.post(
                self.url_prefix + "/search", headers=headers, data=data
            )
            self.assertEqual(422, result.status_code)
            result_data = json.loads(result.text)
            self.assertIn("query_text", result_data["error"])
        except Exception:
            pass
        finally:
            self.delete_library_by_id(library_id_2)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count + 1, count)
            self.delete_library_by_id(library_id_1)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count, count)

    def test_search_count(self):
        library_id = self.create_library(
            {"name": "test_search_count", "user_data": {}}
        )
        try:
            self.do_upload(
                library_id,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "test-18.sd.gz",
                ),
                "application/x-gzip",
            )
            headers, data = self.get_headers(
                {
                    "query_text": "methyl",
                    "library_ids": [library_id],
                    "type": "sub",
                }
            )
            result = requests.post(
                self.url_prefix + "/search", headers=headers, data=data
            )
            self.assertEqual(200, result.status_code)
            result_data = json.loads(result.text)
            self.search_request(headers, data)
            self.assertEqual(10, len(result_data["result"]))
            search_id = result_data["search_id"]
            state = "PENDING"
            task_data = json.loads(result.text)
            while state != "SUCCESS":
                result = requests.get(
                    self.url_prefix + "/search/{0}".format(search_id)
                )
                task_data = json.loads(result.text)
                state = task_data["state"]
                if state == "FAILURE":
                    raise RuntimeError("Failure during search count")
                sleep(1)
            self.assertEqual(11, task_data["result"]["count"])
        finally:
            self.delete_library_by_id(library_id)
            count = self.get_libraries_count()
            self.assertEqual(self.init_count, count)

    def test_search_exceptions(self):
        # search type and ids array are required
        headers, data = self.get_headers({"type": "sub"})
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("library_ids", result_data["error"])
        headers, data = self.get_headers({"library_ids": ["foo"]})
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("type", result_data["error"])
        # search type is wrong
        headers, data = self.get_headers(
            {"library_ids": ["foo"], "type": "foo"}
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("type", result_data["error"])
        # either query_text or query_structure should be present
        headers, data = self.get_headers(
            {"library_ids": ["foo"], "type": "sub"}
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("_schema", result_data["error"])
        # limit is too high
        headers, data = self.get_headers(
            {
                "library_ids": ["foo"],
                "type": "sub",
                "query_text": "COUNT < 20",
                "limit": 101,
            }
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("limit", result_data["error"])
        # min and max are numbers in [0,1] interval
        headers, data = self.get_headers(
            {
                "library_ids": ["foo"],
                "type": "sim",
                "query_text": "COUNT < 20",
                "min": -0.5,
            }
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("_schema", result_data["error"])
        headers, data = self.get_headers(
            {
                "library_ids": ["foo"],
                "type": "sim",
                "query_text": "COUNT < 20",
                "max": 1.01,
            }
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("_schema", result_data["error"])
        # min must be less or equal than max
        headers, data = self.get_headers(
            {
                "library_ids": ["foo"],
                "type": "sim",
                "query_text": "COUNT < 20",
                "min": 0.9,
                "max": 0.8,
            }
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("_schema", result_data["error"])
        # library does not exist
        headers, data = self.get_headers(
            {"library_ids": ["foo"], "type": "sub", "query_text": "COUNT < 20"}
        )
        result = requests.post(
            self.url_prefix + "/search", headers=headers, data=data
        )
        self.assertEqual(404, result.status_code)
